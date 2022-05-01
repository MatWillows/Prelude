//things to do:
//		show party drachs
//		player, merchant, bargainer display (names/portraits)
//		finish transaction

#include "PICKPOCKET.h"
#include "zstext.h"
#include "inventorywin.h"
#include "zsitemslot.h"
#include "zsbutton.h"
#include "creatures.h"
#include "zsutilities.h"
#include "party.h"
#include "script.h"

typedef enum
{
	IDC_PICKPOCKET_PLAYER_INVENTORY,
	IDC_PICKPOCKET_MERCHANT_INVENTORY,
	
	IDC_PICKPOCKET_QUIT,
	
} PICKPOCKET_CONTROLS;


int PickPocketWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(IDFrom == IDC_PICKPOCKET_QUIT)
		{
			State = WINDOW_STATE_DONE;
		}
	}
	if(Command == COMMAND_ITEMSLOT_CHANGING)
	{
		//the param is the ID of the source ItemSlot's parent window
		if(Param == IDC_PICKPOCKET_PLAYER_INVENTORY)
		{

		}
		else
		if(Param == IDC_PICKPOCKET_MERCHANT_INVENTORY)
		{
		
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}


PickPocketWin::PickPocketWin(int NewID, int x, int y, int width, int height, Creature *pNewPlayer, Creature *pNewTarget)
{
	ID = NewID;
	Type = WINDOW_PICKPOCKET;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = y + height;

	FILE *fp;
	RECT rBounds;
	char *FileName;
	int Width;
	int Height;

	fp = SafeFileOpen("gui.ini","rt");

	SeekTo(fp,"[PICKPOCKET]");

	SeekTo(fp,"BACKGROUND");
	
	FileName = GetStringNoWhite(fp);
	Width = GetInt(fp);
	Height = GetInt(fp);

	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(FileName,Width,Height,NULL,0);
	delete FileName;

//Pickpocketteer bargainer should be person in party with best skill??


//create the sub windows
	ZSButton *pButton;
	InventoryWin *pInventory;

	SeekTo(fp,"QUIT");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton(BUTTON_NONE, IDC_PICKPOCKET_QUIT, XYWH(rBounds));
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"IPLAYER");
	LoadRect(&rBounds,fp);
	pInventory = new InventoryWin(IDC_PICKPOCKET_PLAYER_INVENTORY, XYWH(rBounds), pTarget);
	pInventory->Show();
	AddChild(pInventory);

	SeekTo(fp,"IMERCHANT");
	LoadRect(&rBounds,fp);
	pInventory = new InventoryWin(IDC_PICKPOCKET_MERCHANT_INVENTORY, XYWH(rBounds), pNewPlayer);
	pInventory->Show();
	AddChild(pInventory);
	
	fclose(fp);
}

