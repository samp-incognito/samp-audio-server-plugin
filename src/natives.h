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

#ifndef NATIVES_H
#define NATIVES_H

#include <sdk/plugin.h>

#define CHECK_PARAMS(m, n) \
	if (params[0] != (m * 4)) \
	{ \
		logprintf("*** %s: Expecting %d parameter(s), but found %d", n, m, params[0] / 4); \
		return 0; \
	}

namespace Natives
{
	cell AMX_NATIVE_CALL Audio_CreateTCPServer(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_DestroyTCPServer(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_SetPack(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_IsClientConnected(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_SendMessage(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_TransferPack(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_CreateSequence(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_DestroySequence(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_AddToSequence(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_RemoveFromSequence(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_Play(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_PlayStreamed(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_PlaySequence(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_Pause(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_Resume(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_Stop(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_Restart(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_GetPosition(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_SetPosition(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_SetVolume(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_SetFX(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_RemoveFX(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_Set3DPosition(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_Remove3DPosition(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_SetRadioStation(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_StopRadio(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_AddPlayer(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_RenamePlayer(AMX *amx, cell *params);
	cell AMX_NATIVE_CALL Audio_RemovePlayer(AMX *amx, cell *params);
};

#endif
