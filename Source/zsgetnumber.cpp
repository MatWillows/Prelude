#include "zsgetnumber.h"
#include "zsbutton.h"
#include "zstext.h"
#include "zsengine.h"

// +/- 1
#define OFFSET_SLOW_TIME	200
// +/- * 10
#define OFFSET_ADVANCE_AMOUNT_TIME	1000

typedef enum
{
	IDC_GETNUMBER_PLUS,
	IDC_GETNUMBER_MINUS,
	IDC_GETNUMBER_DONE,
	IDC_GETNUMBER_TEXT,
	IDC_GETNUMBER_MAX,
	IDC_GETNUMBER_MIN,
	IDC_GETNUMBER_CLEAR,
} GETNUMBER_CONTROLS;

int ZSGetNumber::Draw()
{
	//basic draw
	//only draw ourself if we're visible
	if(Visible)
	{
		//if our state is normal do normal draw
		if(State == WINDOW_STATE_NORMAL)
		{
			//draw our background if any
			if(BackGroundSurface)
			{
				Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
			}

			//draw our text in the upper left corner
			if(Text)
			{
				RECT DrawTo;
				DrawTo.left = Bounds.left;
				DrawTo.right = Bounds.right;
				DrawTo.top = Bounds.top;
				DrawTo.bottom = Bounds.bottom;
				Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), &DrawTo, Text, TextColor);
			}
	
			//draw our children
			if(pChild)
			{
				pChild->Draw();
			}

		}
		else
		{
			//if we're moving just draw ourselves as a rectangle, witht the color based on whether or not
			//we're currently at a valid location (not intersectin siblings)
			if(ValidateMove(&Bounds))
			{
				Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(),&Bounds,RGB(255,0,0));
			}
			else
			{
				Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(),&Bounds,RGB(0,255,0));			
			}
		}
	}

	//draw our sibling if we have one
	if(pSibling)
	{
		pSibling->Draw();
	}

	//check for advance times;
	if(StartOffsetTime)
	{
		if(timeGetTime() > NextOffsetTime)
		{
			OffsetValue(OffsetAmount);			
			NextOffsetTime += OFFSET_SLOW_TIME;
			if(NextOffsetTime - StartOffsetTime > OFFSET_ADVANCE_AMOUNT_TIME)
			{
				OffsetAmount *= 10;
				StartOffsetTime = NextOffsetTime;
			}
		}
	}

	return TRUE;
}

void ZSGetNumber::OffsetValue(int Amount)
{
	int CurValue = ReturnCode;
	CurValue += Amount;

	if(CurValue < Min)
	{
		CurValue = Min;
	}
	else
	if(CurValue > Max)
	{
		CurValue = Max;
	}

	ReturnCode = CurValue;
	
	ZSWindow *pWin;
	pWin = GetChild(IDC_GETNUMBER_TEXT);
	pWin->SetText(ReturnCode);
}

int ZSGetNumber::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_PRESSED)
	{
		switch(IDFrom)
		{
		case IDC_GETNUMBER_PLUS:
			StartOffsetTime = timeGetTime();
			NextOffsetTime = StartOffsetTime;
			OffsetAmount = 1;
			break;
		case IDC_GETNUMBER_MINUS:
			StartOffsetTime = timeGetTime();
			NextOffsetTime = StartOffsetTime;
			OffsetAmount = -1;
			break;
		default:
			break;
		}

	}
	else
	if(Command == COMMAND_BUTTON_RELEASED)
	{
		StartOffsetTime = 0;
		NextOffsetTime = 0;
		OffsetAmount = 0;
	}
	else
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		StartOffsetTime = 0;
		NextOffsetTime = 0;
		OffsetAmount = 0;
		
		switch(IDFrom)
		{
		case IDC_GETNUMBER_PLUS:
				
			break;
		case IDC_GETNUMBER_MINUS:
			
			break;
		case IDC_GETNUMBER_DONE:
			State = WINDOW_STATE_DONE;
			break;
		default:
			break;
		}
	}

	return TRUE;
}

int ZSGetNumber::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{

	return TRUE;
}

ZSGetNumber::ZSGetNumber()
{
	Type = 	WINDOW_DIALOG;
	ID = -2;
	State = WINDOW_STATE_NORMAL;

	Bounds.left = 350;
	Bounds.right = 450;
	Bounds.top = 250;
	Bounds.bottom = 350;

	Border = 8;

	CreateWoodBorderedBackground(8,1);
	StartOffsetTime = 0;
	NextOffsetTime = 0;

	ReturnCode = 0;

	ZSText *pText;
	pText = new ZSText(IDC_GETNUMBER_TEXT, 364, 272, "000000");
	pText->Show();
	AddChild(pText);

	ZSButton *pButton;
	pButton = new ZSButton(BUTTON_MINUS, IDC_GETNUMBER_MINUS, 358, 300, 24, 24);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_PLUS, IDC_GETNUMBER_PLUS, 382, 300, 24, 24);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_CHECK, IDC_GETNUMBER_DONE, 422, 300, 24, 24);
	pButton->Show();
	AddChild(pButton);

	SetText("GetNumber");


}

int GetNumber(int NewMin, int NewMax, int StartValue, char *StartText, ZSWindow *pParent)
{
	int RetVal;
	ZSGetNumber *pWin;
	pWin = new ZSGetNumber();

	ZSWindow *pNumParent;
	if(!pParent)
	{
		pNumParent = ZSWindow::GetMain();
	}
	else
	{
		pNumParent = pParent;
	}
	pNumParent->AddTopChild(pWin);

	if(StartText)
	{
		pWin->SetText(StartText);
	}

	pWin->SetMin(NewMin);
	pWin->SetMax(NewMax);

	pWin->Show();

	pWin->SetFocus(pWin);

	pWin->OffsetValue(StartValue);

	RetVal = pWin->GoModal();
	
	pWin->ReleaseFocus();

	pNumParent->RemoveChild(pWin);
	
	return RetVal;
}