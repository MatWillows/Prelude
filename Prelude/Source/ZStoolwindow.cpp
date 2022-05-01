#include "ZStoolwindow.h"
#include "worldedit.h"
#include "ZSbutton.h"
#include "zsengine.h"
#include "zsListbox.h"
#include "zstext.h"

#define TOOL_BUTTON_HEIGHT	16

typedef enum
{
	IDC_SELECT_HEIGHT_UP,
	IDC_SELECT_HEIGHT_UP_LARGE,
	IDC_SELECT_HEIGHT_DOWN,
	IDC_SELECT_HEIGHT_DOWN_LARGE,
	IDC_SELECT_TILE_HEIGHT_UP,
	IDC_SELECT_TILE_HEIGHT_UP_LARGE,
	IDC_SELECT_TILE_HEIGHT_DOWN,
	IDC_SELECT_TILE_HEIGHT_DOWN_LARGE,
	IDC_SELECT_FIRE,
	IDC_SELECT_FOUNTAIN,
	IDC_SELECT_FLATTEN,
	IDC_SELECT_CLEAR,
	IDC_SELECT_DONE,
	IDC_SELECT_CLEAR_WALLS,
	IDC_SELECT_FLATTEN_WALLS,
	IDC_SELECT_PAINT_WALLS,
	IDC_SELECT_BLOCK,
	IDC_SELECT_UNBLOCK,
} SELECT_CONTROLS;

int ToolWin::Command(int IDFrom, int Command, int Param)
{
	float BrushWidth;
	ZSWindow *pWin;
		
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		pWin = GetParent();

		if(IDFrom <= EDIT_MODE_FOREST || IDFrom == EDIT_MODE_TILE_HEIGHTS)
		{
			((WorldEditWin *)pWin)->SetEditMode((EDIT_MODE_T)IDFrom);
					
			pWin = GetChild(IDFrom);

			pWin->GetBounds(&rSelected);
		}
		else
		{
			switch(IDFrom)
			{
				case EDIT_BRUSH_LARGER:
					BrushWidth = ((WorldEditWin *)pWin)->GetBrushWidth();
					BrushWidth += 1.0f;
					((WorldEditWin *)pWin)->SetBrushWidth(BrushWidth);
					break;
				case EDIT_BRUSH_SMALLER:
					BrushWidth = ((WorldEditWin *)pWin)->GetBrushWidth();
					BrushWidth -= 1.0f;
					((WorldEditWin *)pWin)->SetBrushWidth(BrushWidth);
					break;
				case EDIT_BRUSH_SPRAY:
					if(((WorldEditWin *)pWin)->GetFillType() == EDIT_FILL_SPRAY)
					{
						int FillRate = ((WorldEditWin *)pWin)->GetFillRate();
						FillRate += 10;
						if(FillRate > 100)
						{
							FillRate = 10;
						}
						((WorldEditWin *)pWin)->SetFillRate(FillRate);
						pWin = GetChild(IDFrom);
						char Blarg[64];
						sprintf(Blarg,"%i%%",FillRate);
						pWin->SetText(Blarg);
					}
					else
					{
						((WorldEditWin *)pWin)->SetFillType(EDIT_FILL_SPRAY);
					}
					break;
				case EDIT_BRUSH_RANDOM:
					((WorldEditWin *)pWin)->SetFillType(EDIT_FILL_RANDOM);
					break;
				case EDIT_BRUSH_NORMAL:
					((WorldEditWin *)pWin)->SetFillType(EDIT_FILL_NORMAL);
					break;
			
			}
		}
	}
	else
	if(IDFrom == EDIT_MODE_FOREST)
	{
		pWin = GetChild(IDFrom);
		pWin->GetBounds(&rSelected);
		CurForest = ((ZSList *)pWin)->GetSelection();
		pWin = GetParent();
		((WorldEditWin *)pWin)->SetEditMode((EDIT_MODE_T)IDFrom);
		((WorldEditWin *)pWin)->SetFillType(EDIT_FILL_SPRAY);
	}

	return TRUE;
}
		
int ToolWin::Draw()
{
if(Visible)
	{
		if(State == WINDOW_STATE_NORMAL)
		{
			if(pChild)
			{
				pChild->Draw();
			}
			Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(), &rSelected, COLOR_LASSO);
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

ToolWin::ToolWin(int NewID, ZSWindow *NewParent)
{
	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;

	ID = NewID;
	NewParent->AddChild(this);

	RECT rParent;
	NewParent->GetBounds(&rParent);
	
	Bounds.left = rParent.right - 128;
	Bounds.right = rParent.right;
	Bounds.top = rParent.top + 224;
	Bounds.bottom = Bounds.top + 256;
	
	ZSButton *pButton;
	
	int yn = 224;

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_TILE, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Paint Terrain");
	yn += TOOL_BUTTON_HEIGHT + 1;
	pButton->GetBounds(&rSelected);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_PLACE_OBJECT, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Objects");
	yn += TOOL_BUTTON_HEIGHT + 1;
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_PLACE_ITEMS, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Items");
	yn += TOOL_BUTTON_HEIGHT + 1;
	AddChild(pButton);
	pButton->Show();
/*
	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_PLACE_CREATURES, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Creatures");
	yn += TOOL_BUTTON_HEIGHT + 1;
	AddChild(pButton);
	pButton->Show();
*/
	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_PLACE_EVENTS, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Events");
	yn += TOOL_BUTTON_HEIGHT + 1;
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_TILE_HEIGHTS, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Height Edit");
	yn += TOOL_BUTTON_HEIGHT + 1;
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_HEIGHT, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Height Change");
	yn += TOOL_BUTTON_HEIGHT + 1;
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_MODE_SMOOTH, Bounds.left, yn,  128, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Smooth Heights");
	AddChild(pButton);
	pButton->Show();
	yn += TOOL_BUTTON_HEIGHT + 1;
	
	pButton = new ZSButton(BUTTON_NORMAL, EDIT_BRUSH_LARGER, Bounds.left, yn,  64, TOOL_BUTTON_HEIGHT );
	pButton->SetText("+Brush");
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_BRUSH_SMALLER, Bounds.left + 64, yn,  64, TOOL_BUTTON_HEIGHT );
	pButton->SetText("-Brush");
	AddChild(pButton);
	pButton->Show();
	yn += TOOL_BUTTON_HEIGHT + 1;

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_BRUSH_NORMAL, Bounds.left, yn,  42, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Norm");
	AddChild(pButton);
	pButton->Show();
	

	pButton = new ZSButton(BUTTON_NORMAL, EDIT_BRUSH_RANDOM, Bounds.left + 43, yn,  42, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Rand");
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_NORMAL, EDIT_BRUSH_SPRAY, Bounds.left + 86, yn,  42, TOOL_BUTTON_HEIGHT );
	pButton->SetText("Norm");
	AddChild(pButton);
	pButton->Show();

	pButton->SetText("%%10");

	yn += TOOL_BUTTON_HEIGHT + 1;

	ZSList *pList;
	pList = new ZSList(EDIT_MODE_FOREST, Bounds.left, yn, 128, TOOL_BUTTON_HEIGHT * 4);
	pList->Show();
	AddChild(pList);

	int NumForest;
	int n;

	pForest = Forest::LoadAll(&NumForest);

	for(n = 0; n < NumForest; n ++)
	{
		pForest[n].Name[10] = '\0';
		pList->AddItem(pForest[n].Name);
	}
	pList->SetSelection(0);
	
}

SelectToolWin::SelectToolWin(int NewID)
{
	ID = NewID;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	Moveable = TRUE;
	Bounds.left = 0;
	Bounds.right = 128;
	Bounds.top = 0;
	Bounds.bottom = 256;

	this->CreateWoodBorderedBackground(8);

	//first row = heights
	ZSText *pT;
	ZSButton *pB;
	pT = new ZSText(-1,8,8,"Height:");
	pT->Show();
	AddChild(pT);

	pB = new ZSButton(BUTTON_SOUTH, IDC_SELECT_HEIGHT_DOWN_LARGE, 8,24,16,16);
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_SOUTH, IDC_SELECT_HEIGHT_DOWN, 24,24,16,16);
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORTH, IDC_SELECT_HEIGHT_UP, 40,24,16,16);
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORTH, IDC_SELECT_HEIGHT_UP_LARGE, 56,24,16,16);
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORMAL, IDC_SELECT_FLATTEN, 80,24,48,16);
	pB->SetText("flat");
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORMAL, IDC_SELECT_CLEAR_WALLS, 8, 48,100,16);
	pB->SetText("clear walls");
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORMAL, IDC_SELECT_PAINT_WALLS, 8, 72,100,16);
	pB->SetText("paint walls");
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORMAL, IDC_SELECT_FLATTEN_WALLS, 8, 96, 100,16);
	pB->SetText("flatten walls");
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORMAL, IDC_SELECT_BLOCK, 8, 120, 64,16);
	pB->SetText("block");
	pB->Show();
	AddChild(pB);

	pB = new ZSButton(BUTTON_NORMAL, IDC_SELECT_UNBLOCK, 72, 120, 64,16);
	pB->SetText("unblock");
	pB->Show();
	AddChild(pB);



	pB = new ZSButton(BUTTON_NORMAL, IDC_SELECT_DONE, 0, 236, 64, 20);
	pB->SetText("Done");
	pB->Show();
	AddChild(pB);
}

int SelectToolWin::Command(int IDFrom, int Command, int Param)
{
	switch(Command)
	{
	case COMMAND_BUTTON_CLICKED:
		switch(IDFrom)
		{
		case IDC_SELECT_HEIGHT_DOWN_LARGE:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_HEIGHT_DOWN_LARGE);
			break;
		case IDC_SELECT_HEIGHT_DOWN:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_HEIGHT_DOWN);
			break;
		case IDC_SELECT_HEIGHT_UP:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_HEIGHT_UP);
			break;
		case IDC_SELECT_HEIGHT_UP_LARGE:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_HEIGHT_UP_LARGE);
			break;
		case IDC_SELECT_FLATTEN:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_FLATTEN);
			break;
		case IDC_SELECT_DONE:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_DONE);
			break;
		case IDC_SELECT_CLEAR_WALLS:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_CLEAR_WALLS);
			break;
		case IDC_SELECT_PAINT_WALLS:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_PAINT_WALLS);
			break;
		case IDC_SELECT_FLATTEN_WALLS:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_FLATTEN_WALLS);
			break;
		case IDC_SELECT_UNBLOCK:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_UNBLOCK);
			break;
		case IDC_SELECT_BLOCK:
			pParent->Command(this->ID, COMMAND_SPECIAL, SELECT_COMMAND_BLOCK);
			break;


		}
		break;
		default:
			break;
	}
	return TRUE;
}


