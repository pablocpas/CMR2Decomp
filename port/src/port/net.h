/*
 * OpenCMR2 network sessions, implemented in src/net (replaces DirectPlay).
 *
 * The game's network code is written for DirectPlay 4 and keeps its model:
 * a service provider ("connection") is chosen, sessions are enumerated,
 * hosted or joined, each machine creates its player, and messages go from a
 * player to one player or to all of them. System messages (players joining
 * and leaving, becoming the host, player data and session changes) arrive
 * through Net_Receive from NET_ID_SYSTEM with DirectPlay's layouts, which the
 * game reads by offset.
 *
 * Result codes and system message types have DirectPlay's values.
 */
#ifndef OPENCMR2_PORT_NET_H
#define OPENCMR2_PORT_NET_H

#include "port/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef DWORD NetPlayerID;

#define NET_ID_SYSTEM 0         /* sender of system messages */
#define NET_ID_ALL 0            /* send to every player */

/* ---- results ---- */
#define NET_OK 0
#define NET_ERR_MAKE(code) ((HRESULT)(0x88770000 | (code)))
#define NET_ERR_ACCESSDENIED NET_ERR_MAKE(10)
#define NET_ERR_ALREADYINITIALIZED NET_ERR_MAKE(5)
#define NET_ERR_BUFFERTOOSMALL NET_ERR_MAKE(30)
#define NET_ERR_CONNECTIONLOST NET_ERR_MAKE(500)
#define NET_ERR_GENERIC ((HRESULT)0x80004005)
#define NET_ERR_INVALIDFLAGS NET_ERR_MAKE(120)
#define NET_ERR_INVALIDOBJECT NET_ERR_MAKE(130)
#define NET_ERR_INVALIDPARAMS ((HRESULT)0x80070057)
#define NET_ERR_INVALIDPASSWORD NET_ERR_MAKE(360)
#define NET_ERR_INVALIDPLAYER NET_ERR_MAKE(150)
#define NET_ERR_NOCONNECTION NET_ERR_MAKE(200)
#define NET_ERR_NOMESSAGES NET_ERR_MAKE(190)
#define NET_ERR_NONEWPLAYERS NET_ERR_MAKE(230)
#define NET_ERR_TIMEOUT NET_ERR_MAKE(270)
#define NET_ERR_UNAVAILABLE NET_ERR_MAKE(280)
#define NET_ERR_UNINITIALIZED NET_ERR_MAKE(350)
#define NET_ERR_USERCANCEL NET_ERR_MAKE(380)

/* ---- data ---- */

typedef struct NetGuid {
    BYTE bytes[16];
} NetGuid;

/* DPSESSIONDESC2. */
typedef struct NetSessionDesc {
    DWORD dwSize;
    DWORD dwFlags;
    NetGuid guidInstance;
    NetGuid guidApplication;
    DWORD dwMaxPlayers;
    DWORD dwCurrentPlayers;
    char *lpszSessionNameA;
    char *lpszPasswordA;
    DWORD dwReserved1;
    DWORD dwReserved2;
    DWORD dwUser1;
    DWORD dwUser2;
    DWORD dwUser3;
    DWORD dwUser4;
} NetSessionDesc;

/* DPNAME. */
typedef struct NetName {
    DWORD dwSize;
    DWORD dwFlags;
    char *lpszShortNameA;
    char *lpszLongNameA;
} NetName;

/* ---- system messages (from NET_ID_SYSTEM) ---- */
#define NET_SYS_CREATEPLAYER 0x0003
#define NET_SYS_DESTROYPLAYER 0x0005
#define NET_SYS_SESSIONLOST 0x0031
#define NET_SYS_HOST 0x0101
#define NET_SYS_SETPLAYERDATA 0x0102
#define NET_SYS_SETSESSIONDESC 0x0104

typedef struct NetMsgCreatePlayer {      /* DPMSG_CREATEPLAYERORGROUP */
    DWORD dwType;
    DWORD dwPlayerType;
    NetPlayerID dpId;
    DWORD dwCurrentPlayers;
    void *lpData;
    DWORD dwDataSize;
    NetName dpnName;
    NetPlayerID dpIdParent;
    DWORD dwFlags;
} NetMsgCreatePlayer;

typedef struct NetMsgDestroyPlayer {     /* DPMSG_DESTROYPLAYERORGROUP */
    DWORD dwType;
    DWORD dwPlayerType;
    NetPlayerID dpId;
    void *lpLocalData;
    DWORD dwLocalDataSize;
    void *lpRemoteData;
    DWORD dwRemoteDataSize;
    NetName dpnName;
    NetPlayerID dpIdParent;
    DWORD dwFlags;
} NetMsgDestroyPlayer;

typedef struct NetMsgSetPlayerData {     /* DPMSG_SETPLAYERORGROUPDATA */
    DWORD dwType;
    DWORD dwPlayerType;
    NetPlayerID dpId;
    void *lpData;
    DWORD dwDataSize;
} NetMsgSetPlayerData;

typedef struct NetMsgSetSessionDesc {    /* DPMSG_SETSESSIONDESC */
    DWORD dwType;
    NetSessionDesc dpDesc;
} NetMsgSetSessionDesc;

/* ---- service providers ---- */

/* Called for each provider; return 0 to stop. */
typedef BOOL (*NetProviderCallback)(int provider, const char *name, void *context);
HRESULT Net_EnumProviders(NetProviderCallback callback, void *context);
HRESULT Net_InitializeConnection(int provider);

/* ---- sessions ---- */

/* Sessions of the application seen on the network; the callback gets each
   one (name and password pointers valid during the call) and returns 0 to
   stop. timeout in milliseconds. */
typedef BOOL (*NetSessionCallback)(const NetSessionDesc *desc, void *context);
HRESULT Net_EnumSessions(const NetGuid *application, DWORD timeout, NetSessionCallback callback, void *context);

enum NetOpenFlags {
    NET_OPEN_JOIN = 1,
    NET_OPEN_CREATE = 2
};
/* Hosts (desc fully filled) or joins (guidInstance and password). */
HRESULT Net_Open(const NetSessionDesc *desc, DWORD flags);
HRESULT Net_Close(void);
/* Copies the session description; the name and password pointers point
   into storage owned by the network layer. */
HRESULT Net_GetSessionDesc(NetSessionDesc *desc);
/* Host only: changes the description and tells the other machines. */
HRESULT Net_SetSessionDesc(const NetSessionDesc *desc);

/* ---- players ---- */

HRESULT Net_CreatePlayer(NetPlayerID *id, const NetName *name, const void *data, DWORD dataSize);
HRESULT Net_DestroyPlayer(NetPlayerID id);
typedef BOOL (*NetPlayerCallback)(NetPlayerID id, const NetName *name, void *context);
HRESULT Net_EnumPlayers(NetPlayerCallback callback, void *context);
HRESULT Net_SetPlayerData(NetPlayerID id, const void *data, DWORD dataSize);

/* ---- messages ---- */

HRESULT Net_Send(NetPlayerID from, NetPlayerID to, BOOL guaranteed, const void *data, DWORD dataSize);
/* Takes the next message for any local player. When the buffer is too small
   it returns NET_ERR_BUFFERTOOSMALL with the size needed in *dataSize and
   keeps the message; NET_ERR_NOMESSAGES when there is none. */
HRESULT Net_Receive(NetPlayerID *from, NetPlayerID *to, void *data, DWORD *dataSize);

#ifdef __cplusplus
}
#endif

#endif
