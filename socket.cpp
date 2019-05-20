#include "socket.h"
#include "picolan.h"

namespace picolan
{

Socket::~Socket() {
	if(iface != nullptr) {
		iface->unbind_socket(*this);
	}
}

uint32_t Socket::available() {
	return ringbuf.available();
}

int Socket::timedRead() {
	uint32_t dt = 0;
	auto startMillis = millis();
	do {
		if(ringbuf.available() > 0) {
			return ringbuf.get();
		}
		iface->read();
		dt = millis() - startMillis;
	} while(dt < timeout);
	return Error::TIMEOUT;
}

#ifndef PICOLAN_NODE_BINDING
uint32_t Socket::read(uint8_t* buffer, uint32_t len) {
	uint32_t count = 0;
	while(count < len) {
		auto c = timedRead();
		if(c < 0) {
			break;
		}
		buffer[count++] = c;
	}
	return count;
}
#else
std::vector<uint8_t> Socket::read(uint32_t len) {
	std::vector<uint8_t> ret;
	while(ret.size() < len) {
		int c = timedRead();
		if(c == Error::TIMEOUT) {
			break;
		}
		ret.push_back(c);
	}
	return ret;
}
#endif

void Socket::destroy() {
	if(iface != nullptr) {
		iface->unbind_socket(*this);
	}
}

}
