#include "ZSmessage.h"
#include "ZSEngine.h"
#include "mainwindow.h"
#include "zshelpwin.h"

#define MESSAGE_WIDTH	200
#define MESSAGE_HEIGHT	100

ZSMessage::ZSMessage(char *Text, char *ConfirmText)
{
	ID = 87087;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Moveable = FALSE;

	Bounds.left = Engine->Graphics()->GetWidth()/2 - MESSAGE_WIDTH/2;	
	Bounds.right = Bounds.left + MESSAGE_WIDTH;
	Bounds.top = Engine->Graphics()->GetHeight()/2 - MESSAGE_HEIGHT/2;
	Bounds.bottom = Bounds.top + MESSAGE_HEIGHT;

	CreateWoodBorderedBackground(8,1);

	//top half of area is devoted to text
	ZSText *pText;
	pText = new ZSText(-1, Bounds.left + 8, Bounds.top + 8, MESSAGE_WIDTH - 16, (MESSAGE_HEIGHT/16)/2, Text, 0);
	pText->Show();
	AddChild(pText);

	ZSButton *pButton;
	
	int BWidth;
	int BHeight;
	
	BHeight = Engine->Graphics()->GetFontEngine()->GetTextHeight();
	BWidth = Engine->Graphics()->GetFontEngine()->GetTextWidth(ConfirmText);

	pButton = new ZSButton(BUTTON_NORMAL, 0, 
									Bounds.left + MESSAGE_WIDTH/2 - BWidth/2,
									Bounds.bottom - MESSAGE_HEIGHT/4 - BHeight/2,
									BWidth,
									BHeight);
	pButton->Show();
	pButton->SetText(ConfirmText);
	AddChild(pButton);

}

int ZSMessage::Command(int IDFrom, int Command, int Param)
{
	switch(Command)
	{
	case COMMAND_BUTTON_CLICKED:
		State = WINDOW_STATE_DONE;
		
		Visible = FALSE;
		break;
	default:
		break;
	}
	return TRUE;

}

int ZSMessage::GoModal()
{
	SetFocus(this);
	
	State = WINDOW_STATE_NORMAL;

	//loop while we're not in a done state
	while(State != WINDOW_STATE_DONE && ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		//ensure that all windows are drawn by call the main window's draw routine
		pMainWindow->Draw();
		Engine->Graphics()->Flip();
		
		//get any user input
		Engine->Input()->Update(pInputFocus);
	}

	ReleaseFocus();

	return ReturnCode;



}

int ZSMessage::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{
	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
	}
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
	//default:  ignore all keys
	return TRUE;
} //HandleKeys


void Message(char *Text, char *ConfirmText)
{
	ZSMessage  *pMessage;
	pMessage = new ZSMessage(Text, ConfirmText);

	ZSWindow::GetMain()->AddTopChild(pMessage);



	pMessage->Show();


	pMessage->GoModal();

	ZSWindow::GetMain()->RemoveChild(pMessage);
		
}

