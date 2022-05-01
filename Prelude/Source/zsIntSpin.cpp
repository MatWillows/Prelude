#include "ZSIntSpin.h"
#include "ZSButton.h"

#define IDC_INT_UP			1
#define IDC_INT_DOWN		2

int ZSIntSpin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
		case IDC_INT_UP:
			if(Value < Max)
				SetValue(Value + 1);
			break;
		case IDC_INT_DOWN:
			if(Value > Min)
				SetValue(Value - 1);
			break;
		}
		if(pParent)
		{
			pParent->Command(ID,COMMAND_EDIT_CHANGED, NULL);
		}
	}
	return TRUE;
}

ZSIntSpin::ZSIntSpin(int NewID, int x, int y, int Width, int Height)
{
	Min = -10000;
	Max = 10000;
	Text = new char[16];
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Bounds.left = x;
	Bounds.right = x + Width;
	Bounds.top = y;
	Bounds.bottom = y + Height;

	ZSButton *pButton;

	pButton = new ZSButton(BUTTON_NORTH,IDC_INT_UP,Bounds.right - 16, Bounds.top, 16, Height / 2);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_SOUTH,IDC_INT_DOWN, Bounds.right - 16, Bounds.top + Height / 2, 16, Height / 2);
	pButton->Show();
	AddChild(pButton);

	SetValue(0);
}

void ZSIntSpin::SetPlusMinus()
{
	ZSWindow *pWin;
	pWin = GetChild(IDC_INT_UP);
	RemoveChild(pWin);

	pWin = GetChild(IDC_INT_DOWN);
	RemoveChild(pWin);

	ZSButton *pButton;
	
	int Height;
	Height = Bounds.bottom - Bounds.top;

	pButton = new ZSButton(BUTTON_PLUS,IDC_INT_UP, Bounds.right - Height, Bounds.top, Height, Height);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_MINUS,IDC_INT_DOWN, Bounds.right - Height * 2, Bounds.top, Height, Height);
	pButton->Show();
	AddChild(pButton);

}


void ZSIntSpin::SetValue(int NewValue)
{
	Value = NewValue;

	sprintf(Text, "%i",Value);
}