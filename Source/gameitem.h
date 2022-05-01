#ifndef GAMEITEM_H
#define GAMEITEM_H

#include "items.h"
#include "objects.H"

typedef enum
{
	LOCATION_NONE,
	LOCATION_PERSON,
	LOCATION_EQUIPPED,
	LOCATION_CONTAINER,
	LOCATION_WORLD,
	LOCATION_BARTER,
} LOCATION_T;

class Region;

class GameItem : public Object
{
private:
	static BOOL VisualScaling;
	LOCATION_T Location;
	Object *pOwner;
	Item *pItem;
	int Quantity;
	Region *pRegionIn;
	int ModStat;
	int ModAmount;
	ULONG ModEnd;
	float VisScale;

public:
	Region *GetRegionIn() { return pRegionIn; }
	void SetRegionIn(Region *pNewRegionIn) { pRegionIn = pNewRegionIn; }

	D3DVECTOR *GetPosition();

	float GetVisScale() { return VisScale; }

	int GetModStat() { return ModStat; }
	int GetModAmount() { return ModAmount; }
	ULONG GetModEnd() { return ModEnd; }

	void SetModStat(int NewStat) { ModStat = NewStat; }
	void SetModAmount(int NewAmount) { ModAmount = NewAmount; }
	void SetModEnd(ULONG NewEnd) { ModEnd = NewEnd; }
	void SetVisScale(float NewScale) { VisScale = NewScale; }

	BOOL TileBlock(int TileX, int TileY);

	BOOL AdvanceFrame();

	int AltHighLight();

	void Draw();

	OBJECT_T GetObjectType() { return OBJECT_ITEM; }

	Item *GetItem() { return pItem; }
	void SetItem(Item *pNewItem, int NewQuantity = 0);
	void SetItem(int CompressedValue);

	int GetQuantity() { if(Quantity) return Quantity; else return 1;}
	void SetQuantity(int NewQuantity) { Quantity = NewQuantity; }
	int GetCompressed();
	int GetDefaultAction(Object *pactor);
	

	void Save(FILE *fp);
	void Load(FILE *fp);
	
	int GetValidActions(Thing *pActor, ActionMenu *pAMenu);

	//split a game item in two
	GameItem *Split(int Amount);
	
	GameItem();

	GameItem(int CompressedValue);

	GameItem(GameItem *pGIFrom);

	LOCATION_T GetLocation() { return Location; }
	void SetLocation(LOCATION_T At, Thing *NewOwner = NULL) { Location = At; pOwner = NewOwner; }

	Object *GetOwner() { return pOwner; };
	void SetOwner(Object *NewOwner) { pOwner = NewOwner; }
	int LookAt(Object *looker);

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
 
	void AddToArea(Area *pArea);
	void AddToWorld();

	operator = (GameItem &OtherThing);

	DATA_FIELD_T GetData(int fieldnum);			
    DATA_FIELD_T GetData(char *fieldname);	

	BOOL GetVisualScaling() { return VisualScaling; }
	void SetVisualScaling(BOOL IsScaled) { VisualScaling = IsScaled; }

};

#endif
