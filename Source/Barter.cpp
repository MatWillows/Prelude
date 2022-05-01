//things to do:
//		show party drachs
//		player, merchant, bargainer display (names/portraits)
//		finish transaction

#include "barter.h"
#include "zstext.h"
#include "inventorywin.h"
#include "zsitemslot.h"
#include "zsbutton.h"
#include "creatures.h"
#include "zsutilities.h"
#include "party.h"
#include "script.h"
#include "gameitem.h"
#include "zshelpwin.h"

typedef enum
{
	IDC_BARTER_MERCHANT_ITEM1,
	IDC_BARTER_MERCHANT_ITEM2,
	IDC_BARTER_MERCHANT_ITEM3,
	IDC_BARTER_MERCHANT_ITEM4,

	IDC_BARTER_PLAYER_ITEM1,
	IDC_BARTER_PLAYER_ITEM2,
	IDC_BARTER_PLAYER_ITEM3,
	IDC_BARTER_PLAYER_ITEM4,

	IDC_BARTER_PLAYER_INVENTORY,
	IDC_BARTER_MERCHANT_INVENTORY,
	
	IDC_BARTER_PLAYER_DRACHS,
	IDC_BARTER_MERCHANT_DRACHS,

	IDC_BARTER_CLEAR_PLAYER,
	IDC_BARTER_CLEAR_MERCHANT,
	IDC_BARTER_CLEAR_ALL,

	IDC_BARTER_QUIT,
	IDC_BARTER_TRANSACT,

	IDC_BARTER_MERCHANT,
	IDC_BARTER_PLAYER,
	IDC_BARTER_BARGAINER,

	IDC_BARTER_NET,

	IDC_PARTY_DRACHS,

	IDC_NEXT_PLAYER,

	IDC_PREVIOUS_PLAYER,


} BARTER_CONTROLS;

void BarterWin::Reset()
{
	ZSWindow *pWin;
	ZSItemSlot *pISlot;
	Thing *pItem;

	for(int n = 0; n < 4; n++)
	{
	//return items on the player's side to the target player
		pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_PLAYER_ITEM1 + n);
		if(pISlot->Look())
		{
			pItem = pISlot->Look()->GetItem();
			if(pItem)
			{
				pTarget->Give(pItem,pISlot->Look()->GetQuantity());
				pISlot->Take();
			}
		}
	//return items on the merchants side to the merchant
		pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_MERCHANT_ITEM1 + n);
		if(pISlot->Look())
		{
			pItem = pISlot->Look()->GetItem();
			if(pItem)
			{
				pMerchant->Give(pItem,pISlot->Look()->GetQuantity());
				pISlot->Take();
			}
		}
	}
	//unset any drach differences
	pWin = GetChild(IDC_BARTER_PLAYER_DRACHS);
	pWin->SetText(0);

	pWin = GetChild(IDC_BARTER_MERCHANT_DRACHS);
	pWin->SetText(0);

	pWin = GetChild(IDC_BARTER_PLAYER_INVENTORY);
	((InventoryWin *)pWin)->Reset();

	pWin = GetChild(IDC_BARTER_MERCHANT_INVENTORY);
	((InventoryWin *)pWin)->Reset();

	DrachPlayer = 0;
	DrachMerchant = 0;

}

int BarterWin::AddMerchantItem(GameItem *ToAdd)
{
	ZSItemSlot *pISlot;

	for(int n = 0; n < 4; n++)
	{
		//return items on the player's side to the target player
		pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_MERCHANT_ITEM1 + n);
		if(!pISlot->Look())
		{
			pISlot->Give(ToAdd);
			return TRUE;
		}
	}
	return FALSE;
}

int BarterWin::AddPlayerItem(GameItem *ToAdd)
{
	ZSItemSlot *pISlot;

	if(ToAdd->GetData("VALUE").Value <= 0)
	{
		Describe("That cannot be bartered.");
		return FALSE;
	}

	for(int n = 0; n < 4; n++)
	{
		//return items on the player's side to the target player
		pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_PLAYER_ITEM1 + n);
		if(!pISlot->Look())
		{
			pISlot->Give(ToAdd);
			return TRUE;
		}
	}
	return FALSE;
}




void BarterWin::Transact()
{
	ZSWindow *pWin;
	ZSItemSlot *pISlot;
	Thing *pItem;
	int Quantity;
	//check to see that the merchant and player have enough room and drachs for the transaction
	//if so
	int DrachDifference;
	DrachDifference = DrachPlayer - DrachMerchant;

	if(DrachPlayer == 0 && DrachMerchant == 0)
	{
		Describe("Nothing on the table!");
	}

	if((int)DrachFlag->Value + DrachDifference >= 0)
	{
		if((DrachPlayer > 5 || DrachMerchant > 5))
		{
			pBargainer->ImproveSkill(INDEX_BARTER);
			if(! (rand() % 4))
				pBargainer->ImproveSkill(INDEX_BARTER);
		}	

		DrachFlag->Value = (void *) ((int)DrachFlag->Value + DrachDifference);

		//place any items on the merchant's side in the target player's inventory
		for(int n = 0; n < 4; n++)
		{
		//return items on the player's side to the target player
			pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_PLAYER_ITEM1 + n);
			if(pISlot->Look())
			{
				pItem = pISlot->Look()->GetItem();
				if(pItem)
				{
					Quantity = pISlot->Look()->GetQuantity();
					pISlot->Take();
					pMerchant->Give(pItem,Quantity);
				}
			}
		//return items on the merchants side to the merchant
			pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_MERCHANT_ITEM1 + n);
			if(pISlot->Look())
			{
				pItem = pISlot->Look()->GetItem();
				if(pItem)
				{
					Quantity = pISlot->Look()->GetQuantity();
					pISlot->Take();
					pTarget->Give(pItem,Quantity);
				}
			}
		}

		//DrachFlag->Value -= DrachPlayer;
		//DrachFlag->Value += DrachMerchant;
		
		//unset any drach differences
		pWin = GetChild(IDC_BARTER_PLAYER_DRACHS);
		pWin->SetText(0);
		DrachPlayer = 0;

		pWin = GetChild(IDC_BARTER_MERCHANT_DRACHS);
		pWin->SetText(0);
		DrachMerchant = 0;

		pWin = GetChild(IDC_BARTER_PLAYER_INVENTORY);
		((InventoryWin *)pWin)->Reset();

		pWin = GetChild(IDC_BARTER_MERCHANT_INVENTORY);
		((InventoryWin *)pWin)->Reset();

		pWin = GetChild(IDC_BARTER_NET);
		pWin->SetText(" 0");
		pWin->SetTextColor(TEXT_LIGHT_GREY_PARCHMENT);
		DrachPlayer = 0;
		DrachMerchant = 0;

	}
	else
	{
		Describe("You do not have enough drachs.");
	}

	pWin = GetChild(IDC_PARTY_DRACHS);
	pWin->SetText((int)DrachFlag->Value);
}

void BarterWin::CalculateValues()
{
	//get the total value of the items on the player's and merchant's sides
	int PlayerTotal = 0;
	int MerchantTotal = 0;
	ZSWindow *pWin;
	ZSItemSlot *pISlot;
	Thing *pItem;

	for(int n = 0; n < 4; n++)
	{
		pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_PLAYER_ITEM1 + n);
		if(pISlot->Look())
		{
			pItem = pISlot->Look()->GetItem();
			if(pItem)
			{
				PlayerTotal += pISlot->Look()->GetQuantity() * pItem->GetData("VALUE").Value;
			}
		}

		pISlot = (ZSItemSlot *)GetChild(IDC_BARTER_MERCHANT_ITEM1 + n);
		
		if(pISlot->Look())
		{
			pItem = pISlot->Look()->GetItem();
			if(pItem)
			{
				MerchantTotal += pISlot->Look()->GetQuantity() * pItem->GetData("VALUE").Value;
			}
		}
	}
	//modify by the player's barter skill

	int SkillValue;

	SkillValue = pBargainer->GetData(INDEX_BARTER).Value;
	
	float SkillPercent;
	SkillPercent = (float)SkillValue / 50.0f; //50 = mastery.
	if(SkillPercent > 1.0f) SkillPercent = 1.0f;

	MerchantTotal = MerchantTotal + (int)(((float)MerchantTotal / 2.0f) * (1.0f - SkillPercent));
	
	PlayerTotal = PlayerTotal / 2  + (int) ((float)(PlayerTotal / 2) * SkillPercent);

	DrachPlayer = PlayerTotal;

	DrachMerchant = MerchantTotal;
	
	pWin = GetChild(IDC_BARTER_MERCHANT_DRACHS);
	pWin->SetText(MerchantTotal);

	pWin = GetChild(IDC_BARTER_PLAYER_DRACHS);
	pWin->SetText(PlayerTotal);

	int NetValue;
	NetValue = PlayerTotal - MerchantTotal;
	int AbsoluteVal;
	AbsoluteVal = abs(NetValue);

	pWin = GetChild(IDC_BARTER_NET);

	char blarg[16];
	sprintf(blarg,"%i",PlayerTotal - MerchantTotal);

	pWin->SetText(blarg);

	if(NetValue < 0)
	{
		pWin->SetTextColor(TEXT_RED_PARCHMENT);
	}
	else
	if(NetValue > 0)
	{
		pWin->SetTextColor(TEXT_GREEN_PARCHMENT);
	}
	else
	{
		pWin->SetTextColor(TEXT_LIGHT_GREY_PARCHMENT);
	}
}

int BarterWin::Command(int IDFrom, int Command, int Param)
{
	int PlayerNum;
	InventoryWin *pIWin;
	ZSWindow *pWin;
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch (IDFrom)
		{
		case IDC_BARTER_QUIT:
			State = WINDOW_STATE_DONE;
			Reset();
			break;
		case IDC_BARTER_CLEAR_ALL:
			Reset();
			CalculateValues();
			break;
		case IDC_BARTER_TRANSACT:
			Transact();
			break;
		case IDC_NEXT_PLAYER:
			PlayerNum = PreludeParty.GetMemberNum(pTarget);
			pTarget = NULL;
			while(!pTarget)
			{
				PlayerNum++;
				if(PlayerNum >= MAX_PARTY_MEMBERS)
					PlayerNum = 0;
				pTarget = PreludeParty.GetMember(PlayerNum);
			}
			pIWin = (InventoryWin *)GetChild(IDC_BARTER_PLAYER_INVENTORY);
			pIWin->SetOwner(pTarget);
			pWin = GetChild(IDC_BARTER_PLAYER);
			pWin->SetText(pTarget->GetData(INDEX_NAME).String);

			break;
		case IDC_PREVIOUS_PLAYER:
			PlayerNum = PreludeParty.GetMemberNum(pTarget);
			pTarget = NULL;
			while(!pTarget)
			{
				PlayerNum--;
				if(PlayerNum < 0)
					PlayerNum = MAX_PARTY_MEMBERS - 1;
				pTarget = PreludeParty.GetMember(PlayerNum);
			}
			pIWin = (InventoryWin *)GetChild(IDC_BARTER_PLAYER_INVENTORY);
			pIWin->SetOwner(pTarget);
			pWin = GetChild(IDC_BARTER_PLAYER);
			pWin->SetText(pTarget->GetData(INDEX_NAME).String);
			break;
		}

	}
	if(Command == COMMAND_ITEMSLOT_CHANGING)
	{
		//the param is the ID of the source ItemSlot's parent window
		if(Param == IDC_BARTER_PLAYER_INVENTORY)
		{
			if(IDFrom != IDC_BARTER_PLAYER_ITEM1 && 
				IDFrom != IDC_BARTER_PLAYER_ITEM2 && 
				IDFrom != IDC_BARTER_PLAYER_ITEM3 && 
				IDFrom != IDC_BARTER_PLAYER_ITEM4)
			{
				return FALSE;
			}
		}
		else
		if(Param == IDC_BARTER_MERCHANT_INVENTORY)
		{
			if(IDFrom != IDC_BARTER_MERCHANT_ITEM1 && 
				IDFrom != IDC_BARTER_MERCHANT_ITEM2 && 
				IDFrom != IDC_BARTER_MERCHANT_ITEM3 && 
				IDFrom != IDC_BARTER_MERCHANT_ITEM4)
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}

		CalculateValues();
	}
	return TRUE;
}


BarterWin::BarterWin(int NewID, int x, int y, int width, int height, Creature *pNewMerchant, Creature *pNewTarget)
{
	ID = NewID;
	Type = WINDOW_BARTER;
	Moveable = FALSE;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = y + height;

	FILE *fp;
	RECT rBounds;
	char *FileName;
	int Width;
	int Height;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[BARTER]");

	SeekTo(fp,"BACKGROUND");
	
	FileName = GetStringNoWhite(fp);
	Width = GetInt(fp);
	Height = GetInt(fp);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName,Width,Height,NULL,0);
	delete FileName;

	pMerchant = pNewMerchant;
	pTarget = pNewTarget;

//bargainer should be person in party with best skill
	pBargainer = PreludeParty.GetBest(pTarget->GetIndex("BARTER"));

//create the sub windows
	ZSButton *pButton;
	ZSText *pText;
	ZSItemSlot *pISlot;
	InventoryWin *pInventory;

	SeekTo(fp,"TRANSACT");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton("barterbutton",	IDC_BARTER_TRANSACT, XYWH(rBounds),63,48,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"QUIT");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton("closebutton", IDC_BARTER_QUIT, XYWH(rBounds),57,34,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"CLEAR");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_NORMAL, IDC_BARTER_CLEAR_ALL, XYWH(rBounds));
	pButton->Show();
	pButton->SetText("clear");
	AddChild(pButton);


	SeekTo(fp,"MERCHANT1");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_MERCHANT_ITEM1, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_MERCHANT);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);
	SeekTo(fp,"MERCHANT2");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_MERCHANT_ITEM2, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_MERCHANT);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);
	SeekTo(fp,"MERCHANT3");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_MERCHANT_ITEM3, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_MERCHANT);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);
	SeekTo(fp,"MERCHANT4");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_MERCHANT_ITEM4, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_MERCHANT);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);

	SeekTo(fp,"PLAYER1");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_PLAYER_ITEM1, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_PLAYER);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);
	SeekTo(fp,"PLAYER2");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_PLAYER_ITEM2, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_PLAYER);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);
	SeekTo(fp,"PLAYER3");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_PLAYER_ITEM3, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_PLAYER);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);
	SeekTo(fp,"PLAYER4");
	LoadRect(&rBounds,fp);
	pISlot = new ZSItemSlot(IDC_BARTER_PLAYER_ITEM4, XYWH(rBounds));
	pISlot->SetSourceType(SOURCE_BARTER_PLAYER);
	pISlot->SetSource((void *)this);
	pISlot->Show();
	AddChild(pISlot);

	SeekTo(fp,"IPLAYER");
	LoadRect(&rBounds,fp);
	pInventory = new InventoryWin(IDC_BARTER_PLAYER_INVENTORY, XYWH(rBounds), pTarget);
	pInventory->Show();
	AddChild(pInventory);
	SeekTo(fp,"IMERCHANT");
	LoadRect(&rBounds,fp);
	pInventory = new InventoryWin(IDC_BARTER_MERCHANT_INVENTORY, XYWH(rBounds), pNewMerchant);
	pInventory->Show();
	AddChild(pInventory);
	
	SeekTo(fp,"DPLAYER");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_BARTER_PLAYER_DRACHS, XYWH(rBounds), "000", 0, TRUE);
	pText->Show();
	pText->SetText(0);
	AddChild(pText);
	
	SeekTo(fp,"DMERCHANT");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_BARTER_MERCHANT_DRACHS, XYWH(rBounds), "000", 0, TRUE);
	pText->Show();
	pText->SetText(0);
	AddChild(pText);

	SeekTo(fp,"PARTYDRACHS");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_PARTY_DRACHS, XYWH(rBounds), "0000", 0, TRUE);
	pText->Show();
	pText->SetText(0);
	AddChild(pText);

	DrachFlag = PreludeFlags.Get("PARTYDRACHS");
	pText->SetText((int)DrachFlag->Value);

	SeekTo(fp,"NETDRACHS");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_BARTER_NET, XYWH(rBounds), "0000", 0, TRUE);
	pText->Show();
	pText->SetText(0);
	AddChild(pText);

	SeekTo(fp, "MERCHANTNAME");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_BARTER_MERCHANT, XYWH(rBounds), pMerchant->GetData(INDEX_NAME).String, 0, FALSE);
	pText->Show();
	AddChild(pText);

	SeekTo(fp, "PLAYERNAME");
	LoadRect(&rBounds,fp);
	pText = new ZSText(IDC_BARTER_PLAYER, XYWH(rBounds), pTarget->GetData(INDEX_NAME).String, 0, FALSE);
	pText->Show();
	AddChild(pText);

	SeekTo(fp, "PREVPLAYER");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_WEST, IDC_PREVIOUS_PLAYER, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp, "NEXTPLAYER");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_EAST, IDC_NEXT_PLAYER, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);



/*
	pText = new ZSText(IDC_BARTER_MERCHANT,
	pText->Show();
	AddChild(pText);
	pText = new ZSText(IDC_BARTER_PLAYER,
	pText->Show();
	AddChild(pText);
	pText = new ZSText(IDC_BARTER_BARGAINER,
	pText->Show();
	AddChild(pText);

	pButton = new ZSButton(IDC_BARTER_CLEAR_PLAYER
	pButton->Show();
	AddChild(pButton);
	pButton = new ZSButton(IDC_BARTER_CLEAR_MERCHANT,
	pButton->Show();
	AddChild(pButton);
*/
	fclose(fp);


}

BOOL BarterWin::ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y)
{
	ZSWindow *pWin;
	pWin = GetChild(x,y);
	if(pWin && pWin->GetType() == WINDOW_INVENTORY)
	{
		return pWin->ReceiveItem(pToReceive,pWinFrom,x,y);
	}

	Object *pPersonFrom;
	if(pWinFrom->GetType() == WINDOW_ITEMSLOT)
	{
		pWin = pWinFrom->GetParent();
		if(pWin->GetType() == WINDOW_INVENTORY)
		{
			InventoryWin *pIWin;
			pIWin = (InventoryWin *)pWin;
			pPersonFrom = pIWin->GetOwner();
		}
		else
		if(pWin->GetType() == WINDOW_BARTER)
		{
			//do nothing, only way to get here is from here.
			return FALSE;
		}
		else
		{
			DEBUG_INFO("Couldn't figure out where Item was from in barter\n");
			return FALSE;
		}
	}

	if(pPersonFrom == pMerchant)
	{
		if(!AddMerchantItem((GameItem *)pToReceive))
			return FALSE;
		
	}
	else
	{
		if(!AddPlayerItem((GameItem *)pToReceive))
			return FALSE;
	}

	CalculateValues();
	return TRUE;

}

int BarterWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ShowHelp("Barter");
		return TRUE;
	}
	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
	}
	return TRUE;


	return TRUE;
}

