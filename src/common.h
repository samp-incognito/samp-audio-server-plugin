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

#ifndef COMMON_H
#define COMMON_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#define CLIENT_VERSION "0.5"
#define SERVER_VERSION "0.5"

#define MAX_BUFFER (512)

class Server;
class Session;

typedef boost::shared_ptr<boost::asio::ip::tcp::acceptor> SharedAcceptor;
typedef boost::shared_ptr<Session> SharedSession;

#endif
