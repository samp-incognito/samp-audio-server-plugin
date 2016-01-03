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

#include "core.h"

#include "main.h"
#include "session.h"

#include <boost/algorithm/string.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>

#include <sdk/plugin.h>

#include <SimpleIni/SimpleIni.h>

#include <fstream>
#include <map>
#include <set>
#include <string>

boost::scoped_ptr<Core> core;

Core::Core()
{
	packAutomated = false;
	packFiles = 0;
	packTransferable = false;
	server.reset(new Server);
}

bool Core::setPack(const std::string &name, bool transferable, bool automated)
{
	CSimpleIniA ini(true, true, true);
	SI_Error error = ini.LoadFile("audio.ini");
	if (error < 0)
	{
		logprintf("*** Audio_SetPack: Error opening audio.ini");
		return false;
	}
	bool result = false;
	const char *item = NULL, *section = NULL;
	CSimpleIniA::TNamesDepend sections;
	ini.GetAllSections(sections);
	for (CSimpleIniA::TNamesDepend::const_iterator s = sections.begin(); s != sections.end(); ++s)
	{
		section = s->pItem;
		if (boost::algorithm::iequals(section, name))
		{
			result = true;
			break;
		}
	}
	if (!result)
	{
		logprintf("*** Audio_SetPack: Audio pack \"%s\" not found in audio.ini", name.c_str());
		return false;
	}
	const CSimpleIniA::TKeyVal *sectionData;
	sectionData = ini.GetSection(section);
	if (sectionData)
	{
		std::string buffer;
		files.clear();
		packFiles = 0;
		for (CSimpleIniA::TKeyVal::const_iterator k = sectionData->begin(); k != sectionData->end(); ++k)
		{
			bool remote = false;
			boost::uint32_t checksum = 0;
			const char *value = k->second;
			int fileID = 0;
			std::size_t size = 0;
			item = k->first.pItem;
			buffer = value;
			try
			{
				fileID = boost::lexical_cast<int>(item);
			}
			catch (boost::bad_lexical_cast &)
			{
				logprintf("*** Audio_SetPack: Error mapping \"%s\" (invalid audio ID: \"%s\")", value, item);
				continue;
			}
			std::map<int, Data::File>::iterator f = files.find(fileID);
			if (f != files.end())
			{
				logprintf("*** Audio_SetPack: Error mapping \"%s\" (audio ID %s has already been mapped)", value, item);
				continue;
			}
			if (boost::algorithm::istarts_with(buffer, "http://"))
			{
				remote = true;
				if (buffer.length() > 256)
				{
					logprintf("*** Audio_SetPack: Error mapping \"%s\" (URL must be 256 characters or less)", buffer.c_str());
					continue;
				}
				std::size_t location = buffer.find_last_of('/');
				std::string name = buffer.substr(location + 1);
				if (name.length() > 128)
				{
					logprintf("*** Audio_SetPack: Error mapping \"%s\" (file name must be 128 characters or less)", buffer.c_str());
					continue;
				}
				if (!transferable)
				{
					buffer = name;
				}
			}
			else
			{
				if (buffer.length() > 128)
				{
					logprintf("*** Audio_SetPack: Error mapping \"%s\" (file name must be 128 characters or less)", buffer.c_str());
					continue;
				}
				if (transferable)
				{
					std::string fileLocation = boost::str(boost::format("audiopacks/%1%/%2%") % name % buffer);
					std::fstream fileCRC(fileLocation.c_str(), std::ios_base::in | std::ios_base::binary);
					if (!fileCRC)
					{
						logprintf("*** Audio_SetPack: Error opening \"%s\" for CRC check", fileLocation.c_str());
						fileCRC.close();
						continue;
					}
					boost::crc_32_type digest;
					char crcBuffer[MAX_BUFFER];
					while (fileCRC)
					{
						fileCRC.read(crcBuffer, MAX_BUFFER);
						digest.process_bytes(crcBuffer, static_cast<std::size_t>(fileCRC.gcount()));
					}
					fileCRC.close();
					checksum = digest.checksum();
					std::fstream fileIn(fileLocation.c_str(), std::ios_base::in | std::ios_base::binary | std::ios_base::ate);
					if (!fileIn)
					{
						logprintf("*** Audio_SetPack: Error opening \"%s\" for file size check", fileLocation.c_str());
						fileIn.close();
						continue;
					}
					size = static_cast<std::size_t>(fileIn.tellg());
					fileIn.close();
				}
			}
			Data::File file;
			file.checksum = checksum;
			file.name = buffer;
			file.remote = remote;
			file.size = size;
			files.insert(std::pair<int, Data::File>(fileID, file));
			packFiles = files.size();
		}
	}
	logprintf("*** Audio_SetPack: Audio pack set to \"%s\" (%s and %s)", name.c_str(), transferable ? "transferable" : "not transferable", automated ? "automated" : "not automated");
	for (std::set<SharedSession>::iterator c = getServer()->sessions.begin(); c != getServer()->sessions.end(); ++c)
	{
		if ((*c)->connected)
		{
			if ((*c)->transferring)
			{
				(*c)->stopTransfer();
			}
			if (automated)
			{
				(*c)->sendAsync(boost::str(boost::format("%1%\t%2%\n") % Server::Connect % name));
				(*c)->startTransfer();
			}
		}
	}
	packAutomated = automated;
	packName = name;
	packTransferable = transferable;
	return true;
}
