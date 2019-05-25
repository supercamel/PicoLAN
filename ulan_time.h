#ifndef PICOLAN_TIME_H
#define PICOLAN_TIME_H

#ifndef ARDUINO

#include <chrono>
#include <thread>
using namespace std::chrono;

inline auto millis() {
	milliseconds ms = duration_cast< milliseconds >(
			system_clock::now().time_since_epoch()
			);
	return ms.count();
}

inline void delay(uint32_t ms) {
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

#endif

#endif

