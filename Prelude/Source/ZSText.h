#ifndef ZSTEXT_H
#define ZSTEXT_H

#include "ZSwindow.h"

//WINDOW_TEXT,
class ZSText : public ZSWindow
{
private:
	int Duration;
	int CurFrame;
public:
	int Draw();
	ZSText(int NewID, int x, int y, char *string);
	ZSText(int NewID, int x, int y, char *string, int Duration);
	ZSText(int NewID, int x, int y, int width, int height, char *string, int NewDuration, BOOL Parchment = FALSE);
};

#endif