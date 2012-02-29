
#include "PyrObject.h"
#include "PyrPrimitive.h"
#include "VMGlobals.h"

#include <mapper/mapper.h>
#include <lo/lo.h>

void handler_freq(mapper_signal sig, mapper_db_signal props, mapper_timetag_t *timetag, void *pfreq);

class Mapper
{

public:
	Mapper();
	//~Mapper() {}

	void devnew( void );
	void add_input( void );
	void poll( void );
	void free( void );
	unsigned int port( void );

private:
	mapper_device m_dev;
	float min0;
	float max1000;

};

Mapper::Mapper( void ) {
	m_dev = NULL;
	min0 = 0;
	max1000 = 1000;
}

void Mapper::devnew( void ) {
	m_dev = mdev_new("supercollider", 9444, 0);
}

void Mapper::add_input( void ) {
    mdev_add_input(m_dev, "/freq", 1, 'f', 0, &min0, &max1000, handler_freq, NULL);
}

void Mapper::poll( void ) {
	int numhandled = 1;
	while ( numhandled > 0 ) {
		numhandled = mdev_poll(m_dev, 0);
	}
}

void Mapper::free( void ) {
	mdev_free( m_dev );
	m_dev = NULL;
}

unsigned int Mapper::port( void ) {
	// return the port number that the device was able to get
	return mdev_port( m_dev );
}

int mapperInit(struct VMGlobals *g, int numArgsPushed);
int mapperInit(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	SetPtr(slotRawObject(a)->slots+0, new Mapper);
	return errNone;
}

int mapperDevNew(struct VMGlobals *g, int numArgsPushed);
int mapperDevNew(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mdev = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	mdev->devnew();
	return errNone;
}

float currentvalue = 0.0;

void handler_freq(mapper_signal sig, mapper_db_signal props, mapper_timetag_t *timetag, void *pfreq)
{
	// store the value in a local variable to poll later
	currentvalue = *(float*)pfreq;
}

int mapperAddInput(struct VMGlobals *g, int numArgsPushed);
int mapperAddInput(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mdev = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	mdev->add_input();
	return errNone;
}

int mapperPoll(struct VMGlobals *g, int numArgsPushed);
int mapperPoll(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mdev = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	mdev->poll();
	return errNone;
}

int mapperDevFree(struct VMGlobals *g, int numArgsPushed);
int mapperDevFree(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mdev = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	mdev->free();
	return errNone;
}

int mapperGetCurrentValue(struct VMGlobals *g, int numArgsPushed);
int mapperGetCurrentValue(struct VMGlobals *g, int numArgsPushed)
{
	// pull the last polled value out of currentvalue

	PyrSlot *receiver = g->sp;

	// set the receiver slot to return the value
	SetFloat(receiver, currentvalue);

	return errNone;
}

int mapperPort(struct VMGlobals *g, int numArgsPushed);
int mapperPort(struct VMGlobals *g, int numArgsPushed)
{
	PyrSlot *a = g->sp;
	Mapper *mdev = (Mapper*)slotRawPtr(&slotRawObject(a)->slots[0]);
	SetInt( a, mdev->port() );
	return errNone;
}

void initMapperPrimitives()
{
	int base, index = 0;

	base = nextPrimitiveIndex();

	// libmapper
	definePrimitive(base, index++, "_MapperInit", mapperInit, 1, 0);
	definePrimitive(base, index++, "_MapperDevNew", mapperDevNew, 1, 0);
	definePrimitive(base, index++, "_MapperAddInput", mapperAddInput, 1, 0);
	definePrimitive(base, index++, "_MapperPoll", mapperPoll, 1, 0);
	definePrimitive(base, index++, "_MapperDevFree", mapperDevFree, 1, 0);
	definePrimitive(base, index++, "_MapperGetCurrentValue", mapperGetCurrentValue, 1, 0);
	definePrimitive(base, index++, "_MapperPort", mapperPort, 1, 0);

	// add getsym()s you need here

}

// vim:sw=4:ts=4:
