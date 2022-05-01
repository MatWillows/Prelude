#include "zsconfirm.h"
#include "zstext.h"
#include "zsbutton.h"
#include "zsengine.h"
#include "zshelpwin.h"

typedef enum
{
	IDC_CONFIRM_TEXT,
	IDC_RESPONSE_A,
	IDC_RESPONSE_B,
} CONFIRM_CONTOLS;

#define CONFIRM_WIDTH	200
#define CONFIRM_HEIGHT	100

int ZSConfirmWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_RESPONSE_A)
		{
			ReturnCode = TRUE;
		}
		else
		{
			ReturnCode = FALSE;

		}
		State = WINDOW_STATE_DONE;
	}
	return TRUE;
}

int ZSConfirmWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ZSHelpWin *pHelp;
		pHelp = new ZSHelpWin(-1);
		pHelp->Show();
		AddTopChild(pHelp);
		SetFocus(pHelp);
		pHelp->GoModal();
		ReleaseFocus();
		RemoveChild(pHelp);
		return TRUE;
	}

	if(CurrentKeys[DIK_Y] & 0x80 && !(LastKeys[DIK_Y] & 0x80))
	{
		ReturnCode = TRUE;
		State = WINDOW_STATE_DONE;
		return TRUE;
	}

	if(CurrentKeys[DIK_N] & 0x80 && !(LastKeys[DIK_N] & 0x80))
	{
		ReturnCode = FALSE;
		State = WINDOW_STATE_DONE;
		return TRUE;
	}




	return TRUE;
}

ZSConfirmWin::ZSConfirmWin(char *Message, char *ResponseA, char *ResponseB)
{
	ID = 87087;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Moveable = FALSE;

	Bounds.left = Engine->Graphics()->GetWidth()/2 - CONFIRM_WIDTH/2;	
	Bounds.right = Bounds.left + CONFIRM_WIDTH;
	Bounds.top = Engine->Graphics()->GetHeight()/2 - CONFIRM_HEIGHT/2;
	Bounds.bottom = Bounds.top + CONFIRM_HEIGHT;

	CreateWoodBorderedBackground(8,1);

	//top half of area is devoted to text
	ZSText *pText;
	pText = new ZSText(IDC_CONFIRM_TEXT, Bounds.left + 8,Bounds.top + 8,CONFIRM_WIDTH - 16, (CONFIRM_HEIGHT/16)/2,Message,0);
	pText->Show();
	AddChild(pText);

	ZSButton *pButton;
	
	int BWidth;
	int BHeight;
	
	BHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight();
	BWidth = Engine->Graphics()->GetFontEngine()->GetTextWidth(ResponseA);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_RESPONSE_A, 
									Bounds.left + CONFIRM_WIDTH/4 - BWidth/2,
									Bounds.bottom - CONFIRM_HEIGHT/4 - BHeight/2,
									BWidth,
									BHeight);
	pButton->Show();
	pButton->SetText(ResponseA);
	AddChild(pButton);

	BWidth = Engine->Graphics()->GetFontEngine()->GetTextWidth(ResponseB);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_RESPONSE_B, 
									Bounds.left + CONFIRM_WIDTH/2 + CONFIRM_WIDTH/4 - BWidth/2,
									Bounds.bottom - CONFIRM_HEIGHT/4 - BHeight/2,
									BWidth,
									BHeight);
	pButton->Show();
	pButton->SetText(ResponseB);
	AddChild(pButton);

}

int ZSConfirmWin::MoveMouse(long *x, long *y, long *z)
{


	return TRUE;
}

BOOL Confirm(ZSWindow *pParent, char *Message,char *ResponseA, char *ResponseB)
{
	ZSConfirmWin *pWin;

	pWin = new ZSConfirmWin(Message, ResponseA, ResponseB);
	pWin->Show();
	pParent->AddTopChild(pWin);
	pWin->SetFocus(pWin);

	int Result;

	Result = pWin->GoModal();

	pWin->ReleaseFocus();

	pParent->RemoveChild(pWin);

	if(Result)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
