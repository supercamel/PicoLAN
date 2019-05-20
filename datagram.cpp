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


#include "datagram.h"
#include "picolan.h"

namespace picolan
{

#ifndef PICOLAN_NODE_BINDING
int Datagram::write(uint8_t dest, uint8_t dest_port,  uint8_t* data, uint32_t len) {

#else
int Datagram::write(uint8_t dest, uint8_t dest_port,  std::vector<uint8_t> data) {
	uint32_t len = data.size();

#endif

	const uint32_t CHUNK_SZ = 58;
	uint32_t chunks = len/CHUNK_SZ;

	for(uint32_t i = 0; i < chunks; i++)
	{
		auto pack = iface->create_packet<datagram_pack>();
		pack.ttl = 6;
		pack.dest_addr = dest;
		pack.source_addr = iface->get_address();
		pack.port = dest_port;

		for(uint32_t j = 0; j < CHUNK_SZ; j++)
		{
			pack.payload.append(data[i*CHUNK_SZ + j]);
		}
		pack.send();
	}

	uint32_t remainder = len%CHUNK_SZ;
	if(remainder != 0) {
		auto pack = iface->create_packet<datagram_pack>();
		pack.ttl = 6;
		pack.dest_addr = dest;
		pack.source_addr = iface->get_address();
		pack.port = dest_port;
		for(uint32_t i = 0; i < remainder; i++)
		{
			pack.payload.append(data[chunks*CHUNK_SZ + i]);
		}
		pack.send();
	}

    iface->flush();
	return Error::NONE;
}

#ifndef PICOLAN_NODE_BINDING
int Datagram::write(uint8_t dest, uint8_t dest_port, const char* data)
{
    uint32_t len = etk::Rope::c_strlen(data, 1024);
    return write(dest, dest_port, (uint8_t*)data, len);
}
#endif

void Datagram::subscribe(bool sub) {
    auto pack = iface->create_packet<subscribe_pack>();
    pack.ttl = 6;
    pack.addr = iface->get_address();
    pack.port = get_port();
    pack.subscribe = (int)sub;
    pack.send();
    iface->flush();
}

void Datagram::on_data(uint8_t r, const uint8_t* data, uint32_t len)
{
	remote = r;
	uint32_t i = 0;
	while(i != len)
	{
		ringbuf.put(data[i++]);
	}
}

}
