#include "equipwin.h"
#include "zsutilities.h"
#include "zsengine.h"
#include "zsitemslot.h"
#include "things.h"
#include "items.h"
#include "creatures.h"
#include "gameitem.h"
#include <assert.h>

typedef enum
{
	EQUIP_SLOT_HEAD = 10,
	EQUIP_SLOT_NECK,
	EQUIP_SLOT_CHEST,
	EQUIP_SLOT_AMMO,
	EQUIP_SLOT_GLOVES,
	EQUIP_SLOT_RHAND,
	EQUIP_SLOT_LHAND,
	EQUIP_SLOT_RRING,
	EQUIP_SLOT_LRING,
	EQUIP_SLOT_LEGS,
	EQUIP_SLOT_FEET,
} EQUIP_SLOT_IDS;

void EquipWin::ClearSlots()
{
	ZSItemSlot *pISlot;
	for(int n = EQUIP_SLOT_HEAD; n <= EQUIP_SLOT_FEET; n++)
	{
		pISlot = (ZSItemSlot *)GetChild(n);
		if(pISlot->Look())
		{
			pISlot->Give(NULL);
		}
	}
}

void EquipWin::SetSlots()
{
	Creature *pCreature;
	pCreature = (Creature *)pTarget;

	ClearSlots();

	ZSItemSlot *pISlot;

	GameItem *pGI;
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_HEAD);
	pGI = pCreature->GetEquipment("HEAD");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_NECK);
	pGI = pCreature->GetEquipment("NECK");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_CHEST);
	pGI = pCreature->GetEquipment("CHEST");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_AMMO);
	pGI = pCreature->GetEquipment("AMMO");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_GLOVES);
	pGI = pCreature->GetEquipment("GLOVES");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_RHAND);
	pGI = pCreature->GetEquipment("RIGHTHAND");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_LHAND);
	pGI = pCreature->GetEquipment("LEFTHAND");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_RRING);
	pGI = pCreature->GetEquipment("RIGHTRING");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_LRING);
	pGI = pCreature->GetEquipment("LEFTRING");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_LEGS);
	pGI = pCreature->GetEquipment("LEGS");
	pISlot->Give(pGI);
	
	pISlot = (ZSItemSlot *)GetChild( EQUIP_SLOT_FEET);
	pGI = pCreature->GetEquipment("FEET");
	pISlot->Give(pGI);
	

}

int EquipWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_ITEMSLOT_CHANGED)
	{
		SetSlots();
	}
	return TRUE;
	/*
		ZSItemSlot *pISlot;
		pISlot = (ZSItemSlot *)GetChild(IDFrom);
		int ItemNum;
		int Index;

		switch(IDFrom)
		{
		case EQUIP_SLOT_HEAD:
			Index = pTarget->GetIndex("HEAD");
			break;
		case EQUIP_SLOT_NECK:
			Index = pTarget->GetIndex("NECK");
			break;
		case EQUIP_SLOT_CHEST:
			Index = pTarget->GetIndex("CHEST");
			break;
		case EQUIP_SLOT_AMMO:
			Index = pTarget->GetIndex("AMMO");
			break;
		case EQUIP_SLOT_GLOVES:
			Index = pTarget->GetIndex("GLOVES");
			break;
		case EQUIP_SLOT_RHAND:
			Index = pTarget->GetIndex("RIGHTHAND");
			break;
		case EQUIP_SLOT_LHAND:
			Index = pTarget->GetIndex("LEFTHAND");
			break;
		case EQUIP_SLOT_RRING:
			Index = pTarget->GetIndex("RIGHTRING");
			break;
		case EQUIP_SLOT_LRING:
			Index = pTarget->GetIndex("LEFTRING");
			break;
		case EQUIP_SLOT_LEGS:
			Index = pTarget->GetIndex("LEGS");
			break;
		case EQUIP_SLOT_FEET:
			Index = pTarget->GetIndex("FEET");
			break;
		default:			
			DEBUG_INFO("Unknown EquipSlot\n");
			exit(1);
			break;
		}
		ItemNum = pTarget->GetData(Index).Value;
		Creature *pCreature;
		pCreature = (Creature *)pTarget;
		if(ItemNum)
		{
			//unequip the current item
			Thing *pThing;
			pThing = Thing::Find(Item::GetFirst(),ItemNum);
			pCreature->Unequip(pThing);
			
			pTarget->SetData(Index,0);
		}

		if(pISlot->Look())
		{
			if(pCreature->Equip(pISlot->Look()->GetItem(), Index))
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	return TRUE;
	*/
}

EquipWin::EquipWin(int NewID, int x, int y, int width, int height, Thing *pNewTarget)
{
	//I can not exist without a target
	assert(pNewTarget);
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Moveable = FALSE;
	Type = WINDOW_EQUIP;

	//set up the bounding rect
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	pTarget = pNewTarget;

	FILE *fp;
	fp = SafeFileOpen("gui.ini","rt");

	if(pTarget->GetData(INDEX_SEX).Value)
	{
		SeekTo(fp,"[FEMALEEQUIP]");
	}
	else
	{
		SeekTo(fp,"[MALEEQUIP]");
	}

	ZSItemSlot *pISlot;

	int ChildX;
	int ChildY;
	char *FileName;

	SeekTo(fp,"BACKGROUND");
	FileName = GetStringNoWhite(fp);
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName,ChildX,ChildY,NULL,0);
	
	delete FileName;

	SeekTo(fp,"HEAD");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_HEAD,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);

	SeekTo(fp,"NECK");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_NECK,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"CHEST");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_CHEST,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"AMMO");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_AMMO,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"GLOVES");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_GLOVES,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"RHAND");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_RHAND,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"LHAND");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_LHAND,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"RRING");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_RRING,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"LRING");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_LRING,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"LEGS");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_LEGS,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);
	
	SeekTo(fp,"FEET");
	ChildX = GetInt(fp);
	ChildY = GetInt(fp);
	pISlot = new ZSItemSlot(EQUIP_SLOT_FEET,x + ChildX,y + ChildY,32,32);
	pISlot->Show();
	pISlot->SetSourceType(SOURCE_EQUIP);
	pISlot->SetSource((void *)this);
	AddChild(pISlot);

	fclose(fp);

	SetSlots();

}

BOOL EquipWin::ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y)
{
	Creature *pCreature;
	pCreature = (Creature *)pTarget;
	GameItem *pGI;
	pGI = (GameItem *)pToReceive;

	if(pCreature->Equip(pGI, pGI->GetQuantity()))
	{
		 SetSlots();
		 return TRUE;
	}
	
	return FALSE;
}

