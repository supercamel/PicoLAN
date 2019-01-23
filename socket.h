#ifndef PICOLAN_SOCKET_H
#define PICOLAN_SOCKET_H


#include <etk/etk.h>
#include "time.h"

namespace picolan
{

	/**
	 * ERROR_NONE means no error occured.
	 */
	constexpr int ERROR_NONE = 0;

	/**
	 * ERROR_TIMEOUT there was no response within the timeout period.
	 */
	constexpr int ERROR_TIMEOUT = -1;

	/**
	 * ERROR_BAD_STATE occurs when a function is called and the socket state is incorrect. For example, if the socket is closed it called read or write.
	 */
	constexpr int ERROR_BAD_STATE = -2;

	/**
	 * ERROR_ACK_OUT_OF_SEQUENCE indicates the connection has been interrupted or somehow broken. 
	 */
	constexpr int ERROR_ACK_OUT_OF_SEQUENCE = -3;


	/**
	 * The Socket class is a base class for the datagram, client and server classes.
	 */
	class Socket
	{
		public:
			/**
			 * \brief The socket class is an abstract base class. 
			 * @param buffer a pointer to a buffer where received bytes can be queued until read() is called.
			 * @param len the length of the buffer. Usually 64 bytes is adequate, however if your application only expects tiny packets this can be reduced and vice versa. 
			 * @param port the port number for the socket to listen on. 
			 */
			Socket(uint8_t* buffer, uint32_t len, uint8_t port) 
				: port(port), ringbuf(buffer, len)
			{}

			/**
			 * \brief The destructor unbinds the socket from the interface.
			 */
			~Socket();


			/**
			 * \brief read will copy the bytes from the sockets internal buffer into the buffer provided.
			 * If len is longer than the number of bytes already received, read will wait until timeout.
			 * This function also calls Interface::read so all sockets on the interface will be serviced.
			 * @param buffer the location to store bytes that are received
			 * @param len the number of bytes to read
			 */
			uint32_t read(uint8_t* buffer, uint32_t len);


			/**
			 * \brief returns the port number that the socket is receiving on.
			 * \returns the port number that the socket is receiving on.
			 */
			uint8_t get_port() const {
				return port;
			}

			/**
			 * \brief gets the source address of the last received packet. 
			 * Useful for datagrams. By calling get_remote a datagram socket can reply to a query.
			 * \return the source address of the last received packet
			 */
			uint8_t get_remote() const { 
				return remote;
			}

			/**
			 * \brief set_timeout sets the number of milliseconds the socket will wait to receive data before
			 * a timeout error will occur.
			 */
			void set_timeout(uint16_t t) {
				timeout = t;
			}

			/**
			 * \brief gets the timeout period in milliseconds
			 * \return the number of milliseconds that certain functions such as read 
			 * will wait before a timeout error will occur.
			 */
			uint16_t get_timeout() const {
				return timeout;
			}

			/**
			 * \brief unbinds the socket from the interface.
			 * No data can be sent or received on the socket until it has been bound again using Interface::bind()
			 */
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

