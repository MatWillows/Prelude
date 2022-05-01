//********************************************************************* 
//********************************************************************* 
//**************                items.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:  7/20                                                    
//*Revisor:   mat williams                                           
//*Purpose:   provide functionality for items in the Prelude                  
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*        	  no actual functionality implemented               
//*				only PDL
//********************************************************************* 
//*********************************************************************
#include "items.h"
#include "creatures.h"
#include "events.h"
#include <assert.h>

#define ITEM_2D_HEIGHT   32
#define ITEM_2D_WIDTH    32

//************** static Members *********************************
char Item::ItemFieldNames[96*32];
int Item::NumItems = 0;
Item *Item::pFirstItem;
LPDIRECTDRAWSURFACE7 Item::ItemPics = NULL;
DATA_T *Item::ItemDataTypes = NULL;


//************** Constructors  ****************************************
//all constructors are handled through base thing class for items

//simple constructor
Item::Item()
{
	//increment the number of items in the game
	NumItems++;
	pNext = NULL;
}

//copy constructor
Item::Item(Thing *)
{
	//increment the number of items in the game
	NumItems++;
}

//end:  Constructors ***************************************************



//*************** Destructor *******************************************

Item::~Item()
{
	//if the item is in someone's inventory or in a container make sure 
	//it is cleared from it's current location
	NumItems--;
	if(DataTypes != ItemDataTypes)
	{
		delete[] DataTypes;
	}
	if(!NumItems)
	{
		DestroyFieldNames();
		if(ItemDataTypes)
		{
			delete[] ItemDataTypes;
		}
		ItemDataTypes = NULL;
	}
}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************


//end: Accessors *******************************************************

int Item::GetValidActions(Thing *pActor, ActionMenu *pAMenu)
{
	//Top is always look at
	pAMenu->AddAction(ACTION_LOOKAT,(void *)pActor,0,"Examine");

	//fill the menu struct with all the actions which may be taken on 
	//this item
	//first check to see where the item is
	//if it is in the game world
		//see how close it is to the actor
			//check the useable field to determine if the item is useable,
				//if the item is useable and the actor is in range
				//add a use option to the action menu 
			//check the pick up field to see if the item is capable of being picked up
				//add a pickup option to the action menu if the actor is in range to pick it up
				//add a grab option to the action menu if the actor is close enough
	//if it is equipped
		//add an unequip option to the menu
		//add a drop option to the menu
		//add a grab option to the menu
		//if the item is useable add a use option
	//if it is in inventory
		//add an equip option to the menu
		//add a drop option to the menu
		//add a grab option to the menu
		//if the item is useable add a use option
	//add a look at option
	//done
	return TRUE;
} 

int Item::GetDefaultAction(Thing *Actor)
{
	//if the item is a container the default action is to use it
	//if the item is capable of being picked up
		//then the default action is to grab it
	//otherwise
		//if the item is useable 
			//then the default action is to use it
	//otherwise
		//there is no default action
	//done
	return TRUE;

}

int Item::GetNumItems()
{
	return NumItems;

}

Item *Item::GetFirst()
{
	return pFirstItem;
}

Item *Item::FindItem(int UID)
{
	Item *pItem;

	pItem = pFirstItem;

	while(pItem)
	{
		if(pItem->GetData("ID").Value == UID)
		{
			return pItem;
		}
		else
		{
			pItem = (Item *)pItem->GetNext();
		}
	}

	//FATAL_ERROR("FindItem(): Item #%i does not exist.",UID);
	return NULL;

}

//************  Display Functions  *************************************
//handled entirely through the base thing class
//except 2d display
void Item::Draw2D(LPDIRECTDRAWSURFACE7 DestSurface, RECT *rTo)
{
	//get the picture number and calculate source rectangle
	HRESULT hr;
	hr = DestSurface->Blt(rTo, ItemPics, &rDraw2DFrom, DDBLT_KEYSRC,NULL);
	if(FAILED(hr))
	{
		Engine->ReportError(hr);
		DEBUG_INFO(GetData(INDEX_NAME).String);
		DEBUG_INFO(":  bad two-d item draw\n");
	}
}
		

void Item::CreateFromRect()
{
	int PicNum;
	PicNum = GetData("PIC").Value - 1;
	rDraw2DFrom.left = (PicNum % 20) * ITEM_2D_WIDTH;
	rDraw2DFrom.right = rDraw2DFrom.left + ITEM_2D_WIDTH;
	rDraw2DFrom.top = (PicNum / 20) * ITEM_2D_HEIGHT;
	rDraw2DFrom.bottom = rDraw2DFrom.top + ITEM_2D_HEIGHT;
}


//end: Display functions ***********************************************



//************ Mutators ************************************************
int Item::Update(void)
{
	//items do nothing upon update
	return TRUE;
	
}	

BOOL Item::AdvanceFrame()
{
	return TRUE;
}
	
//actions that may be performed on items

//use the item
//	useable items other than containers require scripted use functions
int Item::Use(Thing *pUser)
{
	//check to be sure the user is in range to use this item
	//call the use function if it exists in the use over-ride field
	//if the item is a container open a container window with its contents
	return TRUE;
}

//LookAt
//	examine the item
//	unles the designer provides a script override function
//	only basic information is displayed
int Item::LookAt(Object *pLooker)
{
	SafeExit("Should not get into item lookats");
/*
	char LookAtString[256];
	int averagedamage;
	//check to see if there is a lookat over-ride
	if(GetData("LOOKATOVERRIDE").Value)
	{
		PreludeEvents.RunEvent(GetData("LOOKATOVERRIDE").Value);
		return TRUE;
	}
	//otherwise
	//check what type of item this is
	switch(GetData(INDEX_TYPE).Value)
	{
	case ITEM_TYPE_WEAPON:
		//weapon
		//display the item's name, damage, speed, and strength requirements 
		char Blarg[16];
		switch(GetData("SUBTYPE").Value)
		{
		case WEAPON_TYPE_SWORD:
			sprintf(Blarg,"Sword\n");
			break;
		case WEAPON_TYPE_DAGGER:
			sprintf(Blarg,"Dagger\n");
			break;
		case WEAPON_TYPE_AXE:
			sprintf(Blarg,"Axe\n");
			break;
		case WEAPON_TYPE_BLUNT:
			sprintf(Blarg,"Blunt\n");
			break;
		case WEAPON_TYPE_POLEARM:
			sprintf(Blarg,"Polearm\n");
			break;
		case WEAPON_TYPE_MISSILE:
			sprintf(Blarg,"Missile\n");
			break;
		default:
		case WEAPON_TYPE_UNARMED:
			sprintf(Blarg,"Unarmed\n");
			break;
		case WEAPON_TYPE_THROWN:
			sprintf(Blarg,"Thrown\n");
			break;
		}
		averagedamage = (GetData("MINDAMAGE").Value + GetData("MAXDAMAGE").Value + 1)/2;
		sprintf(LookAtString,"%s\n  %s\n  Speed: %i\n  Damage: %i", 
			Blarg,
			GetData("NAME").String,
			GetData("SPEED").Value,
			averagedamage);
		Describe(LookAtString);
		break;	
	case ITEM_TYPE_ARMOR:
		//armor
		//display the items' name, absorbtion rating, and strength requirements
		averagedamage = (GetData("ARMORMAX").Value + GetData("ARMORMIN").Value + 1)/2;
		sprintf(LookAtString,"%s\n  Speed: -%i\n  Absorbtion: %i",
			GetData("NAME").String,
			GetData("SPEED").Value,
			averagedamage);
		Describe(LookAtString);		
		break;
	default:
		//for all other types of items, just display the item's name
		sprintf(LookAtString,"%s",GetData("NAME"));
		Describe(LookAtString);
		break;
	}
*/	return TRUE;
}

//TakeDamage
//	deal damage to an item
//  This may be the direct result of a user action, i.e. attacking the item
//  Or it may be indirect due to an explosion
int Item::TakeDamage(Thing *pDamageSource, int DamageAmount, DAMAGE_T DamageType)
{
	//check for a a takedamage over-ride
		//most commonly the damage override is the same as the use override
		//it could also be used to make a bomb explode or some such
	//subtract the item's durability from the damageamount
	//subtract the remaining damage from the item
	//inform the item's owner (if any) that the source of the damage damaged their item
	//if the item has zero or fewer hit points remaining remove it from the game
	return TRUE;
	
}

//Equip
//	Equip an item to a PC
int Item::Equip(Thing *pEquipper)
{
	//check to see if there is an equip over-ride
	if(strcmp(GetData("UNEQUIPOVERRIDE").String,"NONE"))
	{
		return TRUE;
	}
	//otherwise perform the default equip routine based on the type of item
	switch(GetData("ITEMTYPE").Value)
	{
		case ITEM_TYPE_WEAPON:
			//weapon
			//check to be sure that the equipper is strong enough to wield the weapon
			if(GetData("MINSTR").Value > pEquipper->GetData(INDEX_STRENGTH).Value) 
			{
			//	//GOT_HERE("%s isn't strong enough ",pEquipper->GetData("NAME").String);
			//	//GOT_HERE("to equip a %s\n", GetData("NAME").String);
				return FALSE;
			}
			if(GetData("MINDEX").Value > pEquipper->GetData(INDEX_DEXTERITY).Value)
			{
				//GOT_HERE("%s isn't dextrous enough ",pEquipper->GetData("NAME").String);
				//GOT_HERE("to equip a %s\n", GetData("NAME").String);
				return FALSE;
			}
			//check to see that there are enough free hands to wield the weapon
			if(GetData("NUMBERHANDS").Value == 2)
			{
				//if there is anything in either hand the item can't be equipped
				if(pEquipper->GetData("LEFTHAND").Value || pEquipper->GetData("RIGHTHAND").Value)
				{
					//GOT_HERE("%s doesn't have enough hands free  ",pEquipper->GetData("NAME").String);
					//GOT_HERE("to equip a %s\n", GetData("NAME").String);
					return FALSE;
				}
			}
			else
			{
				if(pEquipper-GetData("RIGHTHAND").Value)
				{
					//GOT_HERE("%s doesn't have enough hands free  ",pEquipper->GetData("NAME").String);
					//GOT_HERE("to equip a %s\n", GetData("NAME").String);
					return FALSE;
				}
			}
			
			//place the weapon in equippers first empty hand
			pEquipper->SetData("RIGHTHAND",GetData("ID").Value);
		
			//add any special bonuses to the equipper's characteristics
			pEquipper->SetData("MINDAMAGE", 
										pEquipper->GetData("MINDAMAGE").Value + GetData("MINDAMAGE").Value);
			
			pEquipper->SetData("MAXDAMAGE", 
										pEquipper->GetData("MAXDAMAGE").Value + GetData("MAXDAMAGE").Value);
			
			pEquipper->SetData("RANGE",
										pEquipper->GetData("RANGE").Value + GetData("RANGE").Value);


			pEquipper->SetData("ATTACKPOINTS",
										pEquipper->GetData("ATTACKPOINTS").Value + GetData("SPEED").Value);
			break;
		case ITEM_TYPE_ARMOR:
			//armor
			//check to see that the correct slot is empty
			if(pEquipper->GetData(GetData("EQUIPLOCATION").String).Value)
			{
				//GOT_HERE("%s doesn't have enough a free location ",pEquipper->GetData("NAME").String);
				//GOT_HERE("to equip a %s\n", GetData("NAME").String);
				return FALSE;
			}
			//check to be sure that the equipper is strong enought to wear the armor
			if(GetData("MINSTR").Value > pEquipper->GetData("STRENGTH").Value) 
			{
				//GOT_HERE("%s isn't strong enough ",pEquipper->GetData("NAME").String);
				//GOT_HERE("to equip a %s\n", GetData("NAME").String);
				return FALSE;
			}
			if(GetData("MINDEX").Value > pEquipper->GetData("DEXTERITY").Value)
			{
				//GOT_HERE("%s isn't dextrous enough ",pEquipper->GetData("NAME").String);
				//GOT_HERE("to equip a %s\n", GetData("NAME").String);
				return FALSE;
			}
			//place the armor in the proper slot
			pEquipper->SetData(pEquipper->GetData(GetData("EQUIPLOCATION").String).Value,
										GetData("UID").Value);
	
			break;
		default:
			//the user should never have the opportunity to equip any other types of items without an override
			//return an error if they do
			//FATAL_ERROR("Attempt to equip unequippable item",0);
			break;
	}
	//add any special bonuses to the equipper's characteristics
	if(GetData("MOD1").Value)
	{
		pEquipper->SetData(GetData("STAT1").String,
								pEquipper->GetData(GetData("STAT1").String).Value + GetData("MOD1").Value);
	}

	if(GetData("MOD2").Value)
	{
		pEquipper->SetData(GetData("STAT2").String,
								pEquipper->GetData(GetData("STAT2").String).Value + GetData("MOD2").Value);
	}

	if(GetData("MOD3").Value)
	{
		pEquipper->SetData(GetData("STAT3").String,
								pEquipper->GetData(GetData("STAT3").String).Value + GetData("MOD3").Value);
	}

	if(GetData("MOD4").Value)
	{
		pEquipper->SetData(GetData("STAT4").String,
								pEquipper->GetData(GetData("STAT4").String).Value + GetData("MOD4").Value);
	}

	if(GetData("MOD5").Value)
	{
		pEquipper->SetData(GetData("STAT5").String,
								pEquipper->GetData(GetData("STAT5").String).Value + GetData("MOD5").Value);
	}

	//remove the item from the equipper's inventory
//	((Creature *)pEquipper)->RemoveItem(this);
	
	//GOT_HERE("%s has equipped ",pEquipper->GetData("NAME").String);
	//GOT_HERE("%s.\n",GetData("NAME").String);
	return TRUE;
}

//UnEquip
//	UnEquip an item from a PC
int Item::UnEquip(Thing *pUnEquipper)
{
	//check to see if there is an unequip over-ride
	if(strcmp(GetData("UNEQUIPOVERRIDE").String,"NONE"))
	{
		return TRUE;
	}
	//check to see if there is enough room in the unequipper's inventory to hold the item
//	if(!((Creature *)pUnEquipper)->AddItem(this))
//	{
		//GOT_HERE("%s doesn't have the room in their inventory ",pUnEquipper->GetData("NAME").String);
		//GOT_HERE("to take off the %s",GetData("NAME").String);
//		return FALSE;
//	}
	
	//otherwise perform the default unUnEquip routine based on the type of item
	switch(GetData("ITEMTYPE").Value)
	{
		case ITEM_TYPE_WEAPON:
			//weapon
			//check to be sure that the UnEquipper is strong enough to wield the weapon
			//place the weapon in UnEquippers first empty hand
			pUnEquipper->SetData("RIGHTHAND",0);
		
			//add any special bonuses to the UnEquipper's characteristics
			pUnEquipper->SetData("MINDAMAGE", 
										pUnEquipper->GetData("MINDAMAGE").Value - GetData("MINDAMAGE").Value);
		
			pUnEquipper->SetData("MAXDAMAGE", 
										pUnEquipper->GetData("MAXDAMAGE").Value - GetData("MAXDAMAGE").Value);
			
			pUnEquipper->SetData("RANGE",
										pUnEquipper->GetData("RANGE").Value - GetData("RANGE").Value);


			pUnEquipper->SetData("ATTACKPOINTS",
										pUnEquipper->GetData("ATTACKPOINTS").Value - GetData("SPEED").Value);
			break;
		case ITEM_TYPE_ARMOR:
			//armor
			//remove the armor in the proper slot
			pUnEquipper->SetData(pUnEquipper->GetData(GetData("EQUIPLOCATION").String).Value,0);
			break;
		default:
			//the user should never have the opportunity to UnEquip any other types of items without an override
			//return an error if they do
			//FATAL_ERROR("Attempt to unUnEquip unUnEquippable item %s",GetData("NAME"));
			break;
	}
	//add any special bonuses to the UnEquipper's characteristics
	if(GetData("MOD1").Value)
	{
		pUnEquipper->SetData(GetData("STAT1").String,
								pUnEquipper->GetData(GetData("STAT1").String).Value - GetData("MOD1").Value);
	}

	if(GetData("MOD2").Value)
	{
		pUnEquipper->SetData(GetData("STAT2").String,
								pUnEquipper->GetData(GetData("STAT2").String).Value - GetData("MOD2").Value);
	}

	if(GetData("MOD3").Value)
	{
		pUnEquipper->SetData(GetData("STAT3").String,
								pUnEquipper->GetData(GetData("STAT3").String).Value - GetData("MOD3").Value);
	}

	if(GetData("MOD4").Value)
	{
		pUnEquipper->SetData(GetData("STAT4").String,
								pUnEquipper->GetData(GetData("STAT4").String).Value - GetData("MOD4").Value);
	}

	if(GetData("MOD5").Value)
	{
		pUnEquipper->SetData(GetData("STAT5").String,
								pUnEquipper->GetData(GetData("STAT5").String).Value - GetData("MOD5").Value);
	}

	//GOT_HERE("%s has UnEquipped ",pUnEquipper->GetData("NAME").String);
	//GOT_HERE("%s.\n",GetData("NAME").String);
	return TRUE;
}

//PlaceFromCursor
//	Place an item into the game world at the specified point from the game cursor
int Item::PlaceFromCursor(Thing *pDropper, D3DVECTOR *pDropPoint)
{
	//make sure the dropper is close enough to the drop point

	//if they are 
		//change the item's location to be in the world
		//change the item's position to equal the drop point
		//remove the item from the cursor
		//add the item to the world reference lists

	//otherwise display an unable to drop there message
	return TRUE;
}

//PickUp
//	take an item from the game world an put it into a creature's inventory
int Item::PickUp(Thing *pPicker)
{
	//make sure the creature picking up the item is close enough

	//if they are and they have room in their inventory
		//change the item's location to be in an inventory
		//if the item's owner is in sight range tell the owner that the item
		//is being taken by the creature
		//make the creature the item's owner
		//clear the item's position
		//add the item to the creature's inventory

	//if the thing picking up the item is a PC 
		//if they are not close enough
			//tell the user they aren't close enough
		//otherwise
		//if they don't have room in their inventory
			//tell the user they don't have room for the item
	return TRUE;
}

//PutDown
//	Drop an item into the game world from a creature's inventory
int Item::PutDown(Thing *pPutter)
{
	//remove the item from the putter's inventory
	//change the item's location to be in the world
	//place the item on the ground by setting it's x and y position to be the
	//same as the putter and it's z position to be the ground height.
	//clear the item's ownership
	return TRUE;
} 

//GrabToCursor
//	Grab the item from it's current location and place it into the cursor
int Item::GrabToCursor(Thing *pGrabber)
{
	//make sure the grabber is in range
	//put the item into the item cursor
	//if the item has an owner and the owner is in sight range tell the owner that
	//the item is being taken by the grabber
	//done
	return TRUE;
}

int Item::SetFirst(Item *pNewFirst)
{
	pFirstItem = pNewFirst;
	return TRUE;
}

//end: Mutators ********************************************************



//************ Outputs *************************************************

//end: Outputs ********************************************************



//************ Debug ***************************************************

//end: Debug ***********************************************************

int LoadBinItems(FILE *fp)
{
	int NumItems;

	Item *pItem, *pNextItem;

	assert(fp);

	fread(&NumItems,sizeof(int),1,fp);

	pItem = new Item;
	pItem->SetFirst(pItem);
	fread(&pItem->NumFields,sizeof(int),1,fp);
	pItem->DataTypes = Item::ItemDataTypes = new DATA_T[pItem->NumFields];
	fread(pItem->DataTypes,sizeof(DATA_T),pItem->NumFields,fp);

	pItem->LoadFieldNames(fp,Item::ItemFieldNames);

	for(int n = 0; n < NumItems -1; n++)
	{
		pItem->LoadBin(fp);
		pItem->CreateFromRect();
		
		pNextItem = new Item;
		pNextItem->SetFieldNames(pItem->GetFieldNames());
		pNextItem->DataTypes = pItem->DataTypes;
		pNextItem->NumFields = pItem->NumFields;
		pItem->SetNext(pNextItem);
		pItem = pNextItem;
	}

	pItem->LoadBin(fp);
	pItem->CreateFromRect();
	pItem->SetNext(NULL);

	DEBUG_INFO("Loading Item pictures\n");

	Item::ItemPics = Engine->Graphics()->CreateSurfaceFromFile("itemart.bmp",640,480,NULL,COLOR_KEY_FROM_FILE);
	
	DEBUG_INFO("Pictures loaded\n");

	DEBUG_INFO("Loaded Binary Items\n");

	char blarg[64];
	sprintf(blarg,"There are %i Items\n",Item::NumItems);
	DEBUG_INFO(blarg);

	return TRUE;
}


int SaveBinItems(FILE *fp)
{
	Item *pItem;

	pItem = Item::GetFirst();

	fwrite(&pItem->NumItems,sizeof(int),1,fp);
	fwrite(&pItem->NumFields,sizeof(int),1,fp);
	fwrite(pItem->DataTypes,sizeof(DATA_T),pItem->NumFields,fp);
	
	pItem->SaveFieldNames(fp);

	while(pItem)
	{
		pItem->SaveBin(fp);
		pItem = (Item *)pItem->GetNext();
	}
	DEBUG_INFO("Saved Bin Items");
	return TRUE;
}

int LoadItems(FILE *fp)
{
	DEBUG_INFO("Loading Items\n");
	Item *pItem, *pNextItem;

	assert(fp);

	pItem = new Item;
	pItem->SetFirst(pItem);

	pItem->LoadFieldNames(fp,Item::ItemFieldNames);

	char c;

	while(TRUE)
	{
		pItem->LoadData(fp);
		pItem->CreateFromRect();
		c = (char)fgetc(fp);

		pItem->SetFrame(pItem->GetData(INDEX_FRAME).Value);

		if(feof(fp) || c == THING_SEPERATION_CHARACTER)
		{
			break;
		}
		else
		{
			fseek(fp,-1,1);
			pNextItem = new Item;
			pNextItem->SetFieldNames(pItem->GetFieldNames());
			pItem->SetNext(pNextItem);
			((Thing *)pItem->GetNext())->SetNumFields(pItem->GetNumFields());
			pItem = pNextItem;
		}
	}

	DEBUG_INFO("Loading Item pictures\n");

	Item::ItemPics = Engine->Graphics()->CreateSurfaceFromFile("itemart.bmp",640,416,NULL,COLOR_KEY_FROM_FILE);
	

	DEBUG_INFO("Pictures loaded\n");


	DEBUG_INFO("Loaded Items\n");
	char blarg[64];
	sprintf(blarg,"There are %i Items\n",Item::NumItems);
	DEBUG_INFO(blarg);
	return TRUE;
}

int SaveItems(FILE *fp)
{
	Item *pItem;

	pItem = Item::GetFirst();

	pItem->SaveFieldNames(fp);

	while(pItem)
	{
		pItem->SaveData(fp);
		pItem = (Item *)pItem->GetNext();
	}
	return TRUE;
}


int DeleteItems()
{
	Item *pItem, *pToDelete;

	//get the first Item
	pToDelete = pItem = Item::pFirstItem;

	while(pItem)
	{
		pToDelete = pItem;
		pItem = (Item *)pItem->GetNext();
		delete pToDelete;
	}

	if(Item::ItemPics) 
	{
		Item::ItemPics->Release();
		Item::ItemPics = NULL;
	}

	DEBUG_INFO("Deleted Items\n");
	char blarg[64];
	sprintf(blarg,"There are %i remaining\n",Item::NumItems);
	DEBUG_INFO(blarg);

	Item::pFirstItem = NULL;

	return TRUE;
}

BOOL Item::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	int ItemX;
	int ItemY;
	int RayX;
	int RayY;
	RayX = (int)vRayEnd->x;
	RayY = (int)vRayEnd->y;
	ItemX = (int)GetPosition()->x;
	ItemY = (int)GetPosition()->y;

	if(RayX == ItemX && RayY == ItemY)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
