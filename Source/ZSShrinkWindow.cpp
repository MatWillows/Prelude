#include "zsshrinkwindow.h"
#include "zsengine.h"

void ZSShrinkWindow::Shrink()
{
	rEnlarged = Bounds;
	Bounds.bottom = Bounds.top + 20;

	ZSWindow *pWin;
	pWin = GetChild();

	while(pWin)
	{
		pWin->Hide();
		pWin = pWin->GetSibling();
	}

	Shrunk = TRUE;

	return;
}

void ZSShrinkWindow::Enlarge()
{
	//show all child windows
	ZSWindow *pWin;
	pWin = GetChild();

	while(pWin)
	{
		pWin->Show();
		pWin = pWin->GetSibling();
	}

	Bounds.bottom = (rEnlarged.bottom - rEnlarged.top) + Bounds.top;

	Shrunk = FALSE;
	return;
}

//LeftButtonDown
//
int ZSShrinkWindow::LeftButtonDown(int x, int y) 
{ 
	ZSWindow *pWin;
	
	//check to see if there's child beneath the cursor who should receive the message
	pWin = GetChild(x,y);
	if(pWin)
	{
		return pWin->LeftButtonDown(x,y);
	}

	//if our cursor is the pointing hand, make the finger depress
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(1);
	}

	//grabe the window and start moving
	SetFocus(this);
	if(x < Bounds.left + 20 && y < Bounds.top + 20 && x > Bounds.left && y > Bounds.top)
	{
		
	}
	else
	if(Moveable)
	{
		State = WINDOW_STATE_MOVING;
		OldBounds = Bounds;
		XMoveOffset = x - Bounds.left;
		YMoveOffset = y - Bounds.top;
	}
	return TRUE; 
} // LeftButtonDown


int ZSShrinkWindow::LeftButtonUp(int x, int y)
{
	
	//if we're moving we can stop
	if(State == WINDOW_STATE_MOVING)
	{
		//validate our current location
		if(!ValidateMove(&Bounds))
		{
			//if we're intersecting a sibling, rebound to our start position
			Move(OldBounds);	
		}
		//return to normal operations
		State = WINDOW_STATE_NORMAL;
	}
	else
	{
		if(x < Bounds.left + 20 && y < Bounds.top + 20 && x > Bounds.left && y > Bounds.top)
		{
			if(Shrunk)
			{
				Enlarge();
			}
			else
			{
				Shrink();
			}
		}
	}
	
	//if our cursor is the pointing hand, raise the finger
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}

	ReleaseFocus();


	return TRUE;
}

int ZSShrinkWindow::Draw()
{
	if(Visible)
	{
		//draw our background if any
		if(BackGroundSurface)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
		}

		if(Shrunk)
		{
			Engine->Graphics()->DrawText(Bounds.left,Bounds.top,"+");
		}
		else
		{
			Engine->Graphics()->DrawText(Bounds.left,Bounds.top,"-");

			if(pChild)
			{
				pChild->Draw();
			}
		}
		Engine->Graphics()->DrawText(Bounds.left + 20, Bounds.top, Text);
	}

	if(pSibling)
	{
		pSibling->Draw();
	}
	
	return TRUE;
}