#include "../detail/prefix.h"

#include "comb.hpp"
#include <operations.h>
// #include <universalis/os/aligned_alloc.hpp>
// #include <psycle/helpers/dsp.hpp>

// Comb filter implementation
//
// Originally Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

comb::comb()
: buffer(0), feedback(0), filterstore(0), damp1(0), damp2(0),
	bufsize(0), bufidx(0)
{}
comb::~comb()
{
	deletebuffer();
}

void comb::setbuffer(int samples)
{
	if (bufsize) {
		deletebuffer();
	}
	bufsize = samples;
	buffer = (float*)dsp.memory_alloc(16, (bufsize + 3) & 0xFFFFFFFC);	
	if(bufidx>=bufsize) bufidx = 0;
}

void comb::mute()
{
	dsp.clear(buffer, bufsize);
}

void comb::setdamp(float val) 
{
	//todo: this will need some work for multiple sampling rates.
	damp1 = val; 
	damp2 = 1-val;
}

float comb::getdamp() 
{
	return damp1;
}

void comb::setfeedback(float val) 
{
	feedback = val;
}

float comb::getfeedback() 
{
	return feedback;
}
void comb::deletebuffer() {
	if (bufsize) {
		dsp.memory_dealloc(buffer);
	}
}
