#ifndef CONTAINERS_H
#define CONTAINERS_H

#include "objects.h"

class Item;
class GameItem;
class Thing;
class ActionMenu;

class Container : public Object
{
private:
	int Size;

public:
	
	OBJECT_T GetObjectType() { return OBJECT_CONTAINER; }

	int AltHighLight();

	int GetDefaultAction(Object *pactor);
	int GetValidActions(Thing *pActor, ActionMenu *pAMenu);

	Container();

	~Container();
	
	


};

#endif