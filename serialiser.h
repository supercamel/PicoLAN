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

#ifndef PICOLAN_SERIALISER_H
#define PICOLAN_SERIALISER_H

#include <stdint.h>

#ifdef ARDUINO
#include <etk.h>
#else
#include <etk/etk.h>
#endif

#include "address_field.h"


namespace picolan
{

	/**
	 * The maximum packet length including header and checksum bytes.
	 */
	constexpr uint16 MAX_PACKET_LENGTH = 64;

	/**
	 * Packet types.
	 * This does not include SocketStream packet types (which are constructed from datagram packets)
	 */
	enum PACKETS : uint8
	{
		INVALID_PACK,
		GET_ADDR_LIST_PACK,
		ADDR_PACK,
		PING_PACK,
		PING_ECHO_PACK,
		DATAGRAM_PACK,
		SUBSCRIBE_PACK,
		NULL_PACK
	};

	union u64b
	{
		uint64 u;
		int64 i;
		uint8 bytes[8];
	};

	union u32b
	{
		uint32 u;
		int32 i;
		float f;
		uint8 bytes[4];
	};

	union u16b
	{
		uint16 u;
		int16 i;
		uint8 bytes[2];
	};



	/**
	 * SerialiserInterface defines several functions that the serialiser must implement.
	 * The packet classes call these functions during serialisation.
	 */
	class SerialiserInterface
	{
		public:
            virtual ~SerialiserInterface() { }
			virtual void start() = 0;
			virtual uint16 finish_checksum() = 0;
			virtual void send_byte(uint8 b) = 0;
			virtual void finish() = 0;
	};

	/**
	 * Base packet class
	 */
	class base_pack
	{
		public:
            virtual ~base_pack() { } 
			base_pack(SerialiserInterface* serialiser) : serialiser(serialiser) { }

			virtual uint8 get_id() = 0;
			virtual void send() = 0;
			virtual void from_bytes(uint8* bytes) = 0;
			virtual uint8 size() = 0;

			void set_serialiser(SerialiserInterface* s) {
				serialiser = s;
			}

			void gen_header()
			{
				serialiser->start();
				put(get_id());
				put(size());
			}

			/*{{{*/
			template <uint32 SZ> void to_bytes(etk::StaticString<SZ>& str)
			{
				for(uint32 i = 0; i < SZ; i++)
				{
					put(str[i]);
				}
			}

			template <uint16 SZ> void to_bytes(etk::List<uint8, SZ>& lst)
			{
				uint8 sz = lst.size();
				put(sz);

				for(auto i : etk::range(sz))
				{
					put(lst.get(i));
				}
			}

			void to_bytes(float f)
			{
				u32b ub;
				ub.f = f;
				for(uint8 i = 0; i < 4; i++) {
					put(ub.bytes[i]);
				}
			}

			void to_bytes(uint8 u)
			{
				put(u);
			}

			void to_bytes(int8 i)
			{
				put(i);
			}

			void to_bytes(uint16 u)
			{
				u16b ub;
				ub.u = u;
				put(ub.bytes[0]);
				put(ub.bytes[1]);
			}

			void to_bytes(int16 i)
			{
				u16b ub;
				ub.i = i;
				put(ub.bytes[0]);
				put(ub.bytes[1]);
			}

			void to_bytes(uint32 u)
			{
				u32b ub;
				ub.u = u;
				put(ub.bytes[0]);
				put(ub.bytes[1]);
				put(ub.bytes[2]);
				put(ub.bytes[3]);
			}

			void to_bytes(int32 i)
			{
				u32b ub;
				ub.i = i;
				put(ub.bytes[0]);
				put(ub.bytes[1]);
				put(ub.bytes[2]);
				put(ub.bytes[3]);
			}

			void to_bytes(uint64 u)
			{
				u64b ub;
				ub.u = u;
				put(ub.bytes[0]);
				put(ub.bytes[1]);
				put(ub.bytes[2]);
				put(ub.bytes[3]);
				put(ub.bytes[4]);
				put(ub.bytes[5]);
				put(ub.bytes[6]);
				put(ub.bytes[7]);
			}

			void to_bytes(int64 i)
			{
				u64b ub;
				ub.i = i;
				put(ub.bytes[0]);
				put(ub.bytes[1]);
				put(ub.bytes[2]);
				put(ub.bytes[3]);
				put(ub.bytes[4]);
				put(ub.bytes[5]);
				put(ub.bytes[6]);
				put(ub.bytes[7]);
			}

			void to_bytes(AddressField af)
			{
				auto len = af.get_num_bytes();
				for(int i = 0; i < len; i++) {
					put(af.get_bitfield(i).get());
				}
			}
			/*}}}*/
			template <uint32 SZ> void from_bytes(uint8* bytes, uint16& pos, etk::StaticString<SZ>& str)/*{{{*/
			{
				for(uint32 i = 0; i < SZ; i++)
				{
					str[i] = bytes[pos++];
				}
			}

			template <uint16 SZ> void from_bytes(uint8* bytes, uint16& pos, etk::List<uint8, SZ>& lst)
			{
				uint8 sz = bytes[pos++];

				for(uint32 i = 0; i < sz; i++)
				{
					lst.append(bytes[pos++]);
				}
			}

			void from_bytes(uint8* bytes, uint16& pos, float& f)
			{
				u32b ub;
				ub.bytes[0] = bytes[pos++];
				ub.bytes[1] = bytes[pos++];
				ub.bytes[2] = bytes[pos++];
				ub.bytes[3] = bytes[pos++];

				f = ub.f;
			}

			void from_bytes(uint8* bytes, uint16& pos, uint8& u)
			{
				u = bytes[pos++];
			}

			void from_bytes(uint8* bytes, uint16& pos, int8& i)
			{
				i = bytes[pos++];
			}

			void from_bytes(uint8* bytes, uint16& pos, uint16& u)
			{
				u16b ub;
				ub.bytes[0] = bytes[pos++];
				ub.bytes[1] = bytes[pos++];
				u = ub.u;
			}

			void from_bytes(uint8* bytes, uint16& pos, int16& i)
			{
				u16b ub;
				ub.bytes[0] = bytes[pos++];
				ub.bytes[1] = bytes[pos++];
				i = ub.i;
			}

			void from_bytes(uint8* bytes, uint16& pos, uint32& u)
			{
				u32b ub;
				ub.bytes[0] = bytes[pos++];
				ub.bytes[1] = bytes[pos++];
				ub.bytes[2] = bytes[pos++];
				ub.bytes[3] = bytes[pos++];
				u = ub.u;
			}

			void from_bytes(uint8* bytes, uint16& pos, int32& i)
			{
				u32b ub;
				ub.bytes[0] = bytes[pos++];
				ub.bytes[1] = bytes[pos++];
				ub.bytes[2] = bytes[pos++];
				ub.bytes[3] = bytes[pos++];
				i = ub.i;
			}

			void from_bytes(uint8* bytes, uint16& pos, uint64& u)
			{
				u64b ub;
				for(auto i : etk::range(8)) {
					ub.bytes[i] = bytes[pos++];
				}
				u = ub.u;
			}

			void from_bytes(uint8* bytes, uint16& pos, int64& i)
			{
				u64b ub;
				for(auto i : etk::range(8)) {
					ub.bytes[i] = bytes[pos++];
				}
				i = ub.i;
			}

			void from_bytes(uint8* bytes, uint16& pos, picolan::AddressField& af)
			{
				auto len = af.get_num_bytes();
				for(int i = 0; i < len; i++) {
					af.get_bitfield(i).set(bytes[pos++]);
				}
			}
			/*}}}*/
			void finish()
			{
				u16b u;	
				u.u = serialiser->finish_checksum();
				put(u.bytes[0]);
				put(u.bytes[1]);
				serialiser->finish();
			}

		protected:
			void put(uint8 b)
			{

				serialiser->send_byte(b);
			}

			SerialiserInterface* serialiser;
	};

	/**
	 * get address list packet
	 * A request for the list of known IP addresses.
	 * Typically sent from a device to the network switch.
	 * Used by switches to build routing tables.
	 */
	class get_addr_list_pack : public base_pack/*{{{*/
	{
		public:
			get_addr_list_pack(SerialiserInterface* t) : base_pack(t) { }
			static constexpr uint16 id = GET_ADDR_LIST_PACK;

			uint8 ttl;
			uint8 size()
			{
				return (sizeof(ttl));
			}
			uint8 get_id() { return id; }
			void send()
			{
				base_pack::gen_header();
				base_pack::to_bytes(ttl);
				base_pack::finish();
			}
			void from_bytes(uint8* bytes)
			{
				uint16 pos = 0;
				base_pack::from_bytes(bytes, pos, ttl);
			}
	};/*}}}*/

	/**
	 * addr_pack contains a bitfield of addresses
	 * Used by switches to build routing tables.
	 */
	class addr_pack : public base_pack/*{{{*/
	{
		public:
			addr_pack(SerialiserInterface* t) : base_pack(t) { }
			static constexpr uint16 id = ADDR_PACK;

			AddressField address_field;

			uint8 size()
			{
				return (sizeof(address_field));
			}
			uint8 get_id() { return id; }
			void send()
			{
				base_pack::gen_header();
				base_pack::to_bytes(address_field);
				base_pack::finish();
			}
			void from_bytes(uint8* bytes)
			{
				uint16 pos = 0;
				base_pack::from_bytes(bytes, pos, address_field);
			}
	};/*}}}*/

	/**
	 * ping_pack sends a ping from one device to another.
	 * The switches route this packet to all available interfaces that have the destination address.
	 */
	class ping_pack : public base_pack/*{{{*/
	{
		public:
			ping_pack(SerialiserInterface* t) : base_pack(t) { }
			static constexpr uint16 id = PING_PACK;

			uint8 ttl;
			uint8 source_addr;
			uint8 dest_addr;
			uint16 payload;
			uint8 size()
			{
				return (sizeof(ttl) +
						sizeof(source_addr) +
						sizeof(dest_addr) +
						sizeof(payload));
			}
			uint8 get_id() { return id; }
			void send()
			{
				base_pack::gen_header();
				base_pack::to_bytes(ttl);
				base_pack::to_bytes(source_addr);
				base_pack::to_bytes(dest_addr);
				base_pack::to_bytes(payload);
				base_pack::finish();
			}
			void from_bytes(uint8* bytes)
			{
				uint16 pos = 0;
				base_pack::from_bytes(bytes, pos, ttl);
				base_pack::from_bytes(bytes, pos, source_addr);
				base_pack::from_bytes(bytes, pos, dest_addr);
				base_pack::from_bytes(bytes, pos, payload);
			}
	};/*}}}*/

	/**
	 * ping_echo_pack is sent by a device in response to a ping_pack
	 * The switches will route this packet to the dest_addr
	 */
	class ping_echo_pack : public base_pack/*{{{*/
	{
		public:
			ping_echo_pack(SerialiserInterface* t) : base_pack(t) { }
			static constexpr uint16 id = PING_ECHO_PACK;

			uint8 ttl;
			uint8 source_addr;
			uint8 dest_addr;
			uint16 payload;
			uint8 size()
			{
				return (sizeof(ttl) +
						sizeof(source_addr) +
						sizeof(dest_addr) +
						sizeof(payload));
			}
			uint8 get_id() { return id; }
			void send()
			{
				base_pack::gen_header();
				base_pack::to_bytes(ttl);
				base_pack::to_bytes(source_addr);
				base_pack::to_bytes(dest_addr);
				base_pack::to_bytes(payload);
				base_pack::finish();
			}
			void from_bytes(uint8* bytes)
			{
				uint16 pos = 0;
				base_pack::from_bytes(bytes, pos, ttl);
				base_pack::from_bytes(bytes, pos, source_addr);
				base_pack::from_bytes(bytes, pos, dest_addr);
				base_pack::from_bytes(bytes, pos, payload);
			}
	};/*}}}*/

	/**
	 * datagram_pack is a packet containing data up to 54 bytes (10 bytes for header/checksum)
	 */
	class datagram_pack : public base_pack/*{{{*/
	{
		public:
			datagram_pack(SerialiserInterface* t) : base_pack(t) { }
			static constexpr uint16 id = DATAGRAM_PACK;

			uint8 ttl;
			uint8 source_addr;
			uint8 dest_addr;
			uint8 port;
			etk::List<uint8, 54> payload;
			uint8 size()
			{
				return (sizeof(ttl) +
						sizeof(source_addr) +
						sizeof(dest_addr) +
						sizeof(port) +
						payload.size()+1);
			}
			uint8 get_id() { return id; }
			void send()
			{
				base_pack::gen_header();
				base_pack::to_bytes(ttl);
				base_pack::to_bytes(source_addr);
				base_pack::to_bytes(dest_addr);
				base_pack::to_bytes(port);
				base_pack::to_bytes(payload);
				base_pack::finish();
			}
			void from_bytes(uint8* bytes)
			{
				uint16 pos = 0;
				base_pack::from_bytes(bytes, pos, ttl);
				base_pack::from_bytes(bytes, pos, source_addr);
				base_pack::from_bytes(bytes, pos, dest_addr);
				base_pack::from_bytes(bytes, pos, port);
				base_pack::from_bytes(bytes, pos, payload);
			}
	};/*}}}*/

	class subscribe_pack : public base_pack/*{{{*/
	{
		public:
			subscribe_pack(SerialiserInterface* t) : base_pack(t) { }
			static constexpr uint16 id = SUBSCRIBE_PACK;

			uint8 ttl = 6;
			uint8 addr = 0;
			uint8 port = 0;
			uint8 subscribe = 1;

			uint8 size()
			{
				return (sizeof(ttl) +
						sizeof(port) +
						sizeof(addr) +
						sizeof(subscribe));
			}
			uint8 get_id() { return id; }
			void send()
			{
				base_pack::gen_header();
				base_pack::to_bytes(ttl);
				base_pack::to_bytes(port);
				base_pack::to_bytes(addr);
				base_pack::to_bytes(subscribe);
				base_pack::finish();
			}
			void from_bytes(uint8* bytes)
			{
				uint16 pos = 0;
				base_pack::from_bytes(bytes, pos, ttl);
				base_pack::from_bytes(bytes, pos, port);
				base_pack::from_bytes(bytes, pos, addr);
				base_pack::from_bytes(bytes, pos, subscribe);
			}
	};/*}}}*/


	/**
	 * ParserSerialiser is responsible for converting packets into a series of bytes
	 * and for parsing bytes into a packet structure.
	 */
	class ParserSerialiser
		: public SerialiserInterface
	{
		public:
			ParserSerialiser() {}/*{{{*/

			/**
			 * \brief sends a byte to the output stream
			 */
			virtual void put(uint8 b) = 0;

			/**
			 * \brief this function is called when a get_addr_list_pack is received
			 */
			virtual void get_addr_list_pack_handler(get_addr_list_pack& p) = 0;

			/**
			 * \brief this function is called when a addr_list_pack is received
			 */
			virtual void addr_pack_handler(addr_pack& p) = 0;

			/**
			 * \brief this function is called when a ping_pack is received
			 */
			virtual void ping_pack_handler(ping_pack& p) = 0;

			/**
			 * \brief this function is called when a ping_echo_pack is received
			 */
			virtual void ping_echo_pack_handler(ping_echo_pack& p) = 0;

			/**
			 * \brief this function is called when a datagram_pack is received.
			 */
			virtual void datagram_pack_handler(datagram_pack& p) = 0;

			/**
			 * \brief this function is called when a subscribe_pack is received.
			 */
			virtual void subscribe_pack_handler(subscribe_pack& p) = 0;
			/*}}}*/
			/**
			 * \brief reads all available bytes from the input stream.
			 * Input bytes are parsed using a state machine and when a valid packet is read
			 * the appropriate handler function is called.
			 */
			void read(uint8_t c)/*{{{*/
			{
				if((c == 0xAA) && (stuff_flag == false)) {
					stuff_flag = true;
				}
				else {
					if(stuff_flag == false) {
						if((c == 0xAB) || (c == 0xAC)) {
							state = MSG_STATE_START;
						}
					}
					stuff_flag = false;

					switch(state)
					{
						case MSG_STATE_START:
							{
								data_pos = 0;
								if(c == 0xAB)
									state = MSG_STATE_ID;
							}
							break;
						case MSG_STATE_ID:
							{
								msg_id = c;
								if(msg_id >= NULL_PACK)
									state = MSG_STATE_START;
								else
								{
									add_byte(c);
									state = MSG_STATE_SIZE;
								}
							}
							break;
						case MSG_STATE_SIZE:
							{
								add_byte(c);
								data_length = c;
								if(data_length == 0) {
									state = MSG_STATE_CHECK_1;
								}
								else {
									state = MSG_STATE_DATA;
								}
							}
							break;
						case MSG_STATE_DATA:
							{
								add_byte(c);
								if(data_pos >= (data_length+2))
									state = MSG_STATE_CHECK_1;
							}
							break;
						case MSG_STATE_CHECK_1:
							{
								checksum_in = c;
								state = MSG_STATE_CHECK_2;
							}
							break;
						case MSG_STATE_CHECK_2:
							{
								checksum_in += (c << 8);

								if(check_checksum())
									read_data();
								state = MSG_STATE_START;
							}
							break;
						default:
							state = MSG_STATE_START;
					}
				}
			}
			/*}}}*/

			/**
			 * Creates a packet that can be sent using this serialiser.
			 */
			template <typename PACK_T> PACK_T create_packet()
			{
				PACK_T pack(this);
				return pack;
			}

			virtual void flush() = 0;


		private:
			friend class base_pack;
			void add_byte(uint8 c)
			{
				data_buf[data_pos++] = c;
			}

			void start() {
				reset_checksum();
				this->put(0xAB);
			}

			void finish() {
				this->put(0xAC);
				this->flush();
			}


			void reset_checksum() {
				sum1 = 0;
				sum2 = 0;
			}

			void step_checksum(uint8 data) {
				sum1 = (sum1 + data) %255;
				sum2 = (sum2 + sum1) %255;
			}

			uint16 finish_checksum() {
				uint16 checksum = (sum2 << 8) | sum1;
				return checksum;
			}

			bool check_checksum()
			{
				uint32 len = data_length+2;
				uint16 s1 = 0;
				uint16 s2 = 0;
				for(auto i : etk::range(len)) {
					s1 = (s1 + (uint8)data_buf[i]) % 255;
					s2 = (s2 + s1) %255;
				}
				uint16 cs = (s2 << 8) | s1;
				return (cs == checksum_in);
			}

			void send_byte(uint8 b) {
				if((b >= 0xAA) && (b <= 0xAC)) {
					this->put(0xAA);
				}
				this->put(b);
				step_checksum(b);
			}

			void read_data()
			{
				switch(msg_id)
				{
					case GET_ADDR_LIST_PACK:
						{
							auto pack = create_packet<get_addr_list_pack>();
							pack.from_bytes(&(data_buf[2]));
							get_addr_list_pack_handler(pack);
						}
						break;
					case ADDR_PACK:
						{
							auto pack = create_packet<addr_pack>();
							pack.from_bytes(&(data_buf[2]));
							addr_pack_handler(pack);
						}
						break;
					case PING_PACK:
						{
							auto pack = create_packet<ping_pack>();
							pack.from_bytes(&(data_buf[2]));
							ping_pack_handler(pack);
						}
						break;
					case PING_ECHO_PACK:
						{
							auto pack = create_packet<ping_echo_pack>();
							pack.from_bytes(&(data_buf[2]));
							ping_echo_pack_handler(pack);
						}
						break;
					case DATAGRAM_PACK:
						{
							auto pack = create_packet<datagram_pack>();
							pack.from_bytes(&(data_buf[2]));
							datagram_pack_handler(pack);
						}
						break;
					case SUBSCRIBE_PACK:
						{
							auto pack = create_packet<subscribe_pack>();
							pack.from_bytes(&(data_buf[2]));
							subscribe_pack_handler(pack);
						}
						break;
				}
			}


			enum MSG_STATE
			{
				MSG_STATE_START,
				MSG_STATE_ID,
				MSG_STATE_SIZE,
				MSG_STATE_DATA,
				MSG_STATE_CHECK_1,
				MSG_STATE_CHECK_2
			};

			MSG_STATE state = MSG_STATE_START;
			bool stuff_flag = false;
			uint16 sum1;
			uint16 sum2;
			uint16 msg_id = 0;
			uint16 checksum_in = 0;
			uint8 data_length = 0;
			uint8 data_pos = 0;

			uint8 data_buf[MAX_PACKET_LENGTH];
	};


}


#endif
