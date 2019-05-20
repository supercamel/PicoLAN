/**

  Copyright 2019 Samuel Cowen <samuel.cowen@camelsoftware.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in
  the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

*/


#ifndef PICOLAN_CLIENT_H
#define PICOLAN_CLIENT_H

#include "socket_stream.h"

namespace picolan
{

	/**
	 * The Client class is used to connect to a Server.
	 * The client will either connect, or timeout if the server is unavailable.
	 * Once the connection is established, the client and server can exchange data
	 * using SocketStream::read() and SocketStream::write()
	 */

	class Client : public SocketStream
	{
		public:
			/**
			 * \brief client constructor
			 * @param bf a pointer to a buffer where bytes can be stored temporarily until they are read()
			 * @param len the length of the buffer. generally 64 bytes is adequate.
			 * @param port the port number to receive on. the number isn't important but it must be unique for each socket per interface.
			 */
            #ifndef PICOLAN_NODE_BINDING
 			Client(uint8_t* bf, uint32_t len, uint8_t port)
 				: SocketStream(bf, len, port)
 			#else
 			Client(uint8_t port) : SocketStream(port)
 			#endif
 			{
 				state = CONNECTION_CLOSED;
 			}


			/*
			   \brief get_remote_port will return the port number of the server
			   \returns the port number of the server that the client is connected to.
			   */
			uint8_t get_remote_port() {
				return remote_port;
			}

			/**
			 * \brief connect() will attempt to establish a connection with a server.
			 * @param r Server address
			 * @param port Server port number
			 *
			 */
			int connect(uint8_t remote, uint8_t port);

		private:
			void on_data(uint8_t remote, const uint8_t* data, uint32_t len);
	};
}

#endif
