#include "datagram.h"
#include "picolan.h"

namespace picolan
{

int Datagram::write(uint8_t dest, uint8_t dest_port,  uint8_t* data, uint32_t len) {
	const uint32_t CHUNK_SZ = 220;
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

	return ERROR_NONE;
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

