/*

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

#ifndef PICOLAN_H
#define PICOLAN_H

/*! \mainpage PicoLAN Documentation
 *
 * \section intro_sec Introduction
 *
 * PicoLAN is a network topology and protocol that enables devices to exchange data with each other over UART.
 *
 * Using only one serial port, a device such as an Arduino UNO can receive data from many different devices.
 * Conversely, one sensor may send data to many devices.
 * Many things can connect to many other things and exchange data simultaneously while only
 * only occupying a single serial port per device.
 *
 * In order to join the network an implementation of the PicoLAN protocol must be used,
 * such as the PicoLAN library for Arduino. This means you could not, for example, plug an
 * off the shelf GPS into a switch and expect it to work. Each device on the network needs
 * an interface that implements the PicoLAN protocol. To connect a GPS, one would need to use a device
 * such as a Teensy or Arduino to present the GPS data to the network as either a broadcast using
 * Datagram or Server.
 *
 * \section network_topology_sec Network Topology
 *
 *

 * \subsection step1 Step 1: Opening the box
 *
 * etc...
 */



#include "serialiser.h"
#include "socket.h"
#include "datagram.h"
#include "server.h"
#include "client.h"
#include "ulan_time.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "usart_driver.h"
#endif

namespace picolan
{
const uint8_t MULTICAST_ADDR = 0xFE;
const uint8_t BROADCAST_ADDR = 0xFF;

class Interface
	: public ParserSerialiser
{
	public:
		/**
		 * \brief The constructor requires a reference to a Stream object so
		 * that it can handle reading and writing over the UART / interface.
		 */
		#ifdef PICOLAN_NODE_BINDING
		Interface(std::string com_port) : serial(com_port) {
			serial.begin(1000000);
		}

		#elif ARDUINO
		Interface(Stream& serial) : serial(serial) { }
        #else
        Interface(Serial& serial) : serial(serial) { }
		#endif

		/**
		 * \brief The interface needs an address. This address should be unique on the network.
		 * No other device on the network should have this address.
		 * The address can be in the range 0 to 244 inclusive. 255 is reserved for broadcasts.
		 * @param addr an 8-bit number (0-254) that is used to identify this interface on the network.
		 */
		void set_address(uint8_t addr)
		{
			auto pack = create_packet<addr_pack>();
			address = addr;
			pack.address_field.set_addr(addr);
			pack.send();
		}

		/**
		 * \brief Returns the address of the interface.
		 * \return The interface address.
		 */
		uint8_t get_address()
		{
			return address;
		}

		/**
		 * \brief get_addr_list will query the switch for a list of addresses
		 * of devices available on the network.
		 * @param timeout_ms if a response isn't received within this number of
		 * milliseconds a timeout error will be returned.
		 * @param list a reference to an etk::List where the addresses are to be stored.
		 * \return either ERROR_NONE or ERROR_TIMEOUT
		 */
		int get_addr_list(uint32_t timeout_ms = 1000)
		{
			auto pack = create_packet<get_addr_list_pack>();
			pack.ttl = 6;

            addr_list_recved = false;
			pack.send();

			auto start = millis();
			while((millis()-start) < timeout_ms) {
				read();
				if(addr_list_recved) {
					return Error::NONE;
				}
			}
			return Error::TIMEOUT;
		}

		bool lookup_addr_list(uint8_t addr)
		{
			return addr_field.get_addr(addr);
		}

		/**
		 * \brief binds a socket to the interface. If successful the socket will be able to read/write
		 * via this interface.
		 * \return true on success, false if a socket is already listening on the same port number.
		 */
		bool bind(Socket& socket)
		{
			for(auto& s : sockets) {
				if(s->get_port() == socket.get_port()) {
					return false;
				}
			}

			socket.iface = this;
			sockets.append(&socket);
			return true;
		}

		/**
		 * \brief Unbinds the socket from the interface.
		 */
		void unbind_socket(Socket& s)
		{
			for(uint32_t i = 0; i < sockets.size(); i++) {
				if(sockets[i]->get_port() == s.get_port()) {
					sockets.remove(i);
				}
			}
		}

		bool bind_datagram(Datagram& dg) {
			return bind((Socket&)dg);
		}

		void unbind_datagram(Datagram& dg) {
			unbind_socket(dg);
		}

        bool bind_server(Server& dg) {
			return bind((Socket&)dg);
		}

		void unbind_server(Server& dg) {
			unbind_socket(dg);
		}

        bool bind_client(Client& dg) {
			return bind((Socket&)dg);
		}

		void unbind_client(Client& dg) {
			unbind_socket(dg);
		}

		/**
		 * \brief Sends a ping to the destination and times the response.
		 * This can be useful for troubleshooting.
		 * @param dest the destination address
		 * @param timeout_ms timeout in milliseconds
		 * \return either the ping time or ERROR_TIMEOUT
		 */

		int ping(uint8_t dest, uint32_t timeout_ms = 1000)
		{
			auto pack = create_packet<ping_pack>();
			pack.ttl = 6;
			pack.dest_addr = dest;
			pack.source_addr = address;
			uint16 payload = millis();
			pack.payload = payload;
			pack.send();

			auto start = millis();
			while((millis()-start) < timeout_ms) {
				read();
				if(ping_echo_payload == payload) {
					return millis()-start;
				}
			}
			return Error::TIMEOUT;
		}

		/**
		 * \brief flushes the network interface stream
		 */
		void flush() {
			serial.flush();
		}

		/**
		 * 	\brief returns true if there are bytes available. If true then read() can be called.
		 *	There is no requirement to call this function before calling read().
		 */
		bool available() {
			return serial.available() != 0;
		}

		void read() {
			while(available()) {
				ParserSerialiser::read(get());
			}
		}

	private:
		friend class ParserSerialiser;

		uint8 get() {
			uint8 r = serial.get();
			return r;
		}

		void put(uint8 c) {
			#ifdef ARDUINO
			serial.write(c);
			#else
			serial.put(c);
			#endif
		}

		void get_addr_list_pack_handler(get_addr_list_pack& pack)
		{
			auto res = create_packet<addr_pack>();
			res.address_field.set_addr(address);
			res.send();
		}

		void addr_pack_handler(addr_pack& pack)
		{
			addr_field = pack.address_field;
			addr_list_recved = true;
		}

		void ping_pack_handler(ping_pack& pack)
		{
			auto res = create_packet<ping_echo_pack>();
			res.ttl = 6;
			res.source_addr = address;
			res.dest_addr = pack.source_addr;
			res.payload = pack.payload;
			res.send();
		}

		void ping_echo_pack_handler(ping_echo_pack& ping_echo)
		{
			ping_echo_payload = ping_echo.payload;
		}

		void datagram_pack_handler(datagram_pack& pack)
		{
			if((pack.dest_addr == address)
					|| (pack.dest_addr == BROADCAST_ADDR)
                    || (pack.dest_addr == MULTICAST_ADDR)) {
				for(auto& l : sockets) {
					if(l->port == pack.port) {
						l->remote = pack.source_addr;
						l->on_data(
								pack.source_addr,
								pack.payload.buffer(),
								pack.payload.size());
					}
				}
			}
		}

        // subscribe packets are only used by switches
        void subscribe_pack_handler(subscribe_pack& pack) {
            etk::unused(pack);
        }

#ifdef PICOLAN_NODE_BINDING
		Stream serial;
#elif ARDUINO
		Stream& serial;
#else
        Serial& serial;
#endif
		uint8_t address;
		uint16_t ping_echo_payload = 0;
		uint8_t addr_list_recved = false;

		AddressField addr_field;

		etk::List<Socket*, 16> sockets;
};


}


#endif
