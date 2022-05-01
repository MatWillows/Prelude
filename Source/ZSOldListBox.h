#ifndef ZSOldListBOX_H
#define ZSOldListBOX_H

#include "zswindow.h"
#include "Zslistbox.h"

class ZSOldList : public ZSWindow
{
private:
	int NumItems;
	
	int NumVisible;
	
	int NumSelectors;

	ListItem *pCurTop;

	int SelectID;

	BOOL Parchment;

	int NormalColor;
	int SelectColor;
	int DisableColor;

public:

	void SetBorderWidth(int NewWidth);
//	int MoveMouse(long *x, long *y, long *z);

	int Draw();

	int GetSelection() { return SelectID; }
	void SetSelection(int NewSelection);

	void DisableItem(int ID);
	void EnableItem(int ID);
		
	void AddItem(char *Text);
	char *GetText(int Number, char *Dest);

	void RemoveItem(int Number);
	int FindItem(char *Text);

	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);
	
	void Clear();
	void ClearExcept(char *ExceptText);
	ZSOldList(int NewID, int x, int y, int Width, int Height, BOOL Parchement = FALSE);
	~ZSOldList();

	int GetNumItems() { return NumItems; }

};

#endif
