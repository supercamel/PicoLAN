#ifndef PICOLAN_SERVER_H 
#define PICOLAN_SERVER_H 

#include "socket_stream.h"

namespace picolan
{

	class Server : public SocketStream
	{
		public:
			Server(uint8_t* bf, uint32_t len, uint8_t port) 
				: SocketStream(bf, len, port)
			{
				state = CONNECTION_CLOSED;
			}

			uint8_t get_remote_port() {
				return remote_port;
			}

			int listen();
			bool connection_pending();
			int accept();

		private:
			void on_data(uint8_t remote, const uint8_t* data, uint32_t len);
	};
}

#endif

