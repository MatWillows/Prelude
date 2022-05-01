#include "zscutscene.h"
#include "zsutilities.h"
#include "zsengine.h"

void CSAction::Load(FILE *fp)
{
	//get the start
	//get the end
	//get the rate
	Start = GetInt(fp);
	End = GetInt(fp);
	Rate = GetInt(fp);
	pNext = NULL;
}

void CSAction2DMove::Load(FILE *fp)
{
	XFrom = GetInt(fp);
	YFrom = GetInt(fp);
	XTo = GetInt(fp);
	YTo = GetInt(fp);
	
	Start = GetInt(fp);
	End = GetInt(fp);
	Rate = GetInt(fp);
}

void CSAction2DMove::Update(DWORD NewTime)
{
	if(NewTime < Start || NewTime > End)
	{
		return;
	}

	int XAt;
	int YAt;
	
	XAt = XFrom + (int)((float)(XTo - XFrom)  *  (float)(NewTime - Start) / (float)(End-Start));
	YAt = YFrom + (int)((float)(YTo - YFrom)  *  (float)(NewTime - Start) / (float)(End-Start));

	CS2DObject *pCS2D;
	pCS2D = (CS2DObject *)pTarget;
	pCS2D->Verts[0].dvSX = XAt;
	pCS2D->Verts[0].dvSY = YAt;

	pCS2D->Verts[1].dvSX = XAt + pCS2D->Width;
	pCS2D->Verts[1].dvSY = YAt;

	pCS2D->Verts[2].dvSX = XAt;
	pCS2D->Verts[2].dvSY = YAt + pCS2D->Height;

	pCS2D->Verts[3].dvSX = XAt + pCS2D->Width;
	pCS2D->Verts[3].dvSY = YAt + pCS2D->Height;
}

void CSActionText::Load(FILE *fp)
{
	AtX = GetInt(fp);
	AtY = GetInt(fp);
	SeekTo(fp, "[");
	Text = GetString(fp, ']');
	Start = GetInt(fp);
	End = GetInt(fp);
	Rate = GetInt(fp);
}

void CSActionText::Update(DWORD NewTime)
{
	if(NewTime > Start && NewTime < End)
	{
		Engine->Graphics()->DrawText(AtX, AtY, Text);
		//drawText
	}
}

void CSAction2DZoom::Load(FILE *fp)
{
	int PercentScale;
	PercentScale = GetInt(fp);
	StartScale = (float)PercentScale / 100.0f;
	PercentScale = GetInt(fp);
	EndScale = (float)PercentScale / 100.0f;
	
	Start = GetInt(fp);
	End = GetInt(fp);
	Rate = GetInt(fp);
}

void CSAction2DZoom::Update(DWORD NewTime)
{
	if(NewTime > Start && NewTime < End)
	{
		CS2DObject *pCS2D;
		pCS2D = (CS2DObject *)pTarget;
		
		int NewWidth;
		int NewHeight;
		float ScaleFactor;
		ScaleFactor = StartScale + ((EndScale - StartScale) *  (float)(NewTime - Start) / (float)(End-Start));

		NewWidth = (int)((float)pCS2D->BaseWidth * ScaleFactor);
		NewHeight = (int)((float)pCS2D->BaseHeight * ScaleFactor);
		
		pCS2D->Width = NewWidth;
		pCS2D->Height = NewHeight;

		pCS2D->Verts[1].dvSX = pCS2D->Verts[0].dvSX + pCS2D->Width;
		pCS2D->Verts[2].dvSY = pCS2D->Verts[0].dvSY + pCS2D->Height;
		pCS2D->Verts[3].dvSX = pCS2D->Verts[0].dvSX + pCS2D->Width;
		pCS2D->Verts[3].dvSY = pCS2D->Verts[0].dvSY + pCS2D->Height;
	}
}

void CSAction2DShow::Load(FILE *fp)
{
	XAt = GetInt(fp);
	YAt = GetInt(fp);
	int PercentScale = GetInt(fp);
	
	Scale = (float)PercentScale / 100.0f;
	
	Start = GetInt(fp);
	End = GetInt(fp);
	Rate = GetInt(fp);
}

void CSAction2DShow::Update(DWORD NewTime)
{
	CS2DObject *pCS2D;
	pCS2D = (CS2DObject *)pTarget;
	if(NewTime < Start || NewTime > End)
	{
		pCS2D->Draw = FALSE;
	}
	else
	{
		if(!Started)
		{
			Started = TRUE;
			pCS2D->Draw = TRUE;
			
			pCS2D->Width = (int)((float)pCS2D->BaseWidth * Scale);
			pCS2D->Height = (int)((float)pCS2D->BaseHeight * Scale);
			pCS2D->Verts[0].dvSX = XAt;
			pCS2D->Verts[0].dvSY = YAt;

			pCS2D->Verts[1].dvSX = XAt + pCS2D->Width;
			pCS2D->Verts[1].dvSY = YAt;

			pCS2D->Verts[2].dvSX = XAt;
			pCS2D->Verts[2].dvSY = YAt + pCS2D->Height;

			pCS2D->Verts[3].dvSX = XAt + pCS2D->Width;
			pCS2D->Verts[3].dvSY = YAt + pCS2D->Height;				
		}
	}
}

void CSAction2DFadeIn::Update(DWORD NewTime)
{
	if(NewTime < Start || NewTime > End)
	{
		return;
	}

	float AlphaLevel;

	AlphaLevel = (float)(NewTime - Start) / (float)(End - Start);

	if(AlphaLevel < 0.0f) 
		AlphaLevel = 0.0f;

	if(AlphaLevel > 1.0f) 
		AlphaLevel = 1.0f;

	CS2DObject *pCS2D;
	pCS2D = (CS2DObject *)pTarget;
	pCS2D->AlphaFactor = D3DRGB(AlphaLevel,AlphaLevel,AlphaLevel);	
	pCS2D->Verts[0].color = pCS2D->AlphaFactor;
	pCS2D->Verts[1].color = pCS2D->AlphaFactor;
	pCS2D->Verts[2].color = pCS2D->AlphaFactor;
	pCS2D->Verts[3].color = pCS2D->AlphaFactor;
}


void CSAction2DFadeOut::Update(DWORD NewTime)
{
	if(NewTime < Start || NewTime > End)
	{
		return;
	}

	float AlphaLevel;

	AlphaLevel = 1.0f - (float)(NewTime - Start) / (float)(End - Start)  ;

	if(AlphaLevel < 0.0f) 
		AlphaLevel = 0.0f;

	if(AlphaLevel > 1.0f) 
		AlphaLevel = 1.0f;

	CS2DObject *pCS2D;
	pCS2D = (CS2DObject *)pTarget;
	pCS2D->AlphaFactor = D3DRGB(AlphaLevel,AlphaLevel,AlphaLevel);
	pCS2D->Verts[0].color = pCS2D->AlphaFactor;
	pCS2D->Verts[1].color = pCS2D->AlphaFactor;
	pCS2D->Verts[2].color = pCS2D->AlphaFactor;
	pCS2D->Verts[3].color = pCS2D->AlphaFactor;
}

void CSAction2DAnimate::Update(DWORD NewTime)
{
	CS2DObject *pCS2D;
	pCS2D = (CS2DObject *)pTarget;
	
	if(NewTime >= Start && NewTime <= End) 
	{
		if(!Started) 
		{
			Started = TRUE;
			pCS2D->CurFrame = 0;
			NextUpdate = NewTime + Rate;
			LastUpdate = NewTime;
		}

		if(NewTime > NextUpdate)
		{
			NextUpdate = NewTime + Rate;
			int AdvanceFrames;
			AdvanceFrames = (int)((float)(NewTime - LastUpdate) / (float)Rate);
			pCS2D->CurFrame += AdvanceFrames;
			while(pCS2D->CurFrame >= pCS2D->NumFrames)
			{
				pCS2D->CurFrame -= pCS2D->NumFrames;
			}
			LastUpdate = NewTime;
		}
	}

}

void CSActionSoundPlay::Update(DWORD NewTime)
{
	if(NewTime > Start && !Started)
	{
		CSSoundObject *pCSSound;
		pCSSound = (CSSoundObject *)pTarget;

		Engine->Sound()->StartMusic(pCSSound->Name);
		Started = TRUE;
	}
	
	if(NewTime > End && Started)
	{
		Engine->Sound()->StopMusic();
	}
}	

void CSActionSoundFadeUp::Update(DWORD NewTime)
{

}

void CSActionSoundFadeDown::Update(DWORD NewTime)
{

}

void CSActionSoundStop::Update(DWORD NewTime)
{
/*	if(NewTime > Start)
	{
		CSSoundObject *pCSSound;
		pCSSound = (CSSoundObject *)pTarget;

		pCSSound->pSound->Play();
	}
*/	
}

void CSSoundObject::Load(FILE *fp)
{
	GetString(fp,Name);
//	pSound = new SoundEffect;
//	pSound->Load(Engine->Sound()->GetDirectSound(),Name);
}

void CS2DObject::Load(FILE *fp)
{
	ZeroMemory(Verts,sizeof(D3DTLVERTEX)*4);
	
	char Blarg[64];
	GetString(fp, Name);
	NumFrames = GetInt(fp);
	
	int TexWidth;
	TexWidth = GetInt(fp);
	
	int TexHeight;
	TexHeight = GetInt(fp);

	RECT rPicture;
	rPicture.left = GetInt(fp);
	rPicture.top = GetInt(fp);
	rPicture.right = GetInt(fp);
	rPicture.bottom = GetInt(fp);

	float PicLeft;
	float PicTop;
	float PicRight;
	float PicBottom;
	PicLeft = (float)rPicture.left / (float)TexWidth;
	PicRight = (float)rPicture.right / (float)TexWidth;
	PicTop = (float)rPicture.top / (float)TexHeight;
	PicBottom = (float)rPicture.bottom / (float)TexHeight;

	CurFrame = 0;
	WORD Mask;
	Mask = 0;
	Draw = 0;

	Mask = Engine->Graphics()->GetMask();
	
	pTexture = new ZSTexture[NumFrames];

	int n;
	for(n = 0; n < NumFrames; n++)
	{
		sprintf(Blarg,"%s%i.bmp",Name,n);
		pTexture[n].Load(Blarg, Engine->Graphics()->GetD3D(),Engine->Graphics()->GetDirectDraw(), &Mask, TexWidth, TexHeight);
	}

	Verts[0].dvSX = 0.0f;
	Verts[0].dvSY = 0.0f;
	Verts[0].dvSZ = 0.0f;
	Verts[0].tu = PicLeft;
	Verts[0].tv = PicTop;
	Verts[0].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[0].rhw = 1.0f;
	Verts[0].specular = D3DRGB(1.0f,1.0f,1.0f);
	
	Verts[1].dvSX = 0.0f + Width;
	Verts[1].dvSY = 0.0f;
	Verts[1].dvSZ = 1.0f;
	Verts[1].tu = PicRight;
	Verts[1].tv = PicTop;
	Verts[1].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[1].rhw = 1.0f;
	Verts[1].specular = D3DRGB(1.0f,1.0f,1.0f);
		
	Verts[2].dvSX = 0.0f;
	Verts[2].dvSY = 0.0f + Height;
	Verts[2].dvSZ = 0.0f;
	Verts[2].tu = PicLeft;
	Verts[2].tv = PicBottom;
	Verts[2].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[2].rhw = 1.0f;
	Verts[2].specular = D3DRGB(1.0f,1.0f,1.0f);
	
	Verts[3].dvSX = 0.0f + Width;
	Verts[3].dvSY = 0.0f + Height;
	Verts[3].dvSZ = 0.0f;
	Verts[3].tu = PicRight;
	Verts[3].tv = PicBottom;
	Verts[3].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[3].rhw = 1.0f;
	Verts[3].specular = D3DRGB(1.0f,1.0f,1.0f);

	Width = BaseWidth = rPicture.right - rPicture.left;
	Height = BaseHeight = rPicture.bottom - rPicture.top;

}

void CS2DObject::Render()
{
	if(Draw)
	{
		Engine->Graphics()->SetTexture(&pTexture[CurFrame]);
		if(FAILED(Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, Verts, 4, NULL)))
		{
			SafeExit("problem in drawing cut-scene 2d object");
		}
	}
}

CSSoundObject::~CSSoundObject() 
{ 
//	pSound->Stop(); 
//	delete pSound; 
//	pSound = NULL;
}

CS2DObject::~CS2DObject()
{
	delete pTexture;
	pTexture = NULL;
}

CSObject *LoadCSObject(FILE *fp)
{
	char blarg[16];

	GetString(fp,blarg);

	if(!strcmp(blarg,"Picture"))
	{
		CS2DObject *pCS2D;
		pCS2D = new CS2DObject;
		pCS2D->Load(fp);
		return (CSObject *)pCS2D;
	}
	else
	if(!strcmp(blarg,"Sound"))
	{
		CSSoundObject *pCSSound;
		pCSSound = new CSSoundObject;
		pCSSound->Load(fp);
		return (CSObject *)pCSSound;
	}
	else
	{
		SafeExit("Error unspecified cut scene object format\n");
	}
	return NULL;
}

CSAction *LoadCSAction(FILE *fp, ZSCutScene* pScene)
{
	char Blarg[32];
	char TargetName[32];
	GetString(fp,Blarg);
	CSObject *pObject;
	

	if(!strcmp(Blarg,"Show"))
	{
		GetString(fp,TargetName);
		pObject = pScene->FindObject(TargetName);
		CSAction2DShow *pShow;
		pShow = new CSAction2DShow;
		pShow->pTarget = pObject;
		pShow->Load(fp);
		return (CSAction *)pShow;
	}
	else
	if(!strcmp(Blarg,"FadeIn"))
	{
		GetString(fp,TargetName);
		pObject = pScene->FindObject(TargetName);
		CSAction2DFadeIn *pFadeIn;
		pFadeIn = new CSAction2DFadeIn;
		pFadeIn->pTarget = pObject;
		pFadeIn->Load(fp);
		return (CSAction *)pFadeIn;

	}
	else
	if(!strcmp(Blarg,"FadeOut"))
	{
		GetString(fp,TargetName);
		pObject = pScene->FindObject(TargetName);
		CSAction2DFadeOut *pFadeOut;
		pFadeOut = new CSAction2DFadeOut;
		pFadeOut->pTarget = pObject;
		pFadeOut->Load(fp);
		return (CSAction *)pFadeOut;
	}
	else
	if(!strcmp(Blarg,"Move"))
	{
		GetString(fp,TargetName);
		pObject = pScene->FindObject(TargetName);
		CSAction2DMove *pMove;
		pMove = new CSAction2DMove;
		pMove->pTarget = pObject;
		pMove->Load(fp);
		return (CSAction *)pMove;
	}
	else
	if(!strcmp(Blarg,"Animate"))
	{
		GetString(fp,TargetName);
		pObject = pScene->FindObject(TargetName);
		CSAction2DAnimate *pAnimate;
		pAnimate = new CSAction2DAnimate;
		pAnimate->pTarget = pObject;
		pAnimate->Load(fp);
		pAnimate->NextUpdate = pAnimate->Start;
		return (CSAction *)pAnimate;
	}
	else
	if(!strcmp(Blarg,"Zoom"))
	{
		GetString(fp,TargetName);
		pObject = pScene->FindObject(TargetName);
		CSAction2DZoom *pZoom;
		pZoom = new CSAction2DZoom;
		pZoom->pTarget = pObject;
		pZoom->Load(fp);
		return (CSAction *)pZoom;
	}
	else
	if(!strcmp(Blarg,"Text"))
	{
		CSActionText *pText;
		pText = new CSActionText;
		pText->Load(fp);
		return (CSAction *)pText;
	}
	else
	if(!strcmp(Blarg,"Play"))
	{
		GetString(fp,TargetName);
		pObject = pScene->FindObject(TargetName);
		CSActionSoundPlay *pPlay;
		pPlay = new CSActionSoundPlay;
		pPlay->Load(fp);
		pPlay->pTarget = pObject;
		return (CSAction *)pPlay;
	}
	SafeExit("Failed to obtain valid cut scene action type.\n");
	return NULL;
}

		
void ZSCutScene::LoadObjects(FILE *fp)
{
	pObjects = new CSObject *[NumObjects];
	
	for(int n = 0; n < NumObjects; n++)
	{
		pObjects[n] = LoadCSObject(fp);
	}
}

void ZSCutScene::LoadActions(FILE *fp)
{
	int n;

	pActions = LoadCSAction(fp,this);
	CSAction *pAct;
	pAct = pActions;
	for(n = 1; n < NumActions; n++)
	{
		pAct->pNext = LoadCSAction(fp,this);
		pAct = pAct->pNext;
	}
}

void ZSCutScene::Load(FILE *fp)
{
	//load the objects
	SeekTo(fp,"NumObjects:");
	NumObjects = GetInt(fp);
	
	
	SeekTo(fp,"NumActions:");
	NumActions = GetInt(fp);
	
	SeekTo(fp,"Length:");
	SceneEnd = GetInt(fp);

	SeekTo(fp,"Objects:");
	LoadObjects(fp);

	SeekTo(fp,"Actions:");
	LoadActions(fp);

}

int CutFrame = 0;

int ZSCutScene::Draw()
{
	CutFrame++;	
	Engine->Graphics()->GetD3D()->BeginScene();
	int n;
	for(n = 0; n < NumObjects; n++)
	{
		pObjects[n]->Render();
	}
	Engine->Graphics()->GetD3D()->EndScene();
	Engine->Graphics()->Flip();
	Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);
	Engine->Graphics()->GetD3D()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0L );
	return TRUE;
}
	
int ZSCutScene::LeftButtonUp(int x, int y)
{
	ReleaseFocus();
	State = WINDOW_STATE_DONE;
	return TRUE;
}

int ZSCutScene::RightButtonUp(int x, int y)
{
	ReleaseFocus();
	State = WINDOW_STATE_DONE;
	return TRUE;
}

int ZSCutScene::Command(int IDFrom, int Command, int Param)
{
	return TRUE;
}

int ZSCutScene::HandleKeys(BYTE *CurrentKeys, BYTE* LastKeys)
{
	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
	}
	return TRUE;
}

int ZSCutScene::GoModal()
{
	DWORD CurTime;
	CSAction *pAction;
	//store our current renderstate
	
	//stop any music
	Engine->Sound()->StopMusic();


	//clear the back buffer and the zbuffer
	Engine->Graphics()->GetD3D()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		               0x00000000, 1.0f, 0L );

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);          
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);

	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_LINEAR);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_LINEAR); 

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCALPHA);

	if(FAILED(Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_ADD)))
	{
		SafeExit("Could not set texture stage state.\n");
	}

	SceneStart = timeGetTime();
	SceneEnd = SceneEnd + SceneStart;
	
	while(State != WINDOW_STATE_DONE && ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		Draw();
		CurTime = timeGetTime() - SceneStart;
		pAction = pActions;
		while(pAction)
		{
			pAction->Update(CurTime);
			pAction = pAction->pNext;
		}

		//get any user input
		Engine->Input()->Update(pInputFocus);

		LastUpdate = SceneStart + CurTime;
		if(LastUpdate > SceneEnd)
		{
			State = WINDOW_STATE_DONE;
		}
	}
	
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_LINEAR);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_LINEAR);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE); 
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	
	
	//stop any music
	Engine->Sound()->StopMusic();

	return TRUE;

}

CSObject *ZSCutScene::FindObject(char *Named)
{
	int n;
	for(n = 0; n < NumObjects; n++)
	{
		if(!strcmp(pObjects[n]->Name, Named))
		{ 
			return pObjects[n];
		}
	}
	DEBUG_INFO("Failed to find scene object!\n");
	DEBUG_INFO(Named);
	DEBUG_INFO("\n");
	SafeExit("failed to find scene object");
	return NULL;
}

ZSCutScene::ZSCutScene()
{
	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;
	Bounds.left = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.top = 0;
	Bounds.bottom = Engine->Graphics()->GetHeight();
}

ZSCutScene::~ZSCutScene()
{
	delete[] pActions;

	int n;
	for(n = 0; n < NumObjects; n++)
	{
		delete	pObjects[n];
	}
	
	delete[] pObjects;
}