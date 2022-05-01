#include "zsgettext.h"
#include "zsengine.h"
#include "zstext.h"
#include "zsbutton.h"
#include "zseditwindow.h"

typedef enum
{
	IDC_GT_TEXT,
	IDC_GT_MESSAGE,
	IDC_GT_CONFIRM,
	IDC_GT_CANCEL,


} GETTEXT_CONTROLS;


int ZSGetText::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
			case IDC_GT_CONFIRM:
				SetText(GetChild(IDC_GT_TEXT)->GetText());
				State = WINDOW_STATE_DONE;
				return TRUE;
			case IDC_GT_CANCEL:
				if(this->Text)
					delete[] this->Text;
				this->Text = NULL;
				State = WINDOW_STATE_DONE;
				return TRUE;
			default:
				break;
		}
	}
	return TRUE;
}

ZSGetText::ZSGetText(char *Message, char *StartText)
{
	ID = -1;
	State = WINDOW_STATE_NORMAL;
	Moveable = FALSE;
	Visible = TRUE;

	int CenterX;
	CenterX = Engine->Graphics()->GetWidth() / 2;
		
	int CenterY;
	CenterY = Engine->Graphics()->GetHeight() / 2;
		
	Bounds.left = CenterX - 100;
	Bounds.right = CenterX + 100;
	Bounds.top = CenterY - 50;
	Bounds.bottom = CenterY + 50;
	
	//first the message box
	ZSText *pText;
	pText = new ZSText(IDC_GT_MESSAGE,Bounds.left,Bounds.top,Message);
	AddChild(pText);
	pText->Show();

	ZSButton *pButton;
	//confirm button
	pButton = new ZSButton(BUTTON_CHECK, IDC_GT_CONFIRM, Bounds.right - 112, Bounds.bottom - 32, 24, 24);
	pButton->Show();
	AddChild(pButton);

/*	//cancel button
	pButton = new ZSButton(BUTTON_NORMAL, IDC_GT_CANCEL, Bounds.right - 16, Bounds.top, 16, 16);
	pButton->SetText("X");
	pButton->Show();
	AddChild(pButton);
*/
	//actual text
	ZSEdit *pEdit;
	pEdit = new ZSEdit(WINDOW_EDIT, IDC_GT_TEXT, Bounds.left + 8, Bounds.top + 32, 184, 32);
	pEdit->CreateEmptyBorderedBackground(2,1);
	pEdit->SetText(StartText);
	pEdit->Show();
	AddChild(pEdit);

	CreateWoodBorderedBackground(8);
}

char *GetModalText(char *Message, char *StartText, int Length)
{
	char *ReturnText = NULL;
	ZSGetText *pGetText;

	pGetText = new ZSGetText(Message,StartText);

	ZSWindow::GetMain()->AddTopChild(pGetText);
	pGetText->Show();

	pGetText->SetFocus(pGetText);

	pGetText->GoModal();

	pGetText->ReleaseFocus();

	if(pGetText->GetText())
	{
		ReturnText = new char[Length];

		strcpy(ReturnText,pGetText->GetText());

		ZSWindow::GetMain()->RemoveChild(pGetText);

	}
	else
	{
		return NULL;
	}
	return ReturnText;

}