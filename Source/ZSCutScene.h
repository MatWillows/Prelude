#ifndef ZSCUTSCENE_H
#define ZSCUTSCENE_H

#include "ZSWindow.h"

class CSAction;
class CSObject;
class SoundEffect;

class ZSCutScene : public ZSWindow
{
private:

	int NumActions;
	int NumObjects;
	DWORD SceneStart;
	DWORD SceneEnd;
	DWORD LastUpdate;

	CSObject **pObjects;
	CSAction *pActions;
		
	void LoadObjects(FILE *fp);
	void LoadActions(FILE *fp);

public:

	void Load(FILE *fp);

	int Draw();
	
	int LeftButtonUp(int x, int y);
	int RightButtonUp(int x, int y);
	int Command(int IDFrom, int Command, int Param);
	int HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys);

	int GoModal();

	CSObject *FindObject(char *Named);

	ZSCutScene();

	~ZSCutScene();

};

class CSAction 
{
public:
	CSObject *pTarget;
	DWORD Start;
	DWORD End;
	DWORD LastUpdate;
	DWORD NextUpdate;
	DWORD Rate;

	CSAction *pNext;

	virtual void Load(FILE *fp);
	virtual void Update(DWORD NewTime)  { return; };
	
	friend CSAction *LoadAction(FILE *fp, ZSCutScene *pScene);
	CSAction() 
	{
		Start = 0;
		End = 0;
		LastUpdate = 0;
		NextUpdate = 0;
		Rate = 0;
		pNext = NULL;
	}

};

class CSActionText : public CSAction
{
public:
	int AtX;
	int AtY;
	char *Text;

	void Load(FILE *fp);
	void Update(DWORD NewTime);

	~CSActionText() { delete[] Text; }
};

class CSAction2DZoom : public CSAction
{	
public:
	float StartScale;
	float EndScale;
	void Load(FILE *fp);
	void Update(DWORD NewTime);
};

class CSAction2DMove : public CSAction
{	
public:
	int XFrom;
	int YFrom;
	int XTo;
	int YTo;
	void Load(FILE *fp);
	void Update(DWORD NewTime);
};

class CSAction2DShow : public CSAction
{
public:
	int XAt;
	int YAt;
	float Scale;
	BOOL Started;
	void Update(DWORD NewTime);
	void Load(FILE *fp);
	
	CSAction2DShow() { Started = FALSE; }
};

class CSAction2DFadeIn : public CSAction
{
public:
	void Update(DWORD NewTime);
};

class CSAction2DFadeOut : public CSAction
{
public:
	void Update(DWORD NewTime);
};

class CSAction2DAnimate : public CSAction
{
public:
	BOOL Started;
	void Update(DWORD NewTime);
	CSAction2DAnimate() { Started = FALSE; }
};

class CSActionSoundPlay : public CSAction
{
public:
	BOOL Started;
	void Update(DWORD NewTime);
	CSActionSoundPlay() { Started = FALSE; }
};

class CSActionSoundFadeUp : public CSAction
{
public:
	void Update(DWORD NewTime);
};

class CSActionSoundFadeDown : public CSAction
{
public:
	void Update(DWORD NewTime);

};

class CSActionSoundStop : public CSAction
{
public:
	void Update(DWORD NewTime);
};

//animobject
class CSObject
{
public:
	char Name[16];

	virtual void Load(FILE *fp) {  return; }
	virtual void Render() { return; }

	friend CSObject *LoadCSObject(FILE *fp);
};

CSObject *LoadCSObject(FILE *fp);

class CSSoundObject : public CSObject
{
public:
//	SoundEffect *pSound;
	void Load(FILE *fp);

	~CSSoundObject();
};

class CS2DObject : public CSObject 
{
public:
	D3DTLVERTEX Verts[4];
	ZSTexture *pTexture;	
	int NumFrames;
	int CurFrame;
	int BaseWidth;
	int BaseHeight;
	D3DCOLOR AlphaFactor;
	int Width;
	int Height;
	BOOL Draw;

	void Load(FILE *fp);
	void Render();

	CS2DObject() { Draw = FALSE; NumFrames = 0; pTexture = NULL; CurFrame = 0; AlphaFactor = 1.0f; };

	~CS2DObject();
};

CSAction *LoadAction(FILE *fp, ZSCutScene *pScene);


#endif