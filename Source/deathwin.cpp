#include "DeathWin.H"
#include "zsbutton.h"
#include "zsengine.h"
#include "zstext.h"


int DeathWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		State = WINDOW_STATE_DONE;
		ZSWindow::GetMain()->SetState(WINDOW_STATE_DONE);
	}
	return TRUE;
}

DeathWin::DeathWin()
{
	ID = -1;
	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;
	Moveable = FALSE;

	Bounds.left = 0;
	Bounds.top = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.bottom = Engine->Graphics()->GetHeight();

	ZSButton *pButton;

	pButton = new ZSButton("closebutton",IDC_DEATH_CLOSE,377,3,57,34,57,34,1);
	pButton->Show();
	AddChild(pButton);

	ZSText *pText;
	pText = new ZSText(IDC_DEATH_TEXT, 250,100,256,256, "    ", 0, FALSE);
	AddChild(pText);
	pText->Show();

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("death.bmp",800,600,NULL,RGB(255,255,255));
}

int DeathWin::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{
	State = WINDOW_STATE_DONE;
	ZSWindow::GetMain()->SetState(WINDOW_STATE_DONE);
	return TRUE;
}
