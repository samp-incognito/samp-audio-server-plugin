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

#include "main.h"

#include "core.h"
#include "natives.h"

#include <sdk/plugin.h>

#include <set>
#include <string>
#include <vector>

logprintf_t logprintf;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	core.reset(new Core);
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];
	logprintf("\n\n*** Audio Plugin v%s by Incognito loaded ***\n", PLUGIN_VERSION);
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	core.reset();
	logprintf("\n\n*** Audio Plugin v%s by Incognito unloaded ***\n", PLUGIN_VERSION);
}

AMX_NATIVE_INFO natives[] =
{
	{ "Audio_CreateTCPServer", Natives::Audio_CreateTCPServer },
	{ "Audio_DestroyTCPServer", Natives::Audio_DestroyTCPServer },
	{ "Audio_SetPack", Natives::Audio_SetPack },
	{ "Audio_IsClientConnected", Natives::Audio_IsClientConnected },
	{ "Audio_SendMessage", Natives::Audio_SendMessage },
	{ "Audio_TransferPack", Natives::Audio_TransferPack },
	{ "Audio_CreateSequence", Natives::Audio_CreateSequence },
	{ "Audio_DestroySequence", Natives::Audio_DestroySequence },
	{ "Audio_AddToSequence", Natives::Audio_AddToSequence },
	{ "Audio_RemoveFromSequence", Natives::Audio_RemoveFromSequence },
	{ "Audio_Play", Natives::Audio_Play },
	{ "Audio_PlayStreamed", Natives::Audio_PlayStreamed },
	{ "Audio_PlaySequence", Natives::Audio_PlaySequence },
	{ "Audio_Pause", Natives::Audio_Pause },
	{ "Audio_Resume", Natives::Audio_Resume },
	{ "Audio_Stop", Natives::Audio_Stop },
	{ "Audio_Restart", Natives::Audio_Restart },
	{ "Audio_GetPosition", Natives::Audio_GetPosition },
	{ "Audio_SetPosition", Natives::Audio_SetPosition },
	{ "Audio_SetVolume", Natives::Audio_SetVolume },
	{ "Audio_SetFX", Natives::Audio_SetFX },
	{ "Audio_RemoveFX", Natives::Audio_RemoveFX },
	{ "Audio_Set3DPosition", Natives::Audio_Set3DPosition },
	{ "Audio_Remove3DPosition", Natives::Audio_Remove3DPosition },
	{ "Audio_SetRadioStation", Natives::Audio_SetRadioStation },
	{ "Audio_StopRadio", Natives::Audio_StopRadio },
	{ "Audio_AddPlayer", Natives::Audio_AddPlayer },
	{ "Audio_RenamePlayer", Natives::Audio_RenamePlayer },
	{ "Audio_RemovePlayer", Natives::Audio_RemovePlayer },
	{ 0, 0 }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx)
{
	core->interfaces.insert(amx);
	return amx_Register(amx, natives, -1);
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx)
{
	core->interfaces.erase(amx);
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	if (!core->messages.empty())
	{
		boost::mutex::scoped_lock lock(core->mutex);
		Data::Message message(core->messages.front());
		core->messages.pop();
		lock.unlock();
		for (std::set<AMX*>::iterator i = core->interfaces.begin(); i != core->interfaces.end(); ++i)
		{
			cell amxAddress = 0;
			int amxIndex = 0;
			switch (message.array.at(0))
			{
				case Data::OnClientConnect:
				{
					if (!amx_FindPublic(*i, "Audio_OnClientConnect", &amxIndex))
					{
						amx_Push(*i, message.array.at(1));
						amx_Exec(*i, NULL, amxIndex);
					}
					break;
				}
				case Data::OnClientDisconnect:
				{
					if (!amx_FindPublic(*i, "Audio_OnClientDisconnect", &amxIndex))
					{
						amx_Push(*i, message.array.at(1));
						amx_Exec(*i, NULL, amxIndex);
					}
					break;
				}
				case Data::OnTransferFile:
				{
					if (!amx_FindPublic(*i, "Audio_OnTransferFile", &amxIndex))
					{
						amx_Push(*i, message.array.at(1));
						amx_Push(*i, message.array.at(2));
						amx_Push(*i, message.array.at(3));
						amx_PushString(*i, &amxAddress, NULL, message.buffer.at(0).c_str(), 0, 0);
						amx_Push(*i, message.array.at(4));
						amx_Exec(*i, NULL, amxIndex);
						amx_Release(*i, amxAddress);
					}
					break;
				}
				case Data::OnPlay:
				{
					if (!amx_FindPublic(*i, "Audio_OnPlay", &amxIndex))
					{
						amx_Push(*i, message.array.at(1));
						amx_Push(*i, message.array.at(2));
						amx_Exec(*i, NULL, amxIndex);
					}
					break;
				}
				case Data::OnStop:
				{
					if (!amx_FindPublic(*i, "Audio_OnStop", &amxIndex))
					{
						amx_Push(*i, message.array.at(1));
						amx_Push(*i, message.array.at(2));
						amx_Exec(*i, NULL, amxIndex);
					}
					break;
				}
				case Data::OnRadioStationChange:
				{
					if (!amx_FindPublic(*i, "Audio_OnRadioStationChange", &amxIndex))
					{
						amx_Push(*i, message.array.at(1));
						amx_Push(*i, message.array.at(2));
						amx_Exec(*i, NULL, amxIndex);
					}
					break;
				}
				case Data::OnTrackChange:
				{
					if (!amx_FindPublic(*i, "Audio_OnTrackChange", &amxIndex))
					{
						amx_PushString(*i, &amxAddress, NULL, message.buffer.at(0).c_str(), 0, 0);
						amx_Push(*i, message.array.at(1));
						amx_Push(*i, message.array.at(2));
						amx_Exec(*i, NULL, amxIndex);
						amx_Release(*i, amxAddress);
					}
					break;
				}
				case Data::OnGetPosition:
				{
					if (!amx_FindPublic(*i, message.buffer.at(0).c_str(), &amxIndex))
					{
						amx_Push(*i, message.array.at(1));
						amx_Push(*i, message.array.at(2));
						amx_Push(*i, message.array.at(3));
						amx_Exec(*i, NULL, amxIndex);
					}
					break;
				}
			}
		}
	}
}
