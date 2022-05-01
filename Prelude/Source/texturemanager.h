#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H
#include "defs.h"
#include "ZSEngine.h"

class TextureGroup
{
private:
	ZSTexture *pUsedList;
	ZSTexture *pUnusedList;
	int NumMembers;
	int Width;
	int Height;
public:
	int GetNumMembers() { return NumMembers; }
	int GetWidth() { return Width; }
	int GetHeight() { return Height; }
	void SetNumMembers(int num);
	void SetWidth(int w) { Width = w; }
	void SetHeight(int h) { Height = h; }

	TextureGroup(int w = 0, int h = 0 )
	{
		pUsedList = NULL;
		pUnusedList = NULL;
		NumMembers = 0;
		Width = w;
		Height = h;
	}

	void AddTexture();
	ZSTexture *GetTexture();
	void FreeTexture(ZSTexture *pTexture);

	~TextureGroup();
};

#endif