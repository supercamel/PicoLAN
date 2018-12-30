#ifndef PICOLAN_DATAGRAM_H
#define PICOLAN_DATAGRAM_H

#include "socket.h"

namespace picolan
{

class Datagram 
	: public Socket
{
	public:
		Datagram(uint8_t* buffer, uint32_t len, uint8_t port) 
			: Socket(buffer, len, port)
		{ }

		int write(uint8_t dest, uint8_t port, uint8_t* data, uint32_t len);


	private:
		void on_data(
				uint8_t remote, const uint8_t* data, uint32_t len);
};

}

#endif

