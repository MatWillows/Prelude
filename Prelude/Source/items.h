//********************************************************************* 
//*                                                                                                                                    ** 
//**************				items.h          ********************* 
//**                                                                                                                                  ** 
//**                                                                                                                                  ** 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:    7/20                                                                                          * 
//*Revisor:     mat williams                                          * 
//*Purpose:     defines class for handling items in Prelude              * 
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*														              * 
//********************************************************************* 
//*********************************************************************
#ifndef ITEMS_H
#define ITEMS_H

#include "things.h"		//case thing class

//*******************************CLASS******************************** 
//**************                                 ********************* 
//**						Item                                  ** 
//******************************************************************** 
//*Purpose:     class Item provides functionality for items in prelude
//* 
//******************************************************************** 
//*Invariants:                                                                                                                     * 
//*                                                                  * 
//********************************************************************

typedef enum
{
	ITEM_TYPE_NONE = 0,
	ITEM_TYPE_QUEST,
	ITEM_TYPE_WEAPON,
	ITEM_TYPE_ARMOR,
	ITEM_TYPE_FOOD,
	ITEM_TYPE_OTHER,
	ITEM_TYPE_AMMO,
} ITEM_T;

class Item : public Thing
{
private:
//************************************************************************************** 
//                             MEMBER VARIABLES 

   static char ItemFieldNames[96*32];
	static int NumItems;
	static Item *pFirstItem;
	static LPDIRECTDRAWSURFACE7 ItemPics;
	static DATA_T *ItemDataTypes;
	RECT rDraw2DFrom;

//************************************************************************************** 

public:

// Display Functions --------------------------------
	void Draw2D(LPDIRECTDRAWSURFACE7 DestSurface, RECT *rTo);
	void CreateFromRect(); //help function to create the 2d from rectangle

// Accessors ----------------------------------------

   int GetValidActions(Thing *pActor, ActionMenu *pAMenu);
   int GetDefaultAction(Thing *pActor);

	int GetNumItems();
	static Item *GetFirst();
	OBJECT_T GetObjectType() { return OBJECT_ITEM; }

// Mutators -----------------------------------------

   int Update(void);				
	BOOL AdvanceFrame();
 
   //actions that may be performed on items
   int Use(Thing *pUser);
   int LookAt(Object *pLooker);
   int TakeDamage(Thing *pDamageSource, int DamageAmount, DAMAGE_T DamageType);
   int Equip(Thing *pEquipper);
   int UnEquip(Thing *pUnEquipper);
   int PlaceFromCursor(Thing *pDropper, D3DVECTOR *pDropPoint);
   int PickUp(Thing *pPicker);
   int PutDown(Thing *pPutter);
   int GrabToCursor(Thing *pGrabber);

	int SetFirst(Item *pNewFirst);

	friend int LoadItems(FILE *fp);
	friend int LoadBinItems(FILE *fp);
	friend int SaveItems(FILE *fp);
	friend int SaveBinItems(FILE *fp);
	friend int DeleteItems();

	static Item *FindItem(int UID);

	BOOL RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
   
// Output ---------------------------------------------

// Constructors ---------------------------------------

   Item();
   Item(Thing *pThingFrom);

// Destructor -----------------------------------------

   ~Item();

// Debug ----------------------------------------------

};

int LoadItems(FILE *fp);
int SaveItems(FILE *fp);
int DeleteItems();


#endif
