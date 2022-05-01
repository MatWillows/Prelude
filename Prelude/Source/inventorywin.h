#ifndef INVENTORYWIN_H
#define INVENTORYWIN_H

#include "zswindow.h"

class Object;

class InventoryWin : public ZSWindow
{
private:
	Object *pOwner;
	int TopRow;
	int NumRows;
	int NumItems;
	int SlotsDeep;
	int SlotsAcross;
	int NumSlots;

	void ClearSlots();

public:
	void SetSlots();
	void Reset();

	Object *GetOwner() { return pOwner; }
	void SetOwner(Object *pNewOwner) { pOwner = pNewOwner; SetSlots(); }

	int Command(int IDFrom, int Command, int Param);

	BOOL ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y);

	InventoryWin(int NewID, int x, int y, int width, int Height, Object *pNewOwner);

};

#endif