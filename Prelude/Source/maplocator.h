#ifndef MAPLOCATOR_H
#define MAPLOCATOR_H

#include "defs.h"

class MapLocator
{
private:
	char Tag[32];
	int MapNum;
	int MapX;
	int MapY;
	int X;
	int Y;
	MapLocator *pNext;
	BOOL Teleport;

public:
	int GetMapNum() { return MapNum; }
	void SetMapNum(int NewNum) { MapNum = NewNum; }
	void SetX(int NewX) { X = NewX; };
	void SetY(int NewY) { Y = NewY; };
	void SetMapX(int NewX) { MapX = NewX; };
	void SetMapY(int NewY) { MapY = NewY; };


	void SetNext(MapLocator *pNewNext) { pNext = pNewNext; }
	void SetTeleport(BOOL NewTele) { Teleport = NewTele; }
	
	char *GetTag() { return Tag; };
	int GetX() { return X; };
	int GetY() { return Y; };
	int GetMapX() { return MapX; };
	int GetMapY() { return MapY; };

	MapLocator *GetNext() { return pNext; }
	BOOL GetTeleport() { return Teleport; }

	void Save(FILE *fp);
	void Load(FILE *fp);

	MapLocator()
	{
		Tag[0] = '\0';
		X = 0;
		Y = 0;
		MapNum = 0;
		MapX = 0;
		MapY = 0;
		pNext = NULL;
		Teleport = FALSE;
	}

	MapLocator(char *NewTag, int NewX, int NewY, BOOL NewTeleport = FALSE)
	{
		strcpy(Tag,NewTag);
		X = NewX;
		Y = NewY;
		pNext = NULL;
		Teleport = NewTeleport;
	}

	~MapLocator()
	{
		delete pNext;
	}

};


#endif