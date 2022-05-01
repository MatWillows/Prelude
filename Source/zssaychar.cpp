#include "zssaychar.h"
#include "zsengine.h"
#include "zsdescribe.h"
#include "mainwindow.h"

#define IDC_SAYCHAR_PORTRAIT 5
#define IDC_SAYCHAR_SAY		  10

#define SAYCHAR_TEXT_WIDTH	256

int ZSSayChar::RightButtonDown(int x, int y)
{
	SetFocus(this);
	RButtonDown = TRUE;
	return TRUE;
}

int ZSSayChar::RightButtonUp(int x, int y)
{
	if(timeGetTime() - StartTime >= 333)
		State = WINDOW_STATE_DONE;
	ReleaseFocus();
	RButtonDown = FALSE;
	return TRUE;
}

int ZSSayChar::LeftButtonDown(int x, int y)
{
	SetFocus(this);
	LButtonDown = TRUE;
	return TRUE;
}

int ZSSayChar::LeftButtonUp(int x, int y)
{
	if(timeGetTime() - StartTime >= 333)
		State = WINDOW_STATE_DONE;
	LButtonDown = FALSE;
	ReleaseFocus();
	return TRUE;
}


int ZSSayChar::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(!LButtonDown && !RButtonDown)
		State = WINDOW_STATE_DONE;
	//loop while we're not in a done state
	return TRUE;
}

int ZSSayChar::GoModal()
{
	State = WINDOW_STATE_NORMAL;
	//loop while we're not in a done state
	while(State != WINDOW_STATE_DONE && ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		//ensure that all windows are drawn by call the main window's draw routine
		pMainWindow->Draw();
		Engine->Graphics()->Flip();
		
		//get any user input
		Engine->Input()->Update(pInputFocus);
		if(timeGetTime() > EndTime && !LButtonDown && !RButtonDown)
		{
			State = WINDOW_STATE_DONE;
		}
	}

	((ZSMainWindow *)GetMain())->SetSpeaker(NULL);
	return ReturnCode;
}

ZSSayChar::ZSSayChar(int NewID, int x, int y, int Width, int Height, char *SayText, Thing *Sayer)
{
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	RButtonDown = FALSE;
	LButtonDown = FALSE;
	Bounds.left = x;
	Bounds.right = x + SAYCHAR_TEXT_WIDTH;
	Bounds.top = y;
	Bounds.bottom = y + 100;

	Cursor = CURSOR_POINT;
	
	BackGroundSurface = NULL;

	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	Border = 0;

	int NumLetters = strlen(SayText);

	int NumSeconds = 5; 

	NumSeconds = (NumLetters / 7) + 1;

	StartTime = timeGetTime();
	EndTime = StartTime + (NumSeconds * 1000);

	ZSWindow *pWin;

	int SayStart;
	int SayWidth;
	SayWidth = SAYCHAR_TEXT_WIDTH;

	if(Sayer)
	{
		SayStart = x + 100;
		Bounds.right += 100;
		
		pWin = new ZSWindow(WINDOW_PORTRAIT, IDC_SAYCHAR_PORTRAIT, Bounds.left, Bounds.top, 100, 100);
		pWin->SetBorderWidth(8);
		pWin->CreatePortrait(Sayer->GetData(INDEX_PORTRAIT).String);
		pWin->SetText(Sayer->GetData(INDEX_NAME).String);
		pWin->AddBorder(8,1);
		pWin->Show();
		AddChild(pWin);
	}
	else
	{
		SayStart = x;
	}
	
	Width = (Bounds.right - SayStart) - 24;

	int TextHeight;
	
	TextHeight = Engine->Graphics()->GetFontEngine()->CountLines(SayText, Width) + 1;
	TextHeight *= (Engine->Graphics()->GetFontEngine()->GetTextHeight() - 1);

	TextHeight += 16;

	Bounds.bottom = Bounds.top + TextHeight;

	
	pWin = new ZSDescribe(IDC_SAYCHAR_SAY, SayStart, y, SAYCHAR_TEXT_WIDTH, TextHeight + 16, TRUE);
	pWin->Show();
	AddChild(pWin);
	((ZSDescribe *)pWin)->AddItem(SayText);

	((ZSMainWindow *)pWin->GetMain())->SetSpeaker((Object *)Sayer);

	if(Bounds.left < 0 || Bounds.top < 0 || Bounds.right >= 800 || Bounds.bottom >= 600)
	{
		RECT rNewBounds;

		int Width;
		int Height;
		Width = Bounds.right - Bounds.left;
		Height = Bounds.bottom - Bounds.top;

		rNewBounds.left = (800 - Width) /2;
		rNewBounds.right = rNewBounds.left + Width;
		rNewBounds.top = (500 - Height) /2;
		rNewBounds.bottom = rNewBounds.top + Height;

		this->Move(rNewBounds);
	}


	
}