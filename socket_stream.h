#ifndef PICOLAN_SOCKET_STREAM_H
#define PICOLAN_SOCKET_STREAM_H

#include <etk/etk.h>
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

class SocketStream : public Socket
{
	public:
		SocketStream(uint8_t* b, uint32_t len, uint8_t port)
			: Socket(b, len, port)
		{ }

		int write(uint8_t* bytes, uint32_t len);
		uint32_t read(uint8_t* buffer, uint32_t len);

		bool closed();
		bool connected();
		void disconnect();

	protected:

		int send_syn();
		int send_ack();

		CONNECTION_STATE state = CONNECTION_CLOSED;

		uint8_t zero_read_count = 0;
		uint8_t sequence_number;
		uint8_t remote_sequence;
		uint8_t remote_port;
		uint8_t last_recved_ack;


};


}


#endif


