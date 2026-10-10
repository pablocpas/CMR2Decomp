// Network layer without providers: the game's network menus list no
// connection, so sessions cannot be opened. The UDP implementation will
// replace this file.

#include "port/net.h"

extern "C" HRESULT Net_EnumProviders(NetProviderCallback, void *)
{
    return NET_OK;
}

extern "C" HRESULT Net_InitializeConnection(int)
{
    return NET_ERR_UNAVAILABLE;
}

extern "C" HRESULT Net_EnumSessions(const NetGuid *, DWORD, NetSessionCallback, void *)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_Open(const NetSessionDesc *, DWORD)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_Close(void)
{
    return NET_OK;
}

extern "C" HRESULT Net_GetSessionDesc(NetSessionDesc *)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_SetSessionDesc(const NetSessionDesc *)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_CreatePlayer(NetPlayerID *, const NetName *, const void *, DWORD)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_DestroyPlayer(NetPlayerID)
{
    return NET_ERR_INVALIDPLAYER;
}

extern "C" HRESULT Net_EnumPlayers(NetPlayerCallback, void *)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_SetPlayerData(NetPlayerID, const void *, DWORD)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_Send(NetPlayerID, NetPlayerID, BOOL, const void *, DWORD)
{
    return NET_ERR_NOCONNECTION;
}

extern "C" HRESULT Net_Receive(NetPlayerID *, NetPlayerID *, void *, DWORD *)
{
    return NET_ERR_NOMESSAGES;
}
