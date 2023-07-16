
#ifndef codec_h
#define codec_h

#include <stdint.h>

struct CodecState
{
	int valprev;
	int index;
};

void encode(CodecState* state, int16_t* input, uint32_t numSamples, uint8_t* output);
void decode(CodecState* state, uint8_t* input, int numSamples, int16_t* output);

#endif
