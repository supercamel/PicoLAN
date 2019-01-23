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

