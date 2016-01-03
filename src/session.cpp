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

#include "session.h"

#include "core.h"
#include "main.h"

#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

Session::Session(boost::asio::io_service &io_service) : heartbeatTimer(io_service), sessionSocket(io_service), transferTimer(io_service)
{
	connected = false;
	downloading = false;
	missedResponse = false;
	playerID = std::numeric_limits<int>::max();
	transferring = false;
	waitingForResponse = false;
	writeInProgress = false;
}

Session::File::File()
{
	id = 0;
	size = 0;
	transferred = 0;
}

void Session::handleHeartbeatTimer(const boost::system::error_code &error)
{
	boost::mutex::scoped_lock lock(core->mutex);
	if (!error)
	{
		if (!file)
		{
			if (!waitingForResponse)
			{
				sendAsync("\n");
				missedResponse = false;
				waitingForResponse = true;
			}
			else
			{
				if (missedResponse)
				{
					stopAsync();
					return;
				}
				missedResponse = true;
			}
		}
		startHeartbeatTimer();
	}
}

void Session::handleRead(const boost::system::error_code &error, std::size_t bytesTransferred)
{
	boost::mutex::scoped_lock lock(core->mutex);
	if (!error)
	{
		std::string buffer(receivedData);
		buffer.resize(bytesTransferred);
		boost::algorithm::erase_last(buffer, "\n");
		boost::algorithm::split(messageTokens, buffer, boost::algorithm::is_any_of("\n"));
		if (messageTokens.empty())
		{
			parseBuffer(buffer);
		}
		else
		{
			for (std::vector<std::string>::iterator i = messageTokens.begin(); i != messageTokens.end(); ++i)
			{
				parseBuffer(*i);
			}
		}
		sessionSocket.async_read_some(boost::asio::buffer(receivedData), boost::bind(&Session::handleRead, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		if (connected)
		{
			logprintf("*** Audio Plugin: %s (ID: %d) has disconnected", playerName.c_str(), playerID);
			Data::Message message;
			message.array.push_back(Data::OnClientDisconnect);
			message.array.push_back(playerID);
			core->messages.push(message);
			connected = false;
		}
		for (std::map<int, Data::File>::iterator f = core->files.begin(); f != core->files.end(); ++f)
		{
			f->second.players.erase(playerID);
		}
		for (std::map<int, Data::Sequence>::iterator s = core->sequences.begin(); s != core->sequences.end(); ++s)
		{
			s->second.transfers.erase(playerID);
		}
		stopAsync();
	}
}

void Session::handleTransferTimer(const boost::system::error_code &error)
{
	boost::mutex::scoped_lock lock(core->mutex);
	if (!error)
	{
		if (!pendingMessages.empty())
		{
			sendAsync(pendingMessages.front());
			pendingMessages.pop();
		}
		sendFileData();
	}
}

void Session::handleWrite(const boost::system::error_code &error)
{
	boost::mutex::scoped_lock lock(core->mutex);
	writeInProgress = false;
	if (!error)
	{
		if (!pendingMessages.empty())
		{
			sendAsync(pendingMessages.front());
			pendingMessages.pop();
		}
	}
	else
	{
		pendingMessages = std::queue<std::string>();
	}
}

void Session::handleWriteFile(const boost::system::error_code &error)
{
	boost::mutex::scoped_lock lock(core->mutex);
	if (!error)
	{
		transferFile();
	}
	else
	{
		registerFile(Error);
	}
}

void Session::sendAsync(const std::string &buffer)
{
	if (downloading || writeInProgress)
	{
		pendingMessages.push(buffer);
	}
	else
	{
		sentData = buffer;
		writeInProgress = true;
		boost::asio::async_write(sessionSocket, boost::asio::buffer(sentData, sentData.length()), boost::bind(&Session::handleWrite, shared_from_this(), boost::asio::placeholders::error));
	}
}

void Session::startAsync()
{
	boost::system::error_code error;
	boost::asio::ip::tcp::endpoint remoteEndpoint = sessionSocket.remote_endpoint(error);
	if (error)
	{
		stopAsync();
		return;
	}
	sessionAddress = remoteEndpoint.address().to_string();
	sessionPort = remoteEndpoint.port();
	for (std::set<SharedSession>::iterator c = core->getServer()->sessions.begin(); c != core->getServer()->sessions.end(); ++c)
	{
		if (boost::algorithm::equals((*c)->sessionAddress, sessionAddress))
		{
			stopAsync();
			return;
		}
	}
	sessionSocket.async_read_some(boost::asio::buffer(receivedData), boost::bind(&Session::handleRead, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	startHeartbeatTimer();
	core->getServer()->sessions.insert(shared_from_this());
}

void Session::stopAsync()
{
	if (sessionSocket.is_open())
	{
		boost::system::error_code error;
		sessionSocket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
		sessionSocket.close(error);
		stopHeartbeatTimer();
		stopTransferTimer();
	}
	if (core->getServer()->isRunning())
	{
		boost::unordered_map<int, SharedSession>::iterator c = core->getServer()->clients.find(playerID);
		if (c != core->getServer()->clients.end())
		{
			core->getServer()->clients.erase(playerID);
		}
		core->getServer()->sessions.erase(shared_from_this());
	}
}

void Session::startTransfer()
{
	file = boost::shared_ptr<File>(new File);
	missedResponse = false;
	waitingForResponse = false;
	transferring = true;
	startTransferTimer();
}

void Session::stopTransfer()
{
	if (downloading)
	{
		downloading = false;
		sendAsync("CANCEL");
	}
	file.reset();
	transferring = false;
	stopTransferTimer();
}

void Session::startHeartbeatTimer()
{
	heartbeatTimer.expires_from_now(boost::posix_time::seconds(10));
	heartbeatTimer.async_wait(boost::bind(&Session::handleHeartbeatTimer, shared_from_this(), boost::asio::placeholders::error));
}

void Session::startTransferTimer()
{
	transferTimer.expires_from_now(boost::posix_time::seconds(1));
	transferTimer.async_wait(boost::bind(&Session::handleTransferTimer, shared_from_this(), boost::asio::placeholders::error));
}

void Session::stopHeartbeatTimer()
{
	boost::system::error_code error;
	heartbeatTimer.cancel(error);
}

void Session::stopTransferTimer()
{
	boost::system::error_code error;
	transferTimer.cancel(error);
}

void Session::parseBuffer(const std::string &buffer)
{
	if (buffer.empty())
	{
		waitingForResponse = false;
		return;
	}
	boost::algorithm::split(commandTokens, buffer, boost::algorithm::is_any_of("\t"));
	if (commandTokens.size() < 2 || commandTokens.size() > 4)
	{
		return;
	}
	for (std::vector<std::string>::iterator i = commandTokens.begin(); i != commandTokens.end(); ++i)
	{
		if (i->empty())
		{
			return;
		}
	}
	int command = 0;
	try
	{
		command = boost::lexical_cast<int>(commandTokens.at(0));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	switch (command)
	{
		case Authenticate:
		{
			return performAuthenticate();
		}
		case Transfer:
		{
			return performTransfer();
		}
		case Play:
		{
			return performPlay();
		}
		case Sequence:
		{
			return performSequence();
		}
		case Stop:
		{
			return performStop();
		}
		case RadioStation:
		{
			return performRadioStation();
		}
		case Track:
		{
			return performTrack();
		}
		case Position:
		{
			return performPosition();
		}
	}
}

void Session::performAuthenticate()
{
	if (connected || commandTokens.size() != 3)
	{
		return;
	}
	if (!boost::algorithm::equals(CLIENT_VERSION, commandTokens.at(2)))
	{
		sendAsync(boost::str(boost::format("%1%\t%2%\n") % Server::Message % (boost::str(boost::format("This server only supports client version %1%") % CLIENT_VERSION))));
		stopAsync();
		return;
	}
	for (std::map<int, Data::Player>::iterator p = core->players.begin(); p != core->players.end(); ++p)
	{
		if (boost::algorithm::equals(p->second.name.front(), commandTokens.at(1)))
		{
			boost::unordered_map<int, SharedSession>::iterator c = core->getServer()->clients.find(p->first);
			if (c != core->getServer()->clients.end())
			{
				sendAsync(boost::str(boost::format("%1%\t%2%\n") % Server::Message % "Player already connected"));
				stopAsync();
				return;
			}
			if (boost::algorithm::equals(sessionAddress, p->second.address))
			{
				if (p->second.name.size() > 1)
				{
					p->second.name.front() = p->second.name.back();
					p->second.name.pop_back();
					sendAsync(boost::str(boost::format("%1%\t%2%\n") % Server::Name % p->second.name.front()));
				}
				if (core->packFiles)
				{
					sendAsync(boost::str(boost::format("%1%\t%2%\n") % Server::Connect % core->packName));
					if (core->packAutomated)
					{
						startTransfer();
					}
				}
				else
				{
					sendAsync(boost::str(boost::format("%1%\n") % Server::Connect));
				}
				logprintf("*** Audio Plugin: %s (ID: %d) has connected", p->second.name.front().c_str(), p->first);
				Data::Message message;
				message.array.push_back(Data::OnClientConnect);
				message.array.push_back(p->first);
				core->messages.push(message);
				connected = true;
				playerID = p->first;
				playerName = p->second.name.front();
				break;
			}
			else
			{
				sendAsync(boost::str(boost::format("%1%\t%2%\n") % Server::Message % "IP address does not match"));
				stopAsync();
				return;
			}
			break;
		}
	}
	if (!connected)
	{
		sendAsync(boost::str(boost::format("%1%\t%2%\n") % Server::Message % "Player not connected"));
		stopAsync();
	}
	else
	{
		core->getServer()->clients.insert(std::make_pair(playerID, shared_from_this()));
	}
}

void Session::performTransfer()
{
	if (!connected || downloading || !file || commandTokens.size() != 2)
	{
		return;
	}
	int code = 0;
	try
	{
		code = boost::lexical_cast<int>(commandTokens.at(1));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	switch (code)
	{
		case Local:
		{
			downloading = true;
			transferFile();
			break;
		}
		case Remote:
		case Check:
		case Error:
		{
			registerFile(code);
			break;
		}
	}
}

void Session::performPlay()
{
	if (!connected || commandTokens.size() != 3)
	{
		return;
	}
	int handleID = 0, result = 0;
	try
	{
		handleID = boost::lexical_cast<int>(commandTokens.at(1));
		result = boost::lexical_cast<int>(commandTokens.at(2));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	std::set<int>::iterator h = handles.find(handleID);
	if (h != handles.end())
	{
		switch (result)
		{
			case Success:
			{
				Data::Message message;
				message.array.push_back(Data::OnPlay);
				message.array.push_back(handleID);
				message.array.push_back(playerID);
				core->messages.push(message);
				break;
			}
			case Failure:
			{
				handles.erase(h);
				break;
			}
		}
	}
}

void Session::performSequence()
{
	if (!connected || commandTokens.size() != 3)
	{
		return;
	}
	int handleID = 0, sequenceID = 0;
	try
	{
		sequenceID = boost::lexical_cast<int>(commandTokens.at(1));
		handleID = boost::lexical_cast<int>(commandTokens.at(2));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	std::map<int, Data::Sequence>::iterator s = core->sequences.find(sequenceID);
	if (s == core->sequences.end())
	{
		return;
	}
	std::map<int, int>::iterator t = s->second.transfers.find(playerID);
	if (t == s->second.transfers.end())
	{
		return;
	}
	std::string output;
	std::vector<int>::iterator b = s->second.audioIDs.begin();
	for (std::vector<int>::iterator a = s->second.audioIDs.begin(); a != s->second.audioIDs.end(); ++a)
	{
		if (std::distance(b, a) >= t->second)
		{
			if (t->second < (t->second + 32))
			{
				try
				{
					output += boost::lexical_cast<std::string>(*a) + " ";
				}
				catch (boost::bad_lexical_cast &)
				{
					continue;
				}
			}
			else
			{
				t->second = std::distance(b, a);
				output += "U";
				break;
			}
		}
	}
	if (output.find("U") == std::string::npos)
	{
		output += "F";
		s->second.transfers.erase(t);
	}
	sendAsync(boost::str(boost::format("%1%\t%2%\t%3%\n") % Server::PlaySequence % handleID % output));
}

void Session::performStop()
{
	if (!connected || commandTokens.size() != 2)
	{
		return;
	}
	int handleID = 0;
	try
	{
		handleID = boost::lexical_cast<int>(commandTokens.at(1));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	std::set<int>::iterator h = handles.find(handleID);
	if (h != handles.end())
	{
		Data::Message message;
		message.array.push_back(Data::OnStop);
		message.array.push_back(handleID);
		message.array.push_back(playerID);
		core->messages.push(message);
		handles.erase(h);
	}
}

void Session::performTrack()
{
	if (!connected || commandTokens.size() != 3)
	{
		return;
	}
	int handleID = 0;
	try
	{
		handleID = boost::lexical_cast<int>(commandTokens.at(1));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	boost::algorithm::replace_all(commandTokens.at(2), "%", "");
	std::set<int>::iterator h = handles.find(handleID);
	if (h != handles.end())
	{
		Data::Message message;
		message.array.push_back(Data::OnTrackChange);
		message.array.push_back(handleID);
		message.array.push_back(playerID);
		message.buffer.push_back(commandTokens.at(2));
		core->messages.push(message);
	}
}

void Session::performRadioStation()
{
	if (!connected || commandTokens.size() != 2)
	{
		return;
	}
	int radioStation = 0;
	try
	{
		radioStation = boost::lexical_cast<int>(commandTokens.at(1));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	if (radioStation < 0 || radioStation > 12)
	{
		return;
	}
	Data::Message message;
	message.array.push_back(Data::OnRadioStationChange);
	message.array.push_back(radioStation);
	message.array.push_back(playerID);
	core->messages.push(message);
}

void Session::performPosition()
{
	if (!connected || commandTokens.size() != 4)
	{
		return;
	}
	int handleID = 0, requestID = 0, seconds = 0;
	try
	{
		requestID = boost::lexical_cast<int>(commandTokens.at(1));
		handleID = boost::lexical_cast<int>(commandTokens.at(2));
		seconds = boost::lexical_cast<int>(commandTokens.at(3));
	}
	catch (boost::bad_lexical_cast &)
	{
		return;
	}
	if (seconds < 0)
	{
		return;
	}
	std::map<int, Data::Message>::iterator r = requests.find(requestID);
	if (r != requests.end())
	{
		if (r->second.array.at(0) == Server::Position)
		{
			std::set<int>::iterator h = handles.find(handleID);
			if (h != handles.end())
			{
				Data::Message message;
				message.array.push_back(Data::OnGetPosition);
				message.array.push_back(seconds);
				message.array.push_back(handleID);
				message.array.push_back(playerID);
				message.buffer.push_back(r->second.buffer.at(0));
				core->messages.push(message);
				requests.erase(r);
			}
		}
	}
}

void Session::registerFile(int code)
{
	if (!connected || !file)
	{
		return;
	}
	std::map<int, Data::File>::iterator f = core->files.find(file->id);
	if (f != core->files.end())
	{
		if (f->second.players.find(playerID) == f->second.players.end())
		{
			Data::Message message;
			message.array.push_back(Data::OnTransferFile);
			message.array.push_back(code);
			message.array.push_back(core->packFiles);
			message.array.push_back(++file->transferred);
			message.array.push_back(playerID);
			if (f->second.remote)
			{
				std::size_t location = f->second.name.find_last_of('/');
				message.buffer.push_back(f->second.name.substr(location + 1));
			}
			else
			{
				message.buffer.push_back(f->second.name);
			}
			boost::algorithm::replace_all(message.buffer.at(0), "%", "");
			core->messages.push(message);
			f->second.players.insert(playerID);
		}
	}
	if (downloading)
	{
		downloading = false;
		sendAsync("CANCEL");
	}
	file->handle.clear();
	file->handle.close();
	if (code != Local)
	{
		sendFileData();
	}
	else
	{
		startTransferTimer();
	}
}

void Session::sendFileData()
{
	if (!connected || !file)
	{
		return;
	}
	if (file->transferred == core->packFiles)
	{
		stopTransfer();
		sendAsync(boost::str(boost::format("%1%\n") % Server::Transfer));
		return;
	}
	for (std::map<int, Data::File>::iterator f = core->files.begin(); f != core->files.end(); ++f)
	{
		if (f->second.players.find(playerID) == f->second.players.end())
		{
			if (!f->second.remote)
			{
				std::string path = boost::str(boost::format("audiopacks/%1%/%2%") % core->packName % f->second.name);
				file->handle.open(path.c_str(), std::ios_base::in | std::ios_base::binary);
				if (!file->handle)
				{
					registerFile(Error);
					return;
				}
			}
			file->id = f->first;
			file->size = f->second.size;
			sendAsync(boost::str(boost::format("%d\t%d\t%d\t%s\t%lu\t%X\n") % Server::Transfer % core->packTransferable % f->first % f->second.name % f->second.size % f->second.checksum));
			break;
		}
	}
}

void Session::transferFile()
{
	if (!connected || !downloading || !file)
	{
		return;
	}
	if (file->handle)
	{
		file->handle.read(file->buffer.c_array(), static_cast<std::streamsize>(file->buffer.size()));
		boost::asio::async_write(sessionSocket, boost::asio::buffer(file->buffer.c_array(), static_cast<std::size_t>(file->handle.gcount())), boost::bind(&Session::handleWriteFile, shared_from_this(), boost::asio::placeholders::error));
	}
	else
	{
		downloading = false;
		registerFile(Local);
	}
}
