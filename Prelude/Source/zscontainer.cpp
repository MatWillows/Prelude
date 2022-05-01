#include "zscontainer.h"
#include "inventorywin.h"
#include "Zsbutton.h"
#include "zshelpwin.h"
#include "zsEngine.h"
#include "zsutilities.h"
#include "party.h"


typedef enum
{
	IDC_CONTAINER_INVENTORY,
	IDC_PLAYER_INVENTORY,
	IDC_CONTAINER_CLOSE,

} CONTAINER_CONTROLS;

int ZSContainerWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		State = WINDOW_STATE_DONE;
	}
	return TRUE;	
}

int ZSContainerWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ZSHelpWin *pHelp;
		pHelp = new ZSHelpWin(-1,NULL);
		pHelp->Show();
		AddTopChild(pHelp);
		SetFocus(pHelp);
		pHelp->GoModal();
		ReleaseFocus();
		RemoveChild(pHelp);
		return TRUE;
	}
	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
	}


	return TRUE;
}

ZSContainerWin::ZSContainerWin(int NewID, int x, int y, int width, int height, Object *pFrom)
{
	ID = NewID;
	State = WINDOW_STATE_NORMAL;
	Type = WINDOW_DIALOG;
	Moveable = FALSE;
	Visible = FALSE;

	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = y + height;

	FILE  *fp;
	fp  = SafeFileOpen ("gui.ini","rt");
	
	RECT rBounds;
	ZSButton *pButton;
	InventoryWin *pInventory;

	SeekTo(fp,"BARTER");

	SeekTo(fp,"QUIT");
	LoadRect(&rBounds,fp);
	pButton = new ZSButton("closebutton", IDC_CONTAINER_CLOSE, XYWH(rBounds),57,34,1);
	pButton->Show();
	AddChild(pButton);

	SeekTo(fp,"IPLAYER");
	LoadRect(&rBounds,fp);
	pInventory = new InventoryWin(IDC_PLAYER_INVENTORY, XYWH(rBounds), PreludeParty.GetLeader());
	pInventory->Show();
	AddChild(pInventory);

	SeekTo(fp,"IMERCHANT");
	LoadRect(&rBounds,fp);
	pInventory = new InventoryWin(IDC_CONTAINER_INVENTORY, XYWH(rBounds), pFrom);
	pInventory->Show();
	AddChild(pInventory);

	fclose(fp);






}
