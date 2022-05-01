#ifndef MINIMAP_H
#define MINIMAP_H

#include "ZSwindow.h"
#include "objects.h"

class ZSModelEx;
#define MINIMAP_ID	32123
#define IDC_COMPASS_UNLOCK 1


class MiniMap : public ZSWindow
{
	private:
		
	public:

		int Draw();

		Object *pMapObject;

		int MoveMouse(long *x, long *y, long *z);

		int RightButtonUp(int x, int y);
		
		int Command(int IDFrom, int Command, int Param);

		MiniMap(ZSWindow *NewParent);

};


class MiniMapObject : public Object
{
private:
	ZSModelEx *pPointerMesh;
	ZSWindow *pMap;
	float Scale;
	
	float LockedAngle;
	float xTo;
	float yTo;

	BOOL Locked;

public:

	OBJECT_T GetObjectType() { return OBJECT_MINIMAP; }

	void CalculatePosition();

	void Draw();

	void AdjustCamera();

	void Lock(float newxTo, float newyTo);

	void Unlock() 
	{ Locked = FALSE; }

	BOOL AdvanceFrame();

	MiniMapObject();

	~MiniMapObject();

	friend class MiniMap;
};

void SetLock(char *Tag, float xTo, float yTo);

#endif