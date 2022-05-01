#ifndef ZSREST_H
#define ZSREST_H

#include "zswindow.h"
#include "script.h"

class ZSRest : public ZSWindow
{
private:
	int NumOptions;
	ScriptBlock RestOptions[8];
	char OptionNames[8][32];
	ScriptBlock RestResults;
	ScriptBlock RestEvent;
	void LoadRestOptions();
	void LoadRestOptionsText();
	char *TownName;

public:
	static LPDIRECTDRAWSURFACE7 RestSurface;

	int Command(int IDFrom, int Command, int Param);

	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	ZSRest(int newid, int x, int y, int width, int height);

	~ZSRest();

	void Show();

};




#endif