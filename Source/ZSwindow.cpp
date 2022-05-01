//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSWindow.cpp           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    Sept 25, 2000			                                  *  
//*Author:      Mat Williams            
//*Purpose:                                                           *
//*		This file provides implementation of ZSWindow class methods
//*
//*********************************************************************
//*Outstanding issues:                                                                                                      *
//*		Need to switch from directx surface as background to zssurface	
//*		No copy contructor or assignment operators defined
//*		Parchment border not implemented
//*********************************************************************
//*********************************************************************
//revision 4. Added old focal stack in place of previous focus
//revision 5. Added virtual methods gain and lose focus
//revision 6. Added flexible border

#include "ZSWindow.h"
#include "ZSHelpWin.h"
#include "ZSEngine.h"
#include "ZSUtilities.h"

//* Static Members
//**********************************************************************

ZSWindow *ZSWindow::pMainWindow = NULL;
ZSWindow *ZSWindow::pInputFocus = NULL;
int ZSWindow::OldFocusStackTop = 0;
ZSWindow *ZSWindow::OldFocusStack[MAX_FOCUS_STACK_DEPTH];
LPDIRECTDRAWSURFACE7 ZSWindow::lpddsParchment = NULL;
LPDIRECTDRAWSURFACE7 ZSWindow::lpddsWood = NULL;
LPDIRECTDRAWSURFACE7 ZSWindow::lpddsVBorder[4] = { NULL, NULL, NULL, NULL};
LPDIRECTDRAWSURFACE7 ZSWindow::lpddsHBorder[4] = { NULL, NULL, NULL, NULL};
LPDIRECTDRAWSURFACE7 ZSWindow::lpddsCorner[4] = { NULL, NULL, NULL, NULL};
DWORD ZSWindow::NextMouseUpdate = 0;


//* Methods
//*********************************************************************

//GetChild
// returns a pointer to a child window whose bounds contain a given x/y
ZSWindow *ZSWindow::GetChild(int x, int y)
{
	ZSWindow *pWin;

	//get the top child
	pWin = pChild;

	//search throught the siblings	
	while(pWin)
	{
		//if a window is visible and its bounds encompass the given x,y
		if(pWin->Visible &&
			x <= pWin->Bounds.right &&
			x >= pWin->Bounds.left &&
			y >= pWin->Bounds.top &&
			y <= pWin->Bounds.bottom)
		{
			//return a pointer to it
			return pWin;
		}
		pWin = pWin->GetSibling();
	}

	//if no children's bounds encompass a the coordinate, return nothing
	return NULL;
} //GetChild

//GetChild
// Get a pointer to a child window with a given ID
ZSWindow *ZSWindow::GetChild(int ID)
{
	ZSWindow *pWin;
	pWin = pChild;

	while(pWin)
	{
		if(pWin->ID == ID)
		{
			break;
		}
		pWin = pWin->GetSibling();
	}

	return pWin;
} //GetChild

//Removechild
// Delete a child window
int ZSWindow::RemoveChild(ZSWindow *ToRemove)
{
	if(!ToRemove)
	{
		return FALSE;
	}

	ZSWindow *pWin, *pLastWin;
	pLastWin = pWin = pChild;

	if(pWin == ToRemove)
	{
		pChild = pChild->pSibling;
		delete ToRemove;
		return TRUE;
	}

	while(pWin)
	{
		pLastWin = pWin;

		pWin = pWin->pSibling;

		if(pWin == ToRemove)
		{
			pLastWin->pSibling = pWin->pSibling;
			delete pWin;
			return TRUE;
		}
	}
	DEBUG_INFO("Failed to remove child window\n")

	return FALSE;
} //RemoveChild

//SetText
// Replace our current text with the new text
int ZSWindow::SetText(char *NewText) 
{ 
	//delete the old text
	if(!Text || strlen(NewText) + 1 > sizeof(Text))
	{
		if(Text)
			delete[] Text;

		//allocate space for the new text
		Text = new char[strlen(NewText)+1];
	}

	//copy the provided text into the newly allocated space
	strcpy(Text,NewText);

	return TRUE; 
} //SetText

//SetText 
// Replace out current text with the textual representation of a an integer
int ZSWindow::SetText(int n) 
{ 

	//create the textual representation
	char blarg[16];
	sprintf(blarg,"%i",n);

	//delete the old text
	if(Text)
		delete[] Text;

	//allocate space for the new text
	Text = new char[strlen(blarg)+1];

	//copy the textual rep into the newly allocated space
	strcpy(Text,blarg);

	return TRUE; 
} //SetText

//AddChild
//
int ZSWindow::AddChild(ZSWindow *ToAdd) 
{ 
	if(pChild)
	{
		ToAdd->pSibling = pChild;
	}

	pChild = ToAdd;
	
	ToAdd->pParent = this;
	return TRUE; 
} //AddChild

//AddTopChild
//  add a child in such a manner as to assure it will be drawn on top of all other children
int ZSWindow::AddTopChild(ZSWindow *ToAdd) 
{ 
	if(pChild)
	{
		pChild->AddTopSibling(ToAdd);
	}
	else
	{
		pChild = ToAdd;
	}

	return TRUE; 
} //AddTopChild

//AddTopSibling
//  Add A sibling in sucha manner as to assure it will be drawn last (on top of all other siblings)
int ZSWindow::AddTopSibling(ZSWindow *ToAdd) 
{ 
	if(pSibling)
	{
		pSibling->AddTopSibling(ToAdd);
	}
	else
	{
		pSibling = ToAdd;
	}
	
	ToAdd->pParent = pParent;
	return TRUE; 
} // AddTopSibling

//AddSibling
// 
int ZSWindow::AddSibling(ZSWindow *ToAdd) 
{ 
	ToAdd->pSibling = pSibling;
	pSibling = ToAdd;
	ToAdd->pParent = pParent;
	return TRUE; 
} //AddSibling

//Move
// move by an x and y offset
int ZSWindow::Move(int xoff, int yoff)
{
	ZSWindow *pWin = pChild;
	while(pWin)
	{
		pWin->Move(xoff,yoff);
		pWin = pWin->pSibling;
	}
	Bounds.left		+= xoff;
	Bounds.right	+= xoff;
	Bounds.top		+= yoff;
	Bounds.bottom	+= yoff;

	return TRUE; 
} //move

//Move
// move the window to a new position
int ZSWindow::Move(RECT NewBounds) 
{ 
	int xoff;
	int yoff;
	xoff = NewBounds.left - Bounds.left;
	yoff = NewBounds.top - Bounds.top;
	
	ZSWindow *pWin = pChild;
	while(pWin)
	{
		pWin->Move(xoff,yoff);
		pWin = pWin->pSibling;
	}
	Bounds = NewBounds;
	return TRUE; 
} //move


//ValidateMove
// confirm that a given rectangle does not overlap any of our sibling's boundaries
int ZSWindow::ValidateMove(RECT *NewBounds) 
{ 
	//we must check the bounds against all our sibilings
	ZSWindow *pWin;
	
	if(!pParent) return TRUE;

	//get the root child from our parent;
	pWin = pParent->GetChild();

	//traverse the list of siblings
	while(pWin)
	{
		//ignore ourselves
		if(pWin != this && pWin->IsVisible())
		{
			//check each corner for overlap
			if(NewBounds->left >= pWin->Bounds.left &&
				NewBounds->left <= pWin->Bounds.right &&
				NewBounds->top >= pWin->Bounds.top &&
				NewBounds->top <= pWin->Bounds.bottom)
			{
				return FALSE;
			}

			if(NewBounds->right >= pWin->Bounds.left &&
				NewBounds->right <= pWin->Bounds.right &&
				NewBounds->top >= pWin->Bounds.top &&
				NewBounds->top <= pWin->Bounds.bottom)
			{
				return FALSE;
			}

			if(NewBounds->left >= pWin->Bounds.left &&
				NewBounds->left <= pWin->Bounds.right &&
				NewBounds->bottom >= pWin->Bounds.top &&
				NewBounds->bottom <= pWin->Bounds.bottom)
			{
				return FALSE;
			}

			if(NewBounds->right >= pWin->Bounds.left &&
				NewBounds->right <= pWin->Bounds.right &&
				NewBounds->bottom >= pWin->Bounds.top &&
				NewBounds->bottom <= pWin->Bounds.bottom)
			{
				return FALSE;
			}
		}
		pWin = pWin->pSibling;
	}
	//no sibling intersections
	//move is valid
	return TRUE; 
} //ValidateMove

//drawmouse
//
int ZSWindow::DrawMouse(int x, int y) 
{ 
	ZSWindow *pWin;

	pWin = GetChild(x, y);
	if(pWin)
	{
		pWin->DrawMouse(x,y);
	}

	if(Cursor != CURSOR_NONE)
	{
		SetCursor(Cursor);
		Engine->Graphics()->DrawCursor(Engine->Input()->GetMouseRect());
		if(timeGetTime() > NextMouseUpdate)
		{
			Engine->Graphics()->AdvanceCursorFrame();
			NextMouseUpdate = timeGetTime() + 25;
		}

	}
	return TRUE; 
}// DrawMouse

//Draw
//
int ZSWindow::Draw() 
{ 
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
				DrawTo.left = Bounds.left + Border;
				DrawTo.right = Bounds.right - Border;
				DrawTo.top = Bounds.top + Border;
				DrawTo.bottom = Bounds.bottom - Border;
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
	return TRUE; 
} // Draw

//LeftButtonDown
//
int ZSWindow::LeftButtonDown(int x, int y) 
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
	if(Moveable)
	{
		State = WINDOW_STATE_MOVING;
		OldBounds = Bounds;
		XMoveOffset = x - Bounds.left;
		YMoveOffset = y - Bounds.top;
	}
	return TRUE; 
} // LeftButtonDown

//LeftButtonUp
//
int ZSWindow::LeftButtonUp(int x, int y) 
{
	/*
	DEBUG_INFO("LeftButtonUp ");
	if(Text)
	{
		DEBUG_INFO(Text);
	}
	char blarg[16];
	sprintf(blarg, ": %i", ID);
	DEBUG_INFO(blarg);
	DEBUG_INFO("\n");
*/
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
	
	//if our cursor is the pointing hand, raise the finger
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}

	ReleaseFocus();
	//done
	return TRUE; 
} // LeftButtonUp

//RightButtonDown
//
int ZSWindow::RightButtonDown(int x, int y) 
{
	ZSWindow *pWin;
	
	//pass the message along to any child beneath the cursor
	pWin = GetChild(x,y);
	if(pWin)
	{
		return pWin->RightButtonDown(x,y);
	}
	else
	{
		SetFocus(this);
	}
	

	//done
	return TRUE; 
} // RightButtonDown

//RightButtonUp
//
int ZSWindow::RightButtonUp(int x, int y) 
{ 
	//pass the message along to any child beneath the cursor
	ReleaseFocus();

	//done
	return TRUE; 
} // RightButtonUp

//MoveMouse
//
int ZSWindow::MoveMouse(long *x, long *y, long *z) 
{ 
	//if we're moving, move
	if(State == WINDOW_STATE_MOVING)
	{
		int Width;
		int Height;
		RECT NewBounds;

		Width = Bounds.right - Bounds.left;
		Height = Bounds.bottom - Bounds.top;

		NewBounds.left = *x - XMoveOffset;
		NewBounds.right = NewBounds.left + Width;
		NewBounds.top = *y - YMoveOffset;
		NewBounds.bottom = NewBounds.top + Height;

		//make sure we haven't left our parent's boundaries
		if(NewBounds.left < pParent->Bounds.left)
		{
			NewBounds.left = pParent->Bounds.left;
			NewBounds.right = NewBounds.left + Width;
		}
		else
		if(NewBounds.right > pParent->Bounds.right)
		{
			NewBounds.right = pParent->Bounds.right;
			NewBounds.left = NewBounds.right - Width;
		}
		
		if(NewBounds.top < pParent->Bounds.top)
		{
			NewBounds.top = pParent->Bounds.top;
			NewBounds.bottom = NewBounds.top + Height;
		}
		else
		if(NewBounds.bottom > pParent->Bounds.bottom)
		{
			NewBounds.bottom = pParent->Bounds.bottom;
			NewBounds.top = NewBounds.bottom - Height;
		}
		Move(NewBounds);
	}

	if(pParent)
	{
		//clamp the mouse position to between our boundaries
		if(*x < pParent->Bounds.left)
		{
			*x = pParent->Bounds.left;
		}
		else
		if(*x > pParent->Bounds.right)
		{
			*x = pParent->Bounds.right;
		}

		if(*y < pParent->Bounds.top)
		{
			*y = pParent->Bounds.top;
		}
		else
		if(*y > pParent->Bounds.bottom)
		{
			*y = pParent->Bounds.bottom;
		}
	}
	//done
	return TRUE; 
} // MoveMouse

//Command
//
int ZSWindow::Command(int IDFrom, int Command, int Param) 
{
	//default
	//do nothing
	return TRUE; 
} //Command

//Base Constructor
//
ZSWindow::ZSWindow()
{
	Cursor = CURSOR_POINT;
	BackGroundSurface = NULL;
	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	State = WINDOW_STATE_NORMAL;
	Border = 0;
	Moveable = TRUE;
	TextColor = 0;
}

//Complex Constructor
//		
ZSWindow::ZSWindow(WINDOW_T NewType, int NewID, int x, int y, int width, int height) 
{ 
	TextColor = 0;
	Type = NewType;
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = TRUE;
	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	Cursor = CURSOR_POINT;
	
	BackGroundSurface = NULL;

	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	Border = 0;
	return; 
}

//Destructor
//
ZSWindow::~ZSWindow() 
{ 
	//make sure we're not on the focal stack
	int n = 0, sn;
	for(n = 0; n < OldFocusStackTop; n++)
	{
		if(OldFocusStack[n] == this)
		{
			DEBUG_INFO("Deleting window on focal stack\n");
			for(sn = n; sn < OldFocusStackTop - 1; sn++)
			{
				OldFocusStack[sn] = OldFocusStack[sn+1];
			}
			OldFocusStackTop--;
			n = 0;
		}
	}

	//delete all child windows
	ZSWindow *pWin;
	ZSWindow *pOldWin;

	pWin = pChild;

	while(pWin)
	{
		pOldWin = pWin;
		pWin = pWin->GetSibling();
		delete pOldWin;
	}

	if(BackGroundSurface)
	{
		BackGroundSurface->Release();
		BackGroundSurface = NULL;
	}
	return; 
}

//HandleKeys
//
int ZSWindow::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{
	//default:  ignore all keys
	if(CurrentKeys[DIK_ESCAPE] & 0x80 &&
		(CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80))
	{
      SafeExit("Forced hard exit\n");
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
/*
	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
	}
*/	return TRUE;
} //HandleKeys


//GoModal
//
int ZSWindow::GoModal()
{
	//State = WINDOW_STATE_NORMAL;
	//loop while we're not in a done state
	while(State != WINDOW_STATE_DONE && ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		//ensure that all windows are drawn by call the main window's draw routine
		pMainWindow->Draw();
		Engine->Graphics()->Flip();
		
		//get any user input
		Engine->Input()->Update(pInputFocus);
	}

	return ReturnCode;
} //GoModal

//SetFocus
//
void ZSWindow::SetFocus(ZSWindow *pToFocus)
{
	/*
	DEBUG_INFO("Setting focus: ");
	if(pToFocus->Text)
	{
		DEBUG_INFO(pToFocus->Text);
	}
	char blarg[16];
	sprintf(blarg, ": %i", pToFocus->ID);
	DEBUG_INFO(blarg);
	DEBUG_INFO("\n");
*/
	if(!pInputFocus)
	{
		OldFocusStack[0] = pToFocus;
		pInputFocus = pToFocus;
		pInputFocus->GainFocus();
		OldFocusStackTop = 1;
	}
	else
	{
		//put the current focus on to of the old focal stack
		OldFocusStack[OldFocusStackTop] = pInputFocus;
		OldFocusStackTop++;
		
		if(pInputFocus != pToFocus) 
		{
			pInputFocus->LoseFocus();
	
			//set the newfocus
			pInputFocus = pToFocus;

			pInputFocus->GainFocus();
		}
	}
	return;
} //SetFocus

//ReleaseFocus
//
void ZSWindow::ReleaseFocus()
{
/*	DEBUG_INFO("Releasing focus: ");
	if(pInputFocus->Text)
	{
		DEBUG_INFO(pInputFocus->Text);
	}
	char blarg[16];
	sprintf(blarg, ": %i", pInputFocus->ID);
	DEBUG_INFO(blarg);
	DEBUG_INFO("\n");
*/
#ifndef NDEBUG
	if(pInputFocus != this)
	{
		FILE *fp;
		fp = SafeFileOpen("debug.txt","a+t");
		fprintf(fp,"\n");
		fprintf(fp,"bad release w/o owning input");
		fprintf(fp,"\n");
		this->OutputDebugInfo(fp);
		fprintf(fp,"\n");
		pInputFocus->OutputDebugInfo(fp);
		fprintf(fp,"\n");
		fclose(fp);
	}
#endif

	pInputFocus->LoseFocus();
	
	OldFocusStackTop--;
	//pop off the top of the old focal stack and make it the focus
	pInputFocus = OldFocusStack[OldFocusStackTop];
	pInputFocus->GainFocus();
	
	return;
} //ReleaseFocus

//GainFocus
//
void ZSWindow::GainFocus()
{
/*	DEBUG_INFO("  Gaining focus: ");
	if(Text)
	{
		DEBUG_INFO(Text);
	}
	char blarg[16];
	sprintf(blarg, ": %i", ID);
	DEBUG_INFO(blarg);
	DEBUG_INFO("\n");
*/
	Engine->Graphics()->SetCursor(Cursor);
  
	 //default, inform the parent if any
	if(pParent)
	{
		pParent->Command(ID,COMMAND_CHILD_GAINFOCUS, 0);
	}
	return;
} //GainFocus

//LoseFocus
//
void ZSWindow::LoseFocus()
{
/*	DEBUG_INFO("  Losinging focus: ");
	if(Text)
	{
		DEBUG_INFO(Text);
	}
	char blarg[16];
	sprintf(blarg, ": %i", ID);
	DEBUG_INFO(blarg);
	DEBUG_INFO("\n");
*/
	//default, inform the parent if any
	if(pParent)
	{
		pParent->Command(ID,COMMAND_CHILD_LOSEFOCUS, 0);
	}
	return;
} //LoseFocus

//OutputDebugInfo
//
void ZSWindow::OutputDebugInfo(FILE *fp)
{
	fprintf(fp, "ID: %i\n", ID);
	if(pParent)
	fprintf(fp, "Parent: %i\n", pParent->ID);
	fprintf(fp, "Type: %i\n", Type);
	fprintf(fp, "State: %i\n",State);
	fprintf(fp, "Text: %s\n", Text);
/*	fprintf(fp, "Visible: %i\n", Visible);
	fprintf(fp, "Moveable: %i\n", Moveable);
	fprintf(fp, "Bounds: (%i,%i) - (%i,%i)\n",Bounds.left,Bounds.top,Bounds.right, Bounds.bottom);
	fprintf(fp, "OldBounds: (%i,%i) - (%i,%i)\n", OldBounds.left, OldBounds.top, OldBounds.right, OldBounds.bottom);
	fprintf(fp, "Cursor: %i\n", Cursor);
	fprintf(fp, "Move Offset: (%i,%i)\n", XMoveOffset, YMoveOffset);
	fprintf(fp, "Background: %i\n",(int)BackGroundSurface);
	fprintf(fp, "Return Code: %i\n",ReturnCode);

	if(pMainWindow)
		fprintf(fp, "MainWindow: %i\n", pMainWindow->ID);

	if(pInputFocus)
		fprintf(fp, "Focus: %i\n", pInputFocus->ID);


	fprintf(fp, "Children  ****************************************\n");
	if(pChild)
		pChild->OutputDebugInfo(fp);

	fprintf(fp, "Siblings: ****************************************\n");
	if(pSibling)
		pSibling->OutputDebugInfo(fp);
*/
}

//OutputDebugInfo
//
void ZSWindow::OutputDebugInfo(const char *FileName)
{
	FILE *fp;

	fp = SafeFileOpen(FileName, "wt");

	OutputDebugInfo(fp);

	fclose(fp);
}  //OuputDebugInfo


void ZSWindow::CreateEmptyBorderedBackground(int BorderWidth, int btype)
{
	Border = BorderWidth;

	int Width;
	int Height;
	Width = Bounds.right - Bounds.left;
	Height = Bounds.bottom - Bounds.top;

	LPDIRECTDRAWSURFACE7 Corner = NULL;
	LPDIRECTDRAWSURFACE7 VBorder = NULL;
	LPDIRECTDRAWSURFACE7 HBorder = NULL;
	
	if(!lpddsVBorder[btype])
	{
		char VName[32];
		char HName[32];
		char CName[32];
		sprintf(VName,"vborder%i.bmp",btype);
		sprintf(HName,"hborder%i.bmp",btype);
		sprintf(CName,"corner%i.bmp",btype);
		lpddsHBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(HName, 32, 32,NULL,NULL);
		lpddsCorner[btype] = Engine->Graphics()->CreateSurfaceFromFile(CName, 64, 64,NULL,NULL);
		lpddsVBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(VName, 32, 32,NULL,NULL);
	}

	if(BorderWidth)
	{
		Corner = lpddsCorner[btype];
		VBorder = lpddsVBorder[btype];
		HBorder = lpddsHBorder[btype];
	}
	
	RECT rDest;
	RECT rSource;
	if(BackGroundSurface)
	{
		BackGroundSurface->Release();
	}

	BackGroundSurface = Engine->Graphics()->CreateSurface(Bounds.right-Bounds.left, Bounds.bottom-Bounds.top,NULL,RGB(255,0,255));
	Engine->Graphics()->FillSurface(BackGroundSurface,RGB(255,0,255),NULL);

	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = 0;
	rDest.bottom = BorderWidth;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32;
	rSource.top = 0;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = 0;
	rDest.bottom = BorderWidth;
	rSource.left = 32;
	rSource.right = 64;
	rSource.bottom = 32;
	rSource.top = 0;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 64;
	rSource.top = 32;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;
	rSource.left = 32;
	rSource.right = 64;
	rSource.bottom = 64;
	rSource.top = 32;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = BorderWidth;
	rDest.bottom = rDest.top + BorderWidth;

	while(rDest.bottom < Height - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
		rDest.top += BorderWidth;
		rDest.bottom += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

	rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

	BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = BorderWidth;
	rDest.bottom = rDest.top + BorderWidth;

	while(rDest.bottom < Height - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
		rDest.top += BorderWidth;
		rDest.bottom += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

	rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

	BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

	rDest.left = BorderWidth;
	rDest.right = rDest.left + BorderWidth;
	rDest.top = 0;
	rDest.bottom = BorderWidth;

	while(rDest.right < Width - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
		rDest.right += BorderWidth;
		rDest.left += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
	rSource.bottom = 32;

	rDest.right += ((Width - BorderWidth) - rDest.right);
	BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);

	rDest.left = BorderWidth;
	rDest.right = rDest.left + BorderWidth;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;

	while(rDest.right < Width - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
		rDest.right += BorderWidth;
		rDest.left += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
	rSource.bottom = 32;

	rDest.right += ((Width - BorderWidth) - rDest.right);
	BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);

//	Corner->Release();
//	VBorder->Release();
//	HBorder->Release();

}

void ZSWindow::CreateWoodBorderedBackground(int BorderWidth, int btype)
{
	Border = BorderWidth;

	int Width;
	int Height;
	Width = Bounds.right - Bounds.left;
	Height = Bounds.bottom - Bounds.top;

	LPDIRECTDRAWSURFACE7 Corner = NULL;
	LPDIRECTDRAWSURFACE7 VBorder = NULL;
	LPDIRECTDRAWSURFACE7 HBorder = NULL;
	LPDIRECTDRAWSURFACE7 Parchment = NULL;

	if(!lpddsVBorder[btype])
	{
		char VName[32];
		char HName[32];
		char CName[32];
		sprintf(VName,"vborder%i.bmp",btype);
		sprintf(HName,"hborder%i.bmp",btype);
		sprintf(CName,"corner%i.bmp",btype);
		lpddsHBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(HName, 32, 32,NULL,NULL);
		lpddsCorner[btype] = Engine->Graphics()->CreateSurfaceFromFile(CName, 64, 64,NULL,NULL);
		lpddsVBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(VName, 32, 32,NULL,NULL);
	}

	if(BorderWidth)
	{
		Corner = lpddsCorner[btype];
		VBorder = lpddsVBorder[btype];
		HBorder = lpddsHBorder[btype];
	}
	
	if(!lpddsWood)
	{
		lpddsWood = Engine->Graphics()->CreateSurfaceFromFile("wood.bmp", 256, 256,NULL,NULL);
	}
	Parchment = lpddsWood;
	
	RECT rDest;
	rDest.left = BorderWidth/2;
	rDest.top = BorderWidth/2;
	rDest.right = Width - BorderWidth/2;
	rDest.bottom = Height - BorderWidth/2;
	RECT rSource;
	if(BackGroundSurface)
	{
		BackGroundSurface->Release();
	}

	BackGroundSurface = Engine->Graphics()->CreateSurface(Width, Height, NULL,RGB(255,0,255));
	Engine->Graphics()->FillSurface(BackGroundSurface,RGB(255,0,255), NULL);

	HRESULT hr;
	hr = BackGroundSurface->Blt(&rDest, Parchment, NULL, NULL, NULL);

	if(hr != DD_OK)
	{
		Engine->ReportError(hr);
		hr = lpddsWood->Restore();
		lpddsWood->Release();
		lpddsWood = Engine->Graphics()->CreateSurfaceFromFile("wood.bmp", 256, 256,NULL,NULL);
		Parchment = lpddsWood;

		hr = BackGroundSurface->Blt(&rDest, Parchment, NULL, NULL, NULL);
		if(hr != DD_OK)
		{
			DEBUG_INFO("could not create wood bordered background.");
		}
	}

	if(BorderWidth)
	{
		rDest.left = 0;
		rDest.right = BorderWidth;
		rDest.top = 0;
		rDest.bottom = BorderWidth;
		rSource.left = 0;
		rSource.right = 32;
		rSource.bottom = 32;
		rSource.top = 0;
		BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

		rDest.left = Width - BorderWidth;
		rDest.right = Width;
		rDest.top = 0;
		rDest.bottom = BorderWidth;
		rSource.left = 32;
		rSource.right = 64;
		rSource.bottom = 32;
		rSource.top = 0;
		BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

		rDest.left = 0;
		rDest.right = BorderWidth;
		rDest.top = Height - BorderWidth;
		rDest.bottom = Height;
		rSource.left = 0;
		rSource.right = 32;
		rSource.bottom = 64;
		rSource.top = 32;
		BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

		rDest.left = Width - BorderWidth;
		rDest.right = Width;
		rDest.top = Height - BorderWidth;
		rDest.bottom = Height;
		rSource.left = 32;
		rSource.right = 64;
		rSource.bottom = 64;
		rSource.top = 32;
		BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);


		rDest.left = 0;
		rDest.right = BorderWidth;
		rDest.top = BorderWidth;
		rDest.bottom = rDest.top + BorderWidth;

		while(rDest.bottom < Height - BorderWidth)
		{
			BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
			rDest.top += BorderWidth;
			rDest.bottom += BorderWidth;
		}
		rSource.top = 0;
		rSource.left = 0;
		rSource.right = 32;
		rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

		rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

		BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

		rDest.left = Width - BorderWidth;
		rDest.right = Width;
		rDest.top = BorderWidth;
		rDest.bottom = rDest.top + BorderWidth;

		while(rDest.bottom < Height - BorderWidth)
		{
			BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
			rDest.top += BorderWidth;
			rDest.bottom += BorderWidth;
		}
		rSource.top = 0;
		rSource.left = 0;
		rSource.right = 32;
		rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

		rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

		BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

		rDest.left = BorderWidth;
		rDest.right = rDest.left + BorderWidth;
		rDest.top = 0;
		rDest.bottom = BorderWidth;

		while(rDest.right < Width - BorderWidth)
		{
			BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
			rDest.right += BorderWidth;
			rDest.left += BorderWidth;
		}
		rSource.top = 0;
		rSource.left = 0;
		rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
		rSource.bottom = 32;

		rDest.right += ((Width - BorderWidth) - rDest.right);
		BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);

		rDest.left = BorderWidth;
		rDest.right = rDest.left + BorderWidth;
		rDest.top = Height - BorderWidth;
		rDest.bottom = Height;

		while(rDest.right < Width - BorderWidth)
		{
			BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
			rDest.right += BorderWidth;
			rDest.left += BorderWidth;
		}
		rSource.top = 0;
		rSource.left = 0;
		rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
		rSource.bottom = 32;

		rDest.right += ((Width - BorderWidth) - rDest.right);
		BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);
	}

	if(BorderWidth)
	{	
	//	Corner->Release();
	//	VBorder->Release();
	//	HBorder->Release();
	}
	//Parchment->Release();
	

}


void ZSWindow::CreateParchmentBorderedBackground(int BorderWidth, int btype)
{
	Border = BorderWidth;

	int Width;
	int Height;
	Width = Bounds.right - Bounds.left;
	Height = Bounds.bottom - Bounds.top;

	LPDIRECTDRAWSURFACE7 Corner = NULL;
	LPDIRECTDRAWSURFACE7 VBorder = NULL;
	LPDIRECTDRAWSURFACE7 HBorder = NULL;
	
	if(!lpddsVBorder[btype])
	{
		char VName[32];
		char HName[32];
		char CName[32];
		sprintf(VName,"vborder%i.bmp",btype);
		sprintf(HName,"hborder%i.bmp",btype);
		sprintf(CName,"corner%i.bmp",btype);
		lpddsHBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(HName, 32, 32,NULL,NULL);
		lpddsCorner[btype] = Engine->Graphics()->CreateSurfaceFromFile(CName, 64, 64,NULL,NULL);
		lpddsVBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(VName, 32, 32,NULL,NULL);
	}


	LPDIRECTDRAWSURFACE7 Parchment = NULL;
	
	if(!lpddsParchment)
	{
		lpddsParchment = Engine->Graphics()->CreateSurfaceFromFile("Parchment.bmp", 256, 256,NULL,NULL);
	}
	
	Parchment = lpddsParchment;

	RECT rDest;
	rDest.left = BorderWidth/2;
	rDest.top = BorderWidth/2;
	rDest.right = Width - BorderWidth/2;
	rDest.bottom = Height - BorderWidth/2;
	RECT rSource;
	if(BackGroundSurface)
	{
		BackGroundSurface->Release();
	}

	BackGroundSurface = Engine->Graphics()->CreateSurface(Width, Height, NULL,RGB(255,0,255));
	Engine->Graphics()->FillSurface(BackGroundSurface,RGB(255,0,255), NULL);


	HRESULT hr;
	hr = BackGroundSurface->Blt(&rDest, Parchment, NULL, NULL, NULL);

	if(hr != DD_OK)
	{
		Engine->ReportError(hr);
		lpddsParchment->Restore();
		lpddsParchment->Release();
		lpddsParchment = Engine->Graphics()->CreateSurfaceFromFile("parchment.bmp", 256, 256,NULL,NULL);
			
		Parchment = lpddsParchment;
		hr = BackGroundSurface->Blt(&rDest, Parchment, NULL, NULL, NULL);
		if(hr != DD_OK)
		{
			DEBUG_INFO("could not create parchment bordered background.");
		}
	}
	if(BorderWidth)
	{

	Corner = lpddsCorner[btype];
	VBorder = lpddsVBorder[btype];
	HBorder = lpddsHBorder[btype];
	
	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = 0;
	rDest.bottom = BorderWidth;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32;
	rSource.top = 0;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = 0;
	rDest.bottom = BorderWidth;
	rSource.left = 32;
	rSource.right = 64;
	rSource.bottom = 32;
	rSource.top = 0;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 64;
	rSource.top = 32;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;
	rSource.left = 32;
	rSource.right = 64;
	rSource.bottom = 64;
	rSource.top = 32;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);


	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = BorderWidth;
	rDest.bottom = rDest.top + BorderWidth;

	while(rDest.bottom < Height - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
		rDest.top += BorderWidth;
		rDest.bottom += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

	rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

	BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = BorderWidth;
	rDest.bottom = rDest.top + BorderWidth;

	while(rDest.bottom < Height - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
		rDest.top += BorderWidth;
		rDest.bottom += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

	rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

	BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

	rDest.left = BorderWidth;
	rDest.right = rDest.left + BorderWidth;
	rDest.top = 0;
	rDest.bottom = BorderWidth;

	while(rDest.right < Width - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
		rDest.right += BorderWidth;
		rDest.left += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
	rSource.bottom = 32;

	rDest.right += ((Width - BorderWidth) - rDest.right);
	BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);

	rDest.left = BorderWidth;
	rDest.right = rDest.left + BorderWidth;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;

	while(rDest.right < Width - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
		rDest.right += BorderWidth;
		rDest.left += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
	rSource.bottom = 32;

	rDest.right += ((Width - BorderWidth) - rDest.right);
	BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);

	}

}

void ZSWindow::AddBorder(int BorderWidth, int btype)
{
	if(!BackGroundSurface)
	{
		BackGroundSurface = Engine->Graphics()->CreateSurface(Bounds.right - Bounds.left, Bounds.bottom - Bounds.top, NULL, 0);
		Engine->Graphics()->FillSurface(BackGroundSurface,0,NULL);

	}
	Border = BorderWidth;

	int Width;
	int Height;
	Width = Bounds.right - Bounds.left;
	Height = Bounds.bottom - Bounds.top;

	LPDIRECTDRAWSURFACE7 Corner = NULL;
	LPDIRECTDRAWSURFACE7 VBorder = NULL;
	LPDIRECTDRAWSURFACE7 HBorder = NULL;
	
	if(!lpddsVBorder[btype])
	{
		char VName[32];
		char HName[32];
		char CName[32];
		sprintf(VName,"vborder%i.bmp",btype);
		sprintf(HName,"hborder%i.bmp",btype);
		sprintf(CName,"corner%i.bmp",btype);
		lpddsHBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(HName, 32, 32,NULL,NULL);
		lpddsCorner[btype] = Engine->Graphics()->CreateSurfaceFromFile(CName, 64, 64,NULL,NULL);
		lpddsVBorder[btype] = Engine->Graphics()->CreateSurfaceFromFile(VName, 32, 32,NULL,NULL);
	}
	
	Corner = lpddsCorner[btype];
	VBorder = lpddsVBorder[btype];
	HBorder = lpddsHBorder[btype];
	



	RECT rDest;
	RECT rSource;

		rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = 0;
	rDest.bottom = BorderWidth;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32;
	rSource.top = 0;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = 0;
	rDest.bottom = BorderWidth;
	rSource.left = 32;
	rSource.right = 64;
	rSource.bottom = 32;
	rSource.top = 0;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 64;
	rSource.top = 32;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;
	rSource.left = 32;
	rSource.right = 64;
	rSource.bottom = 64;
	rSource.top = 32;
	BackGroundSurface->Blt(&rDest,Corner, &rSource,NULL,NULL);


	rDest.left = 0;
	rDest.right = BorderWidth;
	rDest.top = BorderWidth;
	rDest.bottom = rDest.top + BorderWidth;

	while(rDest.bottom < Height - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
		rDest.top += BorderWidth;
		rDest.bottom += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

	rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

	BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

	rDest.left = Width - BorderWidth;
	rDest.right = Width;
	rDest.top = BorderWidth;
	rDest.bottom = rDest.top + BorderWidth;

	while(rDest.bottom < Height - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,VBorder,NULL,NULL,NULL);
		rDest.top += BorderWidth;
		rDest.bottom += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32;
	rSource.bottom = 32 + ((Height - BorderWidth) - rDest.bottom);

	rDest.bottom += ((Height - BorderWidth) - rDest.bottom);

	BackGroundSurface->Blt(&rDest,VBorder,&rSource,NULL,NULL);

	rDest.left = BorderWidth;
	rDest.right = rDest.left + BorderWidth;
	rDest.top = 0;
	rDest.bottom = BorderWidth;

	while(rDest.right < Width - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
		rDest.right += BorderWidth;
		rDest.left += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
	rSource.bottom = 32;

	rDest.right += ((Width - BorderWidth) - rDest.right);
	BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);

	rDest.left = BorderWidth;
	rDest.right = rDest.left + BorderWidth;
	rDest.top = Height - BorderWidth;
	rDest.bottom = Height;

	while(rDest.right < Width - BorderWidth)
	{
		BackGroundSurface->Blt(&rDest,HBorder,NULL,NULL,NULL);
		rDest.right += BorderWidth;
		rDest.left += BorderWidth;
	}
	rSource.top = 0;
	rSource.left = 0;
	rSource.right = 32 + ((Width - BorderWidth) - rDest.right);
	rSource.bottom = 32;

	rDest.right += ((Width - BorderWidth) - rDest.right);
	BackGroundSurface->Blt(&rDest,HBorder,&rSource,NULL,NULL);

//	Corner->Release();
//	VBorder->Release();
//	HBorder->Release();
}


void ZSWindow::SetBackGround(LPDIRECTDRAWSURFACE7 NewBack)
{ 
	if(BackGroundSurface)
	{
		BackGroundSurface->Release();
		BackGroundSurface = NULL;
	}
	BackGroundSurface = NewBack; 
	if(BackGroundSurface)
	{
		BackGroundSurface->AddRef();
	}
}

LPDIRECTDRAWSURFACE7 ZSWindow::CreatePortrait(const char *PortraitName, BOOL GetFace)
{
	LPDIRECTDRAWSURFACE7 Parchment;
	if(!lpddsParchment)
	{
		lpddsParchment = Engine->Graphics()->CreateSurfaceFromFile("Parchment.bmp", 256, 256,NULL,NULL);
	}
	
	Parchment = lpddsParchment;

	RECT rDest;
	rDest.left = Border/2;
	rDest.top = Border/2;
	rDest.right = (Bounds.right - Bounds.left) - Border/2;
	rDest.bottom = (Bounds.bottom - Bounds.top) - Border/2;

	if(BackGroundSurface)
	{
		BackGroundSurface->Release();
	}

	BackGroundSurface = Engine->Graphics()->CreateSurface(Bounds.right - Bounds.left, Bounds.bottom - Bounds.top, NULL, D3DRGB(1.0f,0,1.0f));
	Engine->Graphics()->FillSurface(BackGroundSurface, D3DRGB(1.0f,0,1.0f), NULL);

	BackGroundSurface->Blt(&rDest,Parchment,NULL,NULL,NULL);

	char FileName[64];

	char Directory[256];
	
	strcpy(Directory,Engine->GetRootDirectory());
	strcat(Directory,"\\Portraits");
	SetCurrentDirectory(Directory);

	int Dim = 92;

	if((rDest.right - rDest.left) < 60)
	{
		sprintf(FileName,"%ssm.bmp",PortraitName);
		FILE *tempfp;
		tempfp = fopen(FileName,"rb");
		if(tempfp)
		{
			Dim = 48;
			fclose(tempfp);
		}
		else
		{
			sprintf(FileName,"%s.bmp",PortraitName);
		}
	}
	else
	if((rDest.right - rDest.left) > 100)
	{
		sprintf(FileName,"%slg.bmp",PortraitName);
		FILE *tempfp;
		tempfp = fopen(FileName,"rb");
		if(tempfp)
		{
			Dim = 140;
			fclose(tempfp);
		}
		else
		{
			sprintf(FileName,"%s.bmp",PortraitName);
		}
	}
	else
	{
		sprintf(FileName,"%s.bmp",PortraitName);
	}
	LPDIRECTDRAWSURFACE7 lpddPor = NULL;

	FILE *fp;
	fp = fopen(FileName, "rt");
	if(fp)
	{	
		fclose(fp);
		lpddPor = Engine->Graphics()->CreateSurfaceFromFile(FileName,Dim,Dim,NULL,D3DRGB(1.0f,0,1.0f));
		BackGroundSurface->Blt(&rDest, lpddPor, NULL, NULL, NULL);
		if(!GetFace)
		{
			lpddPor->Release();
			lpddPor = NULL;
		}
	}
	SetCurrentDirectory(Engine->GetRootDirectory());

	//Parchment->Release();

	return lpddPor;
}

ZSWindow *ZSWindow::GetBottomChild(int x, int y)
{
	ZSWindow *pWin;
	
	pWin = GetChild(x,y);

	if(pWin)
	{
		return pWin->GetBottomChild(x,y);
	}
	else
	{
		return this;
	}
}

BOOL ZSWindow::ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y)
{
	ZSWindow *pWin;
	pWin = GetChild(x,y);
	if(pWin)
	{
		return pWin->ReceiveItem(pToReceive, pWinFrom, x, y);
	}
	else
	{
		return FALSE;
	}
}

void ZSWindow::Init()
{


}

void ZSWindow::Shutdown()
{
	if(lpddsParchment)
	{
		lpddsParchment->Release();
		lpddsParchment = NULL;
	}
	if(lpddsWood)
	{
		lpddsWood->Release();
		lpddsWood = NULL;
	}

   for(int n = 0; n < 4; n++)
   {
	   if(lpddsVBorder[n])
	   {
		   lpddsVBorder[n]->Release();
		   lpddsVBorder[n] = NULL;
	   }
	   if(lpddsHBorder[n])
	   {
		   lpddsHBorder[n]->Release();
		   lpddsHBorder[n] = NULL;
	   }
	   if(lpddsCorner[n])
	   {
		   lpddsCorner[n]->Release();
		   lpddsCorner[n] = NULL;
	   }
   }   
}
