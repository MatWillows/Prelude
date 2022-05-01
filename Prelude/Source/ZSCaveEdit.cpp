#include "zscaveedit.h"
#include "world.h"
#include "area.h"
#include "cavewall.h"

int ZSCaveEdit::Draw()
{

	return TRUE;
}

int ZSCaveEdit::LeftButtonDown(int x, int y)
{
	SetFocus(this);
	float MX, MY, MZ;

	PreludeWorld->ConvertMouse(x,y, &MX, &MY, &MZ);

	this->x1 = (int)MX;
	this->y1 = (int)MY;

	return TRUE;
}

int ZSCaveEdit::LeftButtonUp(int x, int y)
{
	ReleaseFocus();
	float MX, MY, MZ;

	PreludeWorld->ConvertMouse(x,y, &MX, &MY, &MZ);

	this->x2 = (int)MX;
	this->y2 = (int)MY;

	if(x1 != x2 && y1 != y2)
	{
		CaveWall *pCaveWall;
		pCaveWall = new CaveWall;
		pCaveWall->CreateSegments(this->x1, this->y1, this->x2, this->y2);
		pCaveWall->SetTexture(Engine->GetTexture("cavewall1"));
		
		Valley->AddObject(pCaveWall);
	}

	return TRUE;
}

//int RightButtonDown(int x, int y);
//int RightButtonUp(int x, int y);
//int MoveMouse(long *x, long *y, long *z);
//int Command(int IDFrom, int Command, int Param);
int ZSCaveEdit::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if((CurrentKeys[DIK_X] & 0x80) && !(LastKeys[DIK_X] & 0x80))
	{
		State = WINDOW_STATE_DONE;
	}

	return TRUE;
}

ZSCaveEdit::ZSCaveEdit(int NewID)
{
	ID = NewID;
	Visible = FALSE;
	Moveable = FALSE;
	State = WINDOW_STATE_NORMAL;
	Bounds.left = 0;
	Bounds.top = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.bottom = Engine->Graphics()->GetHeight();


}