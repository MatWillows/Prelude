#include "ZSFloatSpin.h"
#include "ZSButton.h"

#define IDC_UP			1
#define IDC_DOWN		2

int ZSFloatSpin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
		case IDC_UP:
			SetValue(Value + 0.1f);
			break;
		case IDC_DOWN:
			SetValue(Value - 0.1f);
			break;
		}
		if(pParent)
		{
			pParent->Command(ID,COMMAND_EDIT_CHANGED, NULL);
		}
	}
	return TRUE;
}

ZSFloatSpin::ZSFloatSpin(int NewID, int x, int y, int Width, int Height)
{
	Text = new char[16];
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Bounds.left = x;
	Bounds.right = x + Width;
	Bounds.top = y;
	Bounds.bottom = y + Height;

	ZSButton *pButton;

	pButton = new ZSButton(BUTTON_NORTH,IDC_UP,Bounds.right - 16, Bounds.top, 16, Height / 2);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_SOUTH,IDC_DOWN, Bounds.right - 16, Bounds.top + Height / 2, 16, Height / 2);
	pButton->Show();
	AddChild(pButton);

	SetValue(0.0f);
}

void ZSFloatSpin::SetValue(float NewValue)
{
	Value = NewValue;

	sprintf(Text, "%.3f",Value);
}