/*
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/*
 *  DynNoiseUGens.cpp
 *  xSC3plugins
 *
 *  Created by Alberto de Campo, Sekhar Ramacrishnan, Julian Rohrhuber on Sun May 30 2004.
 *  Copyright (c) 2004 HfbK. All rights reserved.
 *
 */


#include "SC_PlugIn.h"

static InterfaceTable *ft;

struct LFDNoise0 : public Unit
{
	float mLevel;
	float mPhase;
};
struct LFDNoise1 : public Unit
{
	float mPhase;
	float mPrevLevel;
	float mNextLevel;
};


struct LFDNoise3 : public Unit
{
	float mPhase;
	float mLevelA, mLevelB, mLevelC, mLevelD;
};

struct LFDClipNoise : public Unit
{
	float mLevel;
	float mPhase;
};

//////////////////////////////////////////////////////////////////////////////////////////////////


extern "C"
{
	void load(InterfaceTable *inTable);

	void LFDNoise0_next(LFDNoise0 *unit, int inNumSamples);
	void LFDNoise0_next_k(LFDNoise0 *unit, int inNumSamples);
	void LFDNoise0_Ctor(LFDNoise0 *unit);

	void LFDNoise1_next(LFDNoise1 *unit, int inNumSamples);
	void LFDNoise1_next_k(LFDNoise1 *unit, int inNumSamples);
	void LFDNoise1_Ctor(LFDNoise1 *unit);


	void LFDNoise3_next(LFDNoise3 *unit, int inNumSamples);
	void LFDNoise3_next_k(LFDNoise3 *unit, int inNumSamples);
	void LFDNoise3_Ctor(LFDNoise3 *unit);

	void LFDClipNoise_next(LFDClipNoise *unit, int inNumSamples);
	void LFDClipNoise_Ctor(LFDClipNoise *unit);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

// macros to put rgen state in registers
#define RGET \
	RGen& rgen = *unit->mParent->mRGen; \
	uint32 s1 = rgen.s1; \
	uint32 s2 = rgen.s2; \
	uint32 s3 = rgen.s3; 

#define RPUT \
	rgen.s1 = s1; \
	rgen.s2 = s2; \
	rgen.s3 = s3;

//////////////////////////////////////////////////////////////////////////////////////////////////

void LFDNoise0_next(LFDNoise0 *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float level = unit->mLevel;
	float phase = unit->mPhase;
	float smpdur = SAMPLEDUR;	
	RGET
	
	LOOP(inNumSamples,
		phase -= ZXP(freq) * smpdur;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			level = frand2(s1,s2,s3);
		}
		ZXP(out) = level;
	)
	unit->mLevel = level;
	unit->mPhase = phase;
	RPUT
		
}

void LFDNoise0_next_k(LFDNoise0 *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0);
	float level = unit->mLevel;
	float phase = unit->mPhase;
	float smpdur = SAMPLEDUR;
	float dphase = smpdur * freq;
	
	RGET
	
	LOOP(inNumSamples,
		phase -= dphase;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			level = frand2(s1,s2,s3);
		}
		ZXP(out) = level;
	)
	unit->mLevel = level;
	unit->mPhase = phase;
	RPUT
		
}

void LFDNoise0_Ctor(LFDNoise0* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFDNoise0_next);
	} else {
		SETCALC(LFDNoise0_next_k);
	}

	unit->mPhase = 0.f; 
	unit->mLevel = 0.f; 
	
	LFDNoise0_next(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void LFDNoise1_next(LFDNoise1 *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float prevLevel = unit->mPrevLevel;
	float nextLevel = unit->mNextLevel;
	float phase = unit->mPhase;
	float smpdur = SAMPLEDUR;
	
	RGET
	
	LOOP(inNumSamples, 
		phase -= ZXP(freq) * smpdur;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			prevLevel = nextLevel;
			nextLevel = frand2(s1,s2,s3);
		}
		ZXP(out) = nextLevel + ( phase * (prevLevel - nextLevel) );
	)
	unit->mPrevLevel = prevLevel;
	unit->mNextLevel = nextLevel;
	unit->mPhase = phase;
	RPUT
}

void LFDNoise1_next_k(LFDNoise1 *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0);
	float prevLevel = unit->mPrevLevel;
	float nextLevel = unit->mNextLevel;
	float phase = unit->mPhase;
	float smpdur = SAMPLEDUR;
	float dphase = freq * smpdur;
	
	RGET
	
	LOOP(inNumSamples, 
		phase -= dphase;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			prevLevel = nextLevel;
			nextLevel = frand2(s1,s2,s3);
		}
		ZXP(out)= nextLevel + ( phase * (prevLevel - nextLevel) );
	)
	unit->mPrevLevel = prevLevel;
	unit->mNextLevel = nextLevel;
	unit->mPhase = phase;
	RPUT
}

void LFDNoise1_Ctor(LFDNoise1* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFDNoise1_next);
	} else {
		SETCALC(LFDNoise1_next_k);
	}
	
	unit->mPhase = 0.f;
	unit->mPrevLevel = 0.f;
	unit->mNextLevel = unit->mParent->mRGen->frand2();

	LFDNoise1_next(unit, 1);
}

//////////////////////////////////////////////////////////////////////////////////////////////////

static float cubicinterp(float x, float y0, float y1, float y2, float y3)
{
	// 4-point, 3rd-order Hermite (x-form)
	float c0 = y1;
	float c1 = 0.5f * (y2 - y0);
	float c2 = y0 - 2.5f * y1 + 2.f * y2 - 0.5f * y3;
	float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

	return ((c3 * x + c2) * x + c1) * x + c0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void LFDNoise3_next(LFDNoise3 *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float a = unit->mLevelA;
	float b = unit->mLevelB;
	float c = unit->mLevelC;
	float d = unit->mLevelD;
	float phase = unit->mPhase;
	float smpdur = SAMPLEDUR;
	
	RGET
	
	LOOP(inNumSamples, 
		phase -= ZXP(freq) * smpdur;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			a = b;
			b = c;
			c = d; 
			d = frand2(s1,s2,s3) * 0.8f;	// limits max interpol. overshoot to 1.
		}
		ZXP(out) = cubicinterp(1.f - phase, a, b, c, d);
	)
	unit->mLevelA = a;
	unit->mLevelB = b;
	unit->mLevelC = c;
	unit->mLevelD = d;
	unit->mPhase = phase;
	RPUT
}

void LFDNoise3_next_k(LFDNoise3 *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0);
	float a = unit->mLevelA;
	float b = unit->mLevelB;
	float c = unit->mLevelC;
	float d = unit->mLevelD;
	float phase = unit->mPhase;
	float dphase = freq * SAMPLEDUR;
	
	RGET
	
	LOOP(inNumSamples, 
		phase -= dphase;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			a = b;
			b = c;
			c = d; 
			d = frand2(s1,s2,s3) * 0.8f;	// limits max interpol. overshoot to 1.
		}
		ZXP(out) = cubicinterp(1.f - phase, a, b, c, d);
	)
	unit->mLevelA = a;
	unit->mLevelB = b;
	unit->mLevelC = c;
	unit->mLevelD = d;
	unit->mPhase = phase;
	RPUT
}

void LFDNoise3_Ctor(LFDNoise3* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFDNoise3_next);
	} else {
		SETCALC(LFDNoise3_next_k);
	}
	
	RGET
	unit->mPhase = 0.f;
	unit->mLevelA = frand2(s1, s2, s3) * 0.8f;	// limits max interpol. overshoot to 1.
	unit->mLevelB = frand2(s1, s2, s3) * 0.8f;
	unit->mLevelC = frand2(s1, s2, s3) * 0.8f;
	unit->mLevelD = frand2(s1, s2, s3) * 0.8f;
	RPUT
	
	LFDNoise3_next(unit, 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
void LFDClipNoise_next(LFDClipNoise *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float *freq = ZIN(0);
	float level = unit->mLevel;
	float phase = unit->mPhase;
	float smpdur = SAMPLEDUR;	
	RGET

	LOOP(inNumSamples,
		phase -= ZXP(freq) * smpdur;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			level = fcoin(s1,s2,s3);
		}
		ZXP(out) = level;
	)
	
	unit->mLevel = level;
	unit->mPhase = phase;
	RPUT
		
}

void LFDClipNoise_next_k(LFDClipNoise *unit, int inNumSamples)
{
	float *out = ZOUT(0);
	float freq = ZIN0(0);
	float level = unit->mLevel;
	float phase = unit->mPhase;
	float smpdur = SAMPLEDUR;
	float dphase = smpdur * freq;
	
	RGET
	
	LOOP(inNumSamples,
		phase -= dphase;
		if (phase <= 0) {
			phase = sc_wrap(phase, 0.f, 1.f);
			level = fcoin(s1,s2,s3);
		}
		ZXP(out) = level;
	)
	unit->mLevel = level;
	unit->mPhase = phase;
	RPUT
		
}

void LFDClipNoise_Ctor(LFDClipNoise* unit)
{
	if (INRATE(0) == calc_FullRate) {
		SETCALC(LFDClipNoise_next);
	} else {
		SETCALC(LFDClipNoise_next_k);
	}

	unit->mPhase = 0.f; 
	unit->mLevel = 0.f; 
	
	LFDClipNoise_next(unit, 1);
}





////////////////////////////////////////////////////////////////////////////////////////////////////////


void load(InterfaceTable *inTable)
{
	ft = inTable;

	DefineSimpleUnit(LFDNoise0);
	DefineSimpleUnit(LFDNoise1);
	DefineSimpleUnit(LFDNoise3);
	DefineSimpleUnit(LFDClipNoise);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
