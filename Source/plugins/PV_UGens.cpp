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


#include "FFT_UGens.h"


struct PV_OutOfPlace : Unit
{
	int m_numbins;
	float *m_tempbuf;
};

struct PV_MagSmear : PV_OutOfPlace
{
};

struct PV_MagShift : PV_OutOfPlace
{
};

struct PV_BinShift : PV_OutOfPlace
{
};

struct PV_Diffuser : Unit
{
	int m_numbins;
	float m_prevtrig, *m_shift;
	bool m_triggered;
};

struct PV_MagFreeze : Unit
{
	int m_numbins;
	float *m_mags, m_dc, m_nyq;
};

struct PV_RandWipe : Unit
{
	int *m_ordering, m_numbins;
	float m_prevtrig;
	bool m_triggered;
};

struct PV_RandComb : Unit
{
	int *m_ordering, m_numbins;
	float m_prevtrig;
	bool m_triggered;
};

struct PV_BinScramble : Unit
{
	int *m_from, *m_to, m_numbins;
	float m_prevtrig;
	float *m_tempbuf;
	bool m_triggered;
};

//////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{
	void FFT_MaxSize(sc_msg_iter *msg);

	void PV_PhaseShift_Ctor(PV_Unit *unit);
	void PV_PhaseShift_next(PV_Unit *unit, int inNumSamples);

	void PV_PhaseShift90_Ctor(PV_Unit *unit);
	void PV_PhaseShift90_next(PV_Unit *unit, int inNumSamples);

	void PV_PhaseShift270_Ctor(PV_Unit *unit);
	void PV_PhaseShift270_next(PV_Unit *unit, int inNumSamples);

	void PV_MagClip_Ctor(PV_Unit *unit);
	void PV_MagClip_next(PV_Unit *unit, int inNumSamples);

	void PV_MagAbove_Ctor(PV_Unit *unit);
	void PV_MagAbove_next(PV_Unit *unit, int inNumSamples);

	void PV_MagBelow_Ctor(PV_Unit *unit);
	void PV_MagBelow_next(PV_Unit *unit, int inNumSamples);

	void PV_Min_Ctor(PV_Unit *unit);
	void PV_Min_next(PV_Unit *unit, int inNumSamples);

	void PV_Max_Ctor(PV_Unit *unit);
	void PV_Max_next(PV_Unit *unit, int inNumSamples);

	void PV_Mul_Ctor(PV_Unit *unit);
	void PV_Mul_next(PV_Unit *unit, int inNumSamples);

	void PV_Add_Ctor(PV_Unit *unit);
	void PV_Add_next(PV_Unit *unit, int inNumSamples);

	void PV_RectComb_Ctor(PV_Unit *unit);
	void PV_RectComb_next(PV_Unit *unit, int inNumSamples);

	void PV_RectComb2_Ctor(PV_Unit *unit);
	void PV_RectComb2_next(PV_Unit *unit, int inNumSamples);

	void PV_MagSquared_Ctor(PV_Unit *unit);
	void PV_MagSquared_next(PV_Unit *unit, int inNumSamples);

	void PV_MagMul_Ctor(PV_Unit *unit);
	void PV_MagMul_next(PV_Unit *unit, int inNumSamples);

	void PV_MagDiv_Ctor(PV_Unit *unit);
	void PV_MagDiv_next(PV_Unit *unit, int inNumSamples);

	void PV_Copy_Ctor(PV_Unit *unit);
	void PV_Copy_next(PV_Unit *unit, int inNumSamples);

	void PV_CopyPhase_Ctor(PV_Unit *unit);
	void PV_CopyPhase_next(PV_Unit *unit, int inNumSamples);

	void PV_MagSmear_Ctor(PV_MagSmear *unit);
	void PV_MagSmear_Dtor(PV_MagSmear *unit);
	void PV_MagSmear_next(PV_MagSmear *unit, int inNumSamples);

	void PV_BinShift_Ctor(PV_BinShift *unit);
	void PV_BinShift_Dtor(PV_BinShift *unit);
	void PV_BinShift_next(PV_BinShift *unit, int inNumSamples);

	void PV_MagShift_Ctor(PV_MagShift *unit);
	void PV_MagShift_Dtor(PV_MagShift *unit);
	void PV_MagShift_next(PV_MagShift *unit, int inNumSamples);

	void PV_MagNoise_Ctor(PV_Unit *unit);
	void PV_MagNoise_next(PV_Unit *unit, int inNumSamples);

	void PV_BrickWall_Ctor(PV_Unit *unit);
	void PV_BrickWall_next(PV_Unit *unit, int inNumSamples);

	void PV_BinWipe_Ctor(PV_Unit *unit);
	void PV_BinWipe_next(PV_Unit *unit, int inNumSamples);

	void PV_LocalMax_Ctor(PV_Unit *unit);
	void PV_LocalMax_next(PV_Unit *unit, int inNumSamples);

	void PV_RandComb_Ctor(PV_RandComb *unit);
	void PV_RandComb_Dtor(PV_RandComb *unit);
	void PV_RandComb_next(PV_RandComb *unit, int inNumSamples);

	void PV_RandWipe_Ctor(PV_RandWipe *unit);
	void PV_RandWipe_Dtor(PV_RandWipe *unit);
	void PV_RandWipe_next(PV_RandWipe *unit, int inNumSamples);

	void PV_Diffuser_Ctor(PV_Diffuser *unit);
	void PV_Diffuser_Dtor(PV_Diffuser *unit);
	void PV_Diffuser_next(PV_Diffuser *unit, int inNumSamples);

	void PV_MagFreeze_Ctor(PV_MagFreeze *unit);
	void PV_MagFreeze_Dtor(PV_MagFreeze *unit);
	void PV_MagFreeze_next(PV_MagFreeze *unit, int inNumSamples);

	void PV_BinScramble_Ctor(PV_BinScramble *unit);
	void PV_BinScramble_Dtor(PV_BinScramble *unit);
	void PV_BinScramble_next(PV_BinScramble *unit, int inNumSamples);

	
/* spectral feature extractors? :
		bin freq
		bin magnitude
		bin phase
		bin laden ;-}
		average magnitude over a range of bins
		max magnitude over a range of bins
		max magnitude bin freq
		
		
*/

}

//////////////////////////////////////////////////////////////////////////////////////////////////


void FFT_MaxSize(sc_msg_iter *msg)
{
}


//SCPolarBuf* ToPolarApx(SndBuf *buf);
/*
SCPolarBuf* ToPolarApx(SndBuf *buf)
{
	if (buf->coord == coord_Complex) {
		SCComplexBuf* p = (SCComplexBuf*)buf->data;
		int numbins = buf->samples - 2 >> 1;
		for (int i=0; i<numbins; ++i) {
			p->bin[i].ToPolarApxInPlace();
		}
		buf->coord = coord_Polar;
	}
	return (SCPolarBuf*)buf->data;
}
*/
//SCComplexBuf* ToComplexApx(SndBuf *buf);
/*
SCComplexBuf* ToComplexApx(SndBuf *buf)
{
	if (buf->coord == coord_Polar) {
		SCPolarBuf* p = (SCPolarBuf*)buf->data;
		int numbins = buf->samples - 2 >> 1;
		for (int i=0; i<numbins; ++i) {
			p->bin[i].ToComplexApxInPlace();
		}
		buf->coord = coord_Complex;
	}
	return (SCComplexBuf*)buf->data;
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////

void PV_MagAbove_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF

	SCPolarBuf *p = ToPolarApx(buf);
	
	float thresh = ZIN0(1);
	for (int i=0; i<numbins; ++i) {
		float mag = p->bin[i].mag;
		if (mag < thresh) p->bin[i].mag = 0.;
	}
}

void PV_MagAbove_Ctor(PV_Unit *unit)
{
	SETCALC(PV_MagAbove_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_MagBelow_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCPolarBuf *p = ToPolarApx(buf);
	
	float thresh = ZIN0(1);
	
	for (int i=0; i<numbins; ++i) {
		float mag = p->bin[i].mag;
		if (mag > thresh) p->bin[i].mag = 0.;
	}
}

void PV_MagBelow_Ctor(PV_Unit *unit)
{
	SETCALC(PV_MagBelow_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_MagClip_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCPolarBuf *p = ToPolarApx(buf);
	
	float thresh = ZIN0(1);
	
	for (int i=0; i<numbins; ++i) {
		float mag = p->bin[i].mag;
		if (mag > thresh) p->bin[i].mag = thresh;
	}
}

void PV_MagClip_Ctor(PV_Unit *unit)
{
	SETCALC(PV_MagClip_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_LocalMax_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCPolarBuf *p = ToPolarApx(buf);
	
	float thresh = ZIN0(1);
	
	for (int i=1; i<numbins-1; ++i) {
		float mag = p->bin[i].mag;
		if (mag < thresh || mag < p->bin[i-1].mag || mag < p->bin[i+1].mag) {
			p->bin[i].mag = 0.;
		}
	}
}

void PV_LocalMax_Ctor(PV_Unit *unit)
{
	SETCALC(PV_LocalMax_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_MagSmear_next(PV_MagSmear *unit, int inNumSamples)
{
	PV_GET_BUF
	MAKE_TEMP_BUF
		
	SCPolarBuf *p = ToPolarApx(buf);
	SCPolarBuf *q = (SCPolarBuf*)unit->m_tempbuf;
	
	int width = (int)ZIN0(1);
	width = sc_clip(width, 0, numbins-1);
	float scale = 1. / (2*width+1);

	q->dc = p->dc;
	q->nyq = p->nyq;
	for (int j=0; j<numbins; j++) {
		float sum = 0.0;
		for (int pos = j-width; pos <= j+width; pos++) {
			if (pos >= 0 && pos < numbins) {
				sum += p->bin[pos].mag;
			}
		}
		q->bin[j].Set( sum * scale, p->bin[j].phase );
	}
	for (int i=0; i<numbins; i++) {
		p->bin[i] = q->bin[i];
	}
}

void PV_MagSmear_Ctor(PV_MagSmear *unit)
{
	SETCALC(PV_MagSmear_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_tempbuf = 0;
}

void PV_MagSmear_Dtor(PV_MagSmear *unit)
{
	RTFree(unit->mWorld, unit->m_tempbuf);
}

void PV_BinShift_next(PV_BinShift *unit, int inNumSamples)
{
	PV_GET_BUF
	MAKE_TEMP_BUF

	// get shift and stretch params
	float stretch = ZIN0(1);
	float shift = ZIN0(2);
	
	SCComplexBuf *p = ToComplexApx(buf);
	SCComplexBuf *q = (SCComplexBuf*)unit->m_tempbuf;
	
	// initialize output buf to zeroes
	for (int i=0; i<numbins; ++i) {
		q->bin[i] = 0.;
	}
	
	float fpos;
        int i;
	q->dc = p->dc;
	q->nyq = p->nyq;
	for (i=0, fpos = shift+stretch; i < numbins; ++i, fpos += stretch) {
		int32 pos = (int32)(fpos + 0.5);
		if (pos >= 0 && pos < numbins) {
			q->bin[pos] += p->bin[i];
		}
	}
	memcpy(p->bin, q->bin, numbins * sizeof(SCComplex));

}

void PV_BinShift_Ctor(PV_BinShift *unit)
{
	SETCALC(PV_BinShift_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_tempbuf = 0;
}

void PV_BinShift_Dtor(PV_BinShift *unit)
{
	RTFree(unit->mWorld, unit->m_tempbuf);
}

void PV_MagShift_next(PV_MagShift *unit, int inNumSamples)
{
	PV_GET_BUF
	MAKE_TEMP_BUF

	// get shift and stretch params
	float stretch = ZIN0(1);
	float shift = ZIN0(2);
	
	SCPolarBuf *p = ToPolarApx(buf);
	SCPolarBuf *q = (SCPolarBuf*)unit->m_tempbuf;
	
	// initialize output buf to zeroes
	for (int i=0; i<numbins; ++i) {
		q->bin[i].mag = 0.;
		q->bin[i].phase = p->bin[i].phase;
	}
	
	float fpos;
        int i;
	q->dc = p->dc;
	q->nyq = p->nyq;
	for (i=0, fpos = shift+stretch; i < numbins; ++i, fpos += stretch) {
		int32 pos = (int32)(fpos + 0.5);
		if (pos >= 0 && pos < numbins) {
			q->bin[pos].mag += p->bin[i].mag;
		}
	}
	memcpy(p->bin, q->bin, numbins * sizeof(SCComplex));
}

void PV_MagShift_Ctor(PV_MagShift *unit)
{
	SETCALC(PV_MagShift_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_tempbuf = 0;
}

void PV_MagShift_Dtor(PV_MagShift *unit)
{
	RTFree(unit->mWorld, unit->m_tempbuf);
}

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


void PV_MagNoise_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF

	RGET
	if (buf->coord == coord_Complex) {
		SCComplexBuf *p = (SCComplexBuf*)buf->data;
		for (int i=0; i<numbins; ++i) {
			float r = frand2(s1, s2, s3);
			p->bin[i].real *= r;
			p->bin[i].imag *= r;
		}
	} else {
		SCPolarBuf *p = (SCPolarBuf*)buf->data;
		for (int i=0; i<numbins; ++i) {
			float r = frand2(s1, s2, s3);
			p->bin[i].mag *= r;
		}
	}
	RPUT
}

void PV_MagNoise_Ctor(PV_Unit *unit)
{
	SETCALC(PV_MagNoise_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_PhaseShift_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCPolarBuf *p = ToPolarApx(buf);
	
	float shift = ZIN0(1);
	
	for (int i=0; i<numbins; ++i) {
		p->bin[i].phase += shift;
	}
}

void PV_PhaseShift_Ctor(PV_Unit *unit)
{
	SETCALC(PV_PhaseShift_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_PhaseShift90_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCComplexBuf *p = ToComplexApx(buf);
		
	for (int i=0; i<numbins; ++i) {
		float temp = p->bin[i].real;
		p->bin[i].real = -p->bin[i].imag;
		p->bin[i].imag = temp;
	}
}

void PV_PhaseShift90_Ctor(PV_Unit *unit)
{
	SETCALC(PV_PhaseShift90_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_PhaseShift270_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCComplexBuf *p = ToComplexApx(buf);
		
	for (int i=0; i<numbins; ++i) {
		float temp = p->bin[i].real;
		p->bin[i].real = p->bin[i].imag;
		p->bin[i].imag = -temp;
	}
}

void PV_PhaseShift270_Ctor(PV_Unit *unit)
{
	SETCALC(PV_PhaseShift270_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_MagSquared_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCPolarBuf *p = ToPolarApx(buf);
	
	for (int i=0; i<numbins; ++i) {
		float mag = p->bin[i].mag;
		p->bin[i].mag = mag * mag;
	}
}

void PV_MagSquared_Ctor(PV_Unit *unit)
{
	SETCALC(PV_MagSquared_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_BrickWall_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF
	
	SCComplexBuf *p = (SCComplexBuf*)buf->data;

	int wipe = (int)(ZIN0(1) * numbins);
	if (wipe > 0) {
		wipe = sc_min(wipe, numbins);
		for (int i=0; i < wipe; ++i) {
			p->bin[i] = 0.;
		}	
	} else if (wipe < 0) {
		wipe = sc_max(wipe, -numbins);
		for (int i=numbins+wipe; i < numbins; ++i) {
			p->bin[i] = 0.;
		}	
	}
}

void PV_BrickWall_Ctor(PV_Unit *unit)
{
	SETCALC(PV_BrickWall_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_BinWipe_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCComplexBuf *p = (SCComplexBuf*)buf1->data;
	SCComplexBuf *q = (SCComplexBuf*)buf2->data;

	int wipe = (int)(ZIN0(2) * numbins);
	if (wipe > 0) {
		wipe = sc_min(wipe, numbins);
		for (int i=0; i < wipe; ++i) {
			p->bin[i] = q->bin[i];
		}	
	} else if (wipe < 0) {
		wipe = sc_max(wipe, -numbins);
		for (int i=numbins+wipe; i < numbins; ++i) {
			p->bin[i] = q->bin[i];
		}	
	}
}

void PV_BinWipe_Ctor(PV_Unit *unit)
{
	SETCALC(PV_BinWipe_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_MagMul_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCPolarBuf *p = ToPolarApx(buf1);
	SCPolarBuf *q = ToPolarApx(buf2);
	
	p->dc *= q->dc;
	p->nyq *= q->nyq;
	for (int i=0; i<numbins; ++i) {
		p->bin[i].mag *= q->bin[i].mag;
	}
}

void PV_MagMul_Ctor(PV_Unit *unit)
{
	SETCALC(PV_MagMul_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_MagDiv_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCPolarBuf *p = ToPolarApx(buf1);
	SCPolarBuf *q = ToPolarApx(buf2);
	
	float zeroed = ZIN0(2);
	
	p->dc /= sc_max(q->dc, zeroed);
	p->nyq /= sc_max(q->nyq, zeroed);
	for (int i=0; i<numbins; ++i) {
		p->bin[i].mag /= sc_max(q->bin[i].mag, zeroed);
	}
}

void PV_MagDiv_Ctor(PV_Unit *unit)
{
	SETCALC(PV_MagDiv_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_Copy_next(PV_Unit *unit, int inNumSamples)
{
	
	float fbufnum1 = ZIN0(0);
	float fbufnum2 = ZIN0(1);
	if (fbufnum1 < 0.f || fbufnum2 < 0.f) { ZOUT0(0) = -1.f; return; }
	ZOUT0(0) = fbufnum2;
	uint32 ibufnum1 = (int)fbufnum1;
	uint32 ibufnum2 = (int)fbufnum2;
	World *world = unit->mWorld;
	if (ibufnum1 >= world->mNumSndBufs) ibufnum1 = 0;
	if (ibufnum2 >= world->mNumSndBufs) ibufnum2 = 0;
	SndBuf *buf1 = world->mSndBufs + ibufnum1;
	SndBuf *buf2 = world->mSndBufs + ibufnum2;
	if (buf1->samples != buf2->samples) return;
	
	// copy to buf2
	
	buf2->coord = buf1->coord;
	memcpy(buf2->data, buf1->data, buf1->samples * sizeof(float));
	
}

void PV_Copy_Ctor(PV_Unit *unit)
{
	SETCALC(PV_Copy_next);
	ZOUT0(0) = ZIN0(1);
}

void PV_CopyPhase_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCPolarBuf *p = ToPolarApx(buf1);
	SCPolarBuf *q = ToPolarApx(buf2);
	
	p->dc *= q->dc;
	p->nyq *= q->nyq;
	for (int i=0; i<numbins; ++i) {
		p->bin[i].phase = q->bin[i].phase;
	}
}

void PV_CopyPhase_Ctor(PV_Unit *unit)
{
	SETCALC(PV_CopyPhase_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_Mul_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCComplexBuf *p = ToComplexApx(buf1);
	SCComplexBuf *q = ToComplexApx(buf2);
	
	p->dc *= q->dc;
	p->nyq *= q->nyq;
	for (int i=0; i<numbins; ++i) {
		float preal = p->bin[i].real;
		p->bin[i].real = (preal * q->bin[i].real) - (p->bin[i].imag * q->bin[i].imag);
		p->bin[i].imag = (preal * q->bin[i].imag) + (p->bin[i].imag * q->bin[i].real);
	}
}

void PV_Mul_Ctor(PV_Unit *unit)
{
	SETCALC(PV_Mul_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_Add_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCComplexBuf *p = ToComplexApx(buf1);
	SCComplexBuf *q = ToComplexApx(buf2);
	
	p->dc += q->dc;
	p->nyq += q->nyq;
	for (int i=0; i<numbins; ++i) {
		p->bin[i].real += q->bin[i].real;
		p->bin[i].imag += q->bin[i].imag;
	}
}

void PV_Add_Ctor(PV_Unit *unit)
{
	SETCALC(PV_Add_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_Max_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCPolarBuf *p = ToPolarApx(buf1);
	SCPolarBuf *q = ToPolarApx(buf2);
	
	for (int i=0; i<numbins; ++i) {
		if (q->bin[i].mag > p->bin[i].mag) {
			p->bin[i] = q->bin[i];
		}
	}
}

void PV_Max_Ctor(PV_Unit *unit)
{
	SETCALC(PV_Max_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_Min_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2
	
	SCPolarBuf *p = ToPolarApx(buf1);
	SCPolarBuf *q = ToPolarApx(buf2);
	
	for (int i=0; i<numbins; ++i) {
		if (q->bin[i].mag < p->bin[i].mag) {
			p->bin[i] = q->bin[i];
		}
	}
}

void PV_Min_Ctor(PV_Unit *unit)
{
	SETCALC(PV_Min_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_RectComb_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF

	float numTeeth = ZIN0(1);
	float phase = ZIN0(2);
	float width = ZIN0(3);
	float freq = numTeeth / (numbins + 1);
	
	SCComplexBuf *p = (SCComplexBuf*)buf->data;
	
	if (phase > width) p->dc = 0.f;
		phase += freq;
		if (phase >= 1.f) phase -= 1.f;
		else if (phase < 0.f) phase += 1.f;

	for (int i=0; i < numbins; ++i) {
		if (phase > width) p->bin[i] = 0.f;
		phase += freq;
		if (phase >= 1.f) phase -= 1.f;
		else if (phase < 0.f) phase += 1.f;
	}	

	if (phase > width) p->nyq = 0.f;

}

void PV_RectComb_Ctor(PV_Unit *unit)
{
	SETCALC(PV_RectComb_next);
	ZOUT0(0) = ZIN0(0);
}

void PV_RectComb2_next(PV_Unit *unit, int inNumSamples)
{
	PV_GET_BUF2

	float numTeeth = ZIN0(2);
	float phase = ZIN0(3);
	float width = ZIN0(4);
	float freq = numTeeth / (numbins + 1);

	SCComplexBuf *p = (SCComplexBuf*)buf1->data;
	SCComplexBuf *q = (SCComplexBuf*)buf2->data;
	
	if (phase > width) p->dc = q->dc;
		phase += freq;
		if (phase >= 1.f) phase -= 1.f;
		else if (phase < 0.f) phase += 1.f;

	for (int i=0; i < numbins; ++i) {
		if (phase > width) p->bin[i] = q->bin[i];
		phase += freq;
		if (phase >= 1.f) phase -= 1.f;
		else if (phase < 0.f) phase += 1.f;
	}	

	if (phase > width) p->nyq = q->nyq;

}

void PV_RectComb2_Ctor(PV_Unit *unit)
{
	SETCALC(PV_RectComb2_next);
	ZOUT0(0) = ZIN0(0);
}


void PV_RandComb_choose(PV_RandComb* unit);
void PV_RandComb_choose(PV_RandComb* unit)
{
	int numbins = unit->m_numbins;
	for (int i=0; i<numbins; ++i) {
		unit->m_ordering[i] = i;
	}
	RGET
	for (int i=0; i<numbins; ++i) {
		int32 j = (int32)(frand(s1,s2,s3) * (numbins - i));
		int32 temp = unit->m_ordering[i];
		unit->m_ordering[i] = unit->m_ordering[j];
		unit->m_ordering[j] = temp;
	}
	RPUT
}

void PV_RandComb_next(PV_RandComb *unit, int inNumSamples)
{
	float trig = ZIN0(2);
	if (trig > 0.f && unit->m_prevtrig <= 0.f) unit->m_triggered = true;
	unit->m_prevtrig = trig;
	
	PV_GET_BUF
	
	if (!unit->m_ordering) {
		unit->m_ordering = (int*)RTAlloc(unit->mWorld, numbins * sizeof(int));
		unit->m_numbins = numbins;
		PV_RandComb_choose(unit);
	} else {
		if (numbins != unit->m_numbins) return;
		if (unit->m_triggered) {
			unit->m_triggered = false;
			PV_RandComb_choose(unit);
		}
	}

	int n = (int)(ZIN0(1) * numbins);
	n = sc_clip(n, 0, numbins);
	
	SCComplexBuf *p = (SCComplexBuf*)buf->data;
	
	int *ordering = unit->m_ordering;
	for (int i=0; i<n; ++i) {
		p->bin[ordering[i]] = 0.;
	}
	
}


void PV_RandComb_Ctor(PV_RandComb* unit)
{
	SETCALC(PV_RandComb_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_ordering = 0;
	unit->m_prevtrig = 0.f;
	unit->m_triggered = false;
}

void PV_RandComb_Dtor(PV_RandComb* unit)
{
	RTFree(unit->mWorld, unit->m_ordering);
}

//////////////////////

void PV_RandWipe_choose(PV_RandWipe* unit);
void PV_RandWipe_choose(PV_RandWipe* unit)
{
	int numbins = unit->m_numbins;
	for (int i=0; i<numbins; ++i) {
		unit->m_ordering[i] = i;
	}
	RGET
	for (int i=0; i<numbins; ++i) {
		int32 j = (int32)(frand(s1,s2,s3) * (numbins - i));
		int32 temp = unit->m_ordering[i];
		unit->m_ordering[i] = unit->m_ordering[j];
		unit->m_ordering[j] = temp;
	}
	RPUT
}

void PV_RandWipe_next(PV_RandWipe *unit, int inNumSamples)
{
	float trig = ZIN0(3);
	if (trig > 0.f && unit->m_prevtrig <= 0.f) unit->m_triggered = true;
	unit->m_prevtrig = trig;

	PV_GET_BUF2
	
	if (!unit->m_ordering) {
		unit->m_ordering = (int*)RTAlloc(unit->mWorld, numbins * sizeof(int));
		unit->m_numbins = numbins;
		PV_RandWipe_choose(unit);
	} else {
		if (numbins != unit->m_numbins) return;
		if (unit->m_triggered) {
			unit->m_triggered = false;
			PV_RandWipe_choose(unit);
		}
	}

	int n = (int)(ZIN0(2) * numbins);
	n = sc_clip(n, 0, numbins);
	
	SCComplexBuf *p = (SCComplexBuf*)buf1->data;
	SCComplexBuf *q = (SCComplexBuf*)buf2->data;
	
	int *ordering = unit->m_ordering;
	for (int i=0; i<n; ++i) {
		p->bin[ordering[i]] = q->bin[ordering[i]];
	}
	
}


void PV_RandWipe_Ctor(PV_RandWipe* unit)
{
	SETCALC(PV_RandWipe_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_ordering = 0;
	unit->m_prevtrig = 0.f;
	unit->m_triggered = false;
}

void PV_RandWipe_Dtor(PV_RandWipe* unit)
{
	RTFree(unit->mWorld, unit->m_ordering);
}

//////////////////////

void PV_Diffuser_choose(PV_Diffuser* unit);
void PV_Diffuser_choose(PV_Diffuser* unit)
{
	RGET
	for (int i=0; i<unit->m_numbins; ++i) {
		unit->m_shift[i] = frand(s1,s2,s3) * twopi;
	}
	RPUT
}

void PV_Diffuser_next(PV_Diffuser *unit, int inNumSamples)
{
	float trig = ZIN0(1);
	if (trig > 0.f && unit->m_prevtrig <= 0.f) unit->m_triggered = true;
	unit->m_prevtrig = trig;

	PV_GET_BUF
	
	if (!unit->m_shift) {
		unit->m_shift = (float*)RTAlloc(unit->mWorld, numbins * sizeof(float));
		unit->m_numbins = numbins;
		PV_Diffuser_choose(unit);
	} else {
		if (numbins != unit->m_numbins) return;
		if (unit->m_triggered) {
			unit->m_triggered = false;
			PV_Diffuser_choose(unit);
		}
	}

	int n = (int)(ZIN0(1) * numbins);
	n = sc_clip(n, 0, numbins);
	
	SCPolarBuf *p = ToPolarApx(buf);
	
	float *shift = unit->m_shift;
	for (int i=0; i<n; ++i) {
		p->bin[i].phase += shift[i];
	}
	
}


void PV_Diffuser_Ctor(PV_Diffuser* unit)
{
	SETCALC(PV_Diffuser_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_shift = 0;
	unit->m_prevtrig = 0.f;
	unit->m_triggered = false;
}

void PV_Diffuser_Dtor(PV_Diffuser* unit)
{
	RTFree(unit->mWorld, unit->m_shift);
}

//////////////////////

void PV_MagFreeze_next(PV_MagFreeze *unit, int inNumSamples)
{
	PV_GET_BUF
	
	if (!unit->m_mags) {
		unit->m_mags = (float*)RTAlloc(unit->mWorld, numbins * sizeof(float));
		unit->m_numbins = numbins;
	} else if (numbins != unit->m_numbins) return;

	SCPolarBuf *p = ToPolarApx(buf);

	float freeze = ZIN0(1);
	float *mags = unit->m_mags;
	if (freeze > 0.f) {
		for (int i=0; i<numbins; ++i) {
			p->bin[i].mag = mags[i];
		}
		p->dc = unit->m_dc;
		p->nyq = unit->m_nyq;
	} else {
		for (int i=0; i<numbins; ++i) {
			mags[i] = p->bin[i].mag;
		}
		unit->m_dc = p->dc;
		unit->m_nyq = p->nyq;
	}
}


void PV_MagFreeze_Ctor(PV_MagFreeze* unit)
{
	SETCALC(PV_MagFreeze_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_mags = 0;
}

void PV_MagFreeze_Dtor(PV_MagFreeze* unit)
{
	RTFree(unit->mWorld, unit->m_mags);
}

//////////////////////

void PV_BinScramble_choose(PV_BinScramble* unit);
void PV_BinScramble_choose(PV_BinScramble* unit)
{

	int numbins = unit->m_numbins;
	int *to = unit->m_to;
	int *from = unit->m_from;
	
	for (int i=0; i<numbins; ++i) {
		to[i] = i;
	}
	RGET
	for (int i=0; i<numbins; ++i) {
		int32 j = (int32)(frand(s1,s2,s3) * (numbins - i));
		int32 temp = to[i];
		to[i] = to[j];
		to[j] = temp;
	}

	int32 width = (int32)(ZIN0(2) * numbins);
	for (int i=0; i<numbins; ++i) {
		int32 k = to[i];
		int32 minr = sc_max(0, k-width);
		int32 maxr = sc_min(numbins-1, k+width);
		int32 j = (int32)(frand(s1,s2,s3) * (maxr - minr) + minr);
		from[i] = j;
	}
	RPUT
}

void PV_BinScramble_next(PV_BinScramble *unit, int inNumSamples)
{
	float trig = ZIN0(3);
	if (trig > 0.f && unit->m_prevtrig <= 0.f) unit->m_triggered = true;
	unit->m_prevtrig = trig;

	PV_GET_BUF
	
	if (!unit->m_to) {
		unit->m_to = (int*)RTAlloc(unit->mWorld, numbins * 2 * sizeof(int));
		unit->m_from = unit->m_to + numbins;
		unit->m_numbins = numbins;
		unit->m_tempbuf = (float*)RTAlloc(unit->mWorld, buf->samples * sizeof(float));
		PV_BinScramble_choose(unit);
	} else {
		if (numbins != unit->m_numbins) return;
		if (unit->m_triggered) {
			unit->m_triggered = false;
			PV_BinScramble_choose(unit);
		}
	}

	SCComplexBuf *p = (SCComplexBuf*)buf->data;
	SCComplexBuf *q = (SCComplexBuf*)unit->m_tempbuf;
	
	float wipe = ZIN0(1);
	int32 scrambleBins = (int32)(numbins * sc_clip(wipe, 0.f, 1.f));
	
	int *to = unit->m_to;
	int *from = unit->m_from;
	for (int j=0; j<scrambleBins; j++) {
		q->bin[to[j]] = p->bin[from[j]];
	}
	for (int j=scrambleBins; j<numbins; j++) {
		int32 a = to[j];
		q->bin[a] = p->bin[a];
	}
	q->dc = p->dc;
	q->nyq = p->nyq;
	memcpy(p->bin, q->bin, numbins * sizeof(SCComplex));
}


void PV_BinScramble_Ctor(PV_BinScramble* unit)
{
	SETCALC(PV_BinScramble_next);
	ZOUT0(0) = ZIN0(0);
	unit->m_to = 0;
	unit->m_prevtrig = 0.f;
	unit->m_triggered = false;
	unit->m_tempbuf = 0;
}

void PV_BinScramble_Dtor(PV_BinScramble* unit)
{
	RTFree(unit->mWorld, unit->m_to);
	RTFree(unit->mWorld, unit->m_tempbuf);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////

void init_SCComplex(InterfaceTable *inTable);

#define DefinePVUnit(name) \
	(*ft->fDefineUnit)(#name, sizeof(PV_Unit), (UnitCtorFunc)&name##_Ctor, 0, 0);
	

void initPV(InterfaceTable *inTable)
{	
	DefinePVUnit(PV_MagAbove);
	DefinePVUnit(PV_MagBelow);
	DefinePVUnit(PV_MagClip);
	DefinePVUnit(PV_MagMul);
	DefinePVUnit(PV_MagDiv);
	DefinePVUnit(PV_MagSquared);
	DefinePVUnit(PV_MagNoise);
	DefinePVUnit(PV_Copy);
	DefinePVUnit(PV_CopyPhase);
	DefinePVUnit(PV_PhaseShift);
	DefinePVUnit(PV_PhaseShift90);
	DefinePVUnit(PV_PhaseShift270);
	DefinePVUnit(PV_Min);
	DefinePVUnit(PV_Max);
	DefinePVUnit(PV_Mul);
	DefinePVUnit(PV_Add);
	DefinePVUnit(PV_RectComb);
	DefinePVUnit(PV_RectComb2);
	DefinePVUnit(PV_BrickWall);
	DefinePVUnit(PV_BinWipe);
	DefinePVUnit(PV_LocalMax);

	DefineDtorUnit(PV_BinScramble);
	DefineDtorUnit(PV_MagSmear);
	DefineDtorUnit(PV_MagShift);
	DefineDtorUnit(PV_BinShift);
	DefineDtorUnit(PV_RandWipe);
	DefineDtorUnit(PV_Diffuser);
	DefineDtorUnit(PV_RandComb);
	DefineDtorUnit(PV_MagFreeze);
}