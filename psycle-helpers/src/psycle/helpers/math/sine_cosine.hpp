// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the music-dsp mailing list http://www.music.columbia.edu/cmc/music-dsp and of the psycle project http://psycle.sourceforge.net 

#pragma once
#include <cmath>
namespace psycle { namespace helpers { namespace math {

	/// computes both the sine and the cosine at the same time
	template<typename Real>
	void sin_cos(Real const & x, Real & sine, Real & cosine) {
		// some compilers are able to optimise those two calls into one.
		sine = std::sin(x);
		cosine = std::cos(x);
	
		#if 0 // doesn't work for negative cosines
			sine = std::sin(x);
			cosine = std::sqrt(1 - sine * sine);
			// we need to do cosine = -cosine for some values of x
		#endif
	
	}
}}}

// inline implementation
namespace psycle { namespace helpers { namespace math {

	#if __STDC__VERSION__ >= 199901 || \
		(defined DIVERSALIS__COMPILER__GNU && DIVERSALIS__COMPILER__VERSION__MAJOR >= 4)
		
		template<>
		void sin_cos<>(long double const & x, long double & sine, long double & cosine) {
			// some compilers are able to optimise those two calls into one.
			::sincosl(x, &sine, &cosine);
		}

		template<>
		void sin_cos<>(double const & x, double & sine, double & cosine) {
			// some compilers are able to optimise those two calls into one.
			::sincos(x, &sine, &cosine);
		}

		template<>
		void sin_cos<>(float const & x, float & sine, float & cosine) {
			// some compilers are able to optimise those two calls into one.
			::sincosf(x, &sine, &cosine);
		}

	#endif
}}}
