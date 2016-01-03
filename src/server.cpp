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

#include "server.h"

#include "core.h"
#include "main.h"
#include "session.h"

#include <boost/asio.hpp>
#include <boost/unordered_map.hpp>

#include <sdk/plugin.h>

#include <set>

void Server::handleAccept(const boost::system::error_code &error, SharedSession session)
{
	boost::mutex::scoped_lock lock(core->mutex);
	if (acceptor)
	{
		if (!error)
		{
			session->startAsync();
			startAccept();
		}
	}
}

void Server::startAccept()
{
	SharedSession session(new Session(io_service));
	acceptor->async_accept(session->sessionSocket, boost::bind(&Server::handleAccept, this, boost::asio::placeholders::error, session));
}

void Server::startAsync()
{
	unsigned short port = acceptor->local_endpoint().port();
	logprintf("*** Audio Plugin: Started TCP server on port %d", port);
	io_service.reset();
	boost::system::error_code error;
	io_service.run(error);
	logprintf("*** Audio Plugin: Stopped TCP server on port %d", port);
}

void Server::stopAsync()
{
	if (acceptor)
	{
		acceptor->close();
		acceptor.reset();
		for (std::set<SharedSession>::iterator c = sessions.begin(); c != sessions.end(); ++c)
		{
			(*c)->connected = false;
			(*c)->stopAsync();
		}
		clients.clear();
		sessions.clear();
	}
	io_service.stop();
}

bool Server::createAcceptor(unsigned short port)
{
	boost::system::error_code error;
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
	acceptor = SharedAcceptor(new boost::asio::ip::tcp::acceptor(io_service));
	acceptor->open(endpoint.protocol(), error);
	if (error)
	{
		logprintf("*** Audio Plugin: Error opening acceptor: %s", error.message().c_str());
		acceptor.reset();
		return false;
	}
	acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), error);
	if (error)
	{
		logprintf("*** Audio Plugin: Error setting option for acceptor: %s", error.message().c_str());
		acceptor.reset();
		return false;
	}
	acceptor->bind(endpoint, error);
	if (error)
	{
		logprintf("*** Audio Plugin: Error binding endpoint for acceptor: %s", error.message().c_str());
		acceptor.reset();
		return false;
	}
	acceptor->listen(boost::asio::socket_base::max_connections, error);
	if (error)
	{
		logprintf("*** Audio Plugin: Error listening for acceptor: %s", error.message().c_str());
		acceptor.reset();
		return false;
	}
	startAccept();
	return true;
}

bool Server::isRunning()
{
	return (acceptor ? true : false);
}
