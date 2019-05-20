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

#ifndef PICOLAN_SOCKET_STREAM_H
#define PICOLAN_SOCKET_STREAM_H

#ifdef ARDUINO
#include <etk.h>
#else
#include <etk/etk.h>
#endif

#include "time.h"
#include "socket.h"


namespace picolan
{

enum CONNECTION_STATE
{
	CONNECTION_CLOSED,
	CONNECTION_SYN_SENT,
	CONNECTION_LISTENING,
	CONNECTION_SYN_RECVED,
	CONNECTION_PENDING,
	CONNECTION_OPEN,
} typedef CONNECTION_STATE;

namespace MESSAGE_TYPE
{
constexpr uint8_t ACK = 0;
constexpr uint8_t SYN = 1;
constexpr uint8_t DATA = 2;
constexpr uint8_t CLOSE = 3;
}

/**
 * The SocketStream class is a base class used by Client and Server for read/write functions.
 * SocketStream differs from Datagrams in a few ways. Client/Server are used to form a connection whereas
 * Datagrams are stateless. At the frame level, data is checksummed so it is extremely unlikely for
 * data to be corrupted during transfer. However it is likely that packets will be dropped occasionally.
 * Datagrams do not detect or resend dropped packets. SocketStreams do.
 * Datagrams are like fire & forget.
 * SocketStreams form a connection and ensure all data is sent in good order.
 * Datagram are faster with less overhead but socketstreams ensure reliable transfers.
 */

class SocketStream : public Socket
{
	public:
        #ifndef PICOLAN_NODE_BINDING
		SocketStream(uint8_t* b, uint32_t len, uint8_t port)
			: Socket(b, len, port)
		{ }
		#else
		SocketStream(uint8_t port) : Socket(port) { }
		#endif

		/*!
		 write writes a number of bytes
		 \param bytes a pointer to the bytes to write
		 \param len the number of bytes to write
		 \return either a positive number that indicates the number of bytes written, or an error number (such as ERROR_TIMEOUT)
		 */

         #ifndef PICOLAN_NODE_BINDING
		int write(uint8_t* bytes, uint32_t len);
        #else
        int write(std::vector<uint8_t> bytes);
        #endif

		/*!
		 read reads a number of bytes
		 \param buffer a pointer to the buffer for the read bytes
		 \param len the number of bytes to read
		 \return the number of bytes read or an error number that is less than zero
		*/
        #ifndef PICOLAN_NODE_BINDING
		int read(uint8_t* buffer, uint32_t len);
        #else
        std::vector<uint8_t> read(uint32_t len);
        #endif

		/*!
		 * \brief returns true if the connection is closed.
		 * \return true if the connection is closed, otherwise false
		 */
		bool closed();

		/**
		 * \brief returns true if the connection is open
		 * \return true if the connection is open, otherwise false
		 */
		bool connected();

		/**
		 * \brief sends a hang up signal to the other end and closes the connection.
		 * Used to end a connection.
		 */
		void disconnect();

	protected:

		int send_syn();
		int send_ack();

        uint32_t min(uint32_t a, uint32_t b) {
            if(a < b) {
                return a;
            }
            return b;
        }

		CONNECTION_STATE state = CONNECTION_CLOSED;

		uint8_t zero_read_count = 0;
		uint8_t sequence_number = 1;
		uint8_t remote_sequence;
		uint8_t remote_port;
		uint8_t last_recved_ack = 0;


};


}


#endif
