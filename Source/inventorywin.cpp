#include "inventorywin.h"
#include "zsengine.h"
#include "zsitemslot.h"
#include "things.h"
#include "items.h"
#include "zsutilities.h"
#include "gameitem.h"
#include "zsverticalscroll.h"
#include "creatures.h"
#include "barter.h"
#include "party.h"
#include <assert.h>

#define IDC_INVENTORY_SCROLL 1
#define SLOT_ID_START 10

void InventoryWin::Reset()
{
	SetSlots();
}


void InventoryWin::SetSlots()
{
	ClearSlots();
	
	assert(pOwner);

	Object *pObject;

	pObject = pOwner->GetContents();
	int NumItems = 0;
	int CurNum;
	CurNum = 0;
	if(pOwner->GetObjectType() == OBJECT_CREATURE && PreludeParty.IsMember((Creature *)pOwner))
	{

		while(pObject && CurNum < (TopRow * SlotsAcross))
		{
			CurNum++;
			NumItems++;
			pObject = pObject->GetNext();
		}

		if(!pObject)
		{ 
			return;
		}

		int SlotID = SLOT_ID_START;
		ZSItemSlot *pISlot;
		GameItem *pGI;
		int CurSlot = 0;
		while(pObject && CurSlot < NumSlots)
		{
			pGI = (GameItem *)pObject;
			//pGI = new GameItem((GameItem *)pObject);
			pISlot = (ZSItemSlot *)GetChild(SlotID++);
			assert(pISlot);
			//pGI->SetOwner(pOwner);
			pISlot->Give(pGI);
			NumItems++;
			CurSlot++;
			pObject = pObject->GetNext();
		}
	}
	else
	if(pOwner->GetObjectType() != OBJECT_CREATURE)
	{
		while(pObject && CurNum < (TopRow * SlotsAcross))
		{
			if(((GameItem *)pObject)->GetData("VALUE").Value == 0)
			{
		
			}
			else
			{
				CurNum++;
				NumItems++;
			}
			pObject = pObject->GetNext();
		}

		if(!pObject)
		{ 
			return;
		}

		int SlotID = SLOT_ID_START;
		ZSItemSlot *pISlot;
		GameItem *pGI;
		int CurSlot = 0;
		while(pObject && CurSlot < NumSlots)
		{
			if(((GameItem *)pObject)->GetData("VALUE").Value == 0)
			{
		
			}
			else
			{
				pGI = new GameItem((GameItem *)pObject);
				pISlot = (ZSItemSlot *)GetChild(SlotID++);
				assert(pISlot);
				pGI->SetOwner(pOwner);
				pISlot->Give(pGI);
				NumItems++;
				CurSlot++;
			}
			pObject = pObject->GetNext();
		}
	}
	else
	{
		while(pObject && CurNum < (TopRow * SlotsAcross))
		{
			if(((GameItem *)pObject)->GetData("VALUE").Value <= 0)
			{
		
			}
			else
			{
				CurNum++;
				NumItems++;
			}
			pObject = pObject->GetNext();
		}

		if(!pObject)
		{ 
			return;
		}

		int SlotID = SLOT_ID_START;
		ZSItemSlot *pISlot;
		GameItem *pGI;
		int CurSlot = 0;
		while(pObject && CurSlot < NumSlots)
		{
			if(((GameItem *)pObject)->GetData("VALUE").Value <= 0)
			{
		
			}
			else
			{
				pGI = new GameItem((GameItem *)pObject);
				pISlot = (ZSItemSlot *)GetChild(SlotID++);
				assert(pISlot);
				pGI->SetOwner(pOwner);
				pISlot->Give(pGI);
				NumItems++;
				CurSlot++;
			}
			pObject = pObject->GetNext();
		}
	}

	ZSVScroll *pScroll;
	pScroll = (ZSVScroll *)GetChild(IDC_INVENTORY_SCROLL);
	//count how many more;
	while(pObject)
	{
		NumItems++;
		pObject = pObject->GetNext();
	}

	if(NumItems > SlotsAcross * SlotsDeep)
	{
		pScroll->Show();
		pScroll->SetLower(0);
		pScroll->SetUpper(((NumItems / SlotsAcross) + 1) - SlotsDeep);
	}	
	else
	{
		pScroll->Hide();
		if(TopRow != 0)
		{
			TopRow = 0;
			SetSlots();
		}
	}

}

void InventoryWin::ClearSlots()
{
	int xn,yn;
	ZSItemSlot *pISlot;
	int SlotID = SLOT_ID_START;

	for(yn = 0; yn < SlotsDeep; yn++)
	for(xn = 0; xn < SlotsAcross; xn++)
	{
		pISlot = (ZSItemSlot *)GetChild(SlotID++);
		assert(pISlot);

		if(pISlot->Look())
		{
			pISlot->Give(NULL);
			//delete pISlot->Look();
		}
		pISlot->Give(NULL);
	}
}

int InventoryWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_SCROLL)
	{
		if(TopRow != Param)
		{
			TopRow = Param;
			SetSlots();
			return TRUE;
		}
	}
	
	return TRUE;
}


InventoryWin::InventoryWin(int NewID, int x, int y, int width, int height, Object *pNewOwner)
{
	//Inventory Windows can not exist without an owner
	assert(pNewOwner);
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Moveable = FALSE;
	Type = WINDOW_INVENTORY;

	//set up the bounding rect
	Bounds.left = x;
	Bounds.right = x + width + 8;
	Bounds.top = y;
	Bounds.bottom = y + height;

	pOwner = pNewOwner;

	FILE *fp;
	fp = SafeFileOpen ("gui.ini","rt");
	SeekTo(fp, "[INVENTORY]");

	SeekTo(fp, "BACKGROUND");
	int Width;
	int Height;
	int SlotX;
	int SlotY;
	int SlotYOffset;
	int SlotXOffset;
	char *FileName;

	FileName = GetStringNoWhite(fp);
	Width = GetInt(fp);
	Height = GetInt(fp);

	//BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName, Width, Height, NULL, NULL);

	CreateParchmentBorderedBackground(4,3);

	delete[] FileName;

	SeekTo(fp, "SLOTSACROSS");
	SlotsAcross = GetInt(fp);

	SeekTo(fp, "SLOTSDEEP");
	SlotsDeep = GetInt(fp);

	SeekTo(fp, "SLOTX");
	SlotX = GetInt(fp);

	SeekTo(fp, "SLOTY");
	SlotY = GetInt(fp);

	SeekTo(fp, "SLOTWIDTH");
	SlotXOffset = GetInt(fp);

	SeekTo(fp, "SLOTHEIGHT");
	SlotYOffset = GetInt(fp);

	fclose(fp);

	int SlotID = SLOT_ID_START;

	int xn, yn;

	ZSItemSlot *pISlot;

	for(yn = 0; yn < SlotsDeep; yn++)
	for(xn = 0; xn < SlotsAcross; xn++)
	{
		pISlot = new ZSItemSlot(SlotID++, x + xn * SlotXOffset + SlotX,y + yn*SlotYOffset + SlotY,  32, 32);
		pISlot->Show();
		pISlot->SetSource((void *)this);
		pISlot->SetSourceType(SOURCE_INVENTORY);
		AddChild(pISlot);
	}

	TopRow = 0;
	NumRows = 8;
	NumSlots = SlotsDeep * SlotsAcross;

	ZSVScroll *pScroll;
	pScroll = new ZSVScroll(IDC_INVENTORY_SCROLL, Bounds.right - 12, Bounds.top, 12, Bounds.bottom - Bounds.top);
	pScroll->SetPage(1);
	pScroll->Hide();
	AddChild(pScroll);


	SetSlots();
}

BOOL InventoryWin::ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y)
{
	ZSWindow *pWin;
	Creature *pCreature;
	if(pWinFrom->GetType() == WINDOW_ITEMSLOT)
	{
		pWin = pWinFrom->GetParent();
		if(pWin == this)
			return FALSE;
		if(pWin->GetType() == WINDOW_INVENTORY)
		{
			if(this->GetOwner()->GetObjectType() == OBJECT_ITEM ||
				((InventoryWin *)pWin)->GetOwner()->GetObjectType() == OBJECT_ITEM)
			{
				pOwner->AddItem(((GameItem *)pToReceive)->GetItem(),((GameItem *)pToReceive)->GetQuantity());
				SetSlots();
				return TRUE;
			}
			else
				return FALSE;		
		}
		else
		if(pWin->GetType() == WINDOW_BARTER)
		{
			if(((ZSItemSlot *)pWinFrom)->GetSourceType() == SOURCE_BARTER_MERCHANT)
			{
				pCreature = ((BarterWin *)pWin)->GetMerchant();
			}
			else
			{
				pCreature = ((BarterWin *)pWin)->GetBargainer();
			}
			if(pCreature == pOwner)
			{
				pOwner->AddItem(((GameItem *)pToReceive)->GetItem(),((GameItem *)pToReceive)->GetQuantity());
				SetSlots();
				return TRUE;
			}
			return FALSE;
		}
		else
		if(pWin->GetType() == WINDOW_EQUIP)
		{
			pOwner->AddItem(((GameItem *)pToReceive)->GetItem(),((GameItem *)pToReceive)->GetQuantity());
			SetSlots();
			return TRUE;
		}
		else
		{
			DEBUG_INFO("Couldn't figure out where Item was from in inventory\n");
			return FALSE;
		}
	}
	return FALSE;
}

