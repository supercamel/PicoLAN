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

#include "socket_stream.h"
#include "picolan.h"
#include <math.h>

namespace picolan
{

struct seq_tuple {
    seq_tuple() {
        seq = -1;
        pos = 0;
    }

    seq_tuple(uint8_t s, uint32_t bp) {
        seq = s;
        pos = bp;
    }

    int seq;
    uint32_t pos;
};

#ifndef PICOLAN_NODE_BINDING
int SocketStream::write(uint8_t* bytes, uint32_t len)
{
#else
int SocketStream::write(std::vector<uint8_t> bytes)
{
    uint32_t len = bytes.size();
#endif

	if(state != CONNECTION_OPEN) {
		return Error::BAD_STATE;
	}

    if(len == 0) {
        return 0;
    }

	// 12 bytes for packet headers + checksum
	constexpr uint32_t BYTES_PER_FRAME = MAX_PACKET_LENGTH-12;

    // sends this many frames in a burst before checking acks
	constexpr uint32_t FRAME_BURST_SZ = 4;

    seq_tuple frame_byte_pos[FRAME_BURST_SZ];

    //position of next byte to send (counter for how many bytes are sent)
    uint32_t bytes_pos = 0;

    uint32_t no_ack_count = 0;

    do
    {
        // num packets to send is calculated from bytes remaining
        uint32_t packets_to_send = (len-bytes_pos)/BYTES_PER_FRAME;
        if((len-bytes_pos) % BYTES_PER_FRAME) {
            packets_to_send++;
        }

        // constrain packets to send to frame burst size
        if(packets_to_send > FRAME_BURST_SZ) {
            packets_to_send = FRAME_BURST_SZ;
        }

        uint32_t initial_byte_pos = bytes_pos;

        // clear tuple
        for(auto i : etk::range(FRAME_BURST_SZ)) {
            frame_byte_pos[i] = seq_tuple(-1, 0);
        }

        // sequence counter for this burst
        uint8_t burst_seq = 0;
        uint8_t final_seq;
        for(int i = 0; i < packets_to_send; i++) {
            frame_byte_pos[i].seq = final_seq = (uint8_t)(sequence_number + (++burst_seq));

            // create the packet
            auto pack = iface->create_packet<datagram_pack>();
            pack.ttl = 6;
            pack.dest_addr = remote;
            pack.source_addr = iface->get_address();
            pack.port = remote_port;
            pack.payload.append(MESSAGE_TYPE::DATA);
            pack.payload.append(frame_byte_pos[i].seq);

            //stuff bytes into the packet
            uint32_t bytes_to_send = min(BYTES_PER_FRAME, len-bytes_pos);
            for(uint32_t j = 0; j < bytes_to_send; j++) {
                uint8_t bb = bytes[bytes_pos++];
                pack.payload.append(bb);
                if(bytes_pos == len) {
                    break;
                }
            }
            iface->read();
            //send it off
            pack.send();
            frame_byte_pos[i].pos = bytes_pos;
        }

        auto start_time = millis();
        do {
            iface->read();

            if(last_recved_ack == final_seq) {
                break;
            }
        } while(millis() - start_time < timeout);

        if(last_recved_ack == sequence_number) {
            no_ack_count++;
            if(no_ack_count == 3) {
                return Error::TIMEOUT;
            }
            bytes_pos = initial_byte_pos;
        }
        else {
            bool found_pos = false;
            for(int i = 0; i < FRAME_BURST_SZ; i++) {
                if(last_recved_ack == frame_byte_pos[i].seq) {
                    found_pos = true;
                    bytes_pos = frame_byte_pos[i].pos;
                    sequence_number += i;
                }
            }
            if(found_pos == false) {
                return Error::ACK_OUT_OF_SEQUENCE;
            }
        }
    } while(bytes_pos != len);

	return bytes_pos;
}

#ifndef PICOLAN_NODE_BINDING
int SocketStream::read(uint8_t* buffer, uint32_t len)
{
	if(state != CONNECTION_OPEN) {
		return Error::BAD_STATE;
	}
	uint32_t ret = Socket::read(buffer, len);
    if(ret == 0) {
#else
std::vector<uint8_t> SocketStream::read(uint32_t len)
{
	if(state != CONNECTION_OPEN) {
		return std::vector<uint8_t>();
	}
	std::vector<uint8_t> ret = Socket::read(len);
    if(ret.size() == 0) {
#endif
		zero_read_count++;
		if(zero_read_count >= 3) {
			disconnect();
			zero_read_count = 0;
		}
	}
	return ret;
}

bool SocketStream::closed()
{
	return (state == CONNECTION_CLOSED);
}

bool SocketStream::connected()
{
	return (state == CONNECTION_OPEN);
}

void SocketStream::disconnect()
{
	if(state == CONNECTION_CLOSED) {
		return;
	}

	state = CONNECTION_CLOSED;
	if(state == CONNECTION_LISTENING) {
		return;
	}

	auto pack = iface->create_packet<datagram_pack>();
	pack.ttl = 6;
	pack.dest_addr = remote;
	pack.source_addr = iface->get_address();
	pack.port = remote_port;
	pack.payload.append(MESSAGE_TYPE::CLOSE);
	pack.payload.append(sequence_number);
	pack.send();
}

int SocketStream::send_syn()
{
	auto pack = iface->create_packet<datagram_pack>();
	pack.ttl = 6;
	pack.dest_addr = remote;
	pack.source_addr = iface->get_address();
	pack.port = remote_port;
	pack.payload.append(MESSAGE_TYPE::SYN);
	pack.payload.append(sequence_number++);
	pack.payload.append(get_port());
	pack.send();

	return Error::NONE;
}


int SocketStream::send_ack()
{
	auto pack = iface->create_packet<datagram_pack>();
	pack.ttl = 6;
	pack.dest_addr = remote;
	pack.source_addr = iface->get_address();
	pack.port = remote_port;
	pack.payload.append(MESSAGE_TYPE::ACK);
	pack.payload.append(remote_sequence);
	pack.send();

	return Error::NONE;
}


}
