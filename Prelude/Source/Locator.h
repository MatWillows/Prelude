#ifndef LOCATOR_H
#define LOCATOR_H

#include "defs.h"

typedef enum
{
	LOC_STATE_NORMAL = 0,
	LOC_STATE_DIRECTION,
	LOC_STATE_SIT,
	LOC_STATE_LAY,
} LOCATOR_STATE_T;

class Locator
{
private:
	BYTE AreaNum;
	RECT rBounds;
	BYTE Start;
	BYTE End;
	BYTE Angle;
	BYTE State;
	Locator *pNext;

public:

	void GetBounds(RECT *rDest) { *rDest = rBounds; }
	BYTE GetStart() { return Start; }
	BYTE GetEnd() { return End; }
	BYTE GetAngle() { return Angle; }
	BYTE GetState() { return State; }
	BYTE GetArea() { return AreaNum; }
	 
	void SetStart(BYTE NewStart) { Start = NewStart; }
	void SetEnd(BYTE NewEnd) { End = NewEnd; }
	Locator *GetNext() { return pNext; }
	void SetNext(Locator *pNewNext) { pNewNext->pNext = pNext; pNext = pNewNext; }
	void SetBounds(RECT *pNewBounds) { rBounds = *pNewBounds; }
	void SetAngle(BYTE NewAngle) { Angle = NewAngle; }
	void SetState(BYTE NewState) { State = NewState; }
	void SetArea(BYTE NewArea) { AreaNum = NewArea; }

	void Save(FILE *fp);

	void Load(FILE *fp);

	Locator();

};

#endif