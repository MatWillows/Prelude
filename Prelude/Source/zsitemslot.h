#ifndef ZSITEMSLOT_H
#define ZSITEMSLOT_H

#include "ZSWindow.h"

class GameItem;

typedef enum
{
	SOURCE_NONE,
	SOURCE_CONTAINER,
	SOURCE_INVENTORY,
	SOURCE_BARTER_MERCHANT,
	SOURCE_BARTER_PLAYER,
	SOURCE_EQUIP
} ITEM_SLOT_SOURCE_T;

//WINDOW_ITEMSLOT
class ZSItemSlot : public ZSWindow
{
private:
	ITEM_SLOT_SOURCE_T SourceType;
	void *pSource;
	GameItem *pThing;
	static ZSItemSlot *Grabbed;
	DWORD ShowDescTime;
	BOOL LBDown;
	BOOL RBDown;


public:

	void SetSource(void *pNewSource) { pSource = pNewSource; };
	void *GetSource() { return pSource; };

	void SetSourceType(ITEM_SLOT_SOURCE_T NType) { SourceType = NType; };
	ITEM_SLOT_SOURCE_T GetSourceType() { return SourceType; };

	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);

	int DrawMouse(int x, int y);

	int MoveMouse(long *x, long *y, long *z);


	int Give(GameItem *pNewThing)
	{
		pThing = pNewThing;
		return TRUE;
	}
	
	GameItem *Take()
	{
		GameItem *pReturn = pThing;
		pThing = NULL;
		return pReturn;
	}

	GameItem *Look()
	{
		return pThing;
	}

	int Draw();

	ZSItemSlot(int NewID, int x, int y, int width, int height, GameItem *NewThing = NULL);

};

#endif