#ifndef ZSOPTIONS_H
#define ZSOPTIONS_H

#include "zswindow.h"

#define OPTIONS_ID	654654

class ZSOptionWin : public ZSWindow
{

private:

	void SaveSettings();
	void LoadSettings();
	
public:
	static LPDIRECTDRAWSURFACE7 OptionsSurface;

	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	void Show();
	void Hide();

	ZSOptionWin(int NewId);

};

class ZSMainOptionsMenu : public ZSWindow
{

private:
		
public:
	static LPDIRECTDRAWSURFACE7 MainOptionsSurface;

	int Command(int IDFrom, int Command, int Param);

	ZSMainOptionsMenu(int NewID);

};

#endif