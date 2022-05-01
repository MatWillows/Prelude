#ifndef ZSSCRIPTWINDOW_H
#define ZSSCRIPTWINDOW_H

#include "zsWindow.h"

class ZSScriptWin : public ZSWindow
{
private:
	int NumExecutedScripts;

	void AddExecutedScript();
	void ExecuteGameCommand();

public:
	int Draw();
	void Show();
	void Hide();
	void Clear();

	int Command(int IDFrom, int Command, int Param);

	int LeftButtonDown(int x, int y);
	int LeftButtonUp(int x, int y);
	
	int GoModal();
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	ZSScriptWin(int NewID);
	
};

#endif