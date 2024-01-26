#include <vector>

#pragma once

template <unsigned SZ> class Echo {
	public:
	Echo(uint16_t _length = SZ) {
		if(_length > SZ) length = SZ;
		else length = _length;
	}

	int16_t getSample(int16_t insample){
		int16_t s = buffer[pos];
		int32_t news = (mix * (int32_t)insample + feedback * (int32_t)s) >> 16;
		buffer[pos] = news > 32767 ? 32767 : news < -32767 ? -32767 : news;
		pos++;
		if(pos >= length) pos = 0;
		return s;
	}

	void config(uint16_t _feedback, uint16_t _mix, uint16_t _length = SZ) {
		feedback = _feedback;
		mix = _mix;
		if(_length > SZ) length = SZ;
		else length = _length;
	}

	uint16_t length;
	int16_t buffer[SZ];
	uint16_t pos;
	uint16_t feedback;
	uint16_t mix;
};
