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


#ifndef PICOLAN_DATAGRAM_H
#define PICOLAN_DATAGRAM_H

#include "socket.h"

namespace picolan
{

	/**
	 * The Datagram class is used for sending and receiving packets of data to a destination.
	 * There is no connection or acknowledgement of receipt. 
	 * It just fires out a message to a destination.
	 */
	class Datagram 
		: public Socket
	{
		public:
			/**
			 * \brief The constructor. All sockets require some buffer space. 
			 * Typically 64 bytes will do, less if your application is sending smaller packets. 
			 * Also required is a port number for the datagram to listen / receive data on.
			 * After creation, the socket must be bound to an Interface using Interface::bind()  
			 * @param buffer a pointer to a buffer to store received bytes 
			 * before they are read with Socket::read()
			 * @param len the size of the buffer in bytes
			 * @port the port number to listen on
			 */
			Datagram(uint8_t* buffer, uint32_t len, uint8_t port) 
				: Socket(buffer, len, port)
			{ }

			/**
			 * \brief writes data to a destination.
			 * @param dest the destination address
			 * @param port the destination port number
			 * @param data the data to send
			 * @param len the number of bytes to send
			 * \return ERROR_NONE
			 */
			int write(uint8_t dest, uint8_t port, uint8_t* data, uint32_t len);


		private:
			void on_data(
					uint8_t remote, const uint8_t* data, uint32_t len);
	};

}

#endif

