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

#include <stdlib.h>
#include <string.h>
//#include <stdio.h>
#include "SCBase.h"
#include "SimpleStack.h"
#include "InitAlloc.h"

void initLongStack(LongStack *self) 
{
	//dbg("initLongStack");
	self->maxsize = 0;
	self->stak = NULL;
	self->num = 0;
}

void freeLongStack(LongStack *self) 
{
	//dbg("freeLongStack");
	self->maxsize = 0;
	self->num = 0;
	if (self->stak) {
		pyr_pool_compile->Free((void*)self->stak);
		self->stak = NULL;
	}
}

void growLongStack(LongStack *self) 
{
	if (self->maxsize) {
		long *oldstak;
		self->maxsize += self->maxsize >> 1; // grow by 50%
		oldstak = self->stak;
	// pyrmalloc: 
	// lifetime: kill after compile.
		self->stak = (long*)pyr_pool_compile->Alloc(self->maxsize * sizeof(long));
		MEMFAIL(self->stak);
		//BlockMoveData(oldstak, self->stak, self->num * sizeof(long));
		memcpy(self->stak, oldstak, self->num * sizeof(long));
		pyr_pool_compile->Free((void*)oldstak);
	} else {
		self->maxsize = 32;
		self->stak = (long*)pyr_pool_compile->Alloc(self->maxsize * sizeof(long));
		MEMFAIL(self->stak);
	}
}


void
pushls(LongStack *self, long value) {
	//dbg2("pushls %lX", value);
	if (self->num+1 > self->maxsize) {
		growLongStack(self);
	}
	self->stak[self->num++] = value;
}

long
popls(LongStack *self) {
	if (self->num > 0) return self->stak[--self->num];
	else {
		error("stack empty! (pop)\n");
		return 0;
	}
}

int emptyls(LongStack *self) 
{
	return self->num <= 0;
}
