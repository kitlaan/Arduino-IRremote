#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//                                  MAGIQUEST
//==============================================================================

// MagiQuest wand
// It looks like the wands send 56-bits of information, fitting within a ~1150usec
// timeslot (i.e. similar to PWM).  A "0" is usually ~25% signal, whereas a "1"
// is usually ~55% signal.

#define MAGIQUEST_BITS               56

#define MAGIQUEST_TOTAL_USEC       1150
#define MAGIQUEST_ZERO_RATIO         30  // usually <= ~25%
#define MAGIQUEST_ONE_RATIO          38  // usually >= ~50%

#define IS_ZERO(m, s) (((m) * 100 / ((m) + (s))) <= MAGIQUEST_ZERO_RATIO)
#define IS_ONE(m, s)  (((m) * 100 / ((m) + (s))) >= MAGIQUEST_ONE_RATIO)

//+=============================================================================
#if DECODE_MAGIQUEST
bool  IRrecv::decodeMagiquest (decode_results *results)
{
	int bits = 0;
	uint64_t data = 0;
	int offset = 1;  // Skip first SPACE

	if (irparams.rawlen < (2 * MAGIQUEST_BITS))  return false ;

	// Of six wands as datapoints, so far they all start with 8 ZEROs.
	// For example, here is the data from two wands
	// 00000000 00100011 01001100 00100110 00000010 00000010 00010111
	// 00000000 00100000 10001000 00110001 00000010 00000010 10110100

	// Decode the (MARK + SPACE) bits
	while (offset + 1 < irparams.rawlen) {
		int mark  = results->rawbuf[offset+0];
		int space = results->rawbuf[offset+1];

		if (!MATCH(mark + space, MAGIQUEST_TOTAL_USEC))  return false ;

		if      (IS_ZERO(mark, space))  data = (data << 1) | 0;
		else if (IS_ONE( mark, space))  data = (data << 1) | 1;
		else                            return false ;

		bits++;
		offset += 2;
	}

	// Grab the last MARK bit, assuming a good SPACE after it
	if (offset < irparams.rawlen) {
		int mark  = results->rawbuf[offset+0];
		int space = (MAGIQUEST_TOTAL_USEC / USECPERTICK) - mark;

		if      (IS_ZERO(mark, space))  data = (data << 1) | 0;
		else if (IS_ONE( mark, space))  data = (data << 1) | 1;
		else                            return false ;

		bits++;
	}

	if (bits != MAGIQUEST_BITS)  return false ;

	results->decode_type = MAGIQUEST;
	results->bits = 32;
	results->value = data >> 24;
	results->magnitude = data & 0xFFFFFF;

	return true;
}
#endif
