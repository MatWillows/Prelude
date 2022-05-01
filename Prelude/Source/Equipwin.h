#ifndef EQUIPWIN_H
#define EQUIPWIN_H

#include "ZSWindow.h"

class Thing;

class EquipWin : public ZSWindow
{
private:
	Thing *pTarget;

	void ClearSlots();

public:
	void SetSlots();

	Thing *GetTarget() { return pTarget; }
	void SetTarget(Thing *pNewTarget) { pTarget = pNewTarget; SetSlots(); }

	int Command(int IDFrom, int Command, int Param);

	BOOL ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y);

	EquipWin(int NewID, int x, int y, int width, int Height, Thing *pNewTarget);
};



#endif