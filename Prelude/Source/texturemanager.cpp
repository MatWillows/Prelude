#include "texturemanager.h"

void TextureGroup::AddTexture()
{
	ZSTexture *pTexture;
	pTexture = new ZSTexture(Engine->Graphics()->GetD3D(), Width, Height);
	if(pUnusedList)
	{
		pTexture->SetNext(pUnusedList);
		pUnusedList->SetPrev(pTexture);
	}
	pUnusedList = pTexture;
	NumMembers++;
}

ZSTexture *TextureGroup::GetTexture()
{
	ZSTexture *pTexture;
	if(!pUnusedList)
	{
		AddTexture();
	}

	pTexture = pUnusedList;
	
	pUnusedList = pTexture->GetNext();

	if(pUnusedList)
		pUnusedList->SetPrev(NULL);
	
	pTexture->SetNext(pUsedList);
	pTexture->SetPrev(NULL);
	if(pUsedList)
	{
		pUsedList->SetPrev(pTexture);
	}
	pUsedList = pTexture;
	return pTexture;
}

void TextureGroup::FreeTexture(ZSTexture *pFreeTexture)
{
	if(!pFreeTexture) return;
	if(pUsedList == pFreeTexture)
	{
		pUsedList = pFreeTexture->GetNext();
	}

	if(pFreeTexture->GetPrev())
		pFreeTexture->GetPrev()->SetNext(pFreeTexture->GetNext());
	if(pFreeTexture->GetNext())
		pFreeTexture->GetNext()->SetPrev(pFreeTexture->GetPrev());

	pFreeTexture->SetNext(pUnusedList);
	pFreeTexture->SetPrev(NULL);
	if(pUnusedList) pUnusedList->SetPrev(pFreeTexture);

	pUnusedList = pFreeTexture;

}

TextureGroup::~TextureGroup()
{
	ZSTexture *pTexture, *pDel;
	pTexture = pUsedList;

	while(pTexture)
	{
		pDel = pTexture;
		pTexture = pTexture->GetNext();
		delete pDel;
	}
	pUsedList = NULL;

	pTexture = pUnusedList;

	while(pTexture)
	{
		pDel = pTexture;
		pTexture = pTexture->GetNext();
		delete pDel;
	}

	pUnusedList = NULL;

}
