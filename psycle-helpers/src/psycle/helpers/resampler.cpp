///\implementation psycle::helpers::dsp::Cubic.
#include "resampler.hpp"
#include <psycle/helpers/math.hpp>
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS 
	#include <xmmintrin.h>
	#include <emmintrin.h>
#endif
#include <psycle/helpers/math/constants.hpp>

namespace psycle { namespace helpers { namespace dsp {

	bool cubic_resampler::initialized = false;
	float cubic_resampler::cubic_table_[CUBIC_RESOLUTION*4];
	float cubic_resampler::l_table_[CUBIC_RESOLUTION];

	float cubic_resampler::sinc_table_[SINC_TABLESIZE];
#if USE_SINC_DELTA
	float cubic_resampler::sinc_delta_[SINC_TABLESIZE];
#endif
	float cubic_resampler::sinc_pre_table_[SINC_TABLESIZE];

////////////////////////////////////////////////////////////////////////////////////////////////

	cubic_resampler::cubic_resampler() {
		if (!initialized) initTables();
	}
	void cubic_resampler::initTables() {
	// Cubic Resampling  ( 4point cubic spline) {
		initialized = true;
		// Initialize tables
		double const resdouble = 1.0/(double)CUBIC_RESOLUTION;
		for(int i = 0; i < CUBIC_RESOLUTION; ++i) {
			double x = (double)i * resdouble;
			//Cubic resolution is made of four table, but I've put them all in one table to optimize memory access.
			cubic_table_[i*4] = float(-0.5 * x * x * x +       x * x - 0.5 * x);
			cubic_table_[i*4+1] = float( 1.5 * x * x * x - 2.5 * x * x           + 1.0);
			cubic_table_[i*4+2] = float(-1.5 * x * x * x + 2.0 * x * x + 0.5 * x);
			cubic_table_[i*4+3] = float( 0.5 * x * x * x - 0.5 * x * x);

			l_table_[i] = x;
		}
	// }

	// Windowed Sinc Resampling {

		// one-sided-- the function is symmetrical, one wing of the sinc will be sufficient.
		sinc_table_[0] = 1.f; // save the trouble of evaluating 0/0.
		sinc_pre_table_[0] = 1.f;
		for(int i(1); i < SINC_TABLESIZE; ++i) {
			///\todo decide which window we like best.
			///\todo kaiser windows might be our best option, but i have no clue how to calculate one :)

			// we only apply half window (from pi..2pi instead of 0..2pi) because we only have half sinc.
			double tempval;
			#if 1
				// blackman window
			tempval = 0.42659 - 0.49656 * std::cos(math::pi+ math::pi * i/ ((double)SINC_TABLESIZE -1.0)) + 0.076849 * std::cos(2.0 * math::pi * i / ((double)SINC_TABLESIZE-1.0));
			#elif 0
				// hann(ing) window
				tempval = .5f * (1.f - std::cos(math::pi + math::pi * i / ((float)SINC_TABLESIZE -1.f)));
			#elif 0
				// hamming window
				tempval = 0.53836f - 0.46164f * std::cos(math::pi +  math::pi * i / ((float)SINC_TABLESIZE -1.f));
			#endif

			#if USE_SINC_DELTA && !(defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS)
				int write_pos = i;
			#else
				//Optimized version. Instead of putting the sinc linearly in the table, we allocate the zeroes of a specific offset consecutively.
				int idxzero = (i/SINC_RESOLUTION);
				int write_pos = (i%SINC_RESOLUTION)*SINC_ZEROS + idxzero;
			#endif
			tempval /= double(i * math::pi / (double)SINC_RESOLUTION);
			sinc_pre_table_[write_pos] = tempval;
			//sinc runs at half speed of SINC_RESOLUTION (i.e. two zero crossing points per period).
			//This means filter cutoff is at 0.5 (half of samplerate, full bandwidth)
			sinc_table_[write_pos] = std::sin(i * math::pi / (double)SINC_RESOLUTION) * tempval;
		}
		helpers::math::erase_all_nans_infinities_and_denormals(sinc_table_,SINC_TABLESIZE);
#if USE_SINC_DELTA
		for(int i(0); i < SINC_TABLESIZE - 1; ++i) {
			sinc_delta_[i] = sinc_table_[i + 1] - sinc_table_[i];
		}
		sinc_delta_[SINC_TABLESIZE - 1] = 0 - sinc_table_[SINC_TABLESIZE - 1];
		helpers::math::erase_all_nans_infinities_and_denormals(sinc_delta_,SINC_TABLESIZE);
#endif //USE_SINC_DELTA
	// }
	}

	void * cubic_resampler::GetResamplerData() const {
		if (quality() == quality::sinc) {
			sinc_data_t* t = new sinc_data_t();
			return t;
		}
		else if (quality() == quality::soxr) {
			soxr_t thesoxr = new_soxr();
			return thesoxr;
		}
		return NULL;
	}
	void cubic_resampler::UpdateSpeed(void * resampler_data, double speed) const {
		if (quality() == quality::sinc) {
			sinc_data_t* t = static_cast<sinc_data_t*>(resampler_data);
			if (speed > 1.0) {
				t->enabled=true;
				t->fcpi = math::pi/speed;
				t->fcpidivperiodsize = t->fcpi/SINC_RESOLUTION;
			}
			else {
				t->enabled=false;
			}
		}
		else if (quality() == quality::soxr) {
			/* Set the initial resampling ratio (N.B. 3rd parameter = 0): */
			soxr_set_io_ratio(static_cast<soxr_t>(resampler_data), speed, 0);
		}
	}
	void cubic_resampler::DisposeResamplerData(void * resampler_data) const {
		if (quality() == quality::sinc) {
			delete static_cast<sinc_data_t*>(resampler_data);
		}
		else if (quality() == quality::soxr) {
			soxr_delete(static_cast<soxr_t>(resampler_data));
		}
	}
	int cubic_resampler::requiredPresamples() const {
		switch(quality()) {
			case quality::linear: return 0;break;
			case quality::spline: return 1;break;
			case quality::sinc: return SINC_ZEROS-1; break;
			case quality::soxr: return 0;break;
			case quality::zero_order://fallthrough
			default:return 0;break;
		}
	}
	int cubic_resampler::requiredPostSamples() const {
		switch(quality()) {
			case quality::linear: return 1;break;
			case quality::spline: return 2;break;
			case quality::sinc: return SINC_ZEROS; break;
			case quality::soxr: return SOXR_RESAMPLE_POINTS;break;
			case quality::zero_order://fallthrough
			default:return 0;break;
		}
	}


	soxr_t cubic_resampler::new_soxr() {
	  soxr_error_t error;
	  soxr_t thesoxr;
	  /* When creating a var-rate resampler, q_spec must be set as follows: */
	  soxr_quality_spec_t q_spec = soxr_quality_spec(SOXR_HQ, SOXR_VR);

	  /* The ratio of the given input rate and output rates must equate to the
	   * maximum I/O ratio that will be used: */
	  //Note: 32 = 5 octaves, but think that the sample rate of the sample and the 
	  //samplerate of the audio output also add to the ratio.
	  //I.e. playing a 8363Hz sample at C-4 with a 96Khz sampling rate is already a ratio of 11.
	  thesoxr = soxr_create(32, 1, 1, &error, NULL, &q_spec, NULL);
	  return (error)? NULL : thesoxr;
	}


////////////////////////////////////////////////////////////////////////////////////////////////

	/// interpolation work function which does linear interpolation.
	// y0 = y[0]  [sample at x (input)]
	// y1 = y[1]  [sample at x+1]
	float cubic_resampler::linear(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length, void* /*resampler_data*/) {
		const float y0 = *data;
		const float y1 = (offset == length-1) ? 0 : *(data + 1);
		return y0 + (y1 - y0) * l_table_[res >> (32-CUBIC_RESOLUTION_LOG)];
	}
	float cubic_resampler::linear_unchecked(int16_t const * data, uint32_t res, void* /*resampler_data*/) {
		const float y0 = *data;
		const float y1 = *(data + 1);
		return y0 + (y1 - y0) * l_table_[res >> (32-CUBIC_RESOLUTION_LOG)];
	}
	float cubic_resampler::linear_float(float const * data, float offset, uint64_t length, void* /*resampler_data*/) {
		const float foffset = std::floor(offset);
		uint32_t res = (offset - foffset) * CUBIC_RESOLUTION;
		int ioffset = foffset;
		data+=ioffset;
		float y0 = *data;
		float y1 = (ioffset == length-1) ? 0 : *(data + 1);
		return y0 + (y1 - y0) * l_table_[res];
	}			
////////////////////////////////////////////////////////////////////////////////////////////////

	/// interpolation work function which does spline interpolation.
	// yo = y[-1] [sample at x-1]
	// y0 = y[0]  [sample at x (input)]
	// y1 = y[1]  [sample at x+1]
	// y2 = y[2]  [sample at x+2]
#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS && \
	defined DIVERSALIS__COMPILER__MICROSOFT // TODO gcc understands __m128 but the .m128_f32 notation is msvc-specific
	float cubic_resampler::spline(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length, void* /*resampler_data*/) {
		res >>= 32-CUBIC_RESOLUTION_LOG;
		res <<=2;//Since we have four floats in the table, the position is 16byte aligned.
		__m128 y = _mm_setzero_ps();
		if (offset != 0) y.m128_f32[0] = *(data - 1);
		y.m128_f32[1] = *data;
		if (offset < length-2) {
			y.m128_f32[2] = *(data + 1);
			y.m128_f32[3] = *(data + 2);
		}
		else if (offset < length-1) y.m128_f32[2] =  *(data + 1);
		__m128 t = _mm_load_ps(&cubic_table_[res]);
		__m128 result = _mm_mul_ps(y,t);
		return result.m128_f32[0]+result.m128_f32[1]+result.m128_f32[2]+result.m128_f32[3];
	}
	float cubic_resampler::spline_unchecked(int16_t const * data, uint32_t res, void* /*resampler_data*/) {
		res >>= 32-CUBIC_RESOLUTION_LOG;
		res <<=2;//Since we have four floats in the table, the position is 16byte aligned.
		__m128 y;
		y.m128_f32[0] = *(data - 1);
		y.m128_f32[1] = *data;
		y.m128_f32[2] = *(data + 1);
		y.m128_f32[3] = *(data + 2);
		__m128 t = _mm_load_ps(&cubic_table_[res]);
		__m128 result = _mm_mul_ps(y,t);
		return result.m128_f32[0]+result.m128_f32[1]+result.m128_f32[2]+result.m128_f32[3];
	}
	float cubic_resampler::spline_float(float const * data, float offset, uint64_t length, void* /*resampler_data*/) {
		const float foffset = std::floor(offset);
		const int ioffset = static_cast<int>(foffset);
		uint32_t res = (offset - foffset) * CUBIC_RESOLUTION;
		res <<=2;//Since we have four floats in the table, the position is 16byte aligned.
		data+=ioffset;
		__m128 y = _mm_setzero_ps();
		if (ioffset != 0) y.m128_f32[0] = *(data - 1);
		y.m128_f32[1] = *data;
		if (ioffset < length-2) {
			y.m128_f32[2] =  *(data + 1);
			y.m128_f32[3] =  *(data + 2);
		}
		else if (ioffset < length-1) y.m128_f32[2] =  *(data + 1);

		__m128 t = _mm_load_ps(&cubic_table_[res]);
		__m128 result = _mm_mul_ps(y,t);
		return result.m128_f32[0]+result.m128_f32[1]+result.m128_f32[2]+result.m128_f32[3];
	}
#else
	float cubic_resampler::spline(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length, void* /*resampler_data*/) {
		res >>= 32-CUBIC_RESOLUTION_LOG;
		res <<=2;//Since we have four floats in the table, the position is 16byte aligned.
		float yo = (offset == 0) ? 0 : *(data - 1);
		float y0 = *data;
		float y1 = (offset >= length-1) ? 0 : *(data + 1);
		float y2 = (offset >= length-2) ? 0 : *(data + 2);
		float *table = &cubic_table_[res];
		return table[0] * yo + table[1] * y0 + table[2] * y1 + table[3] * y2;
	}
	float cubic_resampler::spline_unchecked(int16_t const * data, uint32_t res, void* /*resampler_data*/) {
		res >>= 32-CUBIC_RESOLUTION_LOG;
		res <<=2;//Since we have four floats in the table, the position is 16byte aligned.
		const float yo = *(data - 1);
		const float y0 = *data;
		const float y1 = *(data + 1);
		const float y2 = *(data + 2);
		float *table = &cubic_table_[res];
		return table[0] * yo + table[1] * y0 + table[2] * y1 + table[3] * y2;
	}
	float cubic_resampler::spline_float(float const * data, float offset, uint64_t length, void* /*resampler_data*/) {
		const float foffset = std::floor(offset);
		const int ioffset = static_cast<int>(foffset);
		uint32_t res = (offset - foffset) * CUBIC_RESOLUTION;
		res <<=2;//Since we have four floats in the table, the position is 16byte aligned.
		data+=ioffset;
		float yo = (ioffset == 0) ? 0 : *(data - 1);
		float y0 = *data;
		float y1 = (ioffset >= length-1) ? 0 : *(data + 1);
		float y2 = (ioffset >= length-2) ? 0 : *(data + 2);
        float *table = &cubic_table_[res];
		return table[0] * yo + table[1] * y0 + table[2] * y1 + table[3] * y2;
	}
#endif
	
////////////////////////////////////////////////////////////////////////////////////////////////

	float cubic_resampler::sinc(int16_t const * data, uint64_t offset, uint32_t res, uint64_t length, void* resampler_data) {
		const int leftExtent = (offset < SINC_ZEROS) ? offset + 1 : SINC_ZEROS;
		const int rightExtent = (length - offset -1 < SINC_ZEROS) ? length - offset -1: SINC_ZEROS;
		sinc_data_t * t = static_cast<sinc_data_t*>(resampler_data);
		if (t->enabled) {
			return sinc_filtered(data, res, leftExtent, rightExtent, t);
		}
		return sinc_internal(data, res, leftExtent, rightExtent);
	}

	float cubic_resampler::sinc_unchecked(int16_t const * data, uint32_t res, void* resampler_data) {
		sinc_data_t * t = static_cast<sinc_data_t*>(resampler_data);
		if (t->enabled) {
			return sinc_filtered(data, res, SINC_ZEROS, SINC_ZEROS, t);
		}
		return sinc_internal(data, res, SINC_ZEROS, SINC_ZEROS);
	}


#if defined DIVERSALIS__CPU__X86__SSE && defined DIVERSALIS__COMPILER__FEATURE__XMM_INTRINSICS && \
	defined DIVERSALIS__COMPILER__MICROSOFT // TODO gcc understands __m128 but the .m128_f32 notation is msvc-specific

	//interpolation work function which does windowed sinc (bandlimited) interpolation.
	float cubic_resampler::sinc_internal(int16_t const * data, uint32_t res, int leftExtent, int rightExtent) {
		res >>= (32-SINC_RESOLUTION_LOG);
	#if defined OPTIMIZED_RES_SHIFT
		res <<= OPTIMIZED_RES_SHIFT;
	#else
		res *= SINC_ZEROS;
	#endif
		//Avoid evaluating position zero. It would need special treatment on the sinc_table of future points
		if (res == 0) return *data;

		float newval = 0.0f;
		register __m128 result =  _mm_setzero_ps();
		int16_t const * pdata = data;

		float *psinc = &sinc_table_[res];
		while (leftExtent > 3) {
			register __m128i data128;
			data128.m128i_i32[0] = pdata[0];
			data128.m128i_i32[1] = pdata[-1];
			data128.m128i_i32[2] = pdata[-2];
			data128.m128i_i32[3] = pdata[-3];
			register __m128 datafloat = _mm_cvtepi32_ps(data128);
	#if OPTIMIZED_RES_SHIFT > 1
			register __m128 sincfloat = _mm_load_ps(psinc);
	#else
			register __m128 sincfloat = _mm_loadu_ps(psinc);
	#endif
			result = _mm_add_ps(result,_mm_mul_ps(datafloat,sincfloat));
			pdata-=4;
			psinc+=4;
			leftExtent-=4;
		}
		while (leftExtent > 0) {
			newval += *psinc * *pdata;
			pdata--;
			psinc++;
			leftExtent--;
		}
		pdata = data+1;
		psinc = &sinc_table_[SINC_TABLESIZE-res];
		while (rightExtent > 3) {
			register __m128i data128;
			data128.m128i_i32[0] = pdata[0];
			data128.m128i_i32[1] = pdata[1];
			data128.m128i_i32[2] = pdata[2];
			data128.m128i_i32[3] = pdata[3];
			register __m128 datafloat = _mm_cvtepi32_ps(data128);
	#if OPTIMIZED_RES_SHIFT > 1
			register __m128 sincfloat = _mm_load_ps(psinc);
	#else
			register __m128 sincfloat = _mm_loadu_ps(psinc);
	#endif
			result = _mm_add_ps(result,_mm_mul_ps(datafloat,sincfloat));
			pdata+=4;
			psinc+=4;
			rightExtent-=4;
		}
		while (rightExtent > 0) {
			newval += *psinc * *pdata;
			pdata++;
			psinc++;
			rightExtent--;
		}
		result = _mm_add_ps(result,_mm_movehl_ps(result, result));
		result = _mm_add_ss(result,_mm_shuffle_ps(result, result, 0x11));
		float newval2;
		_mm_store_ss(&newval2, result);
		return newval+newval2;
	}

	float cubic_resampler::sinc_float(float const * data, float offset, uint64_t length, void* /*resampler_data*/) {
		const float foffset = std::floor(offset);
		uint32_t res = (offset - foffset) * SINC_RESOLUTION;
	#if defined OPTIMIZED_RES_SHIFT
		res <<= OPTIMIZED_RES_SHIFT;
	#else
		res *= SINC_ZEROS;
	#endif
		data+=static_cast<int>(foffset);

		//Avoid evaluating position zero. It would need special treatment on the sinc_table of future points
		if (res == 0) return *data;

		float newval = 0.0f;
		register __m128 result =  _mm_setzero_ps();
		float const * pdata = data;

		float *psinc = &sinc_table_[res];
		int leftExtent = (offset < SINC_ZEROS) ? offset +1: SINC_ZEROS;
		while (leftExtent > 3) {
			register __m128 datafloat;
			datafloat.m128_f32[0] = pdata[0];
			datafloat.m128_f32[1] = pdata[-1];
			datafloat.m128_f32[2] = pdata[-2];
			datafloat.m128_f32[3] = pdata[-3];
	#if OPTIMIZED_RES_SHIFT > 1
			register __m128 sincfloat = _mm_load_ps(psinc);
	#else
			register __m128 sincfloat = _mm_loadu_ps(psinc);
	#endif
			result = _mm_add_ps(result,_mm_mul_ps(datafloat,sincfloat));
			pdata-=4;
			psinc+=4;
			leftExtent-=4;
		}
		while (leftExtent > 0) {
			newval += *psinc * *pdata;
			pdata--;
			psinc++;
			leftExtent--;
		}
		int rightExtent = (length - offset -1< SINC_ZEROS) ? length - offset -1: SINC_ZEROS;
		pdata = data+1;
		psinc = &sinc_table_[SINC_TABLESIZE-res];
		while (rightExtent > 3) {
			register __m128 datafloat = _mm_loadu_ps(pdata);
	#if OPTIMIZED_RES_SHIFT > 1
			register __m128 sincfloat = _mm_load_ps(psinc);
	#else
			register __m128 sincfloat = _mm_loadu_ps(psinc);
	#endif
			result = _mm_add_ps(result,_mm_mul_ps(datafloat,sincfloat));
			pdata+=4;
			psinc+=4;
			rightExtent-=4;
		}
		while (rightExtent > 0) {
			newval += *psinc * *pdata;
			pdata++;
			psinc++;
			rightExtent--;
		}
		result = _mm_add_ps(result,_mm_movehl_ps(result, result));
		result = _mm_add_ss(result,_mm_shuffle_ps(result, result, 0x11));
		float newval2;
		_mm_store_ss(&newval2, result);
		return newval+newval2;

	}

#elif USE_SINC_DELTA
	/// interpolation work function which does windowed sinc interpolation.
	// Version with two tables, one of sinc, and one of deltas which is linearly interpolated
	float cubic_resampler::sinc_internal(int16_t const * data, uint32_t res, int leftExtent, int rightExtent) {
		const float weight = l_table_[res>>31-CUBIC_RESOLUTION];
		res >>= (32-SINC_RESOLUTION_LOG);

		//"Now" point. (would go on the left side of the sinc, but since we use a mirrored one, so we increase from zero)
		float newval = (sinc_table_[res] + sinc_delta_[res] * weight) * *data;

		//Past points (would go on the left side of the sinc, but since we use a mirrored one, so we increase from SINC_RESOLUTION)
		int sincIndex = SINC_RESOLUTION + res;
		for(int i(1); i < leftExtent; ++i, sincIndex += SINC_RESOLUTION)
			newval += (sinc_table_[sincIndex] + sinc_delta_[sincIndex] * weight) * *(data - i);

		//Future points, we decrease from SINC_RESOLUTION, since they are approaching the "Now" point
		sincIndex = SINC_RESOLUTION - res;
		for(int i(0); i < rightExtent; ++i, sincIndex += SINC_RESOLUTION)
			newval += (sinc_table_[sincIndex] + sinc_delta_[sincIndex] * weight) * *(data + i);

		return newval;
	}

	/// interpolation work function which does windowed sinc interpolation.
	// Version with two tables, one of sinc, and one of deltas which is linearly interpolated
	float cubic_resampler::sinc_float(float const * data, float offset, uint64_t length, void* /*resampler_data*/) {
		const float foffset = std::floor(offset);
		uint32_t res = (offset - foffset) * SINC_RESOLUTION;
		const float weight = l_table_[(offset - foffset) * CUBIC_RESOLUTION];
		data += static_cast<int>(foffset);

		//"Now" point. (would go on the left side of the sinc, but since we use a mirrored one, so we increase from zero)
		float newval = (sinc_table_[res] + sinc_delta_[res] * weight) * *data;

		//Past points (would go on the left side of the sinc, but since we use a mirrored one, so we increase from SINC_RESOLUTION)
		int sincIndex = SINC_RESOLUTION + res;
		const int leftExtent = (offset < SINC_ZEROS) ? offset + 1 : SINC_ZEROS;
		for(int i(1); i < leftExtent; ++i, sincIndex += SINC_RESOLUTION)
			newval += (sinc_table_[sincIndex] + sinc_delta_[sincIndex] * weight) * *(data - i);

		//Future points, we decrease from SINC_RESOLUTION, since they are approaching the "Now" point
		sincIndex = SINC_RESOLUTION - res;
		const int rightExtent = (length - offset -1< SINC_ZEROS) ? length - offset -1: SINC_ZEROS;
		for(int i(0); i < rightExtent; ++i, sincIndex += SINC_RESOLUTION)
			newval += (sinc_table_[sincIndex] + sinc_delta_[sincIndex] * weight) * *(data + i);

		return newval;
	}

#else
	float cubic_resampler::sinc_internal(int16_t const * data, uint32_t res, int leftExtent, int rightExtent) {
		res >>= (32-SINC_RESOLUTION_LOG);
	#if defined OPTIMIZED_RES_SHIFT
		res <<= OPTIMIZED_RES_SHIFT;
	#else
		res *= SINC_ZEROS;
	#endif
		//Avoid evaluating position zero. It would need special treatment on the sinc_table of future points
		if (res == 0) return *data;

		int16_t const * pdata = data;
		float *psinc = &sinc_table_[res];
		float newval = 0.0f;

		//Current and Past points (would go on the left side of the sinc, but we use a mirrored one)
		for(int i(0); i < leftExtent; ++i) {
			newval += *psinc * *pdata;
			pdata--;
			psinc++;
		}
		//Future points, they are approaching the "Now" point
		// future points decrease from SINC_RESOLUTION since they are reaching us.
		pdata = data+1;
		psinc = &sinc_table_[SINC_TABLESIZE-res];
		for(int i(0); i < rightExtent; ++i) {
			newval += *psinc * *pdata;
			pdata++;
			psinc++;
		}

		return newval;
	}

	float cubic_resampler::sinc_float(float const * data, float offset, uint64_t length, void* /*resampler_data*/) {
		const float foffset = std::floor(offset);
		uint32_t res = (offset - foffset) * SINC_RESOLUTION;
	#if defined OPTIMIZED_RES_SHIFT
		res <<= OPTIMIZED_RES_SHIFT;
	#else
		res *= SINC_ZEROS;
	#endif
		data += static_cast<int>(foffset);

		//Avoid evaluating position zero. It would need special treatment on the sinc_table of future points
		if (res == 0) return *data;

        float const * pdata = data;
		float *psinc = &sinc_table_[res];
		float newval = 0.0f;

		//Current and Past points (would go on the left side of the sinc, but we use a mirrored one)
		const int leftExtent = (offset < SINC_ZEROS) ? offset +1 : SINC_ZEROS;
		for(int i(0); i < leftExtent; ++i) {
			newval += *psinc * *pdata;
			pdata--;
			psinc++;
		}
		//Future points, they are approaching the "Now" point
		// future points decrease from SINC_RESOLUTION since they are reaching us.
		pdata = data+1;
		psinc = &sinc_table_[SINC_TABLESIZE-res];
		const int rightExtent = (length - offset -1< SINC_ZEROS) ? length - offset -1: SINC_ZEROS;
		for(int i(0); i < rightExtent; ++i) {
			newval += *psinc * *pdata;
			pdata++;
			psinc++;
		}

		return newval;
	}
#endif

	float cubic_resampler::sinc_filtered(int16_t const * data, uint32_t res, int leftExtent, int rightExtent, sinc_data_t* resampler_data) {
		res >>= (32-SINC_RESOLUTION_LOG);
		if (res==0) return *data;

		int16_t const * pdata = data;
		float newval = 0.0f;

		//Current and Past points. They increase from 0 because they are leaving us.
		// (they would decrease from zero if it wasn't a mirrored sinc)
	#if defined OPTIMIZED_RES_SHIFT
		float *ppretab = &sinc_pre_table_[res << OPTIMIZED_RES_SHIFT];
	#else
		float *ppretab = &sinc_pre_table_[res *SINC_ZEROS];
	#endif
		double w = res * resampler_data->fcpidivperiodsize;
		const double fcpi = resampler_data->fcpi;
		for(int i(0); i < leftExtent; ++i, w += fcpi) {
			//double sinc = std::sin(w) *  *ppretab;
			while(w > math::pi) w -= 2.f * math::pi;
			float sinc = math::fast_sin<4>(w) *  *ppretab;
			newval += sinc * *pdata;
			pdata--;
			ppretab++;
		}

		//Future points. They decrease from SINC_RESOLUTION since they are reaching the "Now" point.
	#if defined OPTIMIZED_RES_SHIFT
		ppretab = &sinc_pre_table_[SINC_TABLESIZE - (res << OPTIMIZED_RES_SHIFT)];
	#else
		ppretab = &sinc_pre_table_[SINC_TABLESIZE - (res *SINC_ZEROS)];
	#endif
		w = (SINC_RESOLUTION - res) * resampler_data->fcpidivperiodsize;
		pdata = data+1;
		for(int i(0); i < rightExtent; ++i, w += fcpi) {
			//double sinc = std::sin(w) *  *ppretab;
			while(w > math::pi) w -= 2.f * math::pi;
			float sinc = math::fast_sin<4>(w) *  *ppretab;
			newval += sinc * *pdata;
			pdata++;
			ppretab++;
		}

		return newval;
	}

	float cubic_resampler::soxr(int16_t const * dataIn, uint64_t offset, uint32_t res, uint64_t length, void* /*resampler_data*/) {
		return 0.f;
	}
	/*
	void cubic_resampler::soxr(int16_t const * dataIn, uint64_t offset, uint32_t res, uint64_t length, float const* dataOut, int numSamples) {
		size_t odone;
	    size_t block_len = min(SOXR_RESAMPLE_POINTS, offset - length);

	  // Determine the position in [0,1] of the end of the current block:
	  double pos = (double)(total_odone + block_len) / (double)total_olen;

	  // Output the block of samples, supplying input samples as needed:
	  do {
		size_t len = need_input? length : 0;
		error = soxr_process(soxr, ibuf, len, NULL, obuf, block_len, &odone);
		fwrite(obuf, sizeof(float), odone, stdout);

		// Update counters for the current block and for the total length:
		block_len -= odone;
		total_odone += odone;

		// If soxr_process did not provide the complete block, we must call it
		// again, supplying more input samples:
		need_input = block_len != 0;

	  } while (need_input && !error);

		return 0.f;
	}*/
	/****************************************************************************/


}}}
