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


#include "server.h"
#include "picolan.h"

namespace picolan
{

int Server::listen()
{
	if(state == CONNECTION_CLOSED) {
		state = CONNECTION_LISTENING;
		return ERROR_NONE;
	}
	return ERROR_BAD_STATE;
}

bool Server::connection_pending() 
{
	return (state == CONNECTION_SYN_RECVED);
}

int Server::accept()
{
	if(state == CONNECTION_SYN_RECVED) {
		send_ack();
		send_syn();

		//wait for ack reply
		state = CONNECTION_PENDING;
		auto start = millis();
		uint32_t dt = 0;
		do
		{
			iface->read();

			dt = millis() - start;
			if(dt > get_timeout()) {
				disconnect();
				return ERROR_TIMEOUT;
			}
		} while(state == CONNECTION_PENDING);

		if(state != CONNECTION_OPEN) {
			disconnect();
		} else {
			return ERROR_NONE;
		}
	}
	return ERROR_BAD_STATE;
}


void Server::on_data(uint8_t r, const uint8_t* data, uint32_t len)
{
	switch(state)
	{
		case CONNECTION_CLOSED:
			return;
		case CONNECTION_LISTENING:
			{
				// expecting to receive a syn packet
				if(data[0] == MESSAGE_TYPE::SYN) {
					remote = r;
					remote_sequence = data[1];
					remote_port = data[2];	
					state = CONNECTION_SYN_RECVED;
				}
			}
			break;
		case CONNECTION_SYN_RECVED:
			{
				if(len != 2) {
					return;
				}
				if(remote != r) {
					return;
				}
				if(data[0] == MESSAGE_TYPE::CLOSE) {
					state = CONNECTION_CLOSED;
				}
			}
			break;
		case CONNECTION_PENDING:
			{
				if(remote != r) {
					return;
				}
				if(data[0] == MESSAGE_TYPE::ACK) {
					state = CONNECTION_OPEN;
				}
				if(data[0] == MESSAGE_TYPE::CLOSE) {
					state = CONNECTION_CLOSED;
				}
			}
			break;
		case CONNECTION_OPEN:
			{
				if(remote != r) {
					return;
				}
				if(data[0] == MESSAGE_TYPE::CLOSE) {
					state = CONNECTION_CLOSED;
				}
				if(data[0] == MESSAGE_TYPE::ACK) {
					last_recved_ack = data[1];
				}
				if(data[0] == MESSAGE_TYPE::DATA) {
					uint8_t next_sequence = remote_sequence+1;
					if(data[1] == next_sequence) {
						remote_sequence = next_sequence;
						for(uint32_t i = 2; i < len; i++) {
							ringbuf.put(data[i]);
						}

					} else {
					}
					send_ack();
				}
			}
			break;

	}
}

}

