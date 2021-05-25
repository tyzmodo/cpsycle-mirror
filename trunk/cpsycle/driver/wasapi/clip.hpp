// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2011 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#define PSYCLE__HELPERS__MATH__CLIP__INCLUDED
#pragma once

// #include <universalis.hpp>
// #include <universalis/stdlib/cmath.hpp>
#include <limits>
#include "../../detail/os.h"
#include "../../detail/cpu.h"

#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS 
	#include <xmmintrin.h>
	#include <emmintrin.h>
#endif
#include <algorithm>

#if defined DIVERSALIS__COMPILER__GNU
#define UNIVERSALIS__COMPILER__CONST_FUNCTION
//#define UNIVERSALIS__COMPILER__CONST_FUNCTION UNIVERSALIS__COMPILER__ATTRIBUTE(__const__)
#elif defined DIVERSALIS__COMPILER__INTEL
	///\todo supported.. check the doc
#define UNIVERSALIS__COMPILER__CONST_FUNCTION
#else
#define UNIVERSALIS__COMPILER__CONST_FUNCTION
#endif

namespace psycle { namespace helpers { namespace math {

// using universalis::stdlib::rint;
	/******************************************************************************************/
	/// C1999 *rint* - converts a floating point number to an integer by rounding in an unspecified way.
	/// This function has the same semantic as C1999's *rint* series of functions,
	/// but with C++ overload support, we don't need different names for each type.
	/// On C1999, the rounding mode may be set with fesetround, but msvc does not support it, so the mode is unspecified.
	/// Because the rounding mode is left as is, this is faster than using static_cast (trunc), floor, ceil, or round, which need to change it temporarily.
	template<typename IntegralResult, typename Real> // UNIVERSALIS__COMPILER__CONST_FUNCTION
	IntegralResult inline rint(Real x) noexcept {
		return x;
	}

/// ensures a value stays between two bounds by clamping it
template<typename X> UNIVERSALIS__COMPILER__CONST_FUNCTION
X inline clip(X minimum, X value, X maximum) {
	// it looks a bit dumb to write a function to do that code,
	// but maybe someone will find an optimised way to do this.
	return value < minimum ? minimum : value > maximum ? maximum : value;
}

/// clips with min and max values inferred from bits
template<const unsigned int bits, typename X> UNIVERSALIS__COMPILER__CONST_FUNCTION
X inline clip(X value) {
	int constexpr maxval(int((1u << (bits - 1u)) - 1u));
	int constexpr minval(-maxval - 1);
	return clip(X(minval), value, X(maxval));
}

/// combines float to signed integer conversion with clipping.
template<typename SignedIntegralResult, const unsigned int bits, typename Real> UNIVERSALIS__COMPILER__CONST_FUNCTION
SignedIntegralResult inline rint_clip(Real x) {
	// check that the Result type is a signed integral number
//	BOOST_STATIC_ASSERT((std::numeric_limits<SignedIntegralResult>::is_signed));
//	BOOST_STATIC_ASSERT((std::numeric_limits<SignedIntegralResult>::is_integer));

	return rint<SignedIntegralResult>(clip<bits>(x));
}

/// combines float to signed integer conversion with clipping.
template<typename SignedIntegralResult, typename Real> UNIVERSALIS__COMPILER__CONST_FUNCTION
SignedIntegralResult inline rint_clip(Real x) {
	return rint_clip<SignedIntegralResult, (sizeof(SignedIntegralResult) << 3)>(x);
}

/// combines float to signed integer conversion with clipping.
/// Amount has to be a multiple of 8, and both in and out be 16-byte aligned.
inline void rint_clip_array(const float in[], short out[], int amount) {
	#if DIVERSALIS__CPU__X86__SSE >= 2 && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS
		__m128 *psrc = (__m128*)in;
		__m128i *pdst = (__m128i*)out;
		do
		{
			__m128i tmpps1 = _mm_cvttps_epi32(*psrc++);
			__m128i tmpps2 = _mm_cvttps_epi32(*psrc++);
			*pdst = _mm_packs_epi32(tmpps1,tmpps2);
			pdst++;
			amount-=8;
		} while(amount>0);
	#elif defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__ASSEMBLER__INTEL
		__asm
		{
			mov esi, in
			mov edi, out
			mov eax, [amount]
		LOOPSTART:
			CVTTPS2DQ xmm0, [esi]
			add esi, 10H
			CVTTPS2DQ xmm1, [esi]
			add esi, 10H
			PACKSSDW xmm1, xmm0
			movaps [edi], xmm1

			add edi, 10H
			sub eax, 8
			cmp eax, 0
			jle END
			jmp LOOPSTART
		END:
		}
	#else
		do {
			*out++ = rint_clip<short>(*in++);
		} while(--amount);

	#endif
}

}}}

#endif
