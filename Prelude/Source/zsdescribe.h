#ifndef ZSDESCRIBE_H
#define ZSDESCRIBE_H

#include "zswindow.h"
#include "translucentwindow.h"

#define MAX_DESCRIBE_ITEMS		64




class DescribeItem
{
public:
	char *Text;
	int TextColor;

	DescribeItem *pNext;
	DescribeItem *pPrev;

	DescribeItem(char *NewText, int NewID);
	~DescribeItem();
};

class ZSDescribe : public ZSWindow
{
private:
	int NumItems;
	
	int NumVisible;
	
	int NumSelectors;

	DescribeItem *pCurTop;
	TransWin	 *pTrans;

	BOOL Parchment;

public:

	int Draw();

	void AddItem(char *Text);
	void AddItem(char *Text, int NewTextColor);

	int Command(int IDFrom, int Command, int Param);
	
	void Clear();
	void ResetToTop();
	
	void SetTextColor(int NewColor);


	ZSDescribe(int NewID, int x, int y, int Width, int Height, BOOL IsParchment = FALSE);
	~ZSDescribe();

};

#endif
