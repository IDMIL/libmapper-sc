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


#include "SC_PlugIn.h"

static InterfaceTable *ft;

//////////////////////////////////////////////////////////////////////////////////////////////////

#define MULIN IN(1)
#define ADDIN IN(2)

struct MulAdd : public Unit
{
	float mPrevMul, mPrevAdd;
};

extern "C"
{
	void load(InterfaceTable *inTable);

	void MulAdd_Ctor(MulAdd *unit);

	// mul add functions for every occasion:
	void ampmix_k(MulAdd *unit, int inNumSamples);

	void ampmix_aa(MulAdd *unit, int inNumSamples); 
	void ampmix_ak(MulAdd *unit, int inNumSamples); 
	void ampmix_ai(MulAdd *unit, int inNumSamples);

	void ampmix_ka(MulAdd *unit, int inNumSamples);
	void ampmix_kk(MulAdd *unit, int inNumSamples);
	void ampmix_ki(MulAdd *unit, int inNumSamples);

	void ampmix_ia(MulAdd *unit, int inNumSamples);
	void ampmix_ik(MulAdd *unit, int inNumSamples);
	void ampmix_ii(MulAdd *unit, int inNumSamples);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////


void ampmix_k(MulAdd *unit, int inNumSamples);
void ampmix_k(MulAdd *unit, int inNumSamples)
{
	ZOUT0(0) = ZIN0(0) * MULIN[0] + ADDIN[0];
}

void ampmix_aa(MulAdd *unit, int inNumSamples);
void ampmix_aa(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	float *amp = MULIN - ZOFF;
	float *mix = ADDIN - ZOFF;

	LOOP(inNumSamples, ZXP(out) = ZXP(amp) * ZXP(in) + ZXP(mix); );
}

void ampmix_ak(MulAdd *unit, int inNumSamples);
void ampmix_ak(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	float *amp = MULIN - ZOFF;

	float mix_cur = unit->mPrevAdd;
	float nextMix = ADDIN[0];
	float mix_slope = CALCSLOPE(nextMix, mix_cur);
	if (mix_slope == 0.f) {
		if (mix_cur == 0.f) {
			LOOP(inNumSamples, ZXP(out) = ZXP(amp) * ZXP(in); );
		} else {
			LOOP(inNumSamples, ZXP(out) = ZXP(amp) * ZXP(in) + mix_cur; );
		}
	} else {
		LOOP(inNumSamples, ZXP(out) = ZXP(amp) * ZXP(in) + mix_cur; mix_cur += mix_slope; );
		unit->mPrevAdd = nextMix;
	}
}

void ampmix_ai(MulAdd *unit, int inNumSamples);
void ampmix_ai(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	float *amp = MULIN - ZOFF;
	float mix_cur = unit->mPrevAdd;
	
	LOOP(inNumSamples, ZXP(out) = ZXP(amp) * ZXP(in) + mix_cur; );
}

void ampmix_ka(MulAdd *unit, int inNumSamples);
void ampmix_ka(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	float *mix = ADDIN - ZOFF;
	float amp_cur = unit->mPrevMul;
	float nextAmp = MULIN[0];
	float amp_slope = CALCSLOPE(nextAmp, amp_cur);
	
	if (amp_slope == 0.f) {
		if (amp_cur == 0.f) {
			ZCopy(inNumSamples, out, mix);
		} else if (amp_cur == 1.f) {
			ZAccum(inNumSamples, out, mix);
		} else {
			LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + ZXP(mix); );
		}
	} else {
		LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + ZXP(mix); amp_cur += amp_slope; );
		unit->mPrevMul = nextAmp;
	}
}

void ampmix_kk(MulAdd *unit, int inNumSamples);
void ampmix_kk(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	
	float amp_cur = unit->mPrevMul;
	float nextAmp = MULIN[0];
	float amp_slope = CALCSLOPE(nextAmp, amp_cur);
	float mix_cur = unit->mPrevAdd;
	float nextMix = ADDIN[0];
	float mix_slope = CALCSLOPE(nextMix, mix_cur);
	
	if (amp_slope == 0.f) {
		if (mix_slope == 0.f) {
			if (mix_cur == 0.f) {
				if (amp_cur == 1.f) {
					// do nothing!
				} else if (amp_cur == 0.f) {
					ZClear(inNumSamples, out);
				} else {
					LOOP(inNumSamples, ZXP(out) = ZXP(in) * amp_cur;);
				}
			} else {
				if (amp_cur == 1.f) {
					LOOP(inNumSamples, ZXP(out) = ZXP(in) + mix_cur;);
				} else if (amp_cur == 0.f) {
					LOOP(inNumSamples, ZXP(out) = mix_cur;);
				} else {
					LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; );
				}
			}
		} else {
			if (amp_cur == 1.f) {
				LOOP(inNumSamples, ZXP(out) = ZXP(in) + mix_cur; mix_cur += mix_slope;);
			} else if (amp_cur == 0.f) {
				LOOP(inNumSamples, ZXP(out) = mix_cur; mix_cur += mix_slope;);
			} else {
				LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; mix_cur += mix_slope; );
			}
			unit->mPrevAdd = nextMix;
		}
	} else {
		if (mix_slope == 0.f) {
			if (mix_cur == 0.f) {
				LOOP(inNumSamples, ZXP(out) = ZXP(in) * amp_cur; amp_cur += amp_slope; );
			} else {
				LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; amp_cur += amp_slope; );
			}
		} else {
			LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; amp_cur += amp_slope; mix_cur += mix_slope; );
			unit->mPrevAdd = nextMix;
		}
		unit->mPrevMul = nextAmp;
	}
}

void ampmix_ki(MulAdd *unit, int inNumSamples);
void ampmix_ki(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	
	float amp_cur = unit->mPrevMul;
	float nextAmp = MULIN[0];
	float amp_slope = CALCSLOPE(nextAmp, amp_cur);
	float mix_cur = unit->mPrevAdd;
	//postbuf("ampmix_ki %08X %g %g\n", out, amp_cur, mix_cur);
	
	if (amp_slope == 0.f) {
		if (amp_cur == 1.f) {
			LOOP(inNumSamples, ZXP(out) = ZXP(in) + mix_cur;);
		} else if (amp_cur == 0.f) {
			LOOP(inNumSamples, ZXP(out) = mix_cur;);
		} else {
			LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; );
		}
	} else {
		LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; amp_cur += amp_slope; );
		unit->mPrevMul = nextAmp;
	}
}

void ampmix_ia(MulAdd *unit, int inNumSamples);
void ampmix_ia(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	float *mix = ADDIN - ZOFF;
	float amp_cur = unit->mPrevMul;
	
	LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + ZXP(mix); );
}

void ampmix_ik(MulAdd *unit, int inNumSamples);
void ampmix_ik(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);
	
	float amp_cur = unit->mPrevMul;
	float mix_cur = unit->mPrevAdd;
	float nextMix = ADDIN[0];
	float mix_slope = CALCSLOPE(nextMix, mix_cur);
	
	if (mix_slope == 0.f) {
		if (mix_cur == 0.f) {
			LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in); );
		} else {
			LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; );
		}
	} else {
		LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; mix_cur += mix_slope; );
		unit->mPrevAdd = nextMix;
	}
}

void ampmix_ii(MulAdd *unit, int inNumSamples);
void ampmix_ii(MulAdd *unit, int inNumSamples)
{
	float *in = ZIN(0);
	float *out = ZOUT(0);

	float amp_cur = unit->mPrevMul;
	float mix_cur = unit->mPrevAdd;
	LOOP(inNumSamples, ZXP(out) = amp_cur * ZXP(in) + mix_cur; );
}

#if __VEC__

void v_ampmix_aa(MulAdd *unit, int inNumSamples);
void v_ampmix_aa(MulAdd *unit, int inNumSamples)
{
	float *in = IN(0);
	float *out = OUT(0);
	float *amp = MULIN;
	float *mix = ADDIN;

	int i, len = BUFLENGTH << 2;
	vfloat32 *vin = (vfloat32*)in;
	vfloat32 *vout = (vfloat32*)out;
	vfloat32 *vamp = (vfloat32*)amp;
	//LOOP(inNumSamples, PZ(out); *out = ZXP(amp) * *out + ZXP(mix); ZP(out););
	vfloat32 *vmix = (vfloat32*)mix;
	for (i=0; i<len; i+=16) {
		vec_st(vec_madd(vec_ld(i, vamp), vec_ld(i, vin), vec_ld(i, vmix)), i, vout);
	}
}

void v_ampmix_ak(MulAdd *unit, int inNumSamples);
void v_ampmix_ak(MulAdd *unit, int inNumSamples)
{
	int i, len = BUFLENGTH << 2;

	float *in = IN(0);
	float *out = OUT(0);
	float *amp = MULIN;
	float nextMix = ADDIN[0];
	
	float mix_cur = unit->mPrevAdd;
	float mix_slope = CALCSLOPE(nextMix, mix_cur);
	if (mix_slope == 0.f) {
		if (mix_cur == 0.f) {
			//LOOP(inNumSamples, PZ(out); *out = ZXP(amp) * *out; ZP(out););
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 *vamp = (vfloat32*)amp;
			define_vzero 				
			for (i=0; i<len; i+=16) {
				vec_st(vec_mul(vec_ld(i, vamp), vec_ld(i, vin)), i, vout);
			}
		} else {
			//LOOP(inNumSamples, PZ(out); *out = ZXP(amp) * *out + mix_cur; ZP(out););
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 *vamp = (vfloat32*)amp;
			vfloat32 vmix = vload(mix_cur);
			for (i=0; i<len; i+=16) {
				vec_st(vec_madd(vec_ld(i, vamp), vec_ld(i, vin), vmix), i, vout);
			}
		}
	} else {
		//LOOP(inNumSamples, PZ(out); *out = ZXP(amp) * *out + mix_cur; mix_cur += mix_slope; ZP(out););
		vfloat32 *vin = (vfloat32*)in;
		vfloat32 *vout = (vfloat32*)out;
		vfloat32 *vamp = (vfloat32*)amp;
		vfloat32 vmix_slope = vload(4.f * mix_slope);
		vfloat32 vmix = vstart(mix_cur, vmix_slope);
		for (i=0; i<len; i+=16) {
			vec_st(vec_madd(vec_ld(i, vamp), vec_ld(i, vin), vmix), i, vout);
			vmix = vec_add(vmix, vmix_slope);
		}
		unit->mPrevAdd = nextMix;
	}
	
}

void v_ampmix_ai(MulAdd *unit, int inNumSamples);
void v_ampmix_ai(MulAdd *unit, int inNumSamples)
{
	int i, len = BUFLENGTH << 2;

	float *in = IN(0);
	float *out = OUT(0);
	
	float *amp = MULIN;

	float mix_cur = unit->mPrevAdd;
	//LOOP(inNumSamples, PZ(out); *out = ZXP(amp) * *out + mix_cur; ZP(out););
	vfloat32 *vin = (vfloat32*)in;
	vfloat32 *vout = (vfloat32*)out;
	vfloat32 *vamp = (vfloat32*)amp;
	vfloat32 vmix = vload(mix_cur);
	for (i=0; i<len; i+=16) {
		vec_st(vec_madd(vec_ld(i, vamp), vec_ld(i, vin), vmix), i, vout);
	}
}

void v_ampmix_ka(MulAdd *unit, int inNumSamples);
void v_ampmix_ka(MulAdd *unit, int inNumSamples)
{
	float amp_cur, amp_slope;
	int i, len = BUFLENGTH << 2;

	float *in = IN(0);
	float *out = OUT(0);
	float *mix = ADDIN;
	amp_cur = unit->mPrevMul;
	float nextAmp = MULIN[0];
	amp_slope = CALCSLOPE(nextAmp, amp_cur);

	if (amp_slope == 0.f) {
		if (amp_cur == 0.f) {
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 *vmix = (vfloat32*)mix;
			for (i=0; i<len; i+=16) {
				vec_st(vec_ld(i, vmix), i, vout);
			}
		} else if (amp_cur == 1.f) {
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 *vmix = (vfloat32*)mix;
			for (i=0; i<len; i+=16) {
				vec_st(vec_add(vec_ld(i, vin), vec_ld(i, vmix)), i, vout);
			}
		} else {
			//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + ZXP(mix); ZP(out););
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 vamp = vload(amp_cur);
			vfloat32 *vmix = (vfloat32*)mix;
			for (i=0; i<len; i+=16) {
				vec_st(vec_madd(vamp, vec_ld(i, vin), vec_ld(i, vmix)), i, vout);
			}
			//unit->mPrevMul = *amp;
		}
	} else {
		vfloat32 *vin = (vfloat32*)in;
		vfloat32 *vout = (vfloat32*)out;
		vfloat32 vamp_slope = vload(4.f * amp_slope);
		vfloat32 vamp = vstart(amp_cur, vamp_slope);
		vfloat32 *vmix = (vfloat32*)mix;
		for (i=0; i<len; i+=16) {
			vec_st(vec_madd(vamp, vec_ld(i, vin), vec_ld(i, vmix)), i, vout);
			vamp = vec_add(vamp, vamp_slope);
		}
		unit->mPrevMul = nextAmp;
	}

}

void v_ampmix_kk(MulAdd *unit, int inNumSamples);
void v_ampmix_kk(MulAdd *unit, int inNumSamples)
{
	float amp_cur, mix_cur, amp_slope, mix_slope;
	int i, len = BUFLENGTH << 2;
	define_vzero

	float *in = IN(0);
	float *out = OUT(0);

	amp_cur = unit->mPrevMul;
	float nextAmp = MULIN[0];
	amp_slope = CALCSLOPE(nextAmp, amp_cur);
	mix_cur = unit->mPrevAdd;
	float nextMix = ADDIN[0];
	mix_slope = CALCSLOPE(nextMix, mix_cur);

	if (amp_slope == 0.f) {
		if (mix_slope == 0.f) {
			if (mix_cur == 0.f) {
				if (amp_cur == 1.f) {
					// do nothing!
				} else if (amp_cur == 0.f) {
					vfloat32 *vout = (vfloat32*)out;
					for (i=0; i<len; i+=16) {
						vec_st(vzero, i, vout);
					}
				} else {
					//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out; ZP(out););
					vfloat32 *vin = (vfloat32*)in;
					vfloat32 *vout = (vfloat32*)out;
					vfloat32 vamp = vload(amp_cur);
					for (i=0; i<len; i+=16) {
						vec_st(vec_mul(vamp, vec_ld(i, vin)), i, vout);
					}
				}
			} else {
				if (amp_cur == 1.f) {
					//LOOP(inNumSamples, ZXP(out) += mix_cur;);
					vfloat32 *vin = (vfloat32*)in;
					vfloat32 *vout = (vfloat32*)out;
					vfloat32 vmix = vload(mix_cur);
					for (i=0; i<len; i+=16) {
						vec_st(vec_add(vmix, vec_ld(i, vin)), i, vout);
					}
				} else if (amp_cur == 0.f) {
					//LOOP(inNumSamples, ZXP(out) = mix_cur;);
					vfloat32 *vout = (vfloat32*)out;
					vfloat32 vmix = vload(mix_cur);
					for (i=0; i<len; i+=16) {
						vec_st(vmix, i, vout);
					}
				} else {
					//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; ZP(out););
					vfloat32 *vin = (vfloat32*)in;
					vfloat32 *vout = (vfloat32*)out;
					vfloat32 vamp = vload(amp_cur);
					vfloat32 vmix = vload(mix_cur);
					for (i=0; i<len; i+=16) {
						vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
					}
				}
			}
		} else {
			if (amp_cur == 1.f) {
				//LOOP(inNumSamples, ZXP(out) += mix_cur; mix_cur += mix_slope;);
				vfloat32 *vin = (vfloat32*)in;
				vfloat32 *vout = (vfloat32*)out;
				vfloat32 vmix_slope = vload(4.f * mix_slope);
				vfloat32 vmix = vstart(mix_cur, vmix_slope);
				for (i=0; i<len; i+=16) {
					vec_st(vec_add(vmix, vec_ld(i, vin)), i, vout);
					vmix = vec_add(vmix, vmix_slope);
				}
			} else if (amp_cur == 0.f) {
				//LOOP(inNumSamples, ZXP(out) = mix_cur; mix_cur += mix_slope;);
				vfloat32 *vout = (vfloat32*)out;
				vfloat32 vmix_slope = vload(4.f * mix_slope);
				vfloat32 vmix = vstart(mix_cur, vmix_slope);
				for (i=0; i<len; i+=16) {
					vec_st(vmix, i, vout);
					vmix = vec_add(vmix, vmix_slope);
				}
			} else {
				//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; mix_cur += mix_slope; ZP(out););
				vfloat32 *vin = (vfloat32*)in;
				vfloat32 *vout = (vfloat32*)out;
				vfloat32 vamp = vload(amp_cur);
				vfloat32 vmix_slope = vload(4.f * mix_slope);
				vfloat32 vmix = vstart(mix_cur, vmix_slope);
				for (i=0; i<len; i+=16) {
					vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
					vmix = vec_add(vmix, vmix_slope);
				}
			}
			unit->mPrevAdd = nextMix;
		}
	} else {
		if (mix_slope == 0.f) {
			if (mix_cur == 0.f) {
				//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out; amp_cur += amp_slope; ZP(out););
				vfloat32 *vin = (vfloat32*)in;
				vfloat32 *vout = (vfloat32*)out;
				vfloat32 vamp_slope = vload(4.f * amp_slope);
				vfloat32 vamp = vstart(amp_cur, vamp_slope);
				for (i=0; i<len; i+=16) {
					vec_st(vec_mul(vamp, vec_ld(i, vin)), i, vout);
					vamp = vec_add(vamp, vamp_slope);
				}
			} else {
				//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; amp_cur += amp_slope; ZP(out););
				vfloat32 *vin = (vfloat32*)in;
				vfloat32 *vout = (vfloat32*)out;
				vfloat32 vamp_slope = vload(4.f * amp_slope);
				vfloat32 vamp = vstart(amp_cur, vamp_slope);
				vfloat32 vmix = vload(mix_cur);
				for (i=0; i<len; i+=16) {
					vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
					vamp = vec_add(vamp, vamp_slope);
				}
			}
		} else {
			//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; amp_cur += amp_slope; mix_cur += mix_slope; ZP(out););
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 vamp_slope = vload(4.f * amp_slope);
			vfloat32 vamp = vstart(amp_cur, vamp_slope);
			vfloat32 vmix_slope = vload(4.f * mix_slope);
			vfloat32 vmix = vstart(mix_cur, vmix_slope);
			for (i=0; i<len; i+=16) {
				vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
				vamp = vec_add(vamp, vamp_slope);
				vmix = vec_add(vmix, vmix_slope);
			}
			unit->mPrevAdd = nextMix;
		}
		unit->mPrevMul = nextAmp;
	}

}

void v_ampmix_ki(MulAdd *unit, int inNumSamples);
void v_ampmix_ki(MulAdd *unit, int inNumSamples)
{

	float amp_cur, mix_cur, amp_slope;
	int i, len = BUFLENGTH << 2;

	float *in = IN(0);
	float *out = OUT(0);
	amp_cur = unit->mPrevMul;
	float nextAmp = MULIN[0];
	amp_slope = CALCSLOPE(nextAmp, amp_cur);
	mix_cur = unit->mPrevAdd;
	if (amp_slope == 0.f) {
		if (amp_cur == 1.f) {
			//LOOP(inNumSamples, ZXP(out) += mix_cur;);
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 vmix = vload(mix_cur);
			for (i=0; i<len; i+=16) {
				vec_st(vec_add(vmix, vec_ld(i, vin)), i, vout);
			}
		} else if (amp_cur == 0.f) {
			//LOOP(inNumSamples, ZXP(out) = mix_cur;);
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 vmix = vload(mix_cur);
			for (i=0; i<len; i+=16) {
				vec_st(vmix, i, vout);
			}
		} else {
			//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; ZP(out););
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 vamp = vload(amp_cur);
			vfloat32 vmix = vload(mix_cur);
			for (i=0; i<len; i+=16) {
				vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
			}
		}
	} else {
		//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; amp_cur += amp_slope; ZP(out););
		vfloat32 *vin = (vfloat32*)in;
		vfloat32 *vout = (vfloat32*)out;
		vfloat32 vamp_slope = vload(4.f * amp_slope);
		vfloat32 vamp = vstart(amp_cur, vamp_slope);
		vfloat32 vmix = vload(mix_cur);
		for (i=0; i<len; i+=16) {
			vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
			vamp = vec_add(vamp, vamp_slope);
		}
		unit->mPrevMul = nextAmp;
	}
}

void v_ampmix_ia(MulAdd *unit, int inNumSamples);
void v_ampmix_ia(MulAdd *unit, int inNumSamples)
{
	float amp_cur;

	float *in = IN(0);
	float *out = OUT(0);
	int i, len = BUFLENGTH << 2;

	float *mix = ADDIN;
	amp_cur = unit->mPrevMul;
	//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + ZXP(mix); ZP(out););
	vfloat32 *vin = (vfloat32*)in;
	vfloat32 *vout = (vfloat32*)out;
	vfloat32 vamp = vload(amp_cur);
	vfloat32 *vmix = (vfloat32*)mix;
	for (i=0; i<len; i+=16) {
		vec_st(vec_madd(vamp, vec_ld(i, vin), vec_ld(i, vmix)), i, vout);
	}
}

void v_ampmix_ik(MulAdd *unit, int inNumSamples);
void v_ampmix_ik(MulAdd *unit, int inNumSamples)
{

	float amp_cur, mix_cur, mix_slope;
	int i, len = BUFLENGTH << 2;
	define_vzero

	float *in = IN(0);
	float *out = OUT(0);

	amp_cur = unit->mPrevMul;
	mix_cur = unit->mPrevAdd;
	float nextMix = ADDIN[0];
	mix_slope = CALCSLOPE(nextMix, mix_cur);
	if (mix_slope == 0.f) {
		if (mix_cur == 0.f) {
			//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out; ZP(out););
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 vamp = vload(amp_cur);
			for (i=0; i<len; i+=16) {
				vec_st(vec_mul(vamp, vec_ld(i, vin)), i, vout);
			}
		} else {
			//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; ZP(out););
			vfloat32 *vin = (vfloat32*)in;
			vfloat32 *vout = (vfloat32*)out;
			vfloat32 vamp = vload(amp_cur);
			vfloat32 vmix = vload(mix_cur);
			for (i=0; i<len; i+=16) {
				vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
			}
		}
	} else {
		//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; mix_cur += mix_slope; ZP(out););
		vfloat32 *vin = (vfloat32*)in;
		vfloat32 *vout = (vfloat32*)out;
		vfloat32 vamp = vload(amp_cur);
		vfloat32 vmix_slope = vload(4.f * mix_slope);
		vfloat32 vmix = vstart(mix_cur, vmix_slope);
		for (i=0; i<len; i+=16) {
			vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
			//Print("vmix %d  %g %g %g %g\n", i, ((float*)&vmix)[0],((float*)&vmix)[1],((float*)&vmix)[2],((float*)&vmix)[3]);
			vmix = vec_add(vmix, vmix_slope);
		}
		unit->mPrevAdd = nextMix;
	}
}

void v_ampmix_ii(MulAdd *unit, int inNumSamples);
void v_ampmix_ii(MulAdd *unit, int inNumSamples)
{
	float amp_cur, mix_cur;
	int i, len = BUFLENGTH << 2;

	float *in = IN(0);
	float *out = OUT(0);

	amp_cur = unit->mPrevMul;
	mix_cur = unit->mPrevAdd;

	//LOOP(inNumSamples, PZ(out); *out = amp_cur * *out + mix_cur; ZP(out););
	vfloat32 *vin = (vfloat32*)in;
	vfloat32 *vout = (vfloat32*)out;
	vfloat32 vamp = vload(amp_cur);
	vfloat32 vmix = vload(mix_cur);
	for (i=0; i<len; i+=16) {
		vec_st(vec_madd(vamp, vec_ld(i, vin), vmix), i, vout);
	}
}

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////

void MulAdd_Ctor(MulAdd *unit)
{
	if (unit->mCalcRate != calc_FullRate) {
		unit->mCalcFunc = (UnitCalcFunc)&ampmix_k;
		ampmix_k(unit, 1);
		return;
	}

	unit->mPrevMul = ZIN0(1);
	unit->mPrevAdd = ZIN0(2);
	
	int mulRate = INRATE(1);
	int addRate = INRATE(2);

	//Print("muladd %d %d %g %g\n", mulRate, addRate, mul, add);
	//Print("**** %08X %08X %08X    %08X\n", IN(0), IN(1), IN(2), OUT(0));

#if __VEC__	
	if (!USEVEC) {
#endif
		switch (mulRate) {
			case calc_FullRate :
				switch (addRate) {
					case calc_FullRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_aa;
						break;
					case calc_BufRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_ak;
						break;
					case calc_ScalarRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_ai;
						break;
				}
				break;
			case calc_BufRate :
				switch (addRate) {
					case calc_FullRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_ka;
						break;
					case calc_BufRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_kk;
						break;
					case calc_ScalarRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_ki;
						break;
				}
				break;
			case calc_ScalarRate :
				switch (addRate) {
					case calc_FullRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_ia;
						break;
					case calc_BufRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_ik;
						break;
					case calc_ScalarRate :
						unit->mCalcFunc = (UnitCalcFunc)&ampmix_ii;
						break;
				}
				break;
		}
#if __VEC__
	} else {
		switch (mulRate) {
			case calc_FullRate :
				switch (addRate) {
					case calc_FullRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_aa;
						break;
					case calc_BufRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_ak;
						break;
					case calc_ScalarRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_ai;
						break;
				}
				break;
			case calc_BufRate :
				switch (addRate) {
					case calc_FullRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_ka;
						break;
					case calc_BufRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_kk;
						break;
					case calc_ScalarRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_ki;
						break;
				}
				break;
			case calc_ScalarRate :
				switch (addRate) {
					case calc_FullRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_ia;
						break;
					case calc_BufRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_ik;
						break;
					case calc_ScalarRate :
						unit->mCalcFunc = (UnitCalcFunc)&v_ampmix_ii;
						break;
				}
				break;
		}
	}
#endif
	ampmix_k(unit, 1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////

void load(InterfaceTable *inTable)
{
	ft = inTable;

	DefineSimpleUnit(MulAdd);
}