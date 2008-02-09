/* -*- mode:c++, indent-tabs-mode:t -*- */
//============================================================================
//
//				OneZero
//
//				druttis@darkface.pp.se
//
//============================================================================
#include "OneZero.h"
//============================================================================
//				Constructor
//============================================================================
OneZero::OneZero()
{
}
//============================================================================
//				Destructor
//============================================================================
OneZero::~OneZero()
{
}
//============================================================================
//				Init
//============================================================================
void OneZero::Init()
{
	gain = 1.0f;
	zeroCoeff = 1.0f;
	sgain = 0.5f;
	Clear();
}
//============================================================================
//				Clear
//============================================================================
void OneZero::Clear()
{
	inputs[0] = 0.0f;
	lastOutput = 0.0f;
}
//============================================================================
//				SetGain
//============================================================================
void OneZero::SetGain(float newGain)
{
	gain = newGain;
	if (zeroCoeff > 0.0f)
		sgain = gain / (1.0f + zeroCoeff);
	else
		sgain = gain / (1.0f - zeroCoeff);
}
//============================================================================
//				SetZeroCoeff
//============================================================================
void OneZero::SetCoeff(float newCoeff)
{
	zeroCoeff = newCoeff;
	if (zeroCoeff > 0.0f)
		sgain = gain / (1.0f + zeroCoeff);
	else
		sgain = gain / (1.0f - zeroCoeff);
}
