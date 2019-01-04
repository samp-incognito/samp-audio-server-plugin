/*
 * Copyright (C) 2012 Incognito
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <a_samp>

// Natives (Main)

native Audio_CreateTCPServer(port);
native Audio_DestroyTCPServer();
native Audio_SetPack(const name[], bool:transferable = true, bool:automated = true);
native Audio_IsClientConnected(playerid);
native Audio_SendMessage(playerid, const message[]);
native Audio_TransferPack(playerid);

// Natives (Sequences)

native Audio_CreateSequence();
native Audio_DestroySequence(sequenceid);
native Audio_AddToSequence(sequenceid, audioid);
native Audio_RemoveFromSequence(sequenceid, audioid);

// Natives (Audio)

native Audio_Play(playerid, audioid, bool:pause = false, bool:loop = false, bool:downmix = false);
native Audio_PlayStreamed(playerid, const url[], bool:pause = false, bool:loop = false, bool:downmix = false);
native Audio_PlaySequence(playerid, sequenceid, bool:pause = false, bool:loop = false, bool:downmix = false);
native Audio_Pause(playerid, handleid);
native Audio_Resume(playerid, handleid);
native Audio_Stop(playerid, handleid);
native Audio_Restart(playerid, handleid);
native Audio_GetPosition(playerid, handleid, const callback[] = "Audio_OnGetPosition");
native Audio_SetPosition(playerid, handleid, seconds);
native Audio_SetVolume(playerid, handleid, volume);
native Audio_SetFX(playerid, handleid, type);
native Audio_RemoveFX(playerid, handleid, type);
native Audio_Set3DPosition(playerid, handleid, Float:x, Float:y, Float:z, Float:distance);
native Audio_Remove3DPosition(playerid, handleid);

// Natives (Radio Stations)

native Audio_SetRadioStation(playerid, station);
native Audio_StopRadio(playerid);

// Natives (Internal)

native Audio_AddPlayer(playerid, const ip[], const name[]);
native Audio_RenamePlayer(playerid, const name[]);
native Audio_RemovePlayer(playerid);

// Callbacks (Main)

forward Audio_OnClientConnect(playerid);
forward Audio_OnClientDisconnect(playerid);
forward Audio_OnTransferFile(playerid, file[], current, total, result);
forward Audio_OnPlay(playerid, handleid);
forward Audio_OnStop(playerid, handleid);
forward Audio_OnTrackChange(playerid, handleid, track[]);
forward Audio_OnRadioStationChange(playerid, station);

// Callbacks (Custom)

forward Audio_OnGetPosition(playerid, handleid, seconds);

// Callback Hook Section

static bool:Audio_g_CTS = false;
static bool:Audio_g_OPC = false;
static bool:Audio_g_OPDC = false;

public OnFilterScriptInit()
{
	if (!Audio_g_CTS)
	{
		Audio_g_CTS = true;
		Audio_g_OPC = (funcidx("Audio_OnPlayerConnect") != -1);
		Audio_g_OPDC = (funcidx("Audio_OnPlayerDisconnect") != -1);
		Audio_CreateTCPServer(GetServerVarAsInt("port"));
	}
	if (funcidx("Audio_OnFilterScriptInit") != -1)
	{
		return CallLocalFunction("Audio_OnFilterScriptInit", "");
	}
	return 1;
}

#if defined _ALS_OnFilterScriptInit
	#undef OnFilterScriptInit
#else
	#define _ALS_OnFilterScriptInit
#endif
#define OnFilterScriptInit Audio_OnFilterScriptInit

forward Audio_OnFilterScriptInit();

public OnGameModeInit()
{
	if (!Audio_g_CTS)
	{
		Audio_g_CTS = true;
		Audio_g_OPC = (funcidx("Audio_OnPlayerConnect") != -1);
		Audio_g_OPDC = (funcidx("Audio_OnPlayerDisconnect") != -1);
		Audio_CreateTCPServer(GetServerVarAsInt("port"));
	}
	if (funcidx("Audio_OnGameModeInit") != -1)
	{
		return CallLocalFunction("Audio_OnGameModeInit", "");
	}
	return 1;
}

#if defined _ALS_OnGameModeInit
	#undef OnGameModeInit
#else
	#define _ALS_OnGameModeInit
#endif
#define OnGameModeInit Audio_OnGameModeInit

forward Audio_OnGameModeInit();

public OnPlayerConnect(playerid)
{
	if (!IsPlayerNPC(playerid))
	{
		new ip[16], name[MAX_PLAYER_NAME];
		GetPlayerIp(playerid, ip, sizeof(ip));
		GetPlayerName(playerid, name, sizeof(name));
		Audio_AddPlayer(playerid, ip, name);
	}
	if (Audio_g_OPC)
	{
		return CallLocalFunction("Audio_OnPlayerConnect", "d", playerid);
	}
	return 1;
}

#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect Audio_OnPlayerConnect

forward Audio_OnPlayerConnect(playerid);

public OnPlayerDisconnect(playerid, reason)
{
	if (!IsPlayerNPC(playerid))
	{
		Audio_RemovePlayer(playerid);
	}
	if (Audio_g_OPDC)
	{
		return CallLocalFunction("Audio_OnPlayerDisconnect", "dd", playerid, reason);
	}
	return 1;
}

#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect Audio_OnPlayerDisconnect

forward Audio_OnPlayerDisconnect(playerid, reason);

// Native Hook Section

stock Audio_SetPlayerName(playerid, name[])
{
	new value = SetPlayerName(playerid, name);
	if (value > 0)
	{
		Audio_RenamePlayer(playerid, name);
	}
	return value;
}
#if defined _ALS_SetPlayerName
	#undef SetPlayerName
#else
	#define _ALS_SetPlayerName
#endif
#define SetPlayerName Audio_SetPlayerName
