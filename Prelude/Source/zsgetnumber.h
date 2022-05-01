#ifndef ZSGETNUMBER_H
#define ZSGETNUMBER_H

#include "zswindow.h"

class ZSGetNumber : public ZSWindow
{
private:
	int Min;
	int Max;
	int OffsetAmount;
	DWORD NextOffsetTime;
	DWORD StartOffsetTime;

public:

	void SetMin(int NewMin) { Min = NewMin; }
	void SetMax(int NewMax) { Max = NewMax; }
	int GetMax() { return Max; }
	int GetMin() { return Min; }

	void OffsetValue(int Amount);

	int Command(int IDFrom, int Command, int Param);

	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);

	int Draw();

	ZSGetNumber();

};

int GetNumber(int NewMin, int NewMax, int StartAmount, char *StartText = NULL, ZSWindow *pParent = NULL);

#endif