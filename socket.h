#ifndef PICOLAN_SOCKET_H
#define PICOLAN_SOCKET_H


#include <etk/etk.h>
#include "time.h"

namespace picolan
{

	constexpr int ERROR_NONE = 0;
	constexpr int ERROR_TIMEOUT = -1;
	constexpr int ERROR_BAD_STATE = -2;
	constexpr int ERROR_ACK_OUT_OF_SEQUENCE = -3;


	class Socket
	{
		public:
			Socket(uint8_t* buffer, uint32_t len, uint8_t port) 
				: port(port), ringbuf(buffer, len)
			{}

			~Socket();


			uint32_t read(uint8_t* buffer, uint32_t len);


			uint8_t get_port() const {
				return port;
			}

			uint8_t get_remote() const { 
				return remote;
			}

			void set_timeout(uint16_t t) {
				timeout = t;
			}

			uint16_t get_timeout() {
				return timeout;
			}

			void destroy();

		protected:
			int timedRead();

			friend class Interface;
			class Interface* iface = nullptr;
			uint8_t remote = 0;
			uint8_t port = 0;
			uint16_t timeout = 1000;

			etk::RingBuffer<uint8_t> ringbuf;

			virtual void on_data(
					uint8_t remote, const uint8_t* data, uint32_t len) = 0;
	};
}

#endif

