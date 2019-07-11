
#ifndef ADDRESS_FIELD_H
#define ADDRESS_FIELD_H

#ifdef ARDUINO
#include <etk.h>
#else
#include <etk/etk.h>
#endif


namespace picolan
{

class AddressField
{
public:
	AddressField() {
		for(uint32_t i = 0; i < NUM_BYTES; i++) {
			bits[i].set(0);
		}
	}

	void set_addr(uint8_t addr, bool set = true) {
		int bf = addr/8;
		int bit = addr%8;

		bits[bf].set_bit(bit, set);
	}

	bool get_addr(uint8_t addr) const {
		int bf = addr/8;
		int bit = addr%8;

		return bits[bf].read_bit(bit);
	}

	etk::Bits<uint8>& get_bitfield(uint8_t a) {
		a = a % NUM_BYTES;
		return bits[a];
	}

	uint8_t get_num_bytes() const {
		return NUM_BYTES;
	}

private:
	static constexpr uint32_t NUM_BYTES = 32;
	etk::Bits<uint8> bits[NUM_BYTES];

};



}

#endif
