#ifndef SHRINKWINDOW_H
#define SHRINKWINDOW_H

#include "zswindow.h"
//shrink windows are groups of tools that shrink and enlarge

class ZSShrinkWindow : public ZSWindow
{
protected:
	BOOL Shrunk;
	RECT rEnlarged;

public:
	
	void Shrink();
	void Enlarge();

	int Draw();

	virtual int LeftButtonDown(int x, int y);
	virtual int LeftButtonUp(int x, int y);

};

#endif