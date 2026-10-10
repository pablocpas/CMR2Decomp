// See net_session.h.
//
// Wire format (all little-endian, see net_wire.h):
//
// Search, raw UDP on the session port (ENet's intercept hook), so one port
// does everything:
//   query  "CMR2" 1 version nonce:u32 application:guid
//   reply  "CMR2" 2 version nonce:u32 desc passwordRequired:u8
//
// Session, over ENet (opcode first; control messages on channel 0):
//   JOIN      version instance:guid password            client -> host
//   RESUME    version instance:guid machine:u8          client -> new host
//   ACCEPT    roster                                    host -> client
//   REJECT    result:u32                                host -> client
//   MACHINES  count:u8 (id:u8 address:u32 port:u16)*    host -> all
//   PLAYER_ADD / PLAYER_REMOVE / PLAYER_DATA            both ways, the host forwards
//   SESSION_DESC desc                                   host -> all
//   DATA      origin:u8 from:u32 to:u32 payload         channel 0 (guaranteed) or 1
//
// The host checks that a machine only creates, changes, removes and speaks
// for its own players; everything read from the network is bounds-checked.

#include "net/net_session.h"
#include "net/net_wire.h"

#include <enet/enet.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <chrono>
#include <random>

namespace net {

namespace {

const uint8_t kMagic[4] = { 'C', 'M', 'R', '2' };
const uint8_t kProtocol = 1;
const uint8_t kQuery = 1, kReply = 2;

enum Opcode : uint8_t {
    OP_JOIN = 1,
    OP_RESUME,
    OP_ACCEPT,
    OP_REJECT,
    OP_MACHINES,
    OP_PLAYER_ADD,
    OP_PLAYER_REMOVE,
    OP_PLAYER_DATA,
    OP_SESSION_DESC,
    OP_DATA,
};

const size_t kMaxMessage = 256 * 1024;
// The game copies these with strcpy into fixed buffers: session name and
// password into 0x104 bytes, player names into 100 (SessionPlayerRecord).
// Longer ones are cut when sent and refused when received.
const size_t kMaxText = 255;
const size_t kMaxPlayerName = 63;
const size_t kMaxPlayerData = 64 * 1024;
const size_t kMaxPlayers = 64;
const size_t kMaxMachines = 16;
const size_t kMaxInbox = 16384;
const uint32_t kBrowseIdleMs = 3000;        // the search stops when the game stops asking
const uint32_t kQueryIntervalMs = 1000;
const uint32_t kSessionExpiryMs = 4000;
const uint32_t kResolveIntervalMs = 30000;
const uint32_t kPendingPeerMs = 5000;       // a connection must JOIN or RESUME within this
const uint32_t kRejectGraceMs = 1000;       // a rejected one must hang up within this
const DWORD kPlayerTypePlayer = 1;          // DPPLAYERTYPE_PLAYER
const DWORD kSessionPasswordRequired = 0x400;

thread_local Session *t_servicing;

uint32_t NowMs()
{
    static const auto start = std::chrono::steady_clock::now();
    // Never 0, which the session uses for "not yet".
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start)
               .count() +
           1;
}

bool Elapsed(uint32_t now, uint32_t since, uint32_t ms)
{
    return (uint32_t)(now - since) >= ms;
}

std::string GuidKey(const NetGuid &g)
{
    char s[33];
    for (int i = 0; i < 16; i++)
        snprintf(s + i * 2, 3, "%02x", g.bytes[i]);
    return s;
}

void WriteDesc(Writer &w, const NetSessionDesc &d, uint32_t currentPlayers, const std::string &name,
               const std::string &password)
{
    w.U32(d.dwFlags);
    w.Bytes(d.guidInstance.bytes, 16);
    w.Bytes(d.guidApplication.bytes, 16);
    w.U32(d.dwMaxPlayers);
    w.U32(currentPlayers);
    w.U32(d.dwReserved1);
    w.U32(d.dwReserved2);
    w.U32(d.dwUser1);
    w.U32(d.dwUser2);
    w.U32(d.dwUser3);
    w.U32(d.dwUser4);
    w.Str(name);
    w.Str(password);
}

bool ReadDesc(Reader &r, NetSessionDesc &d, std::string &name, std::string &password)
{
    memset(&d, 0, sizeof(d));
    d.dwSize = sizeof(d);
    d.dwFlags = r.U32();
    r.Bytes(d.guidInstance.bytes, 16);
    r.Bytes(d.guidApplication.bytes, 16);
    d.dwMaxPlayers = r.U32();
    d.dwCurrentPlayers = r.U32();
    d.dwReserved1 = r.U32();
    d.dwReserved2 = r.U32();
    d.dwUser1 = r.U32();
    d.dwUser2 = r.U32();
    d.dwUser3 = r.U32();
    d.dwUser4 = r.U32();
    name = r.Str(kMaxText);
    password = r.Str(kMaxText);
    return r.ok();
}

std::string Text(const char *s, size_t max = kMaxText)
{
    if (s == nullptr)
        return std::string();
    return std::string(s, strnlen(s, max));
}

void SetTimeouts(ENetPeer *peer, uint32_t timeoutMs)
{
    enet_peer_timeout(peer, 32, timeoutMs / 2, timeoutMs);
}

uint8_t MachineOf(ENetPeer *peer)
{
    return (uint8_t)(uintptr_t)peer->data;
}

void SendRaw(ENetHost *host, uint32_t address, uint16_t port, const std::vector<uint8_t> &data)
{
    ENetAddress to;
    to.host = address;
    to.port = port;
    ENetBuffer buffer;
    buffer.data = (void *)data.data();
    buffer.dataLength = data.size();
    enet_socket_send(host->socket, &to, &buffer, 1);
}

} // namespace

// ---- players and the roster ------------------------------------------------------------

namespace {

template <class Player>
void WritePlayer(Writer &w, const Player &p)
{
    w.U32(p.id);
    w.U8(p.machine);
    w.Str(p.shortName);
    w.Str(p.longName);
    w.Blob(p.data);
}

template <class Player>
bool ReadPlayer(Reader &r, Player &p)
{
    p.id = r.U32();
    p.machine = r.U8();
    p.shortName = r.Str(kMaxPlayerName);
    p.longName = r.Str(kMaxPlayerName);
    p.data = r.Blob(kMaxPlayerData);
    return r.ok() && p.id != 0 && p.machine != 0 && (p.id >> 16) == p.machine;
}

} // namespace

Session::Session(const Config &config) : config(config)
{
    std::random_device device;
    rng.seed(((uint64_t)device() << 32) ^ device());
}

Session::~Session()
{
    Close();
    running = false;
    if (thread.joinable())
        thread.join();
    std::lock_guard<std::mutex> lock(mutex);
    if (host != nullptr) {
        enet_host_flush(host);
        enet_host_destroy(host);
        host = nullptr;
    }
}

#define LOG(...)                       \
    do {                               \
        if (config.log != nullptr)     \
            config.log(__VA_ARGS__);   \
    } while (0)

HRESULT Session::Start()
{
    static std::once_flag initialised;
    static bool enetReady;
    std::call_once(initialised, [] { enetReady = enet_initialize() == 0; });
    if (!enetReady)
        return NET_ERR_UNAVAILABLE;

    std::lock_guard<std::mutex> lock(mutex);
    if (host != nullptr)
        return NET_OK;
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = config.port;
    host = enet_host_create(&address, kMaxMachines, 2, 0, 0);
    if (host == nullptr) {
        // The port is taken (another copy hosts on this machine): any port
        // still joins and, after a migration, hosts.
        address.port = 0;
        host = enet_host_create(&address, kMaxMachines, 2, 0, 0);
    }
    if (host == nullptr)
        return NET_ERR_UNAVAILABLE;
    enet_socket_set_option(host->socket, ENET_SOCKOPT_BROADCAST, 1);
    host->intercept = &Session::Intercept;
    ENetAddress bound;
    boundPort = enet_socket_get_address(host->socket, &bound) == 0 ? bound.port : address.port;
    LOG("net: UDP port %u", (unsigned)boundPort);
    running = true;
    thread = std::thread(&Session::Run, this);
    return NET_OK;
}

void Session::Abandon()
{
    running = false;
    if (thread.joinable())
        thread.join();
    std::lock_guard<std::mutex> lock(mutex);
    if (host != nullptr) {
        enet_host_destroy(host);    // resets every peer without a word
        host = nullptr;
    }
    ResetSession();
}

// ---- service thread ----------------------------------------------------------------------

void Session::Run()
{
    while (running) {
        enet_uint32 condition = ENET_SOCKET_WAIT_RECEIVE;
        enet_socket_wait(host->socket, &condition, 5);
        bool resolve = false;
        {
            std::lock_guard<std::mutex> lock(mutex);
            uint32_t now = NowMs();
            Service(now);
            bool browsing = lastSearchCall != 0 && !Elapsed(now, lastSearchCall, kBrowseIdleMs);
            if (browsing && !resolving && (lastResolve == 0 || Elapsed(now, lastResolve, kResolveIntervalMs))) {
                resolving = true;
                resolve = true;
            }
        }
        if (resolve)
            ResolvePeers();
    }
}

void Session::Service(uint32_t now)
{
    if (host == nullptr)
        return;
    t_servicing = this;
    ENetEvent event;
    while (host != nullptr && enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            OnConnect(event.peer);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            OnDisconnect(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            OnReceive(event.peer, event.packet->data, event.packet->dataLength, event.channelID);
            enet_packet_destroy(event.packet);
            break;
        default:
            break;
        }
    }
    t_servicing = nullptr;

    // Connections that never said who they are.
    for (auto it = pendingPeers.begin(); it != pendingPeers.end();) {
        if (Elapsed(now, it->second, kPendingPeerMs)) {
            enet_peer_disconnect(it->first, 0);
            it = pendingPeers.erase(it);
        } else {
            ++it;
        }
    }
    // After a migration, machines that did not reconnect have left.
    if (state == State::Hosting && migrationDeadline != 0 && (int32_t)(now - migrationDeadline) >= 0) {
        std::vector<uint8_t> gone;
        for (auto &m : machines)
            if (m.first != localMachine && !m.second.resumed)
                gone.push_back(m.first);
        for (uint8_t id : gone)
            RemoveMachine(id, true);
        migrationDeadline = 0;
    }
    Search(now);
    enet_host_flush(host);
}

// ---- search ------------------------------------------------------------------------------

void Session::ResolvePeers()
{
    std::vector<std::string> names = config.peers;
    if (config.extraPeers != nullptr) {
        std::vector<std::string> extra = config.extraPeers();
        names.insert(names.end(), extra.begin(), extra.end());
    }
    std::vector<std::pair<uint32_t, uint16_t>> resolved;
    for (const std::string &entry : names) {
        std::string name = entry;
        uint16_t port = config.port;
        size_t colon = name.rfind(':');
        if (colon != std::string::npos) {
            port = (uint16_t)atoi(name.c_str() + colon + 1);
            name.resize(colon);
        }
        ENetAddress address;
        if (!name.empty() && port != 0 && enet_address_set_host(&address, name.c_str()) == 0)
            resolved.push_back({ address.host, port });
        else
            LOG("net: cannot resolve %s", entry.c_str());
    }
    std::lock_guard<std::mutex> lock(mutex);
    peerAddresses = resolved;
    lastResolve = NowMs();
    lastQuery = 0;      // ask the new list straight away
    resolving = false;
}

void Session::Search(uint32_t now)
{
    bool browsing = lastSearchCall != 0 && !Elapsed(now, lastSearchCall, kBrowseIdleMs);
    if (!browsing)
        return;
    for (auto it = found.begin(); it != found.end();) {
        if (Elapsed(now, it->second.lastSeenMs, kSessionExpiryMs))
            it = found.erase(it);
        else
            ++it;
    }
    if (lastQuery != 0 && !Elapsed(now, lastQuery, kQueryIntervalMs))
        return;
    lastQuery = now;
    previousNonce = queryNonce;
    queryNonce = (uint32_t)rng() | 1;
    Writer w;
    w.Bytes(kMagic, 4);
    w.U8(kQuery);
    w.U8(kProtocol);
    w.U32(queryNonce);
    w.Bytes(searchApplication.bytes, 16);
    if (config.broadcast)
        SendRaw(host, ENET_HOST_BROADCAST, config.port, w.data);
    for (const auto &peer : peerAddresses)
        SendRaw(host, peer.first, peer.second, w.data);
}

int Session::Intercept(_ENetHost *host, _ENetEvent *)
{
    Session *self = t_servicing;
    if (self == nullptr || host->receivedDataLength < 6 || memcmp(host->receivedData, kMagic, 4) != 0)
        return 0;
    self->HandleRaw(host->receivedData, host->receivedDataLength, host->receivedAddress.host,
                    host->receivedAddress.port);
    return 1;
}

void Session::HandleRaw(const uint8_t *data, size_t size, uint32_t address, uint16_t port)
{
    Reader r(data + 4, size - 4);
    uint8_t kind = r.U8();
    if (r.U8() != kProtocol)
        return;
    if (kind == kQuery) {
        uint32_t nonce = r.U32();
        NetGuid application;
        r.Bytes(application.bytes, 16);
        if (!r.ok() || state != State::Hosting ||
            memcmp(&application, &desc.guidApplication, sizeof(application)) != 0)
            return;
        Writer w;
        w.Bytes(kMagic, 4);
        w.U8(kReply);
        w.U8(kProtocol);
        w.U32(nonce);
        WriteDesc(w, desc, SessionPlayerCount(), sessionName, std::string());
        w.U8(password.empty() ? 0 : 1);
        SendRaw(host, address, port, w.data);
    } else if (kind == kReply) {
        uint32_t nonce = r.U32();
        if (nonce == 0 || (nonce != queryNonce && nonce != previousNonce))
            return;
        SessionInfo info;
        std::string unused;
        if (!ReadDesc(r, info.desc, info.name, unused))
            return;
        info.passwordRequired = r.U8() != 0;
        if (!r.ok())
            return;
        info.address = address;
        info.port = port;
        info.lastSeenMs = NowMs();
        found[GuidKey(info.desc.guidInstance)] = info;
    }
}

HRESULT Session::EnumSessions(const NetGuid *application, NetSessionCallback callback, void *context)
{
    std::vector<SessionInfo> list;
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (host == nullptr)
            return NET_ERR_UNINITIALIZED;
        uint32_t now = NowMs();
        NetGuid wanted = {};
        if (application != nullptr)
            wanted = *application;
        bool idle = lastSearchCall == 0 || Elapsed(now, lastSearchCall, kBrowseIdleMs) ||
                    memcmp(&wanted, &searchApplication, sizeof(wanted)) != 0;
        lastSearchCall = now;
        searchApplication = wanted;
        if (idle)
            lastQuery = 0;      // the service thread asks at once
        for (const auto &entry : found) {
            const NetSessionDesc &d = entry.second.desc;
            if (memcmp(&d.guidApplication, &wanted, sizeof(wanted)) != 0)
                continue;
            if (d.dwMaxPlayers != 0 && d.dwCurrentPlayers >= d.dwMaxPlayers)
                continue;
            list.push_back(entry.second);
        }
    }
    // The search runs in the background (DirectPlay's asynchronous
    // enumeration): this reports what has answered so far.
    for (const SessionInfo &info : list) {
        NetSessionDesc d = info.desc;
        d.lpszSessionNameA = (char *)info.name.c_str();
        d.lpszPasswordA = nullptr;
        if (info.passwordRequired)
            d.dwFlags |= kSessionPasswordRequired;
        if (callback != nullptr && !callback(&d, context))
            break;
    }
    return NET_OK;
}

// ---- sessions ----------------------------------------------------------------------------

void Session::ResetSession()
{
    state = State::Idle;
    memset(&desc, 0, sizeof(desc));
    sessionName.clear();
    password.clear();
    localMachine = hostMachine = 0;
    hostPeer = nullptr;
    joinResult = 0;
    migrationDeadline = 0;
    nextPlayer = 1;
    machines.clear();
    players.clear();
    pendingReliable.clear();
    pendingPeers.clear();
}

HRESULT Session::Open(const NetSessionDesc *d, DWORD flags)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (host == nullptr)
        return NET_ERR_UNINITIALIZED;
    if (state != State::Idle)
        return NET_ERR_ALREADYINITIALIZED;
    if (d == nullptr)
        return NET_ERR_INVALIDPARAMS;
    inbox.clear();

    if (flags & NET_OPEN_CREATE) {
        ResetSession();
        desc = *d;
        desc.dwSize = sizeof(desc);
        desc.lpszSessionNameA = desc.lpszPasswordA = nullptr;
        for (BYTE &b : desc.guidInstance.bytes)
            b = (BYTE)rng();
        sessionName = Text(d->lpszSessionNameA);
        password = Text(d->lpszPasswordA);
        localMachine = hostMachine = 1;
        machines[1].id = 1;
        state = State::Hosting;
        LOG("net: hosting \"%s\" (up to %u players)", sessionName.c_str(), (unsigned)desc.dwMaxPlayers);
        return NET_OK;
    }
    if (!(flags & NET_OPEN_JOIN))
        return NET_ERR_INVALIDFLAGS;

    auto it = found.find(GuidKey(d->guidInstance));
    if (it == found.end())
        return NET_ERR_NOCONNECTION;
    ResetSession();
    joinInstance = d->guidInstance;
    joinPassword = Text(d->lpszPasswordA);
    ENetAddress address;
    address.host = it->second.address;
    address.port = it->second.port;
    hostPeer = enet_host_connect(host, &address, 2, 0);
    if (hostPeer == nullptr)
        return NET_ERR_NOCONNECTION;
    SetTimeouts(hostPeer, config.timeoutMs);
    state = State::Joining;
    enet_host_flush(host);
    bool answered = changed.wait_for(lock, std::chrono::milliseconds(config.joinTimeoutMs),
                                     [this] { return joinResult != 0; });
    if (answered && joinResult == 1)
        return NET_OK;
    HRESULT result = answered ? (HRESULT)joinResult : NET_ERR_TIMEOUT;
    // A graceful hang-up keeps the connection's slot until the host
    // acknowledges it, so a retry cannot be hit by the old connection.
    if (hostPeer != nullptr)
        enet_peer_disconnect(hostPeer, 0);
    ResetSession();
    LOG("net: join failed (%08x)", (unsigned)result);
    return result;
}

HRESULT Session::Close()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (state == State::Idle || host == nullptr)
        return NET_OK;
    // A graceful goodbye: the host's machines elect a new host at once
    // instead of waiting for the timeout.
    if (state == State::Hosting) {
        for (auto &m : machines)
            if (m.second.peer != nullptr)
                enet_peer_disconnect(m.second.peer, 0);
    } else if (hostPeer != nullptr) {
        enet_peer_disconnect(hostPeer, 0);
    }
    enet_host_flush(host);
    ResetSession();
    inbox.clear();
    return NET_OK;
}

HRESULT Session::GetSessionDesc(NetSessionDesc *d)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (d == nullptr)
        return NET_ERR_INVALIDPARAMS;
    if (state == State::Idle || state == State::Joining)
        return NET_ERR_NOCONNECTION;
    *d = desc;
    d->dwSize = sizeof(*d);
    d->dwCurrentPlayers = SessionPlayerCount();
    d->lpszSessionNameA = (char *)sessionName.c_str();
    d->lpszPasswordA = (char *)password.c_str();
    return NET_OK;
}

HRESULT Session::SetSessionDesc(const NetSessionDesc *d)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (d == nullptr)
        return NET_ERR_INVALIDPARAMS;
    if (state != State::Hosting)
        return NET_ERR_ACCESSDENIED;
    desc.dwFlags = d->dwFlags;
    desc.dwMaxPlayers = d->dwMaxPlayers;
    desc.dwReserved1 = d->dwReserved1;
    desc.dwReserved2 = d->dwReserved2;
    desc.dwUser1 = d->dwUser1;
    desc.dwUser2 = d->dwUser2;
    desc.dwUser3 = d->dwUser3;
    desc.dwUser4 = d->dwUser4;
    if (d->lpszSessionNameA != nullptr)
        sessionName = Text(d->lpszSessionNameA);
    if (d->lpszPasswordA != nullptr)
        password = Text(d->lpszPasswordA);
    SendToMachines(SessionDescPacket(), true, 0);
    enet_host_flush(host);
    return NET_OK;
}

std::vector<uint8_t> Session::SessionDescPacket()
{
    Writer w;
    w.U8(OP_SESSION_DESC);
    WriteDesc(w, desc, SessionPlayerCount(), sessionName, password);
    return w.data;
}

bool Session::IsHost()
{
    std::lock_guard<std::mutex> lock(mutex);
    return state == State::Hosting;
}

// ---- players -----------------------------------------------------------------------------

uint32_t Session::SessionPlayerCount() const
{
    return (uint32_t)players.size();
}

bool Session::HasLocalPlayer(NetPlayerID id) const
{
    auto it = players.find(id);
    return it != players.end() && it->second.machine == localMachine;
}

NetPlayerID Session::FirstLocalPlayer() const
{
    for (const auto &p : players)
        if (p.second.machine == localMachine)
            return p.first;
    return 0;
}

void Session::AddPlayer(const Player &p, bool announce)
{
    players[p.id] = p;
    if (announce && p.machine != localMachine)
        QueueSystem(NET_SYS_CREATEPLAYER, &p);
}

void Session::RemovePlayer(NetPlayerID id, bool announce)
{
    auto it = players.find(id);
    if (it == players.end())
        return;
    if (announce && it->second.machine != localMachine)
        QueueSystem(NET_SYS_DESTROYPLAYER, &it->second);
    players.erase(it);
}

HRESULT Session::CreatePlayer(NetPlayerID *id, const NetName *name, const void *data, DWORD dataSize)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (state != State::Hosting && state != State::Joined)
        return NET_ERR_NOCONNECTION;
    if (id == nullptr || dataSize > kMaxPlayerData || (dataSize != 0 && data == nullptr))
        return NET_ERR_INVALIDPARAMS;
    if ((desc.dwMaxPlayers != 0 && SessionPlayerCount() >= desc.dwMaxPlayers) ||
        SessionPlayerCount() >= kMaxPlayers || nextPlayer == 0)
        return NET_ERR_NONEWPLAYERS;
    // Machines number their own players: no round trip to the host.
    Player p;
    p.id = (NetPlayerID)localMachine << 16 | nextPlayer++;
    p.machine = localMachine;
    if (name != nullptr) {
        p.shortName = Text(name->lpszShortNameA, kMaxPlayerName);
        p.longName = Text(name->lpszLongNameA, kMaxPlayerName);
    }
    if (dataSize != 0)
        p.data.assign((const uint8_t *)data, (const uint8_t *)data + dataSize);
    AddPlayer(p, false);
    Writer w;
    w.U8(OP_PLAYER_ADD);
    WritePlayer(w, p);
    if (state == State::Hosting)
        SendToMachines(w.data, true, 0);
    else
        SendTo(hostPeer, w.data, true);
    enet_host_flush(host);
    *id = p.id;
    return NET_OK;
}

HRESULT Session::DestroyPlayer(NetPlayerID id)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!HasLocalPlayer(id))
        return NET_ERR_INVALIDPLAYER;
    RemovePlayer(id, false);
    Writer w;
    w.U8(OP_PLAYER_REMOVE);
    w.U32(id);
    if (state == State::Hosting)
        SendToMachines(w.data, true, 0);
    else if (hostPeer != nullptr && state == State::Joined)
        SendTo(hostPeer, w.data, true);
    enet_host_flush(host);
    return NET_OK;
}

HRESULT Session::EnumPlayers(NetPlayerCallback callback, void *context)
{
    std::vector<Player> list;
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (state == State::Idle || state == State::Joining)
            return NET_ERR_NOCONNECTION;
        for (const auto &p : players)
            list.push_back(p.second);
    }
    for (const Player &p : list) {
        NetName name;
        name.dwSize = sizeof(name);
        name.dwFlags = 0;
        name.lpszShortNameA = (char *)p.shortName.c_str();
        name.lpszLongNameA = (char *)p.longName.c_str();
        if (callback != nullptr && !callback(p.id, &name, context))
            break;
    }
    return NET_OK;
}

HRESULT Session::SetPlayerData(NetPlayerID id, const void *data, DWORD dataSize)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!HasLocalPlayer(id))
        return NET_ERR_INVALIDPLAYER;
    if (dataSize > kMaxPlayerData || (dataSize != 0 && data == nullptr))
        return NET_ERR_INVALIDPARAMS;
    Player &p = players[id];
    p.data.assign((const uint8_t *)data, (const uint8_t *)data + dataSize);
    Writer w;
    w.U8(OP_PLAYER_DATA);
    w.U32(id);
    w.Blob(p.data);
    if (state == State::Hosting)
        SendToMachines(w.data, true, 0);
    else if (hostPeer != nullptr && state == State::Joined)
        SendTo(hostPeer, w.data, true);
    enet_host_flush(host);
    return NET_OK;
}

// ---- messages ----------------------------------------------------------------------------

void Session::SendTo(_ENetPeer *peer, const std::vector<uint8_t> &packet, bool reliable)
{
    if (peer == nullptr)
        return;
    ENetPacket *p = enet_packet_create(packet.data(), packet.size(),
                                       reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
    if (p != nullptr && enet_peer_send(peer, reliable ? 0 : 1, p) < 0)
        enet_packet_destroy(p);
}

void Session::SendToMachines(const std::vector<uint8_t> &packet, bool reliable, uint8_t except)
{
    // One packet for every connection; ENet frees it after the last send.
    ENetPacket *p = nullptr;
    for (auto &m : machines) {
        if (m.first == localMachine || m.first == except || m.second.peer == nullptr)
            continue;
        if (p == nullptr)
            p = enet_packet_create(packet.data(), packet.size(),
                                   reliable ? ENET_PACKET_FLAG_RELIABLE : ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
        if (p != nullptr)
            enet_peer_send(m.second.peer, reliable ? 0 : 1, p);
    }
    if (p != nullptr && p->referenceCount == 0)
        enet_packet_destroy(p);
}

namespace {

std::vector<uint8_t> DataPacket(uint8_t origin, NetPlayerID from, NetPlayerID to, const uint8_t *data, size_t size)
{
    Writer w;
    w.data.reserve(10 + size);
    w.U8(OP_DATA);
    w.U8(origin);
    w.U32(from);
    w.U32(to);
    w.Bytes(data, size);
    return w.data;
}

} // namespace

void Session::Deliver(NetPlayerID from, NetPlayerID to, const uint8_t *data, size_t size, uint8_t)
{
    for (const auto &p : players) {
        if (p.second.machine != localMachine || p.first == from)
            continue;
        if (to != NET_ID_ALL && to != p.first)
            continue;
        if (inbox.size() >= kMaxInbox) {
            LOG("net: the game is not reading its messages; dropping");
            return;
        }
        Inbound m;
        m.from = from;
        m.to = p.first;
        m.data.assign(data, data + size);
        inbox.push_back(std::move(m));
    }
}

void Session::Relay(uint8_t origin, NetPlayerID from, NetPlayerID to, const uint8_t *data, size_t size,
                    bool reliable)
{
    std::vector<uint8_t> packet = DataPacket(origin, from, to, data, size);
    if (to == NET_ID_ALL) {
        SendToMachines(packet, reliable, origin);
        return;
    }
    auto target = players.find(to);
    if (target == players.end() || target->second.machine == localMachine || target->second.machine == origin)
        return;
    auto m = machines.find(target->second.machine);
    if (m != machines.end())
        SendTo(m->second.peer, packet, reliable);
}

HRESULT Session::Send(NetPlayerID from, NetPlayerID to, BOOL guaranteed, const void *data, DWORD dataSize)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (state != State::Hosting && state != State::Joined && state != State::Migrating)
        return NET_ERR_NOCONNECTION;
    if (!HasLocalPlayer(from))
        return NET_ERR_INVALIDPLAYER;
    if (dataSize > kMaxMessage || (dataSize != 0 && data == nullptr))
        return NET_ERR_INVALIDPARAMS;
    auto target = players.find(to);
    if (to != NET_ID_ALL && target == players.end())
        return NET_ERR_INVALIDPLAYER;
    const uint8_t *bytes = (const uint8_t *)data;
    Deliver(from, to, bytes, dataSize, localMachine);
    if (to != NET_ID_ALL && target->second.machine == localMachine)
        return NET_OK;
    if (state == State::Hosting) {
        Relay(localMachine, from, to, bytes, dataSize, guaranteed != 0);
    } else if (state == State::Joined && hostPeer != nullptr) {
        SendTo(hostPeer, DataPacket(localMachine, from, to, bytes, dataSize), guaranteed != 0);
    } else if (guaranteed) {
        // Migrating: guaranteed messages wait for the new host.
        pendingReliable.push_back(DataPacket(localMachine, from, to, bytes, dataSize));
    }
    enet_host_flush(host);
    return NET_OK;
}

void Session::QueueSystem(DWORD type, const Player *player)
{
    if (inbox.size() >= kMaxInbox)
        return;
    Inbound m;
    m.from = NET_ID_SYSTEM;
    m.to = FirstLocalPlayer();
    m.system = type;
    if (player != nullptr) {
        m.player = player->id;
        m.shortName = player->shortName;
        m.longName = player->longName;
        m.data = player->data;
    }
    if (type == NET_SYS_SETSESSIONDESC) {
        m.desc = desc;
        m.desc.dwCurrentPlayers = SessionPlayerCount();
        m.shortName = sessionName;
        m.longName = password;
    }
    inbox.push_back(std::move(m));
}

namespace {

size_t Align4(size_t n)
{
    return (n + 3) & ~(size_t)3;
}

// Bytes the game needs for a message: system messages are DirectPlay's
// structures followed by their strings and data.
template <class Inbound>
size_t RenderedSize(const Inbound &m)
{
    size_t names = m.shortName.size() + 1 + m.longName.size() + 1;
    switch (m.system) {
    case 0:
        return m.data.size();
    case NET_SYS_CREATEPLAYER:
        return Align4(sizeof(NetMsgCreatePlayer) + names) + m.data.size();
    case NET_SYS_DESTROYPLAYER:
        return Align4(sizeof(NetMsgDestroyPlayer) + names) + m.data.size();
    case NET_SYS_SETPLAYERDATA:
        return sizeof(NetMsgSetPlayerData) + m.data.size();
    case NET_SYS_SETSESSIONDESC:
        return sizeof(NetMsgSetSessionDesc) + names;
    default:
        return sizeof(DWORD);
    }
}

} // namespace

HRESULT Session::Receive(NetPlayerID *from, NetPlayerID *to, void *data, DWORD *dataSize)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (dataSize == nullptr)
        return NET_ERR_INVALIDPARAMS;
    if (inbox.empty())
        return NET_ERR_NOMESSAGES;
    const Inbound &m = inbox.front();
    size_t need = RenderedSize(m);
    if (data == nullptr || *dataSize < need) {
        *dataSize = (DWORD)need;
        return NET_ERR_BUFFERTOOSMALL;
    }
    uint8_t *out = (uint8_t *)data;
    auto copyNames = [&](size_t offset, NetName &name) {
        char *s = (char *)out + offset;
        memcpy(s, m.shortName.c_str(), m.shortName.size() + 1);
        char *l = s + m.shortName.size() + 1;
        memcpy(l, m.longName.c_str(), m.longName.size() + 1);
        name.dwSize = sizeof(NetName);
        name.dwFlags = 0;
        name.lpszShortNameA = s;
        name.lpszLongNameA = l;
        return Align4(offset + m.shortName.size() + 1 + m.longName.size() + 1);
    };
    switch (m.system) {
    case 0:
        if (!m.data.empty())
            memcpy(out, m.data.data(), m.data.size());
        break;
    case NET_SYS_CREATEPLAYER: {
        NetMsgCreatePlayer msg = {};
        msg.dwType = NET_SYS_CREATEPLAYER;
        msg.dwPlayerType = kPlayerTypePlayer;
        msg.dpId = m.player;
        msg.dwCurrentPlayers = SessionPlayerCount();
        size_t at = copyNames(sizeof(msg), msg.dpnName);
        msg.lpData = m.data.empty() ? nullptr : out + at;
        msg.dwDataSize = (DWORD)m.data.size();
        if (!m.data.empty())
            memcpy(out + at, m.data.data(), m.data.size());
        memcpy(out, &msg, sizeof(msg));
        break;
    }
    case NET_SYS_DESTROYPLAYER: {
        NetMsgDestroyPlayer msg = {};
        msg.dwType = NET_SYS_DESTROYPLAYER;
        msg.dwPlayerType = kPlayerTypePlayer;
        msg.dpId = m.player;
        size_t at = copyNames(sizeof(msg), msg.dpnName);
        msg.lpRemoteData = m.data.empty() ? nullptr : out + at;
        msg.dwRemoteDataSize = (DWORD)m.data.size();
        if (!m.data.empty())
            memcpy(out + at, m.data.data(), m.data.size());
        memcpy(out, &msg, sizeof(msg));
        break;
    }
    case NET_SYS_SETPLAYERDATA: {
        NetMsgSetPlayerData msg = {};
        msg.dwType = NET_SYS_SETPLAYERDATA;
        msg.dwPlayerType = kPlayerTypePlayer;
        msg.dpId = m.player;
        msg.lpData = m.data.empty() ? nullptr : out + sizeof(msg);
        msg.dwDataSize = (DWORD)m.data.size();
        if (!m.data.empty())
            memcpy(out + sizeof(msg), m.data.data(), m.data.size());
        memcpy(out, &msg, sizeof(msg));
        break;
    }
    case NET_SYS_SETSESSIONDESC: {
        NetMsgSetSessionDesc msg = {};
        msg.dwType = NET_SYS_SETSESSIONDESC;
        msg.dpDesc = m.desc;
        NetName strings;
        copyNames(sizeof(msg), strings);
        msg.dpDesc.lpszSessionNameA = strings.lpszShortNameA;
        msg.dpDesc.lpszPasswordA = strings.lpszLongNameA;
        memcpy(out, &msg, sizeof(msg));
        break;
    }
    default: {
        DWORD type = m.system;
        memcpy(out, &type, sizeof(type));
        break;
    }
    }
    if (from != nullptr)
        *from = m.from;
    if (to != nullptr)
        *to = m.to;
    *dataSize = (DWORD)need;
    inbox.pop_front();
    return NET_OK;
}

// ---- connections -------------------------------------------------------------------------

std::vector<uint8_t> Session::MachinesPacket()
{
    Writer w;
    w.U8(OP_MACHINES);
    WriteMachines(w);
    return w.data;
}

void Session::WriteMachines(Writer &w)
{
    w.U8((uint8_t)machines.size());
    for (const auto &m : machines) {
        w.U8(m.first);
        w.U32(m.second.address);
        w.U16(m.second.port);
    }
}

bool Session::ReadMachines(Reader &r)
{
    uint8_t count = r.U8();
    if (count > kMaxMachines)
        return false;
    std::map<uint8_t, Machine> list;
    for (int i = 0; i < count; i++) {
        Machine m;
        m.id = r.U8();
        m.address = r.U32();
        m.port = r.U16();
        if (m.id == 0)
            return false;
        list[m.id] = m;
    }
    if (!r.ok())
        return false;
    machines = list;
    return true;
}

std::vector<uint8_t> Session::RosterPacket(uint8_t forMachine)
{
    Writer w;
    w.U8(OP_ACCEPT);
    WriteDesc(w, desc, SessionPlayerCount(), sessionName, password);
    w.U8(hostMachine);
    w.U8(forMachine);
    WriteMachines(w);
    w.U16((uint16_t)players.size());
    for (const auto &p : players)
        WritePlayer(w, p.second);
    return w.data;
}

bool Session::ReadRoster(Reader &r, bool announce)
{
    NetSessionDesc d;
    std::string name, pw;
    if (!ReadDesc(r, d, name, pw))
        return false;
    uint8_t newHost = r.U8();
    uint8_t self = r.U8();
    if (!ReadMachines(r))
        return false;
    uint16_t count = r.U16();
    if (count > kMaxPlayers)
        return false;
    std::map<NetPlayerID, Player> list;
    for (int i = 0; i < count; i++) {
        Player p;
        if (!ReadPlayer(r, p))
            return false;
        list[p.id] = p;
    }
    if (!r.ok() || self == 0 || newHost == 0)
        return false;
    desc = d;
    sessionName = name;
    password = pw;
    hostMachine = newHost;
    localMachine = self;
    // After a migration the new host's roster is the truth: report what
    // changed while the session was without a host.
    if (announce) {
        std::vector<NetPlayerID> gone;
        for (const auto &p : players)
            if (!list.count(p.first) && p.second.machine != localMachine)
                gone.push_back(p.first);
        for (NetPlayerID id : gone)
            RemovePlayer(id, true);
        for (const auto &p : list)
            if (!players.count(p.first))
                AddPlayer(p.second, true);
    }
    // Our own players stay as we know them.
    for (const auto &p : players)
        if (p.second.machine == localMachine)
            list[p.first] = p.second;
    players = list;
    return true;
}

void Session::Reject(_ENetPeer *peer, HRESULT result)
{
    Writer w;
    w.U8(OP_REJECT);
    w.U32((uint32_t)result);
    SendTo(peer, w.data, true);
    // The client hangs up (gracefully, so its retry cannot be hit by this
    // connection); one that does not is dropped after a second.
    pendingPeers[peer] = NowMs() - (kPendingPeerMs - kRejectGraceMs);
}

void Session::OnConnect(_ENetPeer *peer)
{
    SetTimeouts(peer, config.timeoutMs);
    if (peer == hostPeer && state == State::Joining) {
        Writer w;
        w.U8(OP_JOIN);
        w.U8(kProtocol);
        w.Bytes(joinInstance.bytes, 16);
        w.Str(joinPassword);
        SendTo(peer, w.data, true);
    } else if (peer == hostPeer && state == State::Migrating) {
        Writer w;
        w.U8(OP_RESUME);
        w.U8(kProtocol);
        w.Bytes(desc.guidInstance.bytes, 16);
        w.U8(localMachine);
        SendTo(peer, w.data, true);
    } else if (state == State::Hosting) {
        peer->data = nullptr;
        pendingPeers[peer] = NowMs();
    } else {
        enet_peer_disconnect(peer, 0);
    }
}

void Session::OnDisconnect(_ENetPeer *peer)
{
    pendingPeers.erase(peer);
    if (state == State::Hosting) {
        auto it = machines.find(MachineOf(peer));
        if (it != machines.end() && it->second.peer == peer) {
            LOG("net: machine %u left", (unsigned)it->first);
            RemoveMachine(it->first, true);
        }
        return;
    }
    if (peer != hostPeer)
        return;
    hostPeer = nullptr;
    if (state == State::Joining) {
        joinResult = NET_ERR_NOCONNECTION;
        changed.notify_all();
    } else if (state == State::Joined || state == State::Migrating) {
        LOG("net: lost the host");
        Migrate();
    }
}

void Session::RemoveMachine(uint8_t machine, bool announce)
{
    std::vector<NetPlayerID> owned;
    for (const auto &p : players)
        if (p.second.machine == machine)
            owned.push_back(p.first);
    for (NetPlayerID id : owned) {
        RemovePlayer(id, true);
        if (announce) {
            Writer w;
            w.U8(OP_PLAYER_REMOVE);
            w.U32(id);
            SendToMachines(w.data, true, machine);
        }
    }
    machines.erase(machine);
    if (announce)
        SendToMachines(MachinesPacket(), true, 0);
}

bool Session::Migrate()
{
    // The host is gone, with its players. The earliest joined machine left
    // (the lowest id) becomes the host; everyone agrees on it from the same
    // roster. A candidate that does not answer is dropped in turn.
    for (;;) {
        RemoveMachine(hostMachine, false);
        if (machines.empty() || !machines.count(localMachine)) {
            LOG("net: session lost");
            QueueSystem(NET_SYS_SESSIONLOST, nullptr);
            ResetSession();
            return false;
        }
        uint8_t candidate = machines.begin()->first;
        hostMachine = candidate;
        if (candidate == localMachine) {
            BecomeHost();
            return true;
        }
        const Machine &m = machines[candidate];
        if (m.address == 0 || m.port == 0)
            continue;
        ENetAddress address;
        address.host = m.address;
        address.port = m.port;
        hostPeer = enet_host_connect(host, &address, 2, 0);
        if (hostPeer == nullptr)
            continue;
        SetTimeouts(hostPeer, config.joinTimeoutMs);
        state = State::Migrating;
        LOG("net: reconnecting to machine %u, the new host", (unsigned)candidate);
        return true;
    }
}

void Session::BecomeHost()
{
    LOG("net: this machine is the new host");
    state = State::Hosting;
    hostMachine = localMachine;
    hostPeer = nullptr;
    pendingReliable.clear();
    for (auto &m : machines) {
        m.second.peer = nullptr;
        m.second.resumed = m.first == localMachine;
    }
    machines[localMachine].address = 0;
    machines[localMachine].port = 0;
    migrationDeadline = machines.size() > 1 ? NowMs() + config.joinTimeoutMs * 2 : 0;
    QueueSystem(NET_SYS_HOST, nullptr);
}

void Session::OnReceive(_ENetPeer *peer, const uint8_t *data, size_t size, uint8_t channel)
{
    if (size == 0)
        return;
    if (state == State::Hosting) {
        auto it = machines.find(MachineOf(peer));
        if (it != machines.end() && it->second.peer == peer)
            HostReceive(it->second, data, size, channel);
        else if (pendingPeers.count(peer))
            HostGreet(peer, data, size);
        return;
    }
    if (peer == hostPeer)
        ClientReceive(data, size, channel);
}

void Session::HostGreet(_ENetPeer *peer, const uint8_t *data, size_t size)
{
    Reader r(data + 1, size - 1);
    uint8_t version = r.U8();
    NetGuid instance;
    r.Bytes(instance.bytes, 16);
    if (data[0] == OP_JOIN) {
        std::string pw = r.Str(kMaxText);
        if (!r.ok() || version != kProtocol ||
            memcmp(&instance, &desc.guidInstance, sizeof(instance)) != 0)
            return Reject(peer, NET_ERR_ACCESSDENIED);
        if (pw != password)
            return Reject(peer, NET_ERR_INVALIDPASSWORD);
        uint8_t id = machines.empty() ? 1 : (uint8_t)(machines.rbegin()->first + 1);
        if (machines.size() >= kMaxMachines || id == 0 ||
            (desc.dwMaxPlayers != 0 && SessionPlayerCount() >= desc.dwMaxPlayers))
            return Reject(peer, NET_ERR_NONEWPLAYERS);
        Machine &m = machines[id];
        m.id = id;
        m.address = peer->address.host;
        m.port = peer->address.port;
        m.peer = peer;
        m.resumed = true;
        peer->data = (void *)(uintptr_t)id;
        pendingPeers.erase(peer);
        SendTo(peer, RosterPacket(id), true);
        SendToMachines(MachinesPacket(), true, id);
        LOG("net: machine %u joined", (unsigned)id);
    } else if (data[0] == OP_RESUME) {
        uint8_t id = r.U8();
        auto it = machines.find(id);
        if (!r.ok() || version != kProtocol || memcmp(&instance, &desc.guidInstance, sizeof(instance)) != 0 ||
            it == machines.end() || it->second.resumed || id == localMachine)
            return Reject(peer, NET_ERR_ACCESSDENIED);
        it->second.peer = peer;
        it->second.resumed = true;
        it->second.address = peer->address.host;
        it->second.port = peer->address.port;
        peer->data = (void *)(uintptr_t)id;
        pendingPeers.erase(peer);
        SendTo(peer, RosterPacket(id), true);
        SendToMachines(MachinesPacket(), true, id);
        LOG("net: machine %u is back", (unsigned)id);
        bool all = true;
        for (const auto &m : machines)
            all = all && m.second.resumed;
        if (all)
            migrationDeadline = 0;
    } else {
        enet_peer_disconnect(peer, 0);
        pendingPeers.erase(peer);
    }
}

void Session::HostReceive(Machine &from, const uint8_t *data, size_t size, uint8_t channel)
{
    uint8_t machine = from.id;
    Reader r(data + 1, size - 1);
    bool valid = false;
    switch (data[0]) {
    case OP_PLAYER_ADD: {
        Player p;
        valid = ReadPlayer(r, p) && p.machine == machine && !players.count(p.id) &&
                SessionPlayerCount() < kMaxPlayers;
        if (valid) {
            AddPlayer(p, true);
            SendToMachines(std::vector<uint8_t>(data, data + size), true, machine);
        }
        break;
    }
    case OP_PLAYER_REMOVE: {
        NetPlayerID id = r.U32();
        auto it = players.find(id);
        valid = r.ok() && it != players.end() && it->second.machine == machine;
        if (valid) {
            RemovePlayer(id, true);
            SendToMachines(std::vector<uint8_t>(data, data + size), true, machine);
        }
        break;
    }
    case OP_PLAYER_DATA: {
        NetPlayerID id = r.U32();
        std::vector<uint8_t> blob = r.Blob(kMaxPlayerData);
        auto it = players.find(id);
        valid = r.ok() && it != players.end() && it->second.machine == machine;
        if (valid) {
            it->second.data = blob;
            QueueSystem(NET_SYS_SETPLAYERDATA, &it->second);
            SendToMachines(std::vector<uint8_t>(data, data + size), true, machine);
        }
        break;
    }
    case OP_DATA: {
        uint8_t origin = r.U8();
        NetPlayerID src = r.U32();
        NetPlayerID dst = r.U32();
        auto it = players.find(src);
        valid = r.ok() && origin == machine && it != players.end() && it->second.machine == machine &&
                (dst == NET_ID_ALL || players.count(dst));
        if (valid) {
            const uint8_t *payload = data + 10;
            size_t length = size - 10;
            Deliver(src, dst, payload, length, origin);
            Relay(origin, src, dst, payload, length, channel == 0);
        }
        break;
    }
    default:
        break;
    }
    if (!valid) {
        // A machine that breaks the protocol is dropped.
        LOG("net: machine %u sent an invalid message (%u); dropping it", (unsigned)machine, (unsigned)data[0]);
        enet_peer_disconnect(from.peer, 0);
        RemoveMachine(machine, true);
    }
}

void Session::ClientReceive(const uint8_t *data, size_t size, uint8_t)
{
    Reader r(data + 1, size - 1);
    switch (data[0]) {
    case OP_ACCEPT:
        if (state == State::Joining) {
            if (ReadRoster(r, false)) {
                state = State::Joined;
                joinResult = 1;
                LOG("net: joined \"%s\" as machine %u", sessionName.c_str(), (unsigned)localMachine);
            } else {
                joinResult = NET_ERR_GENERIC;
            }
            changed.notify_all();
        } else if (state == State::Migrating && ReadRoster(r, true)) {
            state = State::Joined;
            for (const auto &packet : pendingReliable)
                SendTo(hostPeer, packet, true);
            pendingReliable.clear();
            LOG("net: rejoined machine %u, the new host", (unsigned)hostMachine);
        }
        break;
    case OP_REJECT: {
        HRESULT result = (HRESULT)r.U32();
        if (state == State::Joining) {
            joinResult = r.ok() && result != 0 && result != 1 ? result : NET_ERR_GENERIC;
            changed.notify_all();
        } else if (state == State::Migrating) {
            enet_peer_disconnect(hostPeer, 0);
            hostPeer = nullptr;
            Migrate();
        }
        break;
    }
    case OP_MACHINES:
        if (state == State::Joined)
            ReadMachines(r);
        break;
    case OP_PLAYER_ADD: {
        Player p;
        if (state == State::Joined && ReadPlayer(r, p) && !players.count(p.id) && players.size() < kMaxPlayers)
            AddPlayer(p, true);
        break;
    }
    case OP_PLAYER_REMOVE: {
        NetPlayerID id = r.U32();
        if (state == State::Joined && r.ok() && !HasLocalPlayer(id))
            RemovePlayer(id, true);
        break;
    }
    case OP_PLAYER_DATA: {
        NetPlayerID id = r.U32();
        std::vector<uint8_t> blob = r.Blob(kMaxPlayerData);
        auto it = players.find(id);
        if (state == State::Joined && r.ok() && it != players.end() && it->second.machine != localMachine) {
            it->second.data = blob;
            QueueSystem(NET_SYS_SETPLAYERDATA, &it->second);
        }
        break;
    }
    case OP_SESSION_DESC: {
        NetSessionDesc d;
        std::string name, pw;
        if (state == State::Joined && ReadDesc(r, d, name, pw)) {
            NetGuid instance = desc.guidInstance;
            desc = d;
            desc.guidInstance = instance;
            sessionName = name;
            password = pw;
            QueueSystem(NET_SYS_SETSESSIONDESC, nullptr);
        }
        break;
    }
    case OP_DATA: {
        uint8_t origin = r.U8();
        NetPlayerID src = r.U32();
        NetPlayerID dst = r.U32();
        if (state == State::Joined && r.ok() && origin != localMachine)
            Deliver(src, dst, data + 10, size - 10, origin);
        break;
    }
    default:
        break;
    }
}

} // namespace net
