#ifndef PICOLAN_SERVER_H 
#define PICOLAN_SERVER_H 

#include "socket_stream.h"

namespace picolan
{

	/**
	 * The Server class can accept a connection from a client.
	 * A useage might be to serve sensor data to a client. 
	 */
	
	class Server : public SocketStream
	{
		public:
			/**
			 * The server requires a buffer to store bytes that are received. 
			 * @param bf a pointer to the buffer
			 * @param len the length of the buffer
			 * @param port the port number to listen on
			 */
			Server(uint8_t* bf, uint32_t len, uint8_t port) 
				: SocketStream(bf, len, port)
			{
				state = CONNECTION_CLOSED;
			}

			/**
			 * \brief returns the port number of the client
			 * \return the port number of the client/remote socket
			 */
			uint8_t get_remote_port() {
				return remote_port;
			}

			/**
			 * \brief starts the server listening
			 * \return either ERROR_NONE or ERROR_BAD_STATE
			 */
			int listen();

			/**
			 * \brief checks if a client is attempting to connect.
			 * \return true if a client is trying to connect.
			 */
			bool connection_pending();

			/**
			 * \brief accepts the pending client connection
			 * \returns either ERROR_NONE or another error code
			 */
			int accept();

		private:
			void on_data(uint8_t remote, const uint8_t* data, uint32_t len);
	};
}

#endif

