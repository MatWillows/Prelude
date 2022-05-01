#include "StartScreen.h"
#include "zsbutton.h"
#include "zsengine.h"
#include "zsutilities.h"

#define IDC_START_SCREEN	10101
#define START_FRAME_RATE	66
typedef enum
{
	IDC_START_NEW = 500,
	IDC_START_LOAD,
	IDC_START_OPTIONS,
	IDC_START_QUIT,
} START_SCREEN_CONTROLS;

int PreludeStartScreen::Draw()
{
	if(Visible)
	{
		//draw our background if any
		if(BackGroundSurface)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,NULL,NULL);
		}

		if(pChild)
		{
			pChild->Draw();
		}
		
	}


	return TRUE; 

}

int PreludeStartScreen::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
		case IDC_START_NEW:
			ReturnCode = START_RESULT_NEW;		
			State = WINDOW_STATE_DONE;
			break;
		case IDC_START_LOAD:
			ReturnCode = START_RESULT_LOAD;		
			State = WINDOW_STATE_DONE;
			break;
		case IDC_START_OPTIONS:
			ReturnCode = START_RESULT_OPTIONS;		
			State = WINDOW_STATE_DONE;
			break;
		case IDC_START_QUIT:
		default:
			ReturnCode = START_RESULT_QUIT;		
			State = WINDOW_STATE_DONE;
			break;
		}	
	}


	return TRUE;
}

PreludeStartScreen::PreludeStartScreen()
{
	ID = IDC_START_SCREEN;
	Cursor = CURSOR_POINT;
	BackGroundSurface = NULL;
	pParent = NULL;
	pChild = NULL;
	pSibling = NULL;
	Text = NULL;
	State = WINDOW_STATE_NORMAL;
	Border = 0;
	FILE *fp;
	fp = SafeFileOpen ("startscreen.bmp","rb");

	if(fp)
	{
		fclose(fp);
		
		BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile("StartScreen.bmp",800,600,NULL,NULL);
	}

	RECT rNew;
	RECT rLoad;
	RECT rOptions;
	RECT rQuit;

	fp = SafeFileOpen("GUI.ini","rt");

	SeekTo(fp,"[STARTSCREEN]");

	SeekTo(fp,"NEW");
	
	rNew.left = GetInt(fp);
	rNew.top = GetInt(fp);
	rNew.right = rNew.left + GetInt(fp);
	rNew.bottom = rNew.top + GetInt(fp);
	
	SeekTo(fp,"LOAD");

	rLoad.left = GetInt(fp);
	rLoad.top = GetInt(fp);
	rLoad.right = rLoad.left + GetInt(fp);
	rLoad.bottom = rLoad.top + GetInt(fp);
	
	SeekTo(fp,"OPTIONS");

	rOptions.left = GetInt(fp);
	rOptions.top = GetInt(fp);
	rOptions.right = rOptions.left + GetInt(fp);
	rOptions.bottom = rOptions.top + GetInt(fp);
	

	SeekTo(fp,"QUIT");

	rQuit.left = GetInt(fp);
	rQuit.top = GetInt(fp);
	rQuit.right = rQuit.left + GetInt(fp);
	rQuit.bottom = rQuit.top + GetInt(fp);
	
	fclose(fp);

	//insert rectangle scaling code here
	Bounds.left = 0;
	Bounds.top = 0;
	Bounds.bottom = Engine->Graphics()->GetHeight();
	Bounds.right = Engine->Graphics()->GetWidth();

	RECT rBase;
	rBase.left = 0;
	rBase.top = 0;
	rBase.bottom = 600;
	rBase.right = 800;

	ScaleRect(&rNew,     &rBase,	&Bounds);
	ScaleRect(&rLoad,    &rBase,	&Bounds);
	ScaleRect(&rOptions, &rBase,	&Bounds);
	ScaleRect(&rQuit,    &rBase,	&Bounds);

	ZSButton *pButton;
	pButton = new ZSButton("new",IDC_START_NEW, XYWH(rNew),200,60,6);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("load",IDC_START_LOAD, XYWH(rLoad),200,60,5);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("options",IDC_START_OPTIONS, XYWH(rOptions),200,60,5);
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton("quit",IDC_START_QUIT, XYWH(rQuit),200,60,5);
	pButton->Show();
	AddChild(pButton);
	
}

PreludeStartScreen::~PreludeStartScreen()
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
}

