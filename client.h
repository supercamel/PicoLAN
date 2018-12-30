#ifndef PICOLAN_CLIENT_H 
#define PICOLAN_CLIENT_H 

#include "socket_stream.h"

namespace picolan
{

	class Client : public SocketStream
	{
		public:
			Client(uint8_t* bf, uint32_t len, uint8_t port) 
				: SocketStream(bf, len, port)
			{
				state = CONNECTION_CLOSED;
			}

			uint8_t get_remote_port() {
				return remote_port;
			}

			int connect(uint8_t remote, uint8_t port);

		private:
			void on_data(uint8_t remote, const uint8_t* data, uint32_t len);
	};
}

#endif

