#ifndef TOOLWINDOW_H
#define TOOLWINDOW_H

#include "ZSwindow.h"
#include "forest.h"

typedef enum
{
	SELECT_COMMAND_HEIGHT_UP,
	SELECT_COMMAND_HEIGHT_UP_LARGE,
	SELECT_COMMAND_HEIGHT_DOWN,
	SELECT_COMMAND_HEIGHT_DOWN_LARGE,
	SELECT_COMMAND_TILE_HEIGHT_UP,
	SELECT_COMMAND_TILE_HEIGHT_UP_LARGE,
	SELECT_COMMAND_TILE_HEIGHT_DOWN,
	SELECT_COMMAND_TILE_HEIGHT_DOWN_LARGE,
	SELECT_COMMAND_FIRE,
	SELECT_COMMAND_FOUNTAIN,
	SELECT_COMMAND_FLATTEN,
	SELECT_COMMAND_CLEAR,
	SELECT_COMMAND_DONE,
	SELECT_COMMAND_PAINT_WALLS,
	SELECT_COMMAND_CLEAR_WALLS,
	SELECT_COMMAND_FLATTEN_WALLS,
	SELECT_COMMAND_UNBLOCK,
	SELECT_COMMAND_BLOCK,
} SELECT_SPECIAL_COMMANDS;




class ToolWin : public ZSWindow
{
private:
	RECT rSelected;
	Forest *pForest;
	int CurForest;

public:

	int Command(int IDFrom, int Command, int Param);
		
	int Draw();

	ToolWin(int NewID, ZSWindow *NewParent);

	Forest *GetForest() { if(pForest) return &pForest[CurForest]; else return NULL; }
	
};

class SelectToolWin: public ZSWindow
{
private:

public:
	int Command(int IDFrom, int Command, int Param);

	SelectToolWin(int NewID);
};

#endif