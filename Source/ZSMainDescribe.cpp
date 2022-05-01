#include "ZSMainDescribe.h"
#include "zsdescribe.h"

int ZSMainDescribe::Command(int IDFrom, int Command, int Param)
{	
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(pParent)
		{
			return pParent->Command(IDFrom,Command,Param);
		}
	}
	return TRUE;
}

ZSMainDescribe::ZSMainDescribe(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Visible = FALSE;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = y + height;

	BackGroundSurface = NULL;

	ZSDescribe *pDescribe;
	pDescribe = new ZSDescribe(-1,Bounds.left, Bounds.top, 256, 100);
	AddChild(pDescribe);
	pDescribe->Show();
}

