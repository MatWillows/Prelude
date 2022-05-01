#ifndef PEOPLEEDIT_H
#define PEOPLEEDIT_H

#include "locator.h"
#include "ZSwindow.h"

class Object;

typedef enum
{
	PEDIT_MODE_NONE,
	PEDIT_MODE_PLACE_PERSON,
	PEDIT_SELECT_PERSON,
	PEDIT_ADD_LOCATOR,
} PEDIT_MODE_T;

class Thing;
class Event;

class LocatorEditWin : public ZSWindow
{
private:
	Locator *pLocator;

public:

//	int Draw(); //should be handled fine with base window draw

	int Command(int IDFrom, int Command, int Param);

	Locator *GetLocator() { return pLocator; }
	void SetLocator(Locator *pNewLocator);

	void SetLocatorState();
	void SetLocatorDirection();
	void SetLocatorArea();

	LocatorEditWin(int NewID, int x, int y, int width, int height);
	
};


class PeopleEditWin : public ZSWindow
{
private:
	PEDIT_MODE_T	EditMode;
	BOOL ShowTiles;
	
	float MouseX;
	float MouseY;
	float MouseZ;
	
	D3DVECTOR vLocatorStart;
	D3DVECTOR vLocatorEnd;

	Object *pCurPerson;
	
	int NumLocators;
	
	LocatorEditWin *LocationEditors[8];

	void AddLocator();
	void RemoveLocator(int Num);
	void DrawLocators();

public:
	void SetupLocationEditors();

	int Draw();
	
	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	
	int RightButtonDown(int x, int y);
	int RightButtonUp(int x, int y);
	
	int MoveMouse(long *x, long *y, long *z);
	
	int Command(int IDFrom, int Command, int Param);
	
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	void Show();

	PeopleEditWin();

	void GetPerson();

};

#endif
