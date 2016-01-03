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

#ifndef SESSION_H
#define SESSION_H

#include "common.h"
#include "data.h"

#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <fstream>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>

class Session : public boost::enable_shared_from_this<Session>
{
	friend class Server;

public:
	Session(boost::asio::io_service &io_service);

	void sendAsync(const std::string &buffer);
	void startAsync();
	void stopAsync();

	void startTransfer();
	void stopTransfer();

	bool connected;
	bool downloading;
	std::set<int> handles;
	int playerID;
	std::string playerName;
	std::map<int, Data::Message> requests;
	bool transferring;
private:
	void handleRead(const boost::system::error_code &error, std::size_t transferredBytes);
	void handleHeartbeatTimer(const boost::system::error_code &error);
	void handleTransferTimer(const boost::system::error_code &error);
	void handleWrite(const boost::system::error_code &error);
	void handleWriteFile(const boost::system::error_code &error);

	void startHeartbeatTimer();
	void startTransferTimer();
	void stopHeartbeatTimer();
	void stopTransferTimer();

	void parseBuffer(const std::string &buffer);

	void performAuthenticate();
	void performTransfer();
	void performPlay();
	void performSequence();
	void performStop();
	void performRadioStation();
	void performTrack();
	void performPosition();

	void registerFile(int code);
	void sendFileData();
	void transferFile();

	enum Commands
	{
		Authenticate,
		Transfer,
		Play,
		Sequence,
		Stop,
		RadioStation,
		Track,
		Position
	};

	enum PlayCodes
	{
		Success,
		Failure
	};

	enum TransferCodes
	{
		Local,
		Remote,
		Check,
		Error
	};

	struct File
	{
		File();

		boost::array<char, MAX_BUFFER> buffer;
		int id;
		std::fstream handle;
		std::size_t size;
		int transferred;
	};

	boost::shared_ptr<File> file;

	std::vector<std::string> commandTokens;
	boost::asio::deadline_timer heartbeatTimer;
	std::vector<std::string> messageTokens;
	bool missedResponse;
	std::queue<std::string> pendingMessages;
	char receivedData[MAX_BUFFER];
	std::string sentData;
	std::string sessionAddress;
	unsigned short sessionPort;
	boost::asio::ip::tcp::socket sessionSocket;
	boost::asio::deadline_timer transferTimer;
	bool waitingForResponse;
	bool writeInProgress;
};

#endif
