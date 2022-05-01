#ifndef ZSTALK_H
#define ZSTALK_H

#include "zswindow.h"
#include "script.h"
#include "translucentwindow.h"

#define IDC_PLAYER_PORTRAIT	100
#define IDC_NPC_PORTRAIT		200
#define IDC_SAY					300
#define IDC_REPLY					400

class Thing;

class ZSTalkWin : public ZSWindow
{
private:
	ScriptBlock *PrevContextBlock;
	ZSWindow *PrevContextWindow;
	TransWin *pTrans;


public:
	static LPDIRECTDRAWSURFACE7 TalkSurface;

	void SetPortrait(const char *PortraitName);
	int LeftButtonDown(int x, int y);

	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);

	void AddWord(char *ToAdd);
	void RemoveWord(char *ToRemove);
	
	int RemoveAll();

	void Say(char *ToSay);
	void SayAdd(char *ToAdd);
	void SayClear();
	void SayDesc(char *ToAdd);

	int GoModal();

	ZSTalkWin(int NewID, int x, int y, int Width, int Height, ScriptBlock *SBCharacter);
	~ZSTalkWin();

};

void Talk(Thing *pWho);

#endif