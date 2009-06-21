//////////////////////////////////////////////////////////////////////
//
//				Dsp.cpp
//
//				druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include "Dsp.h"
#include "../blwtbl/blwtbl.h"

//////////////////////////////////////////////////////////////////////
// Variables

// fm & pm tables
float *pfmtable;
float *ppmtable;

//////////////////////////////////////////////////////////////////////
// InitializeDSP
void DRUTTIS__DSP__CALLING_CONVENTION__C InitializeDSP() {
	// Get amptable
	pfmtable = GetFMTable();
	ppmtable = GetPMTable();
}

//////////////////////////////////////////////////////////////////////
// DestroyDSP
void DRUTTIS__DSP__CALLING_CONVENTION__C DestroyDSP() {
}

//////////////////////////////////////////////////////////////////////
// Fill
void DRUTTIS__DSP__CALLING_CONVENTION__FAST Fill(float *pbuf, float value, int nsamples) {
	--pbuf;
	do
	{
		*++pbuf = value;
	}
	while (--nsamples);
}

//////////////////////////////////////////////////////////////////////
// Copy
void DRUTTIS__DSP__CALLING_CONVENTION__FAST Copy(float *pbuf1, float *pbuf2, int nsamples) {
	--pbuf1;
	--pbuf2;
	do
	{
		*++pbuf1 = *++pbuf2;
	}
	while (--nsamples);
}

//////////////////////////////////////////////////////////////////////
// Add
void DRUTTIS__DSP__CALLING_CONVENTION__FAST Add(float *pbuf1, float *pbuf2, int nsamples) {
	--pbuf1;
	--pbuf2;
	do
	{
		*++pbuf1 += *++pbuf2;
	}
	while (--nsamples);
}

//////////////////////////////////////////////////////////////////////
// Sub
void DRUTTIS__DSP__CALLING_CONVENTION__FAST Sub(float *pbuf1, float *pbuf2, int nsamples) {
	--pbuf1;
	--pbuf2;
	do {
		*++pbuf1 -= *++pbuf2;
	}
	while(--nsamples);
}

//////////////////////////////////////////////////////////////////////
// Mul
void DRUTTIS__DSP__CALLING_CONVENTION__FAST Mul(float *pbuf1, float *pbuf2, int nsamples) {
	--pbuf1;
	--pbuf2;
	do {
		*++pbuf1 *= *++pbuf2;
	}
	while(--nsamples);
}
