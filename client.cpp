#include "client.h"
#include "picolan.h"

namespace picolan
{

int Client::connect(uint8_t r, uint8_t port)
{
	if(state != CONNECTION_CLOSED) {
		return ERROR_BAD_STATE;
	}

	remote_port = port;
	remote = r;

	auto err = send_syn();
	if(err != ERROR_NONE) {
		return err;
	}

	state = CONNECTION_SYN_SENT;

	auto start = millis();
	uint32_t dt = 0;
	do
	{
		iface->read();
		dt = millis() - start;
		if(dt > timeout) {
			state = CONNECTION_CLOSED;
			return ERROR_TIMEOUT;
		}
	} while(state == CONNECTION_SYN_SENT);

	if(state != CONNECTION_SYN_RECVED) {
		disconnect();
		return ERROR_BAD_STATE;
	}

	SocketStream::send_ack();
	state = CONNECTION_OPEN;
	return ERROR_NONE;
}

void Client::on_data(uint8_t r, const uint8_t* data, uint32_t len)
{
	if(remote != r) {
		return;
	}

	switch(state)
	{
		case CONNECTION_CLOSED:
			return;
		case CONNECTION_SYN_SENT:
			{
				if(data[0] == MESSAGE_TYPE::SYN) {
					remote_sequence = data[1];
					state = CONNECTION_SYN_RECVED;
				} else if(data[0] == MESSAGE_TYPE::CLOSE) {
					state = CONNECTION_CLOSED;
				}
			}
			break;
		case CONNECTION_SYN_RECVED:
			{
				if(data[0] == MESSAGE_TYPE::CLOSE) {
					state = CONNECTION_CLOSED;
				}
			}
			break;
		case CONNECTION_OPEN:
			{
				if(data[0] == MESSAGE_TYPE::CLOSE) {
					state = CONNECTION_CLOSED;
				}
				if(data[0] == MESSAGE_TYPE::ACK) {
					last_recved_ack = data[1];
				}

				if(data[0] == MESSAGE_TYPE::DATA) {
					if(data[1] == (remote_sequence+1)) {
						remote_sequence++;
						for(uint32_t i = 2; i < len; i++) {
							ringbuf.put(data[i]);
						}
					}
					send_ack();
				}
			}
			break;
	}
}


}

