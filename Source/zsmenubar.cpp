#include "zsmenubar.h"
#include "zsbutton.h"
#include "zstext.h"
#include "zsengine.h"
#include "party.h"
#include "zsdescribe.h"
#include "world.h"

void ZSMenuBar::SetPortraits()
{
	for(int n = 0; n < 6; n ++)
	{
		if(PreludeParty.GetMember(n))
		{
			pPortrait[n]->SetTarget(PreludeParty.GetMember(n));
			PreludeParty.GetMember(n)->SetPortrait(pPortrait[n]);
			pPortrait[n]->Show();
		}
		else
		{
			pPortrait[n]->SetTarget(NULL);
			pPortrait[n]->Hide();
		}
	}

	return;
}	

//Draw
//
int ZSMenuBar::Draw() 
{ 
	//only draw ourself if we're visible
	if(Visible)
	{
		//draw our background if any
		Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface, NULL, NULL, NULL);
		
		RECT rClock;
		int Hour;
		Hour = PreludeWorld->GetHour();

		rClock.left = Hour % 5 * 64;
		rClock.top = Hour / 5 * 64;
		rClock.bottom = rClock.top + 64;
		rClock.right = rClock.left + 64;

		Engine->Graphics()->GetBBuffer()->Blt(&rClockTo,ClockFace,&rClock,NULL,NULL);

		if(Hour != LastHour)
		{
			pHourWin->SetText(Hour);
			LastHour = Hour;
		}
		
		if((int)pDrachFlag->Value != LastDrachs)
		{
			LastDrachs = (int)pDrachFlag->Value;
			pDrachWin->SetText(LastDrachs);
		}
		//draw our children
		if(pChild)
		{
			pChild->Draw();
		}
	}

	//draw our sibling if we have one
	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE; 
} // Draw


int ZSMenuBar::Command(int IDFrom, int Command, int Param)
{
	if(pParent)
	{
		return pParent->Command(IDFrom,Command,Param);
	}
	
	return TRUE;
}

ZSMenuBar::ZSMenuBar(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = FALSE;
	
	Bounds.left = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.bottom = Engine->Graphics()->GetHeight();
	Bounds.top = Bounds.bottom - 103;
	
//	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("interfacebar.bmp",800,103,NULL,NULL);
	
	for(int n = 0; n < 6; n ++)
	{
		pPortrait[n] = new ZSPortrait(NULL, Bounds.left + 41 + (n%3)*51,  Bounds.top + 2 + (n /3)*51);
		AddChild(pPortrait[n]);
	}

	ZSDescribe *pDescribe;
	pDescribe = new ZSDescribe(1, 270, 500, 255, 100, FALSE);
	AddChild(pDescribe);
	pDescribe->Show();

	rClockTo.left = 714;
	rClockTo.top = 517;
	rClockTo.right = rClockTo.left + 64;
	rClockTo.bottom = rClockTo.top + 64;

	ClockFace = Engine->Graphics()->CreateSurfaceFromFile("clockface.bmp",320,320,NULL,NULL);

	pDrachFlag = PreludeFlags.Get("PARTYDRACHS");

	
	ZSText *pText;
	pText = new ZSText(-1, Bounds.left + 613, Bounds.top + 80, "00000");
	pText->Show();
	AddChild(pText);
	
	pDrachWin = pText;
	LastDrachs = 0;

	pText = new ZSText(-1, Bounds.left + 738, Bounds.top + 42, "000");
	pText->Show();
	AddChild(pText);
	
	pHourWin = pText;
	LastHour = 0;

	RECT rBase;
	rBase.left = 0;
	rBase.top = 0;
	rBase.bottom = 600;
	rBase.right = 800;
	RECT rCur;
	rCur.left = 0;
	rCur.right = Engine->Graphics()->GetWidth();
	rCur.top = 0;
	rCur.bottom = Engine->Graphics()->GetHeight();

	FILE *fp;

	RECT rBounds;
	RECT rBaseBounds;
	int BackWidth;
	int BackHeight;
	char *FileName;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[INTERFACEBAR]");

	SeekTo(fp,"BACKGROUND");
	FileName = GetStringNoWhite(fp);

	BackWidth = GetInt(fp);
	BackHeight = GetInt(fp);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName,BackWidth,BackHeight,NULL,NULL);

	delete[] FileName;

	SeekTo(fp,"POSITION");		
	
	LoadRect(&rBounds, fp);
	rBaseBounds = rBounds;

	ScaleRect(&rBounds, &rBase, &rCur);

	Bounds = rBounds;

	ZSButton *pButton;
	pButton = new ZSButton("optionsbutton", IDC_INTERFACEBAR_OPTIONS, 532, 508, 30, 22, 30, 22, 1);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("mapbutton", IDC_INTERFACEBAR_MAP, 236, 568, 30, 22, 30, 22, 1);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("journalbutton", IDC_INTERFACEBAR_JOURNAL, 236, 538, 30, 22, 30, 22, 1);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("restbutton", IDC_INTERFACEBAR_REST, 236, 508, 30, 22, 30, 22, 1);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("helpbutton", IDC_INTERFACEBAR_HELP, 532, 538, 30, 22, 30, 22, 1);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("gatherbutton", IDC_INTERFACEBAR_GATHER, 532, 568, 30, 22, 30, 22, 1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp, "ZOOMIN");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_PLUS, IDC_INTERFACEBAR_ZOOMIN, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp, "ZOOMOUT");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_MINUS, IDC_INTERFACEBAR_ZOOMOUT, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp, "ROTATELEFT");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_CAMERA_ROTATE_LEFT, IDC_INTERFACEBAR_ROTATE_LEFT, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);
	
	SeekTo(fp, "ROTATERIGHT");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_CAMERA_ROTATE_RIGHT, IDC_INTERFACEBAR_ROTATE_RIGHT, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp, "ROTATEOVER");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_CAMERA_ROTATE_OVER, IDC_INTERFACEBAR_ROTATE_OVER, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp, "ROTATEUNDER");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_CAMERA_ROTATE_UNDER, IDC_INTERFACEBAR_ROTATE_UNDER, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);

	fclose(fp);
}

ZSMenuBar::~ZSMenuBar()
{
	ClockFace->Release();

}