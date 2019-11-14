// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "rms.h"
#include <math.h>
#include <stdlib.h>
#if defined SSE
	#include <xmmintrin.h>
#endif

static int numRMSSamples = 1;

// RMSData

void rmsdata_init(RMSData* self)
{
	self->AccumLeft = 0;
	self->AccumRight = 0;
	self->count = 0;
	self->previousLeft = 0;
	self->previousRight = 0;
}

#if defined SSE
void rmsdata_accumulate(RMSData* self, const amp_t* __restrict pSamplesL,
	const amp_t* __restrict pSamplesR, int count)
{
	float result; // does this work with double?

	__m128 acleftvec = _mm_set_ps1(0.0f);
	__m128 acrightvec = _mm_set_ps1(0.0f);
	const __m128 *psrcl = (const __m128*)pSamplesL;
	const __m128 *psrcr = (const __m128*)pSamplesR;
	while (count > 3){
		acleftvec = _mm_add_ps(acleftvec,_mm_mul_ps(*psrcl,*psrcl));
		acrightvec = _mm_add_ps(acrightvec,_mm_mul_ps(*psrcr,*psrcr));
		psrcl++;
		psrcr++;
		count-=4;
	}	
	acleftvec = _mm_add_ps(acleftvec,_mm_movehl_ps(acleftvec, acleftvec)); // add (0= 0+2 and 1=1+3 thanks to the move)
	acleftvec = _mm_add_ss(acleftvec,_mm_shuffle_ps(acleftvec,acleftvec,0x11)); // add (0 = 0+1 thanks to the shuffle)
	_mm_store_ss(&result, acleftvec); //get the position 0
	self->AccumLeft += result;
	acrightvec = _mm_add_ps(acrightvec,_mm_movehl_ps(acrightvec, acrightvec)); // add (0= 0+2 and 1=1+3 thanks to the move)
	acrightvec = _mm_add_ss(acrightvec,_mm_shuffle_ps(acrightvec,acrightvec,0x11)); // add (0 = 0+1 thanks to the shuffle)
	_mm_store_ss(&result, acrightvec); //get the position 0
	self->AccumRight += result;
	if (count > 0 ) {
		const float* pL = (const float*)psrcl;
		const float* pR= (const float*)psrcr;
		while (count--) {
			self->AccumLeft += (*pL * *pL); pL++;
			self->AccumRight += (*pR * *pR); pR++;
		}
	}	
}
#else

void rmsdata_accumulate(RMSData* self, const amp_t* __restrict pSamplesL,
	const amp_t* __restrict pSamplesR, int count)
{
	big_amp_t acleft = self->AccumLeft;
	big_amp_t acright = self->AccumRight;
	--pSamplesL; --pSamplesR;
	while (count--) {
		++pSamplesL; acleft  += *pSamplesL * *pSamplesL;
		++pSamplesR; acright += *pSamplesR * *pSamplesR;
	}
	self->AccumLeft = acleft;
	self->AccumRight = acright;
}

#endif

// RMSVol

void rmsvol_init(RMSVol* self)
{
	rmsdata_init(&self->data);
	self->volume = 0.f;
}

RMSVol* rmsvol_alloc(void)
{
	return (RMSVol*) malloc(sizeof(RMSVol));
}

RMSVol* rmsvol_allocinit(void)
{
	RMSVol* rv;

	rv = rmsvol_alloc();
	if (rv) {
		rmsvol_init(rv);
	}
	return rv;
}

/// Note: Values are accumulated since the standard calculation requires 50ms of data.
void rmsvol_tick(RMSVol* self, const amp_t * __restrict pSamplesL,
	const amp_t * __restrict pSamplesR, int numSamples)
{
	const float * pL = pSamplesL;
	const float * pR = pSamplesR;

	int ns = numSamples;
	int count = numRMSSamples - self->data.count;
	if (ns >= count) {
		// count can be negative when changing the samplerate.
		if (count >= 0) {
			rmsdata_accumulate(&self->data, pSamplesL, pSamplesR, count);				
#if defined SSE
			//small workaround for 16byte boundary (it makes it slightly incorrect, but hopefully just a bit).
			ns -= count&0x3;
			count = count&~0x3;
#endif
			ns -= count;
			pL+=count; pR+=count;
		}
		self->data.previousLeft  = (float) sqrt(self->data.AccumLeft  / numRMSSamples);
		self->data.previousRight = (float) sqrt(self->data.AccumRight / numRMSSamples);
		self->data.AccumLeft = 0;
		self->data.AccumRight = 0;
		self->data.count = 0;		
	}
	self->data.count += ns;
	rmsdata_accumulate(&self->data, pL, pR, ns);
	self->volume = self->data.previousLeft > self->data.previousRight
			? self->data.previousLeft
			: self->data.previousRight;
}

amp_t rmsvol_value(RMSVol* self)
{
	return self->volume;
}

void rmsvol_setsamplerate(unsigned int samplerate)
{
	/// standard calculation requires 50ms of data.
	numRMSSamples = (int) (samplerate * 0.05f);
}
