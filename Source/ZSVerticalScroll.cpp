#include "ZSverticalscroll.h"
#include "zsengine.h"
#include "ZSButton.h"

#define IDC_THUMB			0
#define IDC_SCROLL_UP	1
#define IDC_SCROLL_DOWN	2

int ZSVScroll::Attach(ZSWindow *pTarget)
{
	RECT NewBounds;

	pTarget->GetBounds(&NewBounds);

	int Width;
	Width = Bounds.right - Bounds.left;

	NewBounds.left = NewBounds.right -= Width;

	Move(NewBounds);
	//Now move the buttons to their appropriate places

	return TRUE;
}

int ZSVScroll::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_SCROLL_UP)
		{
			if(CurPosition > LowerBound)
			{
				CurPosition--;
				MoveThumb();
				if(pParent)
				{
					pParent->Command(ID, COMMAND_SCROLL, CurPosition);
				}
			}
		}

		if(IDFrom == IDC_SCROLL_DOWN)
		{
			if(CurPosition < UpperBound)
			{
				CurPosition++;
				MoveThumb();
				if(pParent)
				{
					pParent->Command(ID, COMMAND_SCROLL, CurPosition);
				}
			}
		}

		if(IDFrom == IDC_THUMB)
		{
			State = WINDOW_STATE_NORMAL;
		}

	}
	if(Command == COMMAND_BUTTON_PRESSED)
	{
		if(IDFrom == IDC_THUMB)
		{
			State = WINDOW_STATE_MOVING;
		}
	}

	if(Command == COMMAND_BUTTON_RELEASED)
	{
		if(IDFrom == IDC_THUMB)
		{
			State = WINDOW_STATE_NORMAL;
		}
	}

	return TRUE;
}

ZSVScroll::ZSVScroll(int NewID, int x, int y, int Width, int Height)
{
	ID = NewID;
	Type = WINDOW_SCROLLVERTICAL;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Bounds.left = x;
	Bounds.right = x + Width;
	Bounds.top = y;
	Bounds.bottom = y + Height;

	Cursor = CURSOR_POINT;
	Engine->Graphics()->SetCursor(CURSOR_POINT);
	Engine->Graphics()->SetCursorFrame(0);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("vscroll.bmp", 32, 72, NULL, COLOR_KEY_FROM_FILE);

	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;

	ZSWindow *pWin;

	pWin = new ZSButton(BUTTON_NORTH, IDC_SCROLL_UP, Bounds.left, Bounds.top, Bounds.right-Bounds.left, Bounds.right - Bounds.left);
	pWin->Show();
	AddChild(pWin);

	pWin = new ZSButton(BUTTON_SOUTH, IDC_SCROLL_DOWN, Bounds.left, Bounds.bottom - (Bounds.right-Bounds.left), Bounds.right-Bounds.left, Bounds.right - Bounds.left);
	pWin->Show();
	AddChild(pWin);

	pWin = new ZSButton(BUTTON_THUMB, IDC_THUMB, Bounds.left, Bounds.top + (Bounds.right-Bounds.left), Bounds.right-Bounds.left, (Bounds.right - Bounds.left)/2);
	pWin->Show();
	AddChild(pWin);

	LowerBound = 0;
	UpperBound = 100;
	CurPosition = 0;
	PageLength = 10;

	SetText("VScroll");

	return; 
}

int ZSVScroll::LeftButtonDown(int x, int y)
{
	ZSWindow *pWin;
	
	pWin = GetChild(x,y);

	if(pWin)
	{
		return pWin->LeftButtonDown(x,y);
	}

	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(1);
	}
	SetFocus(this);
	return TRUE;
}

int ZSVScroll::LeftButtonUp(int x, int y)
{
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}
	float DrawRange;
	float Range;
	float RangePercent;
	float FinalPosition;
	int ArrowHeight;

	ArrowHeight = Bounds.right - Bounds.left;

	DrawRange = (float)((Bounds.bottom - Bounds.top) - ArrowHeight * 2 - ArrowHeight/2);

	RangePercent = (float)(y - (Bounds.top + ArrowHeight)) / DrawRange;

	Range = (float)(UpperBound - LowerBound);

	FinalPosition = Range * RangePercent;

	if((int)FinalPosition > CurPosition)
	{
		CurPosition += PageLength;
		if(CurPosition > UpperBound)
		{
			CurPosition = UpperBound;
		}
		if(pParent)
		{
			pParent->Command(ID, COMMAND_SCROLL, CurPosition);
		}
	}
	else
	{
		CurPosition -= PageLength;
		if(CurPosition < LowerBound)
		{
			CurPosition = LowerBound;
		}
		if(pParent)
		{
			pParent->Command(ID, COMMAND_SCROLL, CurPosition);
		}
	}

	MoveThumb();

	ReleaseFocus();
	
	return TRUE;
}

int ZSVScroll::Draw()
{
	//only draw ourself if we're visible
	if(Visible)
	{
		//if our state is normal do normal draw
		//draw our background if any
		if(BackGroundSurface)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
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
}

int ZSVScroll::MoveMouse(long *x, long *y, long *z)
{
	float DrawRange;
	float Range;
	float RangePercent;
	float FinalPosition;

	int ArrowHeight;

	if(State != WINDOW_STATE_MOVING)
	{
		//clamp the mouse position to between our boundaries
		if(*x < Bounds.left)
		{
			*x = Bounds.left;
		}
		else
		if(*x > Bounds.right)
		{
			*x = Bounds.right;
		}

		if(*y < Bounds.top)
		{
			*y = Bounds.top;
		}
		else
		if(*y > Bounds.bottom)
		{
			*y = Bounds.bottom;
		}
	}
	else
	{
		ArrowHeight = Bounds.right - Bounds.left;

		//clamp the mouse position to between our boundaries
		if(*x < Bounds.left)
		{
			*x = Bounds.left;
		}
		else
		if(*x > Bounds.right)
		{
			*x = Bounds.right;
		}

		if(*y < Bounds.top + ArrowHeight)
		{
			*y = Bounds.top + ArrowHeight;
		}
		else
		if(*y > Bounds.bottom - ArrowHeight - ArrowHeight/2)
		{
			*y = Bounds.bottom - ArrowHeight - ArrowHeight/2;
		}
		
		DrawRange = (float)((Bounds.bottom - Bounds.top) - ArrowHeight * 2 - ArrowHeight/2);

		RangePercent = (float)(*y - (Bounds.top + ArrowHeight)) / DrawRange;

		Range = (float)(UpperBound - LowerBound);

		FinalPosition = Range * RangePercent;

		CurPosition = (int)(FinalPosition + 0.5f);

		MoveThumb();
		
		if(pParent)
		{
			pParent->Command(ID, COMMAND_SCROLL, CurPosition);
		}
	}
	return TRUE;
}

void ZSVScroll::MoveThumb()
{
	float DrawRange;
	float Range;
	float RangePercent;
	int ArrowHeight;

	ArrowHeight = Bounds.right - Bounds.left;

	DrawRange = (float)((Bounds.bottom - Bounds.top) - (ArrowHeight * 2) - (ArrowHeight/2));

	Range = (float)(UpperBound - LowerBound);

	RangePercent = (float)(CurPosition) / Range;	

	RECT NewThumbPos;

	NewThumbPos.top = Bounds.top + ArrowHeight + (int)(DrawRange * RangePercent);
	NewThumbPos.left = Bounds.left;
	NewThumbPos.right = Bounds.right;
	NewThumbPos.bottom = NewThumbPos.top + (ArrowHeight / 2);

	GetChild(IDC_THUMB)->Move(NewThumbPos);

}