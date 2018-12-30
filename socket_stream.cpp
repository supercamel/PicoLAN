#include "socket_stream.h"
#include "picolan.h"
#include <math.h>

namespace picolan
{

int SocketStream::write(uint8_t* bytes, uint32_t len)
{
	constexpr uint32_t FRAME_BURST_SZ = 4;
	uint32_t bytes_sent = 0;
	uint32_t frames_sent = 0;
	uint32_t dead_count = 0;
	while(bytes_sent != len) {
		uint8_t start_recved_ack = last_recved_ack;
		uint32_t byte_sent_start = bytes_sent;
		uint8_t burst_sz = 0;
		while((burst_sz < FRAME_BURST_SZ) && (bytes_sent < len)) {
			auto pack = iface->create_packet<datagram_pack>();
			pack.ttl = 6;
			pack.dest_addr = remote;
			pack.source_addr = iface->get_address();
			pack.port = remote_port;
			pack.payload.append(MESSAGE_TYPE::DATA);
			uint8_t next_seq = sequence_number+burst_sz;
			pack.payload.append(next_seq);

#ifdef ETK_MIN
			uint32_t bytes_to_send = etk::min<uint32_t>(218, len-bytes_sent);
#else
			uint32_t bytes_to_send = min(218, len-bytes_sent);
#endif
			for(uint32_t j = 0; j < bytes_to_send; j++) {
				uint8_t bb = bytes[bytes_sent++];
				pack.payload.append(bb);
				if(bytes_sent == len) {
					break;
				}
			}
			iface->read();
			pack.send();
			burst_sz++;
		}

		uint8_t last_sequence = sequence_number+burst_sz;

		auto ack_start = millis();
		do {
			iface->read();
			uint32_t ack_dt = millis()-ack_start;
			if(ack_dt > timeout) {
				break;
			}
		} while(last_recved_ack != last_sequence);


		if(last_recved_ack == sequence_number) {
			dead_count++;
			if(dead_count > 3) {
				return ERROR_TIMEOUT;
			}
		} else {
			dead_count = 0;
		}

		uint8_t distance = last_recved_ack-sequence_number;
		if(distance > FRAME_BURST_SZ) {
			return ERROR_ACK_OUT_OF_SEQUENCE;
		}
		frames_sent += distance;
		sequence_number += distance;
		bytes_sent = (frames_sent*218);
	}
	return bytes_sent;
}

uint32_t SocketStream::read(uint8_t* buffer, uint32_t len)
{
	uint32_t ret = Socket::read(buffer, len);
	if(ret == 0) {
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

	return ERROR_NONE;
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

	return ERROR_NONE;
}


}

