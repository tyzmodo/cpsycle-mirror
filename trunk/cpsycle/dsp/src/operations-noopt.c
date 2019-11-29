// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "operations.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

static void* dsp_memory_alloc(size_t count, size_t size);
static void dsp_memory_dealloc(void* address);
static void dsp_add(amp_t *src, amp_t *dst, uintptr_t num, amp_t vol);
static void dsp_mul(amp_t *dst, uintptr_t num, amp_t mul);
static void dsp_movmul(amp_t *src, amp_t *dst, uintptr_t num, amp_t mul);
static void dsp_clear(amp_t *dst, uintptr_t num);
static void dsp_interleave(amp_t* dst, amp_t* left, amp_t* right, uintptr_t num);
static void dsp_erase_all_nans_infinities_and_denormals(amp_t* dst,
		uintptr_t num);
static void erase_all_nans_infinities_and_denormals(amp_t* sample);
static float dsp_maxvol(const float* pSamples, uintptr_t numSamples);
static void dsp_accumulate(big_amp_t* accumleft, 
					big_amp_t* accumright, 
					const amp_t* __restrict pSamplesL,
					const amp_t* __restrict pSamplesR, int count);

void dsp_noopt_init(Dsp* self)
{	
	self->memory_alloc = dsp_memory_alloc;
	self->memory_dealloc = dsp_memory_dealloc;
	self->add = dsp_add;
	self->mul = dsp_mul;
	self->movmul = dsp_movmul;
	self->clear = dsp_clear;
	self->interleave = dsp_interleave;
	self->erase_all_nans_infinities_and_denormals = dsp_erase_all_nans_infinities_and_denormals;
	self->maxvol = dsp_maxvol;
	self->accumulate = dsp_accumulate;
}

void* dsp_memory_alloc(size_t count, size_t size)
{
	return malloc(count * size);
}

void dsp_memory_dealloc(void* address)
{
	free(address);
}

void dsp_add(amp_t *src, amp_t *dst, uintptr_t num, amp_t vol)
{
	for ( ; num != 0; ++dst, ++src, --num) {
		*dst += (*src * vol);
	}	
}

void dsp_mul(amp_t *dst, uintptr_t num, amp_t mul)
{	
	for ( ; num != 0; ++dst, --num) {
		*dst *= mul;		
	}	
}

void dsp_movmul(amp_t *src, amp_t *dst, uintptr_t num, amp_t mul)
{
	--src;
	--dst;
	do
	{
		*++dst = *++src*mul;
	}
	while (--num);
}

void dsp_clear(amp_t *dst, uintptr_t num)
{
	memset(dst, 0, num * sizeof(amp_t));
}

void dsp_interleave(amp_t* dst, amp_t* left, amp_t* right, uintptr_t num)
{
	uintptr_t i;
	--dst;
	--left;
	--right;	
	i = num;
	do {
		++dst;
		++left;
		++right;
		*dst = *left;
		++dst;
		*dst = *right;
	}
	while (--i);
}

float dsp_maxvol(const float* pSamples, uintptr_t numSamples)
{
	float vol = 0.0f;
	--pSamples;	
	do { /// not all waves are symmetrical
		const float nvol = (float) fabs(*++pSamples);
		if (nvol > vol) vol = nvol;
	} while (--numSamples);
	return vol;
}

void dsp_erase_all_nans_infinities_and_denormals(amp_t* dst,
		uintptr_t num) {
	uintptr_t i;

	for(i = 0; i < num; ++i) {
		erase_all_nans_infinities_and_denormals(&dst[i]);
	}
}

/// Cure for malicious samples
/// Type : Filters Denormals, NaNs, Infinities
/// References : Posted by urs[AT]u-he[DOT]com
void erase_all_nans_infinities_and_denormals(float* sample) {
	#if !defined DIVERSALIS__CPU__X86
		// just do nothing.. not crucial for other archs ?
	#else
		union {
			float sample;
			uint32_t bits;
		} u;
		u.sample = sample;

		uint32_t const exponent_mask(0x7f800000);
		uint32_t const exponent(u.bits & exponent_mask);

		// exponent < exponent_mask is 0 if NaN or Infinity, otherwise 1
		uint32_t const not_nan_nor_infinity(exponent < exponent_mask);

		// exponent > 0 is 0 if denormalized, otherwise 1
		uint32_t const not_denormal(exponent > 0);

		u.bits *= not_nan_nor_infinity & not_denormal;
		*sample = u.sample;
	#endif
}

void dsp_accumulate(big_amp_t* accumleft, 
					big_amp_t* accumright, 
					const amp_t* __restrict pSamplesL,
					const amp_t* __restrict pSamplesR,
					int count)
{
	big_amp_t acleft = *accumleft;
	big_amp_t acright = *accumright;
	--pSamplesL; --pSamplesR;
	while (count--) {
		++pSamplesL; acleft  += *pSamplesL * *pSamplesL;
		++pSamplesR; acright += *pSamplesR * *pSamplesR;
	}
	*accumleft = acleft;
	*accumright = acright;
}