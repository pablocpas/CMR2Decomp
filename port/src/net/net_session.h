// One machine's network endpoint: the DirectPlay 4 model of port/net.h over
// UDP with ENet. The C API (net_api.cpp) drives one instance; tests run
// several in one process.
//
// Topology: the host is the session's server. Every other machine connects
// to it; the host relays messages between machines, so only the host has to
// be reachable (a forwarded UDP port, a LAN, Tailscale or another VPN). When
// the host leaves, the remaining machines elect the earliest joined one and
// reconnect to it (DirectPlay's host migration: the new host gets
// NET_SYS_HOST).
//
// Channels: 0 is reliable and ordered (guaranteed messages and the session
// protocol), 1 unreliable (other messages, such as car states), so a
// retransmitted packet never delays positions.
//
// A thread services the socket, so connections survive while the game does
// not poll (stage loading). Every public method locks the session.

#ifndef OPENCMR2_NET_SESSION_H
#define OPENCMR2_NET_SESSION_H

#include "port/net.h"

#include <stdint.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;

namespace net {

class Reader;
class Writer;

struct Config {
    uint16_t port = 28620;                  // UDP port sessions are hosted on and searched at
    std::vector<std::string> peers;         // extra hosts to search ("host" or "host:port")
    bool broadcast = true;                  // search the local network
    uint32_t timeoutMs = 10000;             // a silent connection is dropped after this
    uint32_t joinTimeoutMs = 5000;          // Open(JOIN) waits this long for the host
    // More hosts to search, asked for on the service thread (Tailscale peers).
    std::vector<std::string> (*extraPeers)() = nullptr;
    void (*log)(const char *format, ...) = nullptr;
};

// A session as seen by the search.
struct SessionInfo {
    NetSessionDesc desc;                    // name/password pointers are NULL here
    std::string name;
    bool passwordRequired = false;
    uint32_t address = 0;                   // host, network byte order
    uint16_t port = 0;
    uint32_t lastSeenMs = 0;
};

class Session {
public:
    explicit Session(const Config &config);
    ~Session();

    // Opens the socket and starts the service thread (InitializeConnection).
    HRESULT Start();
    uint16_t BoundPort() const { return boundPort; }

    HRESULT EnumSessions(const NetGuid *application, NetSessionCallback callback, void *context);
    HRESULT Open(const NetSessionDesc *desc, DWORD flags);
    HRESULT Close();
    HRESULT GetSessionDesc(NetSessionDesc *desc);
    HRESULT SetSessionDesc(const NetSessionDesc *desc);

    HRESULT CreatePlayer(NetPlayerID *id, const NetName *name, const void *data, DWORD dataSize);
    HRESULT DestroyPlayer(NetPlayerID id);
    HRESULT EnumPlayers(NetPlayerCallback callback, void *context);
    HRESULT SetPlayerData(NetPlayerID id, const void *data, DWORD dataSize);

    HRESULT Send(NetPlayerID from, NetPlayerID to, BOOL guaranteed, const void *data, DWORD dataSize);
    HRESULT Receive(NetPlayerID *from, NetPlayerID *to, void *data, DWORD *dataSize);

    bool IsHost();

    // Testing: drops the network without telling anyone (a crash or a cable
    // pulled), so the others only notice through the timeout.
    void Abandon();

private:
    enum class State { Idle, Hosting, Joining, Joined, Migrating };

    struct Player {
        NetPlayerID id = 0;
        uint8_t machine = 0;
        std::string shortName, longName;
        std::vector<uint8_t> data;
    };
    struct Machine {
        uint8_t id = 0;
        uint32_t address = 0;               // as the host sees it (network byte order)
        uint16_t port = 0;
        _ENetPeer *peer = nullptr;          // host side: its connection
        bool resumed = true;                // migration: reconnected to the new host
    };
    // A message for the game, rendered by Receive (system messages take
    // DirectPlay's layouts there, with pointers into the game's buffer).
    struct Inbound {
        NetPlayerID from = 0, to = 0;
        DWORD system = 0;                   // NET_SYS_*, 0 for player messages
        NetPlayerID player = 0;
        std::string shortName, longName;    // the session name and password for SETSESSIONDESC
        std::vector<uint8_t> data;
        NetSessionDesc desc = {};
    };

    void Run();
    void Service(uint32_t now);
    void Search(uint32_t now);
    void ResolvePeers();
    static int Intercept(_ENetHost *host, _ENetEvent *event);
    void HandleRaw(const uint8_t *data, size_t size, uint32_t address, uint16_t port);

    void OnConnect(_ENetPeer *peer);
    void OnDisconnect(_ENetPeer *peer);
    void OnReceive(_ENetPeer *peer, const uint8_t *data, size_t size, uint8_t channel);
    void HostGreet(_ENetPeer *peer, const uint8_t *data, size_t size);
    void HostReceive(Machine &from, const uint8_t *data, size_t size, uint8_t channel);
    void ClientReceive(const uint8_t *data, size_t size, uint8_t channel);
    void Reject(_ENetPeer *peer, HRESULT result);

    void SendTo(_ENetPeer *peer, const std::vector<uint8_t> &packet, bool reliable);
    void SendToMachines(const std::vector<uint8_t> &packet, bool reliable, uint8_t except);
    void Deliver(NetPlayerID from, NetPlayerID to, const uint8_t *data, size_t size, uint8_t originMachine);
    void Relay(uint8_t originMachine, NetPlayerID from, NetPlayerID to, const uint8_t *data, size_t size,
               bool reliable);
    void QueueSystem(DWORD type, const Player *player);
    void AddPlayer(const Player &p, bool announce);
    void RemovePlayer(NetPlayerID id, bool announce);
    void RemoveMachine(uint8_t machine, bool announce);
    std::vector<uint8_t> MachinesPacket();
    void WriteMachines(Writer &w);
    bool ReadMachines(Reader &r);
    std::vector<uint8_t> RosterPacket(uint8_t forMachine);
    bool ReadRoster(Reader &r, bool announce);
    std::vector<uint8_t> SessionDescPacket();
    void BecomeHost();
    bool Migrate();
    void ResetSession();
    bool HasLocalPlayer(NetPlayerID id) const;
    NetPlayerID FirstLocalPlayer() const;
    uint32_t SessionPlayerCount() const;

    Config config;
    std::mutex mutex;
    std::condition_variable changed;    // join and migration results
    std::thread thread;
    std::atomic<bool> running{false};
    _ENetHost *host = nullptr;
    uint16_t boundPort = 0;

    State state = State::Idle;
    NetSessionDesc desc = {};
    std::string sessionName, password;
    uint8_t localMachine = 0;
    uint8_t hostMachine = 0;
    _ENetPeer *hostPeer = nullptr;          // client side: the connection to the host
    HRESULT joinResult = 0;                 // Open(JOIN): 0 waiting, 1 accepted, else the error
    NetGuid joinInstance = {};
    std::string joinPassword;
    uint32_t migrationDeadline = 0;
    uint16_t nextPlayer = 1;
    std::map<uint8_t, Machine> machines;
    std::map<NetPlayerID, Player> players;
    std::deque<Inbound> inbox;
    std::vector<std::vector<uint8_t>> pendingReliable;  // sent while migrating
    std::map<_ENetPeer *, uint32_t> pendingPeers;       // host: connected, not yet joined
    std::mt19937_64 rng;

    // Search.
    uint32_t lastSearchCall = 0, lastQuery = 0;
    uint32_t queryNonce = 0, previousNonce = 0;
    NetGuid searchApplication = {};
    std::map<std::string, SessionInfo> found;   // by instance GUID
    std::vector<std::pair<uint32_t, uint16_t>> peerAddresses;
    uint32_t lastResolve = 0;
    bool resolving = false;
};

} // namespace net

#endif
