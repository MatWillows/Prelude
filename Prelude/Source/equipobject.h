#ifndef EQUIPOBJECT_H
#define EQUIPOBJECT_H

#include "objects.h"
#include "things.h"

class EquipObject : public Object
{
private:
	Thing *pItem; 
	Thing *pLink;
	EQUIP_POSITION LinkSlot;

public:

	Thing *GetItem() { return pItem; }
	EQUIP_POSITION GetEquipPosition() { return LinkSlot; }

	void SetItem(Thing *NewItem) { pItem = NewItem; }
	void SetPosition(EQUIP_POSITION NewPosition) { LinkSlot = NewPosition; }
	
	void Draw();

	Thing *GetLink() { return pLink; }
	void SetLink(Thing *pNewLink) { pLink = pNewLink; }

	EquipObject(Thing *pNewLink, Thing *pItem, EQUIP_POSITION NewLinkSlot);

	OBJECT_T GetObjectType() { return OBJECT_EQUIP; }

};

#endif
