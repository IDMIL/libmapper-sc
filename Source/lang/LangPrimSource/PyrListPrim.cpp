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

#include "PyrKernel.h"
#include "PyrKernelProto.h"
#include "PyrPrimitive.h"
#include "PyrPrimitiveProto.h"
#include "PyrListPrim.h"
#include "SC_InlineUnaryOp.h"
#include "SC_InlineBinaryOp.h"
#include "PyrSignal.h"
#include "PyrMessage.h"
#include "PyrSched.h"
#include "SC_RGen.h"
#include <stdlib.h>
#include <string.h>

int objectPerform(VMGlobals *g, int numArgsPushed);

int ivxIdentDict_array, ivxIdentDict_size, ivxIdentDict_parent, ivxIdentDict_proto, ivxIdentDict_know;

int class_array_index, class_array_maxsubclassindex;
int class_identdict_index, class_identdict_maxsubclassindex;

PyrClass *class_identdict;
PyrSymbol *s_proto, *s_parent;
PyrSymbol *s_delta, *s_dur, *s_stretch;

#define HASHSYMBOL(sym) (sym >> 5)

#define ISKINDOF(obj, lo, hi)  (\
	objClassIndex = obj->classptr->classIndex.ui,	\
	objClassIndex >= lo && objClassIndex <= hi)


int prArrayMultiChanExpand(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *slot, *slots1, *slots2, *slots3, *slots4;
	PyrObject *obj1, *obj2, *obj3, *obj4;
	int i, j, size, len, maxlen;
	
	a = g->sp;
	obj1 = a->uo;
	size = obj1->size;
	slots1 = obj1->slots;
	maxlen = 1;
	for (j=0; j<size; ++j) {
		slot = slots1 + j;
		if (slot->utag == tagObj) {
			if (slot->uo->classptr == class_array) {
				len = slot->uo->size;
				maxlen = len > maxlen ? len : maxlen;
			} else if (isKindOf(slot->uo, class_sequenceable_collection) && (slot->uo->classptr != class_string)) {
				return errFailed; // this primitive only handles Arrays.
			}
		}
	}
	
	obj2 = newPyrArray(g->gc, maxlen, 0, true);
	obj2->size = maxlen;
	slots2 = obj2->slots;
	for (i=0; i<maxlen; ++i) {
		obj3 = newPyrArray(g->gc, size, 0, false);
		obj3->size = size;
		SetObject(slots2 + i, obj3);
		slots1 = obj1->slots;
		slots3 = obj3->slots;
		for (j=0; j<size; ++j) {
			slot = slots1 + j;
			if (slot->utag == tagObj) {
				if (slot->uo->classptr == class_array && slot->uo->size > 0) {
					obj4 = slot->uo;
					slots4 = obj4->slots;
					slots3[j].ucopy = slots4[i % obj4->size].ucopy;
				} else {
					slots3[j].ucopy = slot->ucopy;
				}
			} else {
				slots3[j].ucopy = slot->ucopy;
			}
		}
	}

	SetObject(a, obj2);

	return errNone;
}


int arrayAtIdentityHash(PyrObject *array, PyrSlot *key)
{
	PyrSlot *slots, *test;
	unsigned int i, start, end, hash, maxHash;
	
	hash = calcHash(key);
	maxHash = array->size;
	start = hash % maxHash;
	end = array->size;
	slots = array->slots;
	for (i=start; i<end; i++) {
		test = slots + i;
		if (IsNil(test) || (test->utag == key->utag && test->ui == key->ui)) {
			return i;
		}
	}
	end = start - 1;
	for (i=0; i<=end; i++) {
		test = slots + i;
		if (IsNil(test) || (test->utag == key->utag && test->ui == key->ui)) {
			return i;
		}
	}
	return -1;
} 


int prArray_AtIdentityHash(struct VMGlobals *g, int numArgsPushed);
int prArray_AtIdentityHash(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	PyrObject *array;
	int index;
	
	a = g->sp - 1;  // array
	b = g->sp;		// key

	array = a->uo;
	index = arrayAtIdentityHash(array, b);
	SetInt(a, index);
	return errNone;
}


int arrayAtIdentityHashInPairs(PyrObject *array, PyrSlot *key)
{
	PyrSlot *slots, *test;
	unsigned int i, start, end, hash, maxHash;
	
	hash = calcHash(key);
	maxHash = array->size >> 1;
	start = (hash % maxHash) << 1;
	end = array->size;
	slots = array->slots;
	for (i=start; i<end; i+=2) {
		test = slots + i;
		if (IsNil(test) || (test->utag == key->utag && test->ui == key->ui)) {
			return i;
		}
	}
	end = start - 2;
	for (i=0; i<=end; i+=2) {
		test = slots + i;
		if (IsNil(test) || (test->utag == key->utag && test->ui == key->ui)) {
			return i;
		}
	}
	return -2;
} 


int arrayAtIdentityHashInPairsWithHash(PyrObject *array, PyrSlot *key, int hash)
{
	PyrSlot *slots, *test;
	unsigned int i, start, end, maxHash;
	
	maxHash = array->size >> 1;
	start = (hash % maxHash) << 1;
	end = array->size;
	slots = array->slots;
	for (i=start; i<end; i+=2) {
		test = slots + i;
		if (IsNil(test) || (test->utag == key->utag && test->ui == key->ui)) {
			return i;
		}
	}
	end = start - 2;
	for (i=0; i<=end; i+=2) {
		test = slots + i;
		if (IsNil(test) || (test->utag == key->utag && test->ui == key->ui)) {
			return i;
		}
	}
	return -2;
} 


int identDictPut(struct VMGlobals *g, PyrObject *dict, PyrSlot *key, PyrSlot *value);
int identDictPut(struct VMGlobals *g, PyrObject *dict, PyrSlot *key, PyrSlot *value)
{
	PyrSlot *slot, *newslot;
	int i, index, size;
	PyrObject *array;
	
	bool knows = IsTrue(dict->slots + ivxIdentDict_know);
	if (knows && IsSym(key)) {
		if (key->us == s_parent) {
			dict->slots[ivxIdentDict_parent].ucopy = value->ucopy;
			g->gc->GCWrite(dict, value);
			return errNone;
		}
		if (key->us == s_proto) {
			dict->slots[ivxIdentDict_proto].ucopy = value->ucopy;
			g->gc->GCWrite(dict, value);
			return errNone;
		}
	}
	array = dict->slots[ivxIdentDict_array].uo;
	if (!isKindOf((PyrObject*)array, class_array)) return errFailed;
	
	index = arrayAtIdentityHashInPairs(array, key);
	slot = array->slots + index;
	slot[1].ucopy = value->ucopy;
	g->gc->GCWrite(array, value);
	if (IsNil(slot)) {
		slot->ucopy = key->ucopy;
		g->gc->GCWrite(array, key);
		size = ++dict->slots[ivxIdentDict_size].ui;
		if (array->size < size*3) {
			PyrObject *newarray;
			newarray = newPyrArray(g->gc, size*3, 0, false);
			newarray->size = ARRAYMAXINDEXSIZE(newarray);
			nilSlots(newarray->slots, newarray->size);
			slot = array->slots;
			for (i=0; i<array->size; i+=2, slot+=2) {
				if (NotNil(slot)) {
					index = arrayAtIdentityHashInPairs(newarray, slot);
					newslot = newarray->slots + index;
					newslot[0].ucopy = slot[0].ucopy;
					newslot[1].ucopy = slot[1].ucopy;
				}
			}
			dict->slots[ivxIdentDict_array].uo = newarray;
			g->gc->GCWrite(dict, newarray);
		}
	}
	return errNone;
}

int prIdentDict_Put(struct VMGlobals *g, int numArgsPushed);
int prIdentDict_Put(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c;
	
	a = g->sp - 2;  // dict
	b = g->sp - 1;	// key
	c = g->sp;		// value
	if(IsNil(b)) return errWrongType;
	if(IsNil(c)) return errFailed; // will call removeAt
	return identDictPut(g, a->uo, b, c);
}	

int prIdentDict_PutGet(struct VMGlobals *g, int numArgsPushed);
int prIdentDict_PutGet(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b, *c, *d, *slot, *newslot;
	int i, index, size;
	PyrObject *dict;
	PyrObject *array;
	
	a = g->sp - 2;  // dict
	b = g->sp - 1;	// key
	c = g->sp;		// value
	d = ++g->sp;	// push the stack to save the receiver
	
	d->ucopy = a->ucopy;
	dict = d->uo;
	array = dict->slots[ivxIdentDict_array].uo;
	if (!isKindOf((PyrObject*)array, class_array)) {
		SetNil(a);
		--g->sp;
		return errFailed;
	}
	
	index = arrayAtIdentityHashInPairs(array, b);
	slot = array->slots + index;
	a->ucopy = slot[1].ucopy;
	slot[1].ucopy = c->ucopy;
	g->gc->GCWrite(array, c);
	if (IsNil(slot)) {
		slot->ucopy = b->ucopy;
		g->gc->GCWrite(array, b);
		size = ++dict->slots[ivxIdentDict_size].ui;
		if (array->size < size*3) {
			PyrObject *newarray;
			newarray = newPyrArray(g->gc, size*3, 0, true);
			newarray->size = ARRAYMAXINDEXSIZE(newarray);
			nilSlots(newarray->slots, newarray->size);
			slot = array->slots;
			for (i=0; i<array->size; i+=2, slot+=2) {
				if (NotNil(slot)) {
					index = arrayAtIdentityHashInPairs(newarray, slot);
					newslot = newarray->slots + index;
					newslot[0].ucopy = slot[0].ucopy;
					newslot[1].ucopy = slot[1].ucopy;
				}
			}
			dict->slots[ivxIdentDict_array].uo = newarray;
			g->gc->GCWrite(dict, newarray);
		}
	}
	--g->sp;
	return errNone;
}	



int prArray_AtIdentityHashInPairs(struct VMGlobals *g, int numArgsPushed);
int prArray_AtIdentityHashInPairs(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	unsigned int i;
	
	a = g->sp - 1;  // array
	b = g->sp;		// key
	
	i = arrayAtIdentityHashInPairs(a->uo, b);
	SetInt(a, i);
	return errNone;
}


bool identDict_lookupNonNil(PyrObject *dict, PyrSlot *key, int hash, PyrSlot *result);
bool identDict_lookupNonNil(PyrObject *dict, PyrSlot *key, int hash, PyrSlot *result)
{
again:	
	PyrSlot *dictslots = dict->slots;
	PyrSlot *arraySlot = dictslots + ivxIdentDict_array;
	
	if (isKindOfSlot(arraySlot, class_array)) {
		PyrObject *array = arraySlot->uo;
	
		int index = arrayAtIdentityHashInPairsWithHash(array, key, hash);
		if (SlotEq(key, array->slots + index)) {
			result->ucopy = array->slots[index + 1].ucopy;
			return true;
		}
	}

	PyrClass *identDictClass = s_identitydictionary->u.classobj;
	PyrSlot *parentSlot = dictslots + ivxIdentDict_parent;
	PyrSlot * protoSlot = dictslots + ivxIdentDict_proto;
	if (isKindOfSlot(parentSlot, identDictClass)) {
		if (isKindOfSlot(protoSlot, identDictClass)) {
			// recursive call.
			if (identDict_lookupNonNil(protoSlot->uo, key, hash, result)) return true;
		}
	
		dict = parentSlot->uo;
		goto again; // tail call
	} else {
		if (isKindOfSlot(protoSlot, identDictClass)) {
			dict = protoSlot->uo;
			goto again; // tail call
		}
	}
	return false;
}

bool identDict_lookup(PyrObject *dict, PyrSlot *key, int hash, PyrSlot *result);
bool identDict_lookup(PyrObject *dict, PyrSlot *key, int hash, PyrSlot *result)
{
again:	
	PyrSlot *dictslots = dict->slots;
	PyrSlot *arraySlot = dictslots + ivxIdentDict_array;
	
	if (isKindOfSlot(arraySlot, class_array)) {
		PyrObject *array = arraySlot->uo;
	
		int index = arrayAtIdentityHashInPairsWithHash(array, key, hash);
		if (SlotEq(key, array->slots + index)) {
			result->ucopy = array->slots[index + 1].ucopy;
			return true;
		}
	}

	PyrClass *identDictClass = s_identitydictionary->u.classobj;
	PyrSlot *parentSlot = dictslots + ivxIdentDict_parent;
	PyrSlot * protoSlot = dictslots + ivxIdentDict_proto;
	if (isKindOfSlot(parentSlot, identDictClass)) {
		if (isKindOfSlot(protoSlot, identDictClass)) {
			// recursive call.
			if (identDict_lookup(protoSlot->uo, key, hash, result)) return true;
		}
	
		dict = parentSlot->uo;
		goto again; // tail call
	} else {
		if (isKindOfSlot(protoSlot, identDictClass)) {
			dict = protoSlot->uo;
			goto again; // tail call
		}
	}
	SetNil(result);
	return false;
}

int prIdentDict_At(struct VMGlobals *g, int numArgsPushed);
int prIdentDict_At(struct VMGlobals *g, int numArgsPushed)
{	
	PyrSlot* a = g->sp - 1;  // dict
	PyrSlot* key = g->sp;		// key
	PyrObject *dict = a->uo;
	
	bool knows = IsTrue(dict->slots + ivxIdentDict_know);
	if (knows && IsSym(key)) {
		if (key->us == s_parent) {
			a->ucopy = dict->slots[ivxIdentDict_parent].ucopy;
			return errNone;
		}
		if (key->us == s_proto) {
			a->ucopy = dict->slots[ivxIdentDict_proto].ucopy;
			return errNone;
		}
	}
	
	identDict_lookup(dict, key, calcHash(key), a);
	return errNone;
}

int prSymbol_envirGet(struct VMGlobals *g, int numArgsPushed);
int prSymbol_envirGet(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, result;
	int objClassIndex;
	
	a = g->sp;  // key
	
	PyrSlot* currentEnvironmentSlot = &g->classvars->slots[1];
	PyrObject *dict = currentEnvironmentSlot->uo;
	
	if (!IsObj(currentEnvironmentSlot)) return errFailed;

	if (!ISKINDOF(dict, class_identdict_index, class_identdict_maxsubclassindex)) return errFailed;

	identDict_lookup(dict, a, calcHash(a), &result);
	a->ucopy = result.ucopy;
	
	return errNone;
}


int prSymbol_envirPut(struct VMGlobals *g, int numArgsPushed);
int prSymbol_envirPut(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, *b;
	int objClassIndex;
	
	a = g->sp - 1;  // key
	b = g->sp;  // value
	
	PyrSlot* currentEnvironmentSlot = &g->classvars->slots[1];
	PyrObject *dict = currentEnvironmentSlot->uo;

	if (!IsObj(currentEnvironmentSlot)) return errFailed;

	if (!ISKINDOF(dict, class_identdict_index, class_identdict_maxsubclassindex)) return errFailed;
	
	int err = identDictPut(g, dict, a, b);
	if (err) return err;
	
	a->ucopy = b->ucopy;
	
	return errNone;
}


int prEvent_Delta(struct VMGlobals *g, int numArgsPushed);
int prEvent_Delta(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a, key, dur, stretch, delta;
	double fdur, fstretch;
	int err;
		
	a = g->sp;  // dict
	
	SetSymbol(&key, s_delta);
	identDict_lookup(a->uo, &key, calcHash(&key), &delta);
	
	if (NotNil(&delta)) {
		a->ucopy = delta.ucopy;
	} else {
		SetSymbol(&key, s_dur);
		identDict_lookup(a->uo, &key, calcHash(&key), &dur);
		
		err = slotDoubleVal(&dur, &fdur);
		if (err) {
			if (NotNil(&dur)) return err;
			SetNil(a);
			return errNone;
		}
		
		SetSymbol(&key, s_stretch);
		identDict_lookup(a->uo, &key, calcHash(&key), &stretch);
		
		err = slotDoubleVal(&stretch, &fstretch);
		if (err) {
			if (NotNil(&stretch)) return err;
			SetNil(a);
			return errNone;
		}
				
		SetFloat(a, fdur * fstretch );
	}
		
	return errNone;
}

void PriorityQueueAdd(struct VMGlobals *g, PyrObject* queueobj, PyrSlot* item, double time);
void PriorityQueueAdd(struct VMGlobals *g, PyrObject* queueobj, PyrSlot* item, double time)
{
	PyrObject *schedq, *newschedq;
	int size, maxsize;

	PyrSlot *schedqSlot = queueobj->slots;
	if (!IsObj(schedqSlot)) {
		size = 16;
		schedq = newPyrArray(g->gc, size, 0, true);
		SetObject(schedqSlot, schedq);
		g->gc->GCWrite(queueobj, schedq);
	} else {
		schedq = schedqSlot->uo;
		maxsize = ARRAYMAXINDEXSIZE(schedq);
		size = schedq->size;
		if (size+2 > maxsize) {
			double *pslot, *qslot;

			newschedq = newPyrArray(g->gc, maxsize*2, 0, true);
			newschedq->size = size;
			
			pslot = (double*)schedq->slots - 1;
			qslot = (double*)newschedq->slots - 1;
			for (int i=0; i<size; ++i) *++qslot = *++pslot;
			
			SetObject(schedqSlot, newschedq);
			g->gc->GCWrite(queueobj, newschedq);
			
			schedq = newschedq;
		}
	}
	
	addheap(g, schedq, time, item);
}

int prPriorityQueueAdd(struct VMGlobals *g, int numArgsPushed);
int prPriorityQueueAdd(struct VMGlobals *g, int numArgsPushed)
{
	
	PyrSlot *a = g->sp - 2;	// priority queue
	PyrSlot *b = g->sp - 1;	// time
	PyrSlot *c = g->sp;		// item
	
	double time;
	int err = slotDoubleVal(b, &time);
	if (err) return errNone;	// nil is OK, nothing gets added
	
	PriorityQueueAdd(g, a->uo, c, time);
	return errNone;
}


void PriorityQueuePop(PyrObject *queueobj, PyrSlot *result);
void PriorityQueuePop(PyrObject *queueobj, PyrSlot *result)
{
	PyrSlot *schedqSlot = queueobj->slots;
	
	if (IsObj(schedqSlot)) {
		PyrObject *schedq = schedqSlot->uo;
		double time;
		if (!getheap(schedq, &time, result)) {
			SetNil(result);
		}
	} else {
		SetNil(result);
	}
}

void PriorityQueueTop(PyrObject *queueobj, PyrSlot *result);
void PriorityQueueTop(PyrObject *queueobj, PyrSlot *result)
{
	PyrSlot *schedqSlot = queueobj->slots;
	
	if (IsObj(schedqSlot)) {
		PyrObject *schedq = schedqSlot->uo;
		if (schedq->size > 0) {
			result->ucopy = schedq->slots[0].ucopy;
		} else {
			SetNil(result);
		}
	} else {
		SetNil(result);
	}
}

void PriorityQueueClear(PyrObject *queueobj);
void PriorityQueueClear(PyrObject *queueobj)
{
	PyrSlot *schedqSlot = queueobj->slots;
	
	if (IsObj(schedqSlot)) {
		PyrObject *schedq = schedqSlot->uo;
		schedq->size = 0;
	}
}

bool PriorityQueueEmpty(PyrObject *queueobj);
bool PriorityQueueEmpty(PyrObject *queueobj)
{
	PyrSlot *schedqSlot = queueobj->slots;
	
	if (IsObj(schedqSlot)) {
		PyrObject *schedq = schedqSlot->uo;
		if (schedq->size > 0) {
			return false;
		}
	}
	return true;
}

int prPriorityQueuePop(struct VMGlobals *g, int numArgsPushed);
int prPriorityQueuePop(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot* a = g->sp;	// priority queue
	
	PriorityQueuePop(a->uo, a);
	return errNone;
}

int prPriorityQueueTop(struct VMGlobals *g, int numArgsPushed);
int prPriorityQueueTop(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot* a = g->sp;	// priority queue
	
	PriorityQueueTop(a->uo, a);
	return errNone;
}

int prPriorityQueueClear(struct VMGlobals *g, int numArgsPushed);
int prPriorityQueueClear(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot* a = g->sp;	// priority queue
	
	PriorityQueueClear(a->uo);
	return errNone;
}

int prPriorityQueueEmpty(struct VMGlobals *g, int numArgsPushed);
int prPriorityQueueEmpty(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a;
	
	a = g->sp;	// priority queue
	
	if (PriorityQueueEmpty(a->uo)) {
		SetTrue(a);
	} else {
		SetFalse(a);
	}
	return errNone;
}

void PriorityQueuePostpone(PyrObject* queueobj, double time);
void PriorityQueuePostpone(PyrObject* queueobj, double time)
{
	PyrSlot *schedqSlot = queueobj->slots;
	
	if (IsObj(schedqSlot)) {
		PyrObject *schedq = schedqSlot->uo;
		PyrSlot* slots = schedq->slots;
		for (int i=0; i < schedq->size; i+=2) {
			slots[i].uf += time;
		}
	}
}

int prPriorityQueuePostpone(struct VMGlobals *g, int numArgsPushed);
int prPriorityQueuePostpone(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp - 1;	// priority queue
	PyrSlot *b = g->sp;		// time
	
	double time;
	int err = slotDoubleVal(b, &time);
	if (err) return err;
	
	PyrObject *queueobj = a->uo;
	PriorityQueuePostpone(queueobj, time);
	return errNone;
}


void initListPrimitives();
void initListPrimitives()
{
	int base, index;
		
	base = nextPrimitiveIndex();
	index = 0;
	definePrimitive(base, index++, "_Array_AtIdentityHash", prArray_AtIdentityHash, 2, 0);
	definePrimitive(base, index++, "_Array_AtIdentityHashInPairs", prArray_AtIdentityHashInPairs, 2, 0);
	definePrimitive(base, index++, "_IdentDict_Put", prIdentDict_Put, 3, 0);
	definePrimitive(base, index++, "_IdentDict_PutGet", prIdentDict_PutGet, 3, 0);
	definePrimitive(base, index++, "_IdentDict_At", prIdentDict_At, 2, 0);
	definePrimitive(base, index++, "_Symbol_envirGet", prSymbol_envirGet, 1, 0);
	definePrimitive(base, index++, "_Symbol_envirPut", prSymbol_envirPut, 2, 0);
	definePrimitive(base, index++, "_ArrayMultiChannelExpand", prArrayMultiChanExpand, 1, 0);

	definePrimitive(base, index++, "_PriorityQueueAdd", prPriorityQueueAdd, 3, 0);
	definePrimitive(base, index++, "_PriorityQueuePop", prPriorityQueuePop, 1, 0);
	definePrimitive(base, index++, "_PriorityQueueTop", prPriorityQueueTop, 1, 0);
	definePrimitive(base, index++, "_PriorityQueueClear", prPriorityQueueClear, 1, 0);
	definePrimitive(base, index++, "_PriorityQueueEmpty", prPriorityQueueEmpty, 1, 0);
	definePrimitive(base, index++, "_PriorityQueuePostpone", prPriorityQueuePostpone, 2, 0);
	
	definePrimitive(base, index++, "_Event_Delta", prEvent_Delta, 1, 0);
}

void initPatterns();
void initPatterns()
{
	PyrSymbol *sym;

	ivxIdentDict_array  = instVarOffset("IdentityDictionary", "array");
	ivxIdentDict_size   = instVarOffset("IdentityDictionary", "size");
	ivxIdentDict_parent = instVarOffset("IdentityDictionary", "parent");
	ivxIdentDict_proto = instVarOffset("IdentityDictionary", "proto");
	ivxIdentDict_know = instVarOffset("IdentityDictionary", "know");
	
	sym = getsym("IdentityDictionary");
	class_identdict = sym ? sym->u.classobj : NULL;
	class_identdict_index = class_identdict->classIndex.ui;
	class_identdict_maxsubclassindex = class_identdict->maxSubclassIndex.ui;

	class_array_index = class_array->classIndex.ui;
	class_array_maxsubclassindex = class_array->maxSubclassIndex.ui;
	
	s_parent = getsym("parent");
	s_proto = getsym("proto");
	s_delta = getsym("delta");
	s_dur = getsym("dur");
	s_stretch = getsym("stretch");

}