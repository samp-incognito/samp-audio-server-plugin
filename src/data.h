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

#ifndef DATA_H
#define DATA_H

#include <boost/cstdint.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace Data
{
	enum Callbacks
	{
		OnClientConnect,
		OnClientDisconnect,
		OnTransferFile,
		OnPlay,
		OnStop,
		OnRadioStationChange,
		OnTrackChange,
		OnGetPosition
	};

	struct File
	{
		File();

		boost::uint32_t checksum;
		std::string name;
		std::set<int> players;
		bool remote;
		std::size_t size;
	};

	struct Message
	{
		std::vector<int> array;
		std::vector<std::string> buffer;
	};

	struct Sequence
	{
		std::vector<int> audioIDs;
		std::map<int, int> transfers;
	};

	struct Player
	{
		std::string address;
		std::vector<std::string> name;
	};
};

#endif
