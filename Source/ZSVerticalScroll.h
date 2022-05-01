#ifndef ZSVERTICALSCROLL_H
#define ZSVERTICALSCROLL_H

#include "ZSWindow.h"

class ZSVScroll : public ZSWindow
{
private:
	int UpperBound;
	int LowerBound;
	int CurPosition;
	int PageLength;

	void MoveThumb();

public:
	int Draw();

	void SetUpper(int NewUpper) { UpperBound = NewUpper; }
	void SetLower(int NewLower) { LowerBound = NewLower; }
	void SetPos(int NewPos)		 { CurPosition = NewPos; }
	void SetPage(int NewLength) { PageLength = NewLength; }

	int GetPage()  { return PageLength; }
	int GetPos()   { return CurPosition; }
	int GetUpper() { return UpperBound; }
	int GetLower() { return LowerBound; }

	int Attach(ZSWindow *pTarget);

	int Command(int IDFrom, int Command, int Param);

	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	int MoveMouse(long *x, long *y, long *z);	

	ZSVScroll(int NewID, int x, int y, int Width, int Height);

};

#endif