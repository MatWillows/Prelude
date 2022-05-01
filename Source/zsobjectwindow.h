#ifndef OBJECTWINDOW_H
#define OBJECTWINDOW_H

#include "ZSwindow.h"

class ObjectWin : public ZSWindow
{

private:
	int CurMesh;

public:
	int Draw();

	int GetMesh() { return CurMesh; }
	

	int Command(int IDFrom, int Command, int Param);

	ObjectWin(int NewId, ZSWindow *NewParent);

};

#endif