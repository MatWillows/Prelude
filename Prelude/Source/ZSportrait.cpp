#include "ZSportrait.h"
#include "things.h"
#include "creatures.h"
#include <assert.h>
#include "zsengine.h"
#include "inventorywin.h"
#include "equipwin.h"
#include "Characterwin.h"
#include "zsmenubar.h"
#include "party.h"
#include "zsitemslot.h"
#include "world.h"

//LPDIRECTDRAWSURFACE7 ZSPortrait::PortraitBackground = NULL;
LPDIRECTDRAWSURFACE7 ZSPortrait::PortraitGems = NULL;
int ZSPortrait::NumPortraits = 0;
	
RECT rPortraitFrame =	{	0,		0,		50,		50};
RECT rHitPoints =		{	1,		1,		3,		47};
RECT rRestPoints =		{	3,		1,		5,		47};
RECT rWillPoints =		{	5,		1,		7,		47};
RECT rActionPoints =	{	46,		3,		48,		45};
RECT rNailOne =			{	6,		0,		16,		10};
RECT rNailTwo =			{	1,		46,		4,		49};
RECT rNailThree =		{	45,		2,		48,		4};
RECT rNailFour =		{	45,		46,		48,		49};

#define PORTRAIT_WIDTH	100
#define PORTRAIT_HEIGHT	100

void ZSPortrait::Update()
{
	if(!pTarget)
	{
		return;
	}

//	return;

	
	HRESULT hr;
	RECT rFromGems;

	if(pddFace)
	{
		hr = BackGroundSurface->Blt(NULL, pddFace, NULL, DDBLT_WAIT, NULL);

		if(hr != DD_OK)
		{
			//Engine->Graphics()->ReportError(hr);
			SafeExit("problem in portraits");
		}
	}

	int Data;
	int DataMax;
	int height;
	float ratio;

	Data = pTarget->GetData(INDEX_HITPOINTS).Value;
	DataMax = pTarget->GetData(INDEX_MAXHITPOINTS).Value;

	ratio = (float)Data/(float)DataMax;
	
	if(ratio > 1.0f) ratio = 1.0f;
	
	rFromGems = rHitPoints;
	
	height = rFromGems.bottom - rFromGems.top;
	height = (int)((float)height * ratio);
	if(height > 0)
	{
		rFromGems.top = rFromGems.bottom - height;

		hr = BackGroundSurface->Blt(&rFromGems,PortraitGems,&rFromGems, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

		if(hr != DD_OK)
		{
			//Engine->ReportError(hr);
			SafeExit("problem in portraits");

		}
	}
	
	if(pTarget->GetData(INDEX_POWER_OF_FLAME).Value)
	{
		Data = pTarget->GetData(INDEX_RESTPOINTS).Value;
		DataMax = pTarget->GetData(INDEX_MAXRESTPOINTS).Value;

		if(DataMax)
		{
			ratio = (float)Data/(float)DataMax;
			if(ratio > 1.0f) ratio = 1.0f;
			
			rFromGems = rRestPoints;
			
			height = rFromGems.bottom - rFromGems.top;
			height = (int)((float)height * ratio);
			if(height > 0)
			{
				rFromGems.top = rFromGems.bottom - height;

				hr = BackGroundSurface->Blt(&rFromGems,PortraitGems,&rFromGems, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

				if(hr != DD_OK)
				{
					//Engine->ReportError(hr);
					SafeExit("problem in portraits");
				}
			}
		}
	}

	if(pTarget->GetData(INDEX_THAUMATURGY).Value || pTarget->GetData(INDEX_GIFTS_OF_GODDESS).Value)
	{
		Data = pTarget->GetData(INDEX_WILLPOINTS).Value;
		DataMax = pTarget->GetData(INDEX_MAXWILLPOINTS).Value;

		if(DataMax)
		{
			ratio = (float)Data/(float)DataMax;
			if(ratio > 1.0f) ratio = 1.0f;
	
			rFromGems = rWillPoints;

			height = rFromGems.bottom - rFromGems.top;
			height = (int)((float)height * ratio);
			if(height > 0)
			{
				rFromGems.top = rFromGems.bottom - height;

				hr = BackGroundSurface->Blt(&rFromGems,PortraitGems,&rFromGems, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

				if(hr != DD_OK)
				{
					//Engine->ReportError(hr);
					SafeExit("problem in portraits");
				}
			}
		}
	}
	/*
	Data = pTarget->GetData("ACTIONPOINTS").Value;
	DataMax = pTarget->GetData("MAXACTIONPOINTS").Value;

	ratio = (float)Data/(float)DataMax;
	
	rFromGems = rActionPoints;
	
	height = rFromGems.bottom - rFromGems.top;
	height = (int)((float)height * ratio);

	if(height > 0)
	{
		rFromGems.top = rFromGems.bottom - height;

		hr = BackGroundSurface->Blt(&rFromGems,PortraitGems,&rFromGems, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

		if(hr != DD_OK)
		{
			//Engine->ReportError(hr);
			SafeExit("problem in portraits");
		}
	}

	if(((Creature *)pTarget)->IsActive())
	{
		hr = BackGroundSurface->Blt(&rNailOne,PortraitGems,&rNailOne, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

		if(hr != DD_OK)
		{
			//Engine->ReportError(hr);
			SafeExit("problem in portraits");
		}
	}
	
		hr = BackGroundSurface->Blt(&rNailTwo,PortraitGems,&rNailTwo, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

		if(hr != DD_OK)
		{
			//Engine->ReportError(hr);
			SafeExit("problem in portraits");
		}
	
		hr = BackGroundSurface->Blt(&rNailThree,PortraitGems,&rNailThree, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

		if(hr != DD_OK)
		{
			//Engine->ReportError(hr);
			SafeExit("problem in portraits");
		}
	
		hr = BackGroundSurface->Blt(&rNailFour,PortraitGems,&rNailFour, DDBLT_WAIT | DDBLT_KEYSRC, NULL);

		if(hr != DD_OK)
		{
			//Engine->ReportError(hr);
			SafeExit("problem in portraits");
		}
	}
*/
	Clean();
}

int ZSPortrait::RightButtonUp(int x, int y)
{
	ReleaseFocus();

/*	InventoryWin *pIWin = new InventoryWin(66666,100,100,136,326,pTarget);
	pIWin->Show();
	GetMain()->AddTopChild(pIWin);

	EquipWin *pEWin = new EquipWin(66777, 300, 100, 186, 332, pTarget);
	pEWin->Show();
	GetMain()->AddTopChild(pEWin);
*/
	if(pTarget 
		&& (!PreludeWorld->InCombat() 
		||
		(pTarget->GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_UNCONSCIOUS 
		 && pTarget->GetData(INDEX_BATTLESTATUS).Value != CREATURE_STATE_DEAD)))
	{
		CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, (Creature *)pTarget);
		pCWin->Show();
		GetMain()->AddTopChild(pCWin);

		SetFocus(pCWin);
		pCWin->GoModal();
		ReleaseFocus();

		GetMain()->RemoveChild(pCWin);
	}
	else
	{
		Describe("They are currently disabled!");
	}

	return TRUE;
}

int ZSPortrait::LeftButtonUp(int x, int y)
{
	ReleaseFocus();

	if(x >= Bounds.left && x <= Bounds.right && y >= Bounds.top && y <= Bounds.bottom)
	{
		if(PreludeWorld->GetGameState() == GAME_STATE_NORMAL)
			PreludeParty.SetLeader((Creature *)GetTarget());	
	}
	else
	{
		ZSWindow *pWin;
		pWin = ZSWindow::GetMain()->GetBottomChild(x,y);
		if(pWin && pWin->GetType() == WINDOW_PORTRAIT)
		{
			Thing *pHere;
			pHere = GetTarget();
			Thing *pThere;
			pThere = ((ZSPortrait *)pWin)->GetTarget();

			int OldNum;
			OldNum = PreludeParty.GetMemberNum((Creature *)pHere);
						
			int NewNum;
			NewNum = PreludeParty.GetMemberNum((Creature *)pThere);

			PreludeParty.SetMember(OldNum, (Creature *)pThere);
			PreludeParty.SetMember(NewNum, (Creature *)pHere);

			((ZSMenuBar *)ZSWindow::GetMain()->GetChild(IDC_MAIN_MENUBAR))->SetPortraits();
		}
	}

	return TRUE;
}


void ZSPortrait::SetTarget(Thing *pNewTarget)
{
	pTarget = pNewTarget;
	if(pTarget)
	{
		pddFace = CreatePortrait(pTarget->GetData(INDEX_PORTRAIT).String, TRUE);
	}
	else
	{
		CreateParchmentBorderedBackground(0,1);
	}
}


ZSPortrait::ZSPortrait(Thing *pNewTarget, int x, int y)
{
	ID = 66666;
	pTarget = pNewTarget;

	BackGroundSurface = NULL;

	Bounds.left = x;
	Bounds.right = x + 48;//PORTRAIT_WIDTH;
	Bounds.top = y;
	Bounds.bottom = y + 48;//PORTRAIT_HEIGHT;

	Border = 0;
	pddFace = NULL;

	if(pTarget)
	{
		pddFace = CreatePortrait(pNewTarget->GetData(INDEX_PORTRAIT).String, TRUE);
	}
	else
	{
		CreateParchmentBorderedBackground(0,1);
	}

	if(NumPortraits == 0)
	{
	//	PortraitBackground = Engine->Graphics()->CreateSurfaceFromFile("Portraitback.bmp", 50, 50, NULL,COLOR_KEY_FROM_FILE);
		PortraitGems = Engine->Graphics()->CreateSurfaceFromFile("Gems.bmp", 8, 48, NULL,COLOR_KEY_FROM_FILE);
	
		//assert(PortraitBackground);
		assert(PortraitGems);
	}

	NumPortraits++;

	
	State = WINDOW_STATE_NORMAL;
	Type = WINDOW_PORTRAIT;
	Moveable = FALSE;
	
	Update();
	Visible = FALSE;
}


ZSPortrait::~ZSPortrait()
{
	if(BackGroundSurface)
	{
		BackGroundSurface->Release();
		BackGroundSurface = NULL;	
	}

	NumPortraits--;
	if(NumPortraits = 0)
	{
	//	PortraitBackground->Release();
	//	PortraitBackground = NULL;
		PortraitGems->Release();
		PortraitGems = NULL;
	}
}

int ZSPortrait::Draw()
{
	if(NeedRedraw)
	{
		Update();
	}

	if(Visible)
	{
		Engine->Graphics()->GetBBuffer()->Blt(&Bounds, BackGroundSurface, NULL, DDBLT_KEYSRC, NULL);
		
		if(pTarget)
		{
			if((PreludeWorld->GetGameState() == GAME_STATE_COMBAT && ((Creature *)pTarget)->IsActive())
				||
				(PreludeWorld->GetGameState() != GAME_STATE_COMBAT && PreludeParty.IsLeader((Creature *)pTarget)))
			{
				Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(),Bounds.left + 7, Bounds.top, "L", TEXT_DARK_GREY_PARCHMENT);
			}
		}
			
			
		if(pChild)
		{
			pChild->Draw();
		}
	}
	
	if(pSibling)
	{
		pSibling->Draw();
	}

	return TRUE; 

}

BOOL ZSPortrait::ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y)
{
	ZSItemSlot *pISlot;
	pISlot = (ZSItemSlot *)pWinFrom;
	if(pISlot->GetSourceType() == SOURCE_INVENTORY)
	{
		InventoryWin *pIWin;
		pIWin = (InventoryWin *)pWinFrom->GetParent();
		if(pIWin->GetOwner()->GetObjectType() == OBJECT_CREATURE)
		{
			if(PreludeParty.IsMember((Creature *)pIWin->GetOwner()))
			{
				pTarget->AddItem((GameItem *)pToReceive);
				return TRUE;
			}
		}
		else
		{
			pTarget->AddItem((GameItem *)pToReceive);
			return TRUE;
		}
	}
	else
	if(pISlot->GetSourceType() == SOURCE_EQUIP)
	{
		EquipWin *pEquip;
		pEquip = (EquipWin *)pWinFrom->GetParent();
		if(pEquip->GetTarget()->GetObjectType() == OBJECT_CREATURE)
		{
			if(PreludeParty.IsMember((Creature *)pEquip->GetTarget()))
			{
				pTarget->AddItem((GameItem *)pToReceive);
				return TRUE;
			}
		}
		else
		{
			pTarget->AddItem((GameItem *)pToReceive);
			return TRUE;
		}
		return TRUE;
	}
	return FALSE;
}