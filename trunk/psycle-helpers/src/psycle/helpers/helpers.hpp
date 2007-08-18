///\interface psycle::helpers
#pragma once
#include <psycle/helpers/math/pi.hpp>
#include <psycle/helpers/math/truncate.hpp>
#include <psycle/helpers/math/round.hpp>
#include <psycle/helpers/math/log.hpp>
#include <universalis/compiler/numeric.hpp>
#include <boost/static_assert.hpp>
#include <string> // to declare hexstring_to_integer
#include <cstdint>
namespace psycle
{
	namespace helpers
	{
		/// the pi constant as a 32-bit floating point number
		float const F_PI = math::pi_f;

		/// parses an hexadecimal string to convert it to an integer
		template<typename X>
		void hexstring_to_integer(std::string const &, X &);

		/// linearly maps a byte (0 to 255) to a float (0 to 1).
		///\todo needs some explanation about why the tables have a length of 257.
		///\todo make that a namespace
		class CValueMapper
		{
			public:
				/// contructor.
				///\todo not needed since everything is static
				CValueMapper() {}

				/// destructor.
				///\todo not needed since everything is static
				~CValueMapper() {}

				/// maps a byte (0 to 255) to a float (0 to 1).
				static inline float Map_255_1(int byte)
				{
					///\todo the conditional branches probably make it slower than direct calculation
					if(0 <= byte && byte <= 256)
						return CValueMapper::fMap_255_1[byte];
					else
						return byte * 0.00390625f;
				}

				/// maps a byte (0 to 255) to a float (0 to 100).
				static inline float Map_255_100(int byte)
				{
					///\todo the conditional branches probably make it slower than direct calculation
					if(0 <= byte && byte <= 256)
						return CValueMapper::fMap_255_100[byte];
					else
						return byte * 0.390625f;
				}
			private:
				static float fMap_255_1[257];
				static float fMap_255_100[257];
		};

		///\todo doc
		inline float fast_log2(float f) UNIVERSALIS__COMPILER__CONST
		{
			return math::log2(f);
		}

		/// converts a floating point number to an integer by rounding to the nearest integer.
		/// note: it is unspecified whether rounding x.5 rounds up or down.
		std::int32_t inline UNIVERSALIS__COMPILER__CONST
		f2i(double d)
		{ 
			return math::rounded(d);
		}
		
		/// converts a floating point number to an integer by rounding to the nearest integer.
		/// note: it is unspecified whether rounding x.5 rounds up or down.
		std::int32_t inline UNIVERSALIS__COMPILER__CONST
		f2i(float f)
		{ 
			return math::rounded(f);
		}
		
		/// clipping.
		template<unsigned int const bits>
		typename universalis::compiler::numeric<bits>::signed_int inline f2iclip(float const & f) UNIVERSALIS__COMPILER__CONST
		{
			typedef typename universalis::compiler::numeric<bits>::signed_int result_type;
			typedef std::numeric_limits<result_type> type_traits;
			if(f < type_traits::min) return type_traits::min;
			if(f > type_traits::max) return type_traits::max;
			return static_cast<result_type>(f);
		}

		/// clipping.
		inline int f2iclip16(float f) UNIVERSALIS__COMPILER__CONST
		{ 
			int const l(32767);
			if(f < -l) return -l;
			if(f > +l) return +l;
			return f2i(f);
		}

		/// clipping.
		inline int f2iclip18(float f) UNIVERSALIS__COMPILER__CONST
		{ 
			int const l(131071);
			if(f < -l) return -l;
			if(f > +l) return +l;
			return f2i(f);
		}

		/// clipping.
		inline int f2iclip20(float f) UNIVERSALIS__COMPILER__CONST
		{ 
			int const l(524287);
			if(f < -l) return -l;
			if(f > +l) return +l;
			return f2i(f);
		}

		/// clipping.
		inline int f2iclip24(float f) UNIVERSALIS__COMPILER__CONST
		{ 
			int const l(8388607);
			if(f < -l) return -l;
			if(f > +l) return +l;
			return f2i(f);
		}

		/// clipping.
		inline int f2iclip32(float f) UNIVERSALIS__COMPILER__CONST
		{ 
			int const l(2147483647);
			if(f < -l) return -l;
			if(f > +l) return +l;
			return f2i(f);
		}
	}
}
