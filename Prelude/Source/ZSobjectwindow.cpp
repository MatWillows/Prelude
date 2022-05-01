#include "ZSobjectwindow.h"
#include "ZSbutton.h"
#include "ZStext.h"
#include "WorldEdit.H"
#include "zsengine.h"

#define OBJECT_WIN_HEIGHT	128
#define OBJECT_WIN_WIDTH	128

#define IDC_NEXT_MESH		1
#define IDC_PREV_MESH		2
#define IDC_NEXT_TEN_MESH	3
#define IDC_PREV_TEN_MESH	4
#define IDC_MESH_NAME		5


int ObjectWin::Draw()
{
	if(Visible)
	{
		if(State == WINDOW_STATE_NORMAL)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
			if(pChild)
			{
				pChild->Draw();
			}
		}
		else
		{
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

	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE;
}

int ObjectWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
			case IDC_PREV_MESH:
				CurMesh--;
				break;
			case IDC_NEXT_MESH:
				CurMesh++;
				break;
			case IDC_NEXT_TEN_MESH:
				CurMesh += 10;
				break;
			case IDC_PREV_TEN_MESH:
				CurMesh -= 10;
				break;
			default:
				break;
		}
		if(CurMesh < 0)
		{
			CurMesh = 0;
		}
		if(CurMesh >= Engine->GetNumMesh())
		{
			CurMesh = Engine->GetNumMesh() - 1;
		}
		
		ZSWindow *pWin;
		pWin = GetChild(IDC_MESH_NAME);
		pWin->SetText(Engine->GetMesh(CurMesh)->GetName());

		((WorldEditWin *)pParent)->SetCurMesh(CurMesh);

	}
	return TRUE;
}


ObjectWin::ObjectWin(int NewId, ZSWindow *NewParent)
{
	BackGroundSurface = Engine->Graphics()->CreateSurface(128,128,NULL, Engine->Graphics()->GetMask());
	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;
	ID = NewId;

	NewParent->AddChild(this);

	RECT rParent;

	NewParent->GetBounds(&rParent);
	
	Bounds.left = rParent.right - OBJECT_WIN_WIDTH;
	Bounds.right = rParent.right;

	Bounds.top = rParent.top;
	Bounds.bottom = rParent.top + OBJECT_WIN_HEIGHT;

	ZSWindow *pButton;
	
	pButton = new ZSButton(BUTTON_NORTH, IDC_PREV_MESH, Bounds.right - 16, Bounds.top + 16, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORTH, IDC_PREV_TEN_MESH, Bounds.right - 16, Bounds.top, 16, 16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_SOUTH, IDC_NEXT_MESH, Bounds.right - 16, Bounds.bottom - 32, 16, 16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_SOUTH, IDC_NEXT_TEN_MESH, Bounds.right - 16, Bounds.bottom - 16, 16, 16);
	AddChild(pButton);
	pButton->Show();
	

	pButton = new ZSText(IDC_MESH_NAME, Bounds.left, Bounds.top, "blargblargblargblargblarg");
	AddTopChild(pButton);
	pButton->Show();
	
	CurMesh = 0;
}