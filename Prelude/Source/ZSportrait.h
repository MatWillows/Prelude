#ifndef ZSPORTRAIT_H
#define ZSPORTRAIT_H

#include "ZSwindow.h"

class Thing;

class ZSPortrait : public ZSWindow
{
private:
	Thing *pTarget;
	static LPDIRECTDRAWSURFACE7 PortraitBackground;
	static LPDIRECTDRAWSURFACE7 PortraitGems;
	static NumPortraits;
	LPDIRECTDRAWSURFACE7 pddFace;

	BOOL NeedRedraw;
	
public:
	Thing *GetTarget() { return pTarget; }
	void SetTarget(Thing *pNewTarget);

	void Update();
	
	int RightButtonUp(int x, int y);
	int LeftButtonUp(int x, int y);

	void Clean() { NeedRedraw = FALSE; }
	void Dirty() { NeedRedraw = TRUE; }
	
	int Draw();

	BOOL ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y);

	ZSPortrait(Thing *pTarget, int x, int y);
	~ZSPortrait();
};

#endif