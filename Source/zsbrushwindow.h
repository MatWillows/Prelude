#ifndef MYBRUSHWINDOW_H
#define MYBRUSHWINDOW_H

#include "zswindow.h"

//terrain
#define NUM_TERRAIN 12
#define NUM_OVERLAY 24

class BrushWin : public ZSWindow
{
private:
	int TopBrush;
	int CurBrush;
	int BrushX;
	int BrushY;

	LPDIRECTDRAWSURFACE7 CreateBackground();

public:

	int Draw();
	
	int LeftButtonUp(int x,int y);
	int LeftButtonDown(int x,int y);

	BrushWin(int ID, ZSWindow *NewParent);

	int GetBrush() { return CurBrush; }
	void SetBrush(int NewBrush) { CurBrush = NewBrush; }
	
};

#endif
