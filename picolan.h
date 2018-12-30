#ifndef CBUS_H
#define CBUS_H

#include "serialiser.h"
#include "socket.h"
#include "datagram.h"
#include "server.h"
#include "client.h"
#include "usart_driver.h"
#include "time.h"


namespace picolan
{
const uint8_t BROADCAST_ADDR = 0xFF;

class Interface 
	: public ParserSerialiser 
{
	public:
		Interface(Stream& serial) : serial(serial) { }

		void set_address(uint8_t addr)
		{
			address = addr;
			auto pack = create_packet<addr_list_pack>();
			pack.list.append(addr);
			pack.send();
		}

		uint8_t get_address()
		{
			return address;
		}

		int get_addr_list(uint32_t timeout_ms, etk::List<uint8_t, 32>& list)
		{
			auto pack = create_packet<get_addr_list_pack>();
			pack.ttl = 6;
			pack.send();
			addr_list_recved = false;
			auto start = millis();
			while((millis()-start) < timeout_ms) {
				read();
				if(addr_list_recved) {
					list = addr_list;
					return ERROR_NONE;
				}
			}
			return ERROR_TIMEOUT;
		}

		bool bind(Socket* socket)
		{
			for(auto& s : sockets) {
				if(s->get_port() == socket->get_port()) {
					return false;
				}	
			}

			socket->iface = this;
			sockets.append(socket);
			return true;
		}

		void unbind_socket(Socket* s)
		{
			for(uint32_t i = 0; i < sockets.size(); i++) {
				if(sockets[i]->get_port() == s->get_port()) {
					sockets.remove(i);
				}
			}
		}

		/*
		*/ 

		int ping(uint8_t dest, uint32_t timeout_ms, uint32_t& trip_time) 
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
					trip_time = millis()-start;
					return ERROR_NONE;
				}
			}
			return ERROR_TIMEOUT;
		}

		void flush() {
			serial.flush();
		}

	private:
		friend class ParserSerialiser;

		bool available() {
			return serial.available() != 0;
		}

		uint8 get() {
			uint8 r = serial.read();
			return r;
		}

		void put(uint8 c) {
			serial.put(c);
		}

		void get_addr_list_pack_handler(get_addr_list_pack& pack)
		{
			auto res = create_packet<addr_list_pack>();
			res.list.append(address);
			res.send();
		}

		void addr_list_pack_handler(addr_list_pack& pack)
		{
			addr_list = pack.list;
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
					|| (pack.dest_addr == BROADCAST_ADDR)) {
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

		Stream& serial;
		uint8_t address;
		uint16_t ping_echo_payload = 0;
		uint8_t addr_list_recved = false;

		etk::List<uint8_t, 32> addr_list;
		etk::List<Socket*, 16> sockets;	
};


}


#endif


