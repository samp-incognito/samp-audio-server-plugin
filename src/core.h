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

#ifndef CORE_H
#define CORE_H

#include "data.h"
#include "server.h"

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include <sdk/plugin.h>

#include <map>
#include <queue>
#include <set>

class Core
{
public:
	Core();

	bool setPack(const std::string &name, bool transferable, bool automated);

	inline Server *getServer()
	{
		return server.get();
	}

	boost::mutex mutex;

	bool packAutomated;
	int packFiles;
	std::string packName;
	bool packTransferable;

	std::set<AMX*> interfaces;
	std::queue<Data::Message> messages;

	std::map<int, Data::File> files;
	std::map<int, Data::Sequence> sequences;
	std::map<int, Data::Player> players;
private:
	boost::scoped_ptr<Server> server;
};

extern boost::scoped_ptr<Core> core;

#endif
