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

#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#include <boost/asio.hpp>
#include <boost/unordered_map.hpp>

#include <set>

class Server
{
	friend class Session;

public:
	void startAsync();
	void stopAsync();

	bool createAcceptor(unsigned short port);
	bool isRunning();

	enum Commands
	{
		Connect,
		Message,
		Name,
		Transfer,
		Play,
		PlaySequence,
		Pause,
		Resume,
		Stop,
		Restart,
		GetPosition,
		SetPosition,
		SetVolume,
		SetFX,
		RemoveFX,
		Set3DPosition,
		Remove3DPosition,
		GetRadioStation,
		SetRadioStation,
		StopRadio
	};

	enum Requests
	{
		Position
	};

	boost::unordered_map<int, SharedSession> clients;
	std::set<SharedSession> sessions;
private:
	void handleAccept(const boost::system::error_code &error, SharedSession session);

	void startAccept();

	SharedAcceptor acceptor;
	boost::asio::io_service io_service;
};

#endif
