#include "minimap.h"
#include "world.h"
#include "translucentwindow.h"
#include "zsengine.h"
#include "zsutilities.h"
#include "party.h"
#include "zsbutton.h"
#include "zshelpwin.h"

#define MINIMAP_BORDER_WIDTH	10
#define MINIMAP_BORDER_HEIGHT	10

#define MINIMAP_HEIGHT	100
#define MINIMAP_WIDTH	100
#define MINIMAP_RADIUS	20.0f
#define MINIMAP_PIXEL_RADIUS 50.0f;

#define MINIMAP_BACKGROUND_VIEW_FACTOR	(1.0 / (20.0f * (512.0f / 1600.0f)))

#define MAP_COLOR D3DRGBA(0.7f, 0.7f, 0.7f, 0.6f)

int MiniMap::RightButtonUp(int x, int y)
{	
	ReleaseFocus();
	ShowHelp("Compass");
	return TRUE;
}



int MiniMap::Draw()
{
	if(Visible)
	{
		//draw our text in the upper left corner
		if(Text)
		{
			RECT DrawTo;
			DrawTo.left = Bounds.left + Border;
			DrawTo.right = Bounds.right - Border;
			DrawTo.top = Bounds.top + Border;
			DrawTo.bottom = Bounds.bottom - Border;
			Engine->Graphics()->GetFontEngine()->DrawText(Engine->Graphics()->GetBBuffer(), &DrawTo, Text, TextColor);
		}

		//draw our children
		if(pChild)
		{
			pChild->Draw();
		}	
	}

	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE;
}


//MoveMouse
//
int MiniMap::MoveMouse(long *x, long *y, long *z) 
{ 
	RECT ParentBounds;
	pParent->GetBounds(&ParentBounds);

	//if we're moving, move
	if(State == WINDOW_STATE_MOVING)
	{
		int Width;
		int Height;
		RECT NewBounds;
	

		Width = Bounds.right - Bounds.left;
		Height = Bounds.bottom - Bounds.top;

		NewBounds.left = *x - XMoveOffset;
		NewBounds.right = NewBounds.left + Width;
		NewBounds.top = *y - YMoveOffset;
		NewBounds.bottom = NewBounds.top + Height;

		//make sure we haven't left our parent's boundaries
		if(NewBounds.left < ParentBounds.left)
		{
			NewBounds.left = ParentBounds.left;
			NewBounds.right = NewBounds.left + Width;
		}
		else
		if(NewBounds.right > ParentBounds.right)
		{
			NewBounds.right = ParentBounds.right;
			NewBounds.left = NewBounds.right - Width;
		}
		
		if(NewBounds.top < ParentBounds.top)
		{
			NewBounds.top = ParentBounds.top;
			NewBounds.bottom = NewBounds.top + Height;
		}
		else
		if(NewBounds.bottom > ParentBounds.bottom)
		{
			NewBounds.bottom = ParentBounds.bottom;
			NewBounds.top = NewBounds.bottom - Height;
		}
		Move(NewBounds);
		pMapObject->AdjustCamera();
	}

	if(pParent)
	{
		//clamp the mouse position to between our boundaries
		if(*x < ParentBounds.left)
		{
			*x = ParentBounds.left;
		}
		else
		if(*x > ParentBounds.right)
		{
			*x = ParentBounds.right;
		}

		if(*y < ParentBounds.top)
		{
			*y = ParentBounds.top;
		}
		else
		if(*y > ParentBounds.bottom)
		{
			*y = ParentBounds.bottom;
		}
	}
	//done
	return TRUE; 
} // MoveMouse

int MiniMap::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		GetChild(IDC_COMPASS_UNLOCK)->Hide();
		if(Text)
			delete[] Text;
		Text = NULL;
		((MiniMapObject *)pMapObject)->Unlock();
	}

	return TRUE;
}


MiniMap::MiniMap(ZSWindow *NewParent)
{
	State = WINDOW_STATE_NORMAL;
	ID = MINIMAP_ID;
	FILE *fp;
	fp = SafeFileOpen("gui.ini","rt");

	fpos_t fptStart;

	SeekTo(fp,"[MINIMAP]");

	fgetpos(fp,&fptStart);

	SeekTo (fp,"BACKGROUND");

	char *BFName;

	BFName = GetStringNoWhite(fp);

	int BFWidth;
	int BFHeight;

	BFWidth = GetInt(fp);
	BFHeight = GetInt(fp);

	DWORD Mask = 0;

//	BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(BFName,BFWidth,BFHeight,0,Mask);//Engine->Graphics()->GetMask());
	
	NewParent->GetBounds(&Bounds);

	Bounds.top = 16;
	Bounds.bottom = 116;
	Bounds.left = 16;
	Bounds.right = 116;
	
	Visible = TRUE;
	
	NewParent->AddChild(this);

	MiniMapObject *pMMOb;

	pMMOb = new MiniMapObject();

//	pMMOb->BackGroundSurface = Engine->Graphics()->CreateSurfaceFromFile(BFName,BFWidth,BFHeight,0,Mask);//Engine->Graphics()->GetMask());
//
//	pMMOb->DrawSurface = this->BackGroundSurface;
	ZSButton *pButton;
	pButton = new ZSButton(BUTTON_CHECK, IDC_COMPASS_UNLOCK, Bounds.left, Bounds.top, 16, 16);
	pButton->Hide();
	AddChild(pButton);
	Border = 16;
	
	pMMOb->pMap = (ZSWindow *)this;
	
	pMMOb->AdjustCamera();
	
	PreludeWorld->AddMainObject((Object *)pMMOb);

	pMapObject = (Object *)pMMOb;

	delete[] BFName;
	fclose(fp);

}

void MiniMapObject::Draw()
{
	if(!pMap->IsVisible())
	{
		return;
	}

//	Engine->Graphics()->SetTexture(pTexture);

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

	
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	
	if(!pMesh)
	{
		
		pTexture = Engine->GetTexture("compass");
		Engine->Graphics()->SetTexture(pTexture);
		pMesh = Engine->GetMesh("compass");
		pPointerMesh = Engine->GetMesh("compasspointer");
	}
	
	pMesh->Draw(Engine->Graphics()->GetD3D(), Position.x, Position.y, Position.z, 0, Scale, Scale, Scale, 0);
	if(Locked)
	{
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_GREEN));
		pPointerMesh->Draw(Engine->Graphics()->GetD3D(), Position.x, Position.y, Position.z, LockedAngle, Scale, Scale, Scale, 0);
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	}

//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);

	return;
}

void MiniMapObject::AdjustCamera()
{
	CalculatePosition();
	Scale = 1.0f/PreludeWorld->GetScaling();
}
 
MiniMapObject::MiniMapObject()
{
	pMesh = NULL;
	pPointerMesh = NULL;
	pTexture = NULL;
	Scale = 1.0f;
	Locked = FALSE;
}

MiniMapObject::~MiniMapObject()
{
}

BOOL MiniMapObject::AdvanceFrame()
{
	
	return TRUE;
}

void MiniMapObject::CalculatePosition()
{
	int ScreenX;
	int ScreenY;
	RECT rBounds;
	D3DVECTOR vMouseNear;
	
	pMap->GetBounds(&rBounds);

	ScreenX = (rBounds.left + rBounds.right) / 2;
	ScreenY = (rBounds.top + rBounds.bottom) / 2;
	
	//convert mouse to camera space
	PreludeWorld->ConvertToWorld(ScreenX, ScreenY, &vMouseNear);

	SetPosition(vMouseNear.x, vMouseNear.y, vMouseNear.z);

	if(Locked)
	{
		D3DVECTOR vCurLookAt;
		vCurLookAt = *PreludeParty.GetLeader()->GetPosition();
		
		float CameraAngle;
		
		CameraAngle = PreludeWorld->GetCameraAngle();
		
		D3DVECTOR vLock;
		vLock.x = -(xTo - vCurLookAt.x);
		vLock.y = yTo - vCurLookAt.y;
		vLock.z = 0.0f;

		vLock = Normalize(vLock);

		D3DVECTOR vNorth = _D3DVECTOR(0.0f,-1.0f,0.0f);
		
		LockedAngle = GetAngle(&vNorth, &vLock);

	}

}

void MiniMapObject::Lock(float newxto, float newyto)
{
	Locked = TRUE;
	xTo = newxto;
	yTo = newyto;
	AdjustCamera();
}

void SetLock(char *Tag, float xTo, float yTo)
{
	MiniMap *pMap;
	pMap = (MiniMap *)ZSWindow::GetMain()->GetChild(MINIMAP_ID);
	MiniMapObject *pMMO;
	pMMO = (MiniMapObject *)pMap->pMapObject;

	if(Tag)
		pMap->SetText(Tag);
	else
	{
		pMMO->Unlock();
		pMap->GetChild(IDC_COMPASS_UNLOCK)->Hide();
		pMap->SetText(" ");
		return;
	}
	pMap->GetChild(IDC_COMPASS_UNLOCK)->Show();


	pMMO->Lock(xTo, yTo);

}


