#include "socket.h"
#include "picolan.h"

namespace picolan
{

Socket::~Socket() {
	if(iface != nullptr) {
		iface->unbind_socket(this);
	}
}

int Socket::timedRead() {
	int c;
	uint32_t dt = 0;
	auto startMillis = millis();
	do {
		if(ringbuf.available() > 0) {
			return ringbuf.get();
		}
		iface->read();
		dt = millis() - startMillis;
	} while(dt < timeout);
	return ERROR_TIMEOUT;
}

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

void Socket::destroy() {
	iface->unbind_socket(this);
}

}

