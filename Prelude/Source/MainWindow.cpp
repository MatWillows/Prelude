//mainwindow.cpp
//the main game window provides routines for non-combat interaction with the game world

#include "mainwindow.h"
#include "ZStext.h"
#include <assert.h>
#include "creatures.h"
#include "actionmenuclass.h"
#include "zsactionwindow.h"
#include "ZSdescribe.h"
#include "equipeditorwin.h"
#include "world.h"
#include "WorldEdit.h"
#include "mmsystem.h"
#include "zsengine.h"
#include "minimap.h"
#include "zsscriptwindow.h"
#include "zsregionedit.h"
#include "mapwin.h"
#include "zshelpwin.h"
#include "zsOptions.h"
#include "party.h"
#include "defs.h"
#include "zstalk.h"
#include "peopleedit.h"
#include "zsbutton.h"
#include "journal.h"
#include "mapwin.h"
#include "ZSMainDescribe.h"
#include "ZSconfirm.h"
#include "ZSRest.h"
#include "ZSSaveLoad.h"
#include "characterwin.h"
#include "movepointer.h"
#include "zsmenubar.h"
#include "zsitemslot.h"
#include "equipwin.h"
#include "gameitem.h"
#include "inventorywin.h"
#include "zscaveedit.h"
#include "combatmanager.h"

#ifndef NDEBUG
#define VERSION_NUMBER			"v1.5"
#else
#define VERSION_NUMBER			"v1.5 release"
#endif

#define IDC_DESCRIBE			99887766
#define IDC_SCRIPT_WINDOW	666666

ZSMainWindow::ZSMainWindow()
{
	Type = WINDOW_MAIN;
	Cursor = CURSOR_POINT;
	this->pTarget = NULL;
	this->MouseX = 20.0f;
	this->MouseY = 20.0f;
	this->MouseZ = 0.0f;
	LastX = 0;
	LastY = 0;

	pEEWin = NULL;
	pWorldEdit = NULL;
	pRegionEdit = NULL;
	pScriptWin = NULL;
	pFPS = NULL;
	pDrawTime = NULL;
	pMenuBar = NULL;
	pDescribe = NULL;
	Frame = 0;

	ShowInv = TRUE;

	HighLightColor = COLOR_RED;
	ShowRange = FALSE;
	HighlightNonStatic = FALSE;

	SkipFrames = 0;

	DrawMouseSquare = TRUE;

	vRayStart = _D3DVECTOR(0.0f,0.0f,0.0f);
	vRayEnd = _D3DVECTOR(0.0f,0.0f,0.0f);

	Blue.r = 0.5f;
	Blue.g = 0.5f;
	Blue.b = 1.0f;
	Blue.a = 0.25f;

	Red.r = 1.0f;
	Red.g = 0.0f;
	Red.b = 0.0f;
	Red.a = 0.5f;

	Bounds.left = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.top = 0;
	Bounds.bottom = Engine->Graphics()->GetHeight();
	Pathing = FALSE;
	ZSWindow::pMainWindow = this;
	ZSWindow::pInputFocus = this;

	FrameRate = 33;
/*
	ZSMainDescribe *DescribeWin;
	DescribeWin = new ZSMainDescribe(IDC_DESCRIBE, 0, 0, 286, 100);
	AddChild(DescribeWin);


	RECT rNewBounds;
	rNewBounds.left = 0;	
	rNewBounds.right = 256;
	rNewBounds.top = 0;
	rNewBounds.bottom = 110;


	DescribeWin->SetBounds(&rNewBounds);
*/
	pMenuBar = new ZSMenuBar(IDC_MAIN_MENUBAR,0,0,800,100);
	pMenuBar->Show();
	AddChild(pMenuBar);

	pDescribe = (ZSDescribe *)pMenuBar->GetChild(1);
	


//	DescribeWin->Show();

	MiniMap *pMiniMapWin;
	pMiniMapWin = new MiniMap(this);
	pMiniMapWin->Show();
	DrawWorld = FALSE;
	Visible = FALSE;
//framerate display only for debug version

	pFPS = new ZSText(124500, 0, 256,"BLARGBLARGBLARG");
	AddChild(pFPS);
	pFPS->Hide();
	pFPS->SetText("             ");

	pDrawTime = new ZSText(124500, 0, 288,"BLARGBLARGBLARG");
	AddChild(pDrawTime);
	pDrawTime->Hide();
	pDrawTime->SetText("             ");

	pTargetString = new ZSText(124500, 0, 475,"wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww: hp:wwwww rp:wwwww wp: wwwww");
	AddChild(pTargetString);
	pTargetString->Hide();
	pTargetString->SetText(" ");
	pTargetString->SetTextColor(TEXT_WHITE);


	ScrollFactor = 0.60f;

	vOffset = ScrollVector = _D3DVECTOR(0.0f, 0.0f, 0.0f);

	Zooming = FALSE;
	Rotating = FALSE;
	Scrolling = FALSE;
	Arcing = FALSE;
	ZoomFactor = 0.0f;
	RotationFactor = 0.0f;
	Arcfactor = 0.0f;

	LastMoved = timeGetTime();
	GotTarget = FALSE;

	pSpeaker = NULL;
	LBDown = FALSE;
	ShowFrames = FALSE;

	iMouseX = 32;
	iMouseY = 32;

	pGoddessTextures[0] = Engine->GetTexture("goddess");
	pGoddessTextures[1] = Engine->GetTexture("goddess2");
	pGoddessTextures[2] = Engine->GetTexture("goddess3");
	pGoddessTextures[3] = Engine->GetTexture("goddess4");
}

void DrawIntersectRay()
{
/*	D3DLVERTEX Verts[3];
	Verts[0].x = vRayStart.x;
	Verts[0].y = vRayStart.y;
	Verts[0].z = vRayStart.z;
	Verts[0].color = Verts[0].specular = D3DRGB(1.0f,1.0f,1.0f);

	Verts[1].x = vRayEnd.x;
	Verts[1].y = vRayEnd.y;
	Verts[1].z = vRayEnd.z;
	Verts[1].color = Verts[1].specular = D3DRGB(1.0f,0.0f,0.0f);

	Verts[2].x = vRayStart.x + 0.3f;
	Verts[2].y = vRayStart.y + 0.3f;
	Verts[2].z = vRayStart.z + 0.3f;
	Verts[2].color = Verts[1].specular = D3DRGB(1.0f,1.0f,1.0f);

	HRESULT hr;
	
	Engine->Graphics()->ClearTexture();
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

	hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLELIST, D3DFVF_LVERTEX, Verts, 3, 0);

	if(hr != D3D_OK)
	{
		Engine->ReportError(hr);
	}
*/

}

void ZSMainWindow::GetTarget()
{
	if(Scrolling ||	Zooming || Rotating || Arcing)
	{
		GotTarget = TRUE;
		pTarget = NULL;
		return;
	}


	if((int)MouseX == LastX && (int)MouseY == LastY)
		return;

	LastX = (int)MouseX;
	LastY = (int)MouseY;
	
	//create the test ray
	vRayEnd = _D3DVECTOR(this->MouseX,this->MouseY,this->MouseZ);

	D3DVECTOR vMouseNear;

	//convert mouse to camera space
	vMouseNear.x = -(1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->left/(float)Engine->Graphics()->GetWidth());
	vMouseNear.y = (1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->top/(float)(Engine->Graphics()->GetHeight() - 100));
	vMouseNear.z = 0.0f;

	D3DMATRIX matView;
	D3DMATRIX matProj;
	D3DMATRIX matWorld;

	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
	D3DXVECTOR4 vNear;
	D3DXVECTOR4 vCameraLocation;
	
	D3DXMATRIX matInverseView;
	D3DXMATRIX matInverseProj;
	D3DXMATRIX matInverseWorld;
	D3DXMATRIX matTemp;
	D3DXMATRIX matFinal;
	float blarg;

	D3DXMatrixInverse(&matInverseView,&blarg,(D3DXMATRIX *)&matView);
	D3DXMatrixInverse(&matInverseProj,&blarg,(D3DXMATRIX *)&matProj);
	
	D3DXMatrixMultiply(&matFinal,&matInverseProj,&matInverseView);
	
	D3DXVec3Transform(&vNear,(D3DXVECTOR3 *)&vMouseNear,&matFinal);
	
	//now we have the start and end points of our line
	vRayStart = (D3DVECTOR)vNear;
	
	this->pTarget = Valley->GetTarget(&vRayStart,&vRayEnd);
	if(PreludeWorld->HighlightInfo()) SetTargetString();
		
	GotTarget = TRUE;

	if(this->pTarget)
	{
		Thing *pActive;
		pActive = (Thing *)PreludeWorld->GetActive();
		if(PreludeParty.IsMember((Creature *)pActive))
		{
			if(this->pTarget->GetObjectType() == OBJECT_CREATURE)
			{
				Creature *pCreature;
				pCreature = (Creature *)this->pTarget;
				
				if(pActive->GetData(INDEX_BATTLESIDE).Value != pCreature->GetData(INDEX_BATTLESIDE).Value)
				{
					HighLightColor = COLOR_RED;
				}
				else
				{
					HighLightColor = COLOR_GREEN;
				}

				switch(pCreature->GetDefaultAction(pActive))
				{
					case ACTION_ATTACK:
						//GOT_HERE("Attacking %s!",this->pTarget->GetData("NAME").String);
						//pActive->InsertAction(ACTION_ATTACK,(ULONG)this->pTarget,pActive->GetData("LASTATTACK").Value);
						Cursor = CURSOR_ATTACK;
						Engine->Graphics()->SetCursor(CURSOR_ATTACK);
						break;
					case ACTION_DEFEND:
						Cursor = CURSOR_DEFEND;
						Engine->Graphics()->SetCursor(CURSOR_DEFEND);
						break;
					case ACTION_TALK:
						Cursor = CURSOR_TALK;
						Engine->Graphics()->SetCursor(CURSOR_TALK);
						break;
					default:
						Cursor = CURSOR_POINT;
	  Engine->Graphics()->SetCursor(CURSOR_POINT);
					
						break;
				}
			}
			else
			{
				if(this->pTarget->GetDefaultAction(pActive))
				{
					Cursor = CURSOR_GRAB;
					Engine->Graphics()->SetCursor(CURSOR_GRAB);
				}
				HighLightColor = COLOR_BLUE;
			}
		}//active is not a party member
	}
	
}

//use default window destructor

int ZSMainWindow::Draw()
{
	if(DrawWorld && Visible)
	{	
		Engine->Graphics()->GetD3D()->BeginScene();
	
		Valley->Draw();

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

		if(DrawMouseSquare)
		{
			Engine->Graphics()->ClearTexture();
			Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
			Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
			if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
				Valley->CombatHighlightTile(this->MouseX,this->MouseY);
			else
				Valley->HighlightTile(this->MouseX,this->MouseY);
			/*
			Engine->Graphics()->ClearTexture();
			Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
			
			Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
			
			HRESULT hr;
			hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, Engine->Graphics()->GetMouseVerts(), 4, NULL);

			if(hr != DD_OK)
			{
				Engine->ReportError(hr);
			}
			*/
		}

		
		if(Pathing && PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
		{
			MainPath.Draw((Thing *)PreludeWorld->GetActive());
		}
		
		//Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_ADD);
	
		//Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_ADDSIGNED2X);
		//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR);
		//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_DESTCOLOR);
	
		//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
	
		//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
		//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);

		if(this->pTarget)
		{
			Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
			//Valley->CircleTile((int)this->MouseX, (int)this->MouseY, COLOR_BLUE);
			//this->pTarget->Circle(HighLightColor);
			
			this->pTarget->HighLight(HighLightColor);
			this->pTarget->AltHighLight();

			//Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);

			if(this->pTarget->GetObjectType() == OBJECT_CREATURE)
			{
				((Creature *)this->pTarget)->DrawData();
			}

		}
	
		
		//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		//Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		
		
		if(ShowRange)
			((Creature *)PreludeWorld->GetActive())->ShowRange();

		if(HighlightNonStatic)
			Valley->DrawAltHighlight();


		if(((Thing *)PreludeWorld->GetActive()))
			((Thing *)PreludeWorld->GetActive())->Arrow(Red);

		if(pSpeaker)
		{
			if(pSpeaker->GetObjectType() == OBJECT_CREATURE)
			((Thing *)pSpeaker)->WordBalloon();
		}

//		Valley->HighlightTile((int)this->MouseX,(int)this->MouseY);

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

		//DrawIntersectRay();
	
		Engine->Graphics()->GetD3D()->EndScene();
//		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);

	}
	else
	{
		Engine->Graphics()->FillSurface(Engine->Graphics()->GetBBuffer(),0,NULL);
	}
	
	if(pChild)
	{
		pChild->Draw();
	}
	GetFocus()->DrawMouse(Engine->Input()->GetMouseRect()->left, Engine->Input()->GetMouseRect()->top);
	

	return TRUE;
}

int ZSMainWindow::LeftButtonDown(int x, int y)
{
	SetFocus(this);
	if(x >= Bounds.left && x <= Bounds.right &&
		y >= Bounds.top && y <= Bounds.bottom)
	{
		ZSWindow *pWin;

		pWin = GetChild(x,y);

		if(pWin)
		{
			pWin->LeftButtonDown(x,y);
		}
		else
		{
			if(Cursor == CURSOR_POINT)
			{
				Engine->Graphics()->SetCursorFrame(1);
			}
			LBDown = TRUE;
			xLeft = (int)this->MouseX;
			yLeft = (int)this->MouseY;
		}
	}	
	return TRUE;
}

int ZSMainWindow::LeftButtonUp(int x, int y)
{
	HighlightNonStatic = FALSE;
	
	ReleaseFocus();
	LBDown = FALSE;
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}

	if(DrawWorld && x >= Bounds.left && x <= Bounds.right &&
		y >= Bounds.top && y <= Bounds.bottom)
	{
		
		ACTION_RESULT_T ActionResult = ACTION_RESULT_NONE;
			
		int EndX = (int)this->MouseX;
		int EndY = (int)this->MouseY;

		if(xLeft == EndX  && yLeft == EndY)
		{

		Creature *pActive;

		pActive = (Creature *)PreludeWorld->GetActive();
		if(PreludeParty.IsMember((Creature *)pActive))
		{
			if(this->pTarget)
			{
				switch(this->pTarget->GetDefaultAction(pActive))
				{
					case ACTION_ATTACK:
						pActive->InsertAction(ACTION_ATTACK,(void *)this->pTarget,(void *)pActive->GetData(INDEX_LASTATTACK).Value);
						//Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
						break;
					case ACTION_DEFEND:
						pActive->InsertAction(ACTION_DEFEND,NULL,NULL);
						//Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
						break;
					case ACTION_OPEN:
						pActive->InsertAction(ACTION_OPEN,(void *)this->pTarget,NULL);
						break;
					case ACTION_CLOSE:
						pActive->InsertAction(ACTION_CLOSE,(void *)this->pTarget,NULL);
						break;
					case ACTION_TALK:
						if(((Creature *)this->pTarget)->GetRegionIn() == PreludeParty.GetLeader()->GetRegionIn()
							&&
							GetDistance((Creature *)this->pTarget, PreludeParty.GetLeader()) < 12.0f)
						{
							Talk((Thing *)this->pTarget);
						}
						else
						{
							Describe("Too far away.");
						}
					//	pActive->InsertAction(ACTION_TALK,(void *)this->pTarget,NULL);
					//	pActive->Update();
						break;
					case ACTION_PICKUP:
						pActive->InsertAction(ACTION_PICKUP,(void *)this->pTarget,NULL);
						break;
					default:
						break;
				}
			}
			else
			if(Pathing && PreludeWorld->GetGameState() == GAME_STATE_COMBAT && MainPath.IsOnPath(EndX,EndY))
			{
				Path *pPath;
				pPath = new Path;
				memcpy(pPath, &MainPath, sizeof(Path));
				pPath->SetEnd(EndX,EndY);
				pActive->InsertAction(ACTION_FOLLOWPATH,(void *)pPath,(void *)NULL);
				//Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
				Pathing = FALSE;
				return TRUE;
			}
			else
			if(Valley->IsClear(EndX,EndY))
			{
				Pathing = FALSE;
				MainPath.SetTraveller(pActive);
				MainPath.SetTarget(NULL);
				MainPath.SetTargetSize(NULL);
				if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
				{
					if(MainPath.FindPath((int)pActive->GetPosition()->x,(int)pActive->GetPosition()->y, EndX, EndY,  0.0f, pActive))
					{
						PreludeParty.MoveParty(EndX,EndY);
						//Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
					}
				}
				else
				{
					if((Creature *)pActive->IsLarge())
					{
						MainPath.SetTravellerSize(2);
						if(MainPath.FindLargeCombatPath((int)pActive->GetPosition()->x,(int)pActive->GetPosition()->y, EndX, EndY,  0.0f, pActive))
						{
							Pathing = TRUE;
						}

					}
					else
					{
						MainPath.SetTravellerSize(1);
						if(MainPath.FindCombatPath((int)pActive->GetPosition()->x,(int)pActive->GetPosition()->y, EndX, EndY,  0.0f, pActive))
						{
							Pathing = TRUE;
						//	Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
						}
					}

				}
				this->pTarget = NULL;
				return TRUE;
			}
		//active is not a member
		}
		}
		Pathing = FALSE;
	}	
	return TRUE;
}

int ZSMainWindow::RightButtonUp(int x, int y)
{
	HighlightNonStatic = FALSE;
	
	ReleaseFocus();
	Pathing = FALSE;
	if(DrawWorld && x >= Bounds.left && x <= Bounds.right &&
		y >= Bounds.top && y <= Bounds.bottom)
	{
		Creature *pActive;
		float EndX;
		float EndY;
		float EndZ;

		PreludeWorld->ConvertMouseTile(x,y,&EndX,&EndY,&EndZ);

		pActive = (Creature *)PreludeWorld->GetActive();
		if(PreludeParty.IsMember((Creature *)pActive))
		{
			ActionMenu *pMenu = new ActionMenu;
			BOOL CreatedTarget = FALSE;
			if(!this->pTarget)
			{
				CreatedTarget = TRUE;
				this->pTarget = new Object;
				this->pTarget->SetPosition(EndX, EndY, EndZ);
			}

			if(pActive->GetValidActions(this->pTarget, pMenu))
			{
				ZSActionWin *pActionWin;
				pActionWin = new ZSActionWin(pMenu,x,y,this);
				pActionWin->SetActor((Creature *)pActive);
				pActionWin->Show();
			
				pActionWin->GoModal();

				RemoveChild(pActionWin);

				//if(CreatedTarget)
				//{
				//	delete this->pTarget;
				//	this->pTarget = NULL;
				//}
		
			}
		}
	}
	return TRUE;
}

int ZSMainWindow::MoveMouse(long *x, long *y, long *z)
{
	LastMoved = timeGetTime();
	
	
	if(*x < Bounds.left) 
		*x = Bounds.left;
	if(*x > Bounds.right)
		*x = Bounds.right;
	if(*y < Bounds.top)
		*y = Bounds.top;
	if(*y > Bounds.bottom)
		*y = Bounds.bottom;

	
	if(GetChild(*x,*y))
	{
		Cursor = CURSOR_POINT;
		Engine->Graphics()->SetCursor(CURSOR_POINT);
		this->pTarget = NULL;
	}
	else
	if(DrawWorld && *x >= Bounds.left && *x <= Bounds.right &&
		*y >= Bounds.top && *y <= Bounds.bottom)
	{
		PreludeWorld->ConvertMouseTile(Engine->Input()->GetMouseRect()->left,Engine->Input()->GetMouseRect()->top,&this->MouseX,&this->MouseY, &this->MouseZ);
		if((int)MouseX != LastX || (int)MouseY != LastY)
		{
			GotTarget = FALSE;
			pTarget = NULL;
		}
		
		Creature *pActive;

		pActive = (Creature *)PreludeWorld->GetActive();
		if(PreludeParty.IsMember((Creature *)pActive))
		{
			if(!this->pTarget)
			{
				if(Valley->IsClear((int)(this->MouseX + 0.5f),(int)(this->MouseY + 0.5f)))
				{
					Cursor = CURSOR_MOVE;
					Engine->Graphics()->SetCursor(CURSOR_MOVE);
				}
				else
				{
					Cursor = CURSOR_POINT;
					Engine->Graphics()->SetCursor(CURSOR_POINT);
				}
			}
			
		}
	
	}

	//clamp the mouse position to between our boundaries
//	D3DXMATRIX mRotation;
//	D3DXVECTOR4 vResult;

	iMouseX = *x;
	iMouseY = *y;
	
	if(*z > 2 || *z < -2)
	{
//		if(*z > 0)
//		{
			PreludeWorld->ArcCamera(0.0125f * (float)*z);
			//		}
//		else
//		{
//			PreludeWorld->ArcCamera(-0.025f * (float)*z);
//		}
		MouseWheel = *z;
	}
/*
	if(LBDown)
	{
		if(abs(*x - xLeft) > 8)
		{
			if(*x - xLeft < 0)
			{
				PreludeWorld->RotateCamera(-ROTATION_ANGLE);
			}
			else
			{
				PreludeWorld->RotateCamera(ROTATION_ANGLE);
			}
			PreludeWorld->ChangeCamera();	
			xLeft = *x;
			SetCursor(CURSOR_CAMERA);
		}
		if(abs(*y - yLeft) > 8)
		{
			if(*y - yLeft < 0)
			{
				PreludeWorld->ArcCamera(-ROTATION_ANGLE);
			}
			else
			{
				PreludeWorld->ArcCamera(ROTATION_ANGLE);
			}
			PreludeWorld->ChangeCamera();	
			yLeft = *y;
			SetCursor(CURSOR_CAMERA);
		}
	}
*/
	return TRUE; 
}

int ZSMainWindow::Command(int IDFrom, int Command, int Param)
{
	switch (Command)
	{
		case COMMAND_BUTTON_PRESSED:
			switch(IDFrom)
			{
				case IDC_INTERFACEBAR_ZOOMIN:
					Zooming = TRUE;
					ZoomFactor = -ZOOM_FACTOR;
					break;
				case IDC_INTERFACEBAR_ZOOMOUT:
					Zooming = TRUE;
					ZoomFactor = ZOOM_FACTOR;
					break;
				case IDC_INTERFACEBAR_ROTATE_OVER:
					Arcing = TRUE;
					Arcfactor = -0.025f;
					break;
				case IDC_INTERFACEBAR_ROTATE_UNDER:
					Arcing = TRUE;
					Arcfactor = 0.025f;
					break;
				case IDC_INTERFACEBAR_ROTATE_RIGHT:
					//rotate right
					Rotating = TRUE;
					RotationFactor = -ROTATION_ANGLE;
					break;
				case IDC_INTERFACEBAR_ROTATE_LEFT:
					//rotate left
					Rotating = TRUE;
					RotationFactor = ROTATION_ANGLE;
					break;
			}
			break;
			
		case COMMAND_BUTTON_CLICKED:
		switch(IDFrom)
		{
			case IDC_INTERFACEBAR_ZOOMIN:
			case IDC_INTERFACEBAR_ZOOMOUT:
				Zooming = FALSE;
				ZoomFactor = 0.0f;
				break;
			case IDC_INTERFACEBAR_ROTATE_OVER:
			case IDC_INTERFACEBAR_ROTATE_UNDER:
				Arcing = FALSE;
				break;
			case IDC_INTERFACEBAR_ROTATE_RIGHT:
			case IDC_INTERFACEBAR_ROTATE_LEFT:
				Rotating = FALSE;
				RotationFactor = 0.0f;
				break;
		case IDC_INTERFACEBAR_GATHER:
			if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
			{
				int tx;
				int ty;
				tx = (int)PreludeParty.GetLeader()->GetPosition()->x;
				ty = (int)PreludeParty.GetLeader()->GetPosition()->y;
			
				PreludeParty.Teleport(tx, ty);
			}
			break;
		case IDC_INTERFACEBAR_REST:
			if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT
				&&
				PreludeWorld->GetCurAreaNum() == 0)
			{
				ZSRest *pRest;
				pRest = new ZSRest(66666,100,100,600,400);
				pRest->Show();
				ZSWindow::GetMain()->AddTopChild(pRest);
				ZSWindow::GetMain()->SetFocus(pRest);
				pRest->GoModal();
				pRest->ReleaseFocus();
				ZSWindow::GetMain()->RemoveChild(pRest);
			}
			break;
		case IDC_INTERFACEBAR_OPTIONS:
			ZSMainOptionsMenu *pOpMenu;
			pOpMenu = new ZSMainOptionsMenu(66666);
			pOpMenu->Show();
			ZSWindow::GetMain()->AddTopChild(pOpMenu);
			ZSWindow::GetMain()->SetFocus(pOpMenu);
			pOpMenu->GoModal();
			pOpMenu->ReleaseFocus();
			ZSWindow::GetMain()->RemoveChild(pOpMenu);
			break;
		case IDC_INTERFACEBAR_JOURNAL:
			if(PreludeParty.GetBest(INDEX_LITERACY_AND_LORE)->GetData(INDEX_LITERACY_AND_LORE).Value)
			{
				//open journal screen
				JournalWin *pJW;
				pJW = new JournalWin(66666,000,100,800,400);
				pJW->Show();
				ZSWindow::GetMain()->AddTopChild(pJW);

				pJW->SetFocus(pJW);
				pJW->GoModal();
				pJW->ReleaseFocus();

				ZSWindow::GetMain()->RemoveChild(pJW);
			}
			break;
		case IDC_INTERFACEBAR_MAP:
			if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT
				&&
				Valley == PreludeWorld->GetArea(0))
			{
				MapWin *pMap;
				pMap = new MapWin;
				pMap->Show();
				ZSWindow::GetMain()->AddTopChild(pMap);
				ZSWindow::GetMain()->SetFocus(pMap);
				pMap->GoModal();
				pMap->ReleaseFocus();
				ZSWindow::GetMain()->RemoveChild(pMap);
			}
			break;

		case IDC_INTERFACEBAR_HELP:
			//help window
			ZSHelpWin *HelpWin;
			HelpWin = new ZSHelpWin(54654);
			HelpWin->Show();
			AddTopChild(HelpWin);
			SetFocus(HelpWin);
			HelpWin->GoModal();
			ReleaseFocus();
			RemoveChild(HelpWin);
			break;
		}
		break;
		default:
			break;
	}
	return TRUE;
}

#define PRESSED(x) (CurrentKeys[x] & 0x80) 

int ZSMainWindow::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	Creature *pThing;
	char blarg[128];

	pThing = (Creature *)PreludeWorld->GetActive();
	
	

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_V] & 0x80 && !(LastKeys[DIK_V] & 0x80))
	{
		Describe(VERSION_NUMBER);
	}




	if(PRESSED(DIK_F))
	{
		ShowFrames = TRUE;
		pFPS->Show();
		pDrawTime->Show();
	}
	else
	{
		ShowFrames = FALSE;
		pFPS->Hide();
		pDrawTime->Hide();
	}

	D3DXMATRIX mRotation;
	D3DXVECTOR4 vResult;

//camera controls
	if(!PRESSED(DIK_NUMPAD8) && !PRESSED(DIK_NUMPAD4) && !PRESSED(DIK_NUMPAD2) && !PRESSED(DIK_NUMPAD6))
	{
		Scrolling = FALSE;
		vOffset = ScrollVector = _D3DVECTOR(0.0f,0.0f,0.0f);
	}
	else
	{
		if(PRESSED(DIK_NUMPAD8) || PRESSED(DIK_UP))
		{
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.y = -ScrollFactor;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			ScrollVector = (D3DVECTOR)vResult;
		}
		else
		if(PRESSED(DIK_NUMPAD2) || PRESSED(DIK_DOWN))
		{
			//move to bottom of screen
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.y = ScrollFactor;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			ScrollVector = (D3DVECTOR)vResult;
		}
		else
		{
			//move to bottom of screen
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.y = 0;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			ScrollVector = (D3DVECTOR)vResult;
		}
		
		if(PRESSED(DIK_NUMPAD6) || PRESSED(DIK_RIGHT))
		{
			//move to right of screen
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.x = ScrollFactor;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			ScrollVector = (D3DVECTOR)vResult;
		}
		else
		if(PRESSED(DIK_NUMPAD4) || PRESSED(DIK_LEFT))
		{
			//move to left of screen
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.x = -ScrollFactor;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			ScrollVector = (D3DVECTOR)vResult;
		}
		else
		{
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.x = 0;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			ScrollVector = (D3DVECTOR)vResult;
		}
	
		Scrolling = TRUE;
	}
	
	if(!PRESSED(DIK_NUMPAD9) && !PRESSED(DIK_PRIOR) && !PRESSED(DIK_NUMPAD7) && !PRESSED(DIK_HOME))
	{
		Rotating = FALSE;
		RotationFactor = 0.0f;
	}
	else
	{
		if(CurrentKeys[DIK_NUMPAD9] & 0x80 || CurrentKeys[DIK_PRIOR] & 0x80)
		{
			//rotate right
			Rotating = TRUE;
			RotationFactor = -ROTATION_ANGLE;
			//Valley->RotateCamera(-ROTATION_ANGLE);
			//Valley->ChangeCamera();
		}
		else
		if(CurrentKeys[DIK_NUMPAD7] & 0x80 || CurrentKeys[DIK_HOME] & 0x80)
		{
			//rotate left
			Rotating = TRUE;
			RotationFactor = ROTATION_ANGLE;
			//Valley->RotateCamera(ROTATION_ANGLE);
			//Valley->ChangeCamera();
		}
	}

	if(!PRESSED(DIK_NUMPAD1) && !PRESSED(DIK_MINUS) &&!PRESSED(DIK_NUMPAD3) && !PRESSED(DIK_EQUALS))
	{
		Zooming = FALSE;
		ZoomFactor = 0.0f;
	}
	else
	{
		if(CurrentKeys[DIK_NUMPAD1] & 0x80 || CurrentKeys[DIK_MINUS] & 0x80)
		{
			Zooming = TRUE;
			ZoomFactor = ZOOM_FACTOR;
		}

		if(CurrentKeys[DIK_NUMPAD3] & 0x80 || CurrentKeys[DIK_EQUALS] & 0x80)
		{
			Zooming = TRUE;
			ZoomFactor = -ZOOM_FACTOR;
		}
	}

	if(!PRESSED(DIK_ADD) && !PRESSED(DIK_SUBTRACT))
	{
		Arcing = FALSE;
	}
	else
	{
		if(PRESSED(DIK_ADD))
		{
			Arcing = TRUE;
			Arcfactor = 0.025f;
		}
		else
		if(PRESSED(DIK_SUBTRACT))
		{
			Arcing = TRUE;
			Arcfactor = -0.025f;
		}
	}


//centerscreen
	if(CurrentKeys[DIK_NUMPAD5] & 0x80 || CurrentKeys[DIK_RETURN] & 0x80)
	{
		PreludeWorld->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
		PreludeWorld->LookAt((Thing *)PreludeParty.GetLeader());
	}



//bring up help window
	if(CurrentKeys[DIK_SLASH] & 0x80 && !(LastKeys[DIK_SLASH] & 0x80))
	{
		//help window
		ZSHelpWin *HelpWin;

		HelpWin = new ZSHelpWin(54654);;

		HelpWin->Show();
		AddTopChild(HelpWin);
		SetFocus(HelpWin);
		HelpWin->GoModal();
		ReleaseFocus();
		RemoveChild(HelpWin);
		return TRUE;	
	}

	if(CurrentKeys[DIK_F1] & 0x80 && !(LastKeys[DIK_F1] & 0x80))
	{
		//help
		ZSHelpWin *pHelp;
		pHelp = new ZSHelpWin(-1);
		pHelp->Show();
		AddTopChild(pHelp);
		SetFocus(pHelp);
		pHelp->GoModal();
		ReleaseFocus();
		RemoveChild(pHelp);
		return TRUE;
	}

#ifndef NDEBUG
if(PreludeWorld->IsEdittingEnabled())
{
	static float fBias = 0.0f;
	
	if(CurrentKeys[DIK_N] & 0x80 && !(LastKeys[DIK_N] & 0x80) && (PRESSED(DIK_LCONTROL) || PRESSED(DIK_RCONTROL)))
	{
		fBias += 0.1f;
		Engine->Graphics()->GetD3D()->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&fBias)) );

	//	PreludeWorld->GetArea(PreludeWorld->GetCurAreaNum())->ConvertChunks();
	}

	if(CurrentKeys[DIK_K] & 0x80 && !(LastKeys[DIK_K] & 0x80) && (PRESSED(DIK_LCONTROL) || PRESSED(DIK_RCONTROL)))
	{
		fBias -= 0.1f;
		Engine->Graphics()->GetD3D()->SetTextureStageState( 0, D3DTSS_MIPMAPLODBIAS, *((LPDWORD) (&fBias)) );
		//PreludeWorld->SwitchDrawMethod();
	}


	if(CurrentKeys[DIK_O] & 0x80 && !(LastKeys[DIK_O] & 0x80) && (PRESSED(DIK_LCONTROL) || PRESSED(DIK_RCONTROL)))
	{
		pThing->OutputDebugInfo();
		Chunk *pChunk;
		pChunk = Valley->GetChunk((int)MouseX/CHUNK_TILE_WIDTH, (int)MouseY/CHUNK_TILE_HEIGHT);
		pChunk->OutPutDebugInfo("chunk.txt");
	}

	if(CurrentKeys[DIK_O] & 0x80 && !(LastKeys[DIK_O] & 0x80) && (PRESSED(DIK_LCONTROL) || PRESSED(DIK_RCONTROL)))
	{
		pThing->OutputDebugInfo();
		Chunk *pChunk;
		pChunk = Valley->GetChunk((int)MouseX/CHUNK_TILE_WIDTH, (int)MouseY/CHUNK_TILE_HEIGHT);
		pChunk->OutPutDebugInfo("chunk.txt");
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_G] & 0x80 && !(LastKeys[DIK_G] & 0x80))
	{
		Valley->GenerateBase(NULL);
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_W] & 0x80 && !(LastKeys[DIK_W] & 0x80))
	{
		SetCurrentDirectory(".\\Areas");
		Valley->Save();
		SetCurrentDirectory(Engine->GetRootDirectory());
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_B] & 0x80 && !(LastKeys[DIK_B] & 0x80))
	{
		PreludeWorld->Save("WorldBase.bin");
	}


	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_F] & 0x80 && !(LastKeys[DIK_F] & 0x80))
	{
		if(Engine->Graphics()->GetFilterState() == FILTER_NONE)
		{
			Engine->Graphics()->SetFilterState(FILTER_TERRAIN);
		}
		else
		if(Engine->Graphics()->GetFilterState() == FILTER_TERRAIN)
		{
			Engine->Graphics()->SetFilterState(FILTER_BOTH);
		}
		else
		{
			Engine->Graphics()->SetFilterState(FILTER_NONE);
		}
	}


	//toggle backfaceterrain draw
	if(CurrentKeys[DIK_B] & 0x80 && !(LastKeys[DIK_B] & 0x80))
	{
		PreludeWorld->ToggleBackTerrain();
	}

	if(!(CurrentKeys[DIK_LSHIFT] & 0x80) && !(CurrentKeys[DIK_RSHIFT] & 0x80))
	{
		if(CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
		{
			FrameRate--;
			sprintf(blarg,"Framerate:  %f",1000.0f / (float)FrameRate);
			Describe(blarg);
		}
	}
	else
	{
		if(CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
		{
			FrameRate++;
			sprintf(blarg,"Framerate:  %f",1000.0f / (float)FrameRate);
			Describe(blarg);
		}
	}


	if(CurrentKeys[DIK_M] & 0x80 && !(LastKeys[DIK_M] & 0x80) 
		&& (CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80))
	{
		if(!pEEWin)
			pEEWin = new EEWin(ZSWindow::GetMain());
		else
		{
			pEEWin->Show();
		}
		SetFocus(pEEWin);
		
		pEEWin->GoModal();

		pEEWin->ReleaseFocus();

		SetFocus(this);
	}


	if(CurrentKeys[DIK_E] & 0x80 && !(LastKeys[DIK_E] & 0x80) 
		&& (CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80))
	{
		ZSWindow *pWin;

		pWin = GetChild();

		while(pWin)
		{
			pWin->Hide();
			pWin = pWin->GetSibling();
		}

		PreludeWorld->SetGameState(GAME_STATE_EDIT);
	
		if(!pWorldEdit)
			pWorldEdit = new WorldEditWin();
		else
		{
			pWorldEdit->Show();
		}

		pWorldEdit->GoModal();

		PreludeWorld->SetGameState(GAME_STATE_NORMAL);
		pWin = GetChild();

		while(pWin)
		{
			if(pWin != pRegionEdit && pWin != pWorldEdit && pWin != pEEWin && pWin != pScriptWin)
			{
				pWin->Show();
			}
			pWin = pWin->GetSibling();
		}

		SetFocus(this);
	}

	if(CurrentKeys[DIK_C] & 0x80 && !(LastKeys[DIK_C] & 0x80) 
		&& (CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80))
	{
		ZSWindow *pWin;

		pWin = GetChild();

		while(pWin)
		{
			pWin->Hide();
			pWin = pWin->GetSibling();
		}

		PreludeWorld->SetGameState(GAME_STATE_EDIT);
	
		ZSCaveEdit *pCEdit;

		pCEdit = new ZSCaveEdit(-5);

		SetFocus(pCEdit);
		AddChild(pCEdit);
		pCEdit->GoModal();
		ReleaseFocus();

		RemoveChild(pCEdit);

		PreludeWorld->SetGameState(GAME_STATE_NORMAL);
		pWin = GetChild();

		while(pWin)
		{
			if(pWin != pRegionEdit && pWin != pWorldEdit && pWin != pEEWin && pWin != pScriptWin)
			{
				pWin->Show();
			}
			pWin = pWin->GetSibling();
		}

		SetFocus(this);
	}

	if(CurrentKeys[DIK_R] & 0x80 && !(LastKeys[DIK_R] & 0x80) 
		&& (CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80))
	{
		ZSWindow *pWin;

		pWin = GetChild();

		while(pWin)
		{
			pWin->Hide();
			pWin = pWin->GetSibling();
		}

		if(!pRegionEdit)
			pRegionEdit = new RegionEditWin();
		
		pRegionEdit->Show();
		SetFocus(pRegionEdit);

		pRegionEdit->GoModal();

		ReleaseFocus();
		
		pWin = GetChild();

		while(pWin)
		{
			if(pWin != pRegionEdit && pWin != pWorldEdit && pWin != pEEWin && pWin != pScriptWin)
			{
				pWin->Show();
			}
			pWin = pWin->GetSibling();
		}

		return TRUE;
	}


	if(CurrentKeys[DIK_P] & 0x80 && !(LastKeys[DIK_P] & 0x80) 
		&& (CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80))
	{
		ZSWindow *pWin;

		pWin = GetChild();

		while(pWin)
		{
			pWin->Hide();
			pWin = pWin->GetSibling();
		}

		pPeopleEdit = new PeopleEditWin();
		
		pPeopleEdit->Show();
		SetFocus(pPeopleEdit);
		
		pPeopleEdit->GoModal();

		pPeopleEdit->ReleaseFocus();
		
		RemoveChild(pPeopleEdit);
		pPeopleEdit = NULL;

		
		pWin = GetChild();

		while(pWin)
		{
			if(pWin != pRegionEdit && pWin != pWorldEdit && pWin != pEEWin && pWin != pScriptWin)
			{
				pWin->Show();
			}
			pWin = pWin->GetSibling();
		}

		return TRUE;
	}

	if((CurrentKeys[DIK_LALT] & 0x80 || CurrentKeys[DIK_RALT] & 0x80) && (CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) 
		&& CurrentKeys[DIK_S] & 0x80 && !(LastKeys[DIK_S] &0x80))
	{
		pScriptWin = (ZSScriptWin *)GetChild(IDC_SCRIPT_WINDOW);
		if(pScriptWin)
		{
			pScriptWin->Show();
			pScriptWin->GoModal();
			pScriptWin->Hide();
		}
		else
		{
			pScriptWin = new ZSScriptWin(IDC_SCRIPT_WINDOW);
			AddTopChild(pScriptWin);
			pScriptWin->Show();
			pScriptWin->GoModal();	
			pScriptWin->Hide();
		}
		return TRUE;
	}

	if((CurrentKeys[DIK_LALT] & 0x80 || CurrentKeys[DIK_RALT] & 0x80) && (CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) 
		&& CurrentKeys[DIK_Z] & 0x80 && !(LastKeys[DIK_Z] &0x80))
	{
		SetCurrentDirectory(".\\Areas");
		Valley->SaveBrief();
		SetCurrentDirectory(Engine->GetRootDirectory());
		return TRUE;
	}

	if(CurrentKeys[DIK_LBRACKET] & 0x80 && !(LastKeys[DIK_LBRACKET] & 0x80))
	{
		if(SkipFrames)
		{
			SkipFrames = 0;
		}
		else
		{
			SkipFrames = 1;
		}
	}

	if(PRESSED(DIK_C) && (PRESSED(DIK_LCONTROL) || PRESSED(DIK_RCONTROL)))
	{
		PreludeWorld->ClearRegions();
	}

}
#endif

//defend if in combat
	if(CurrentKeys[DIK_D] & 0x80 && !(LastKeys[DIK_D] & 0x80))
	{
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT && pThing->GetMajorAction()->GetType() == ACTION_USER)
		{
			pThing->InsertAction(ACTION_DEFEND, NULL, NULL);
			//Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
		}
	}

//wait if in combat
	if(CurrentKeys[DIK_W] & 0x80 && !(LastKeys[DIK_W] & 0x80))
	{
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT && pThing->GetMajorAction()->GetType() == ACTION_USER)
		{
			pThing->InsertAction(ACTION_WAITACTION, NULL, NULL);
			//Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
		}
	}

//highlight non-static objects
	if(CurrentKeys[DIK_LALT] & 0x80 || CurrentKeys[DIK_RALT] & 0x80)
	{
		HighlightNonStatic = TRUE;
	}
	else
	{
		HighlightNonStatic = FALSE;
	}


//	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
//	{
//		State = WINDOW_STATE_DONE;
//	}

//draw the mouse square
	if(CurrentKeys[DIK_M] & 0x80 && !(LastKeys[DIK_M] & 0x80))
	{
		if(DrawMouseSquare)
		{
			DrawMouseSquare = FALSE;
		}
		else
		{
			DrawMouseSquare = TRUE;
		}
	}

	if(CurrentKeys[DIK_Q] & 0x80 && !(LastKeys[DIK_Q] & 0x80))
	{
		if(PreludeWorld->InCombat() || CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80)
		{

		}
		else
		{
			PreludeWorld->QuickSave();
		}
	}

	if(CurrentKeys[DIK_S] & 0x80 && !(LastKeys[DIK_S] & 0x80))
	{
		if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT && CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80)
		{
			//bring up save window
			SaveWin *pSaveWin;
			pSaveWin = new SaveWin(-1,100,100,500,400);
			pSaveWin->Show();
			this->AddTopChild(pSaveWin);
			SetFocus(pSaveWin);
			pSaveWin->GoModal();
			ReleaseFocus();
			this->RemoveChild(pSaveWin);
			return TRUE;
		}
		else
		{
			if(Engine->Sound()->FxAreOn())
			{
				Describe("SoundFX Off");
				Engine->Sound()->SetFX(FALSE);
			}
			else
			{
				Describe("SoundFX On");
				Engine->Sound()->SetFX(TRUE);
			}
		}
	}



	
	if(PRESSED(DIK_I))
	{
		CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, PreludeParty.GetLeader());
		pCWin->Show();
		this->AddTopChild(pCWin);

		SetFocus(pCWin);
		pCWin->GoModal();
		ReleaseFocus();

		this->RemoveChild(pCWin);
		return TRUE;
	}

if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
{
	if(CurrentKeys[DIK_L] & 0x80 && !(LastKeys[DIK_L] & 0x80))
	{
		LoadWin *pLoadWin;
		pLoadWin = new LoadWin(-1,100,100,500,400);
		pLoadWin->Show();
		this->AddTopChild(pLoadWin);
		SetFocus(pLoadWin);
		pLoadWin->GoModal();
		ReleaseFocus();
		this->RemoveChild(pLoadWin);
		return TRUE;
	}

	if(CurrentKeys[DIK_TAB] & 0x80 && !(LastKeys[DIK_TAB] & 0x80))
	{
		if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT
				&&
				Valley == PreludeWorld->GetArea(0))
		{
			MapWin *pMap;
			pMap = new MapWin;
			pMap->Show();
			this->AddTopChild(pMap);
			SetFocus(pMap);
			pMap->GoModal();
			pMap->ReleaseFocus();
			this->RemoveChild(pMap);
		}
		else
		if(PreludeWorld->InCombat())
		{
			TabToNextTarget();

		}
	}

	if(PRESSED(DIK_1))
	{
		if(PreludeParty.GetNumMembers() >= 1)
		{
			CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, PreludeParty.GetMember(0));
			pCWin->Show();
			this->AddTopChild(pCWin);

			SetFocus(pCWin);
			pCWin->GoModal();
			ReleaseFocus();

			this->RemoveChild(pCWin);
		}
		return TRUE;
	}


	if(PRESSED(DIK_2))
	{
		if(PreludeParty.GetNumMembers() >= 2)
		{
			CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, PreludeParty.GetMember(1));
			pCWin->Show();
			this->AddTopChild(pCWin);

			SetFocus(pCWin);
			pCWin->GoModal();
			ReleaseFocus();

			this->RemoveChild(pCWin);
		}
		return TRUE;
	}
	if(PRESSED(DIK_3))
	{
		if(PreludeParty.GetNumMembers() >= 3)
		{
			CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, PreludeParty.GetMember(2));
			pCWin->Show();
			this->AddTopChild(pCWin);

			SetFocus(pCWin);
			pCWin->GoModal();
			ReleaseFocus();

			this->RemoveChild(pCWin);
		}
		return TRUE;
	}
	
	if(PRESSED(DIK_4))
	{
		if(PreludeParty.GetNumMembers() >= 4)
		{
			CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, PreludeParty.GetMember(3));
			pCWin->Show();
			this->AddTopChild(pCWin);

			SetFocus(pCWin);
			pCWin->GoModal();
			ReleaseFocus();

			this->RemoveChild(pCWin);
		}
		return TRUE;
	}
	
	if(PRESSED(DIK_5))
	{
		if(PreludeParty.GetNumMembers() >= 5)
		{
			CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, PreludeParty.GetMember(4));
			pCWin->Show();
			this->AddTopChild(pCWin);

			SetFocus(pCWin);
			pCWin->GoModal();
			ReleaseFocus();

			this->RemoveChild(pCWin);
		}
		return TRUE;
	}

	if(PRESSED(DIK_6))
	{
		if(PreludeParty.GetNumMembers() >= 6)
		{
			CharacterWin *pCWin = new CharacterWin(CHARACTER_WINDOW_ID,100,100,600,400, PreludeParty.GetMember(5));
			pCWin->Show();
			this->AddTopChild(pCWin);

			SetFocus(pCWin);
			pCWin->GoModal();
			ReleaseFocus();

			this->RemoveChild(pCWin);
		}
		return TRUE;
	}

	if(PRESSED(DIK_O) || PRESSED(DIK_ESCAPE))
	{
		//open map screen
		ZSMainOptionsMenu *pOpMenu;
		pOpMenu = new ZSMainOptionsMenu(66666);
		pOpMenu->Show();
		ZSWindow::GetMain()->AddTopChild(pOpMenu);
		ZSWindow::GetMain()->SetFocus(pOpMenu);
		pOpMenu->GoModal();
		pOpMenu->ReleaseFocus();
		ZSWindow::GetMain()->RemoveChild(pOpMenu);
	}
}

	if(PreludeWorld->InCombat() && CurrentKeys[DIK_TAB] & 0x80 && !(LastKeys[DIK_TAB] & 0x80))
	{
		
			TabToNextTarget();
	}
	
	if(PRESSED(DIK_R))
	{
		//show ranges
		if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT || PreludeWorld->GetCurAreaNum())
		{

		}
		else
		{
			ZSRest *pRest;
			pRest = new ZSRest(66666,100,100,600,400);
			pRest->Show();
			ZSWindow::GetMain()->AddTopChild(pRest);
			ZSWindow::GetMain()->SetFocus(pRest);
			pRest->GoModal();
			pRest->ReleaseFocus();
			ZSWindow::GetMain()->RemoveChild(pRest);
		}
	}

	if(PRESSED(DIK_Q) && (PRESSED(DIK_LCONTROL) || PRESSED(DIK_RCONTROL)))
	{
		if(Confirm(this, "Quit game?", "yes", "no"))
		{
			State = WINDOW_STATE_DONE;
		}
	}




	return TRUE;
}

//the mainwindow's modal loop is the main update loop for the entire game

int ZSMainWindow::GoModal()
{
	FrameNum = 0;
	Frame = 0;
	State = WINDOW_STATE_NORMAL;
	
	LastFrame = timeGetTime();

	//loop while we're not in a done state
	while(State != WINDOW_STATE_DONE)
	{
		FrameLength = (timeGetTime() - LastFrame) + 1;
		LastFrame = timeGetTime();
		if(ShowFrames)		
			sprintf(pFPS->GetText(),"%05.2f",(1000.0f/(float)FrameLength));
		NextFrame = LastFrame + GetFrameRate();

		Draw();
		Engine->Graphics()->Flip();

		if(ShowFrames && !(FrameNum % 30))
		{
			FrameLength =  (timeGetTime() - LastFrame) + 1;
			sprintf(pDrawTime->GetText(),"%06.2f",(1000.0f/(float)FrameLength));
		}

		//get any user input
		
		Engine->Input()->Update(pInputFocus);

		//Engine->Graphics()->AdvanceCursorFrame();
		
		if(Arcing)
		{
			PreludeWorld->ArcCamera(Arcfactor);
		}

		if(Scrolling)
		{
			PreludeWorld->UpdateCameraOffset(ScrollVector);
		//	Valley->LookAt(NULL);
		//	Valley->ChangeCamera();
		}

		if(Rotating)
		{
			PreludeWorld->RotateCamera(RotationFactor);
//			Valley->ChangeCamera();
		}

		if(Zooming)
		{
			Engine->Graphics()->Zoom(ZoomFactor);
			PreludeWorld->ChangeCamera();
		}

		PreludeWorld->UpdateOffScreenCreatures();
		PreludeWorld->CleanOffScreenChunks();

		if(SkipFrames)
		{
			if(!HighlightNonStatic && !PreludeWorld->InCombat())
			{
				PreludeWorld->Update();
				PreludeWorld->Update();
			}
	//		Valley->LookAt(NULL);
	//		if(PreludeParty.IsMember((Creature *)Valley->GetActiveCombatant()))
	//			Valley->LookAt(Valley->GetActiveCombatant());
		}
		else
		{
			if(!HighlightNonStatic || PreludeWorld->InCombat())
			{
				PreludeWorld->Update();
#ifdef AUTOTEST
				if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT &&
					!PreludeWorld->InCombat() &&
					PreludeParty.GetLeader()->GetAction()->GetType() == ACTION_ANIMATE &&
					 (PreludeParty.GetLeader()->GetAction()->GetNext()->GetType() == ACTION_IDLE ||
					PreludeParty.GetLeader()->GetAction()->GetNext()->GetType() == ACTION_USER))
				{
					if(!(rand() % 30))
					{
						Thing *pCreature;
						pCreature = Thing::Find((Thing *)Creature::GetFirst(),rand() % PreludeParty.GetLeader()->GetNumCreatures());
						if(pCreature)
						{
							Talk(pCreature);
						}
					}
					else
					if(!(rand() % 15))
					{
						int TX;
						int TY;
						TX = (rand() % 3000) + 200;
						TY = (rand() % 3000) + 200;
						PreludeParty.Teleport(TX,TY);
					}
					else
					if(!(rand() % 30))
					{
						PreludeWorld->LoadGame("save1.gam");
					}
					else
					{
						Pathing = FALSE;
						MainPath.SetTraveller(PreludeParty.GetLeader());
						MainPath.SetTarget(NULL);
						MainPath.SetTargetSize(NULL);
						int XOff = rand() % 50;
						int YOff = rand() % 50;
						int EndX = (int)PreludeParty.GetLeader()->GetPosition()->x - 25 + XOff;
						int EndY = (int)PreludeParty.GetLeader()->GetPosition()->y - 25 + YOff;
						
						
						if(MainPath.FindPath((int)PreludeParty.GetLeader()->GetPosition()->x,(int)PreludeParty.GetLeader()->GetPosition()->y, EndX, EndY,  0.0f, PreludeParty.GetLeader()))
						{
							PreludeParty.MoveParty(EndX,EndY);
							PreludeWorld->LookAt(PreludeParty.GetLeader());
								//Valley->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
						}
					}
				}
#endif
			}
		//	Valley->LookAt(NULL);
		//	if(PreludeParty.IsMember((Creature *)Valley->GetActiveCombatant()))
		//		Valley->LookAt(Valley->GetActiveCombatant());
		}
		
		if(LastFrame - LastMoved > (FrameLength * 3) && !GotTarget)
		{
			GetTarget();
		}
				
		FrameNum ++;
		Frame++;
		Frame = Frame % 30;
		D3DXMATRIX mRotation;
		D3DXVECTOR4 vResult;

		if(iMouseX <= (Bounds.left + 2))
		{
			if(!LBDown)
			{
				D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
				if(Scrolling)
				{
					vOffset.x = -ScrollFactor;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					ScrollVector = (D3DVECTOR)vResult;
				}
				else
				{
					vOffset.x = -ScrollFactor;
					vOffset.y = 0.0f;
					vOffset.z = 0.0f;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
				}
			}
			else
			{
				PreludeWorld->RotateCamera(ROTATION_ANGLE);
				PreludeWorld->ChangeCamera();	
			}
			Cursor = CURSOR_CAMERA;
			Engine->Graphics()->SetCursor(CURSOR_CAMERA);
		}
		else
		if(iMouseX >= (Bounds.right - 2))
		{
			if(!LBDown)
			{
				D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
				if(Scrolling)
				{
					vOffset.x = ScrollFactor;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					ScrollVector = (D3DVECTOR)vResult;
				}
				else
				{
					vOffset.x = ScrollFactor;
					vOffset.y = 0.0f;
					vOffset.z = 0.0f;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
				}
			}
			else
			{
				PreludeWorld->RotateCamera(-ROTATION_ANGLE);
				PreludeWorld->ChangeCamera();	
			}
			Cursor = CURSOR_CAMERA;
			Engine->Graphics()->SetCursor(CURSOR_CAMERA);
		}

		if(iMouseY <= (Bounds.top + 2))
		{
			if(!LBDown)
			{
				D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
				if(Scrolling)
				{
					vOffset.y = -ScrollFactor;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					ScrollVector = (D3DVECTOR)vResult;
				}
				else
				{
					vOffset.y = -ScrollFactor;
					vOffset.x = 0.0f;
					vOffset.z = 0.0f;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
				}
			}
			else
			{
				PreludeWorld->ArcCamera(-ROTATION_ANGLE);
				PreludeWorld->ChangeCamera();
			}
			Cursor = CURSOR_CAMERA;
			Engine->Graphics()->SetCursor(CURSOR_CAMERA);
		}
		else
		if(iMouseY >= (Bounds.bottom - 2))
		{
			if(!LBDown)
			{
				D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
				if(Scrolling)
				{
					vOffset.y = ScrollFactor;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					ScrollVector = (D3DVECTOR)vResult;
				}
				else
				{
					vOffset.y = ScrollFactor;
					vOffset.x = 0.0f;
					vOffset.z = 0.0f;
					D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
					//move to tope of screen
					PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
				}
			}
			else
			{
				PreludeWorld->ArcCamera(ROTATION_ANGLE);
				PreludeWorld->ChangeCamera();
			}
			Cursor = CURSOR_CAMERA;
			Engine->Graphics()->SetCursor(CURSOR_CAMERA);
		}

		//rotate the goddess surfaces
		LPDIRECTDRAWSURFACE7 lpddsTemp;

		lpddsTemp = pGoddessTextures[0]->GetSurface();

		pGoddessTextures[0]->SetSurface(pGoddessTextures[1]->GetSurface());
		pGoddessTextures[1]->SetSurface(pGoddessTextures[2]->GetSurface());
		pGoddessTextures[2]->SetSurface(pGoddessTextures[3]->GetSurface());
		pGoddessTextures[3]->SetSurface(lpddsTemp);
		
		while(timeGetTime() < NextFrame) 
		{
			//update creatures
			PreludeWorld->UpdateOffScreenCreatures();
	
			//perform memory clean-up;
			PreludeWorld->CleanOffScreenChunks();
			PreludeWorld->CleanOffScreenChunks();
			//do AI updates and what-not here
		}
	}
	
	
	//perform game cleanup here
	Engine->Sound()->StopMusic();
	
	return ReturnCode;
}

void ZSMainWindow::Describe(char *DText)
{
	if(pDescribe)
		pDescribe->AddItem(DText);
}

void ZSMainWindow::ClearDescribe()
{
	if(pDescribe)
		pDescribe->Clear();
}

BOOL ZSMainWindow::ReceiveItem(Object *pToReceive, ZSWindow *pWinFrom, int x, int y)
{
	ZSWindow *pWin;
	ZSItemSlot *pISlot;
	pISlot = (ZSItemSlot *)pWinFrom;
	pWin = GetChild(x,y);
	if(pWin)
	{
		return pWin->ReceiveItem(pToReceive,pWinFrom,x,y);
	}
	else
	{
		switch(pISlot->GetSourceType())
		{
		case SOURCE_EQUIP:
			EquipWin *pEquip;
			pEquip = (EquipWin *)pWinFrom->GetParent();
			//we're all set
			//drop the item into the world
			Creature *pActive;
			pActive = (Creature *)pEquip->GetTarget();		
			
			GameItem *pGI;
			pGI = (GameItem *)pToReceive;
			pGI->SetPosition(pActive->GetPosition());
			pGI->SetRegionIn(Valley->GetRegion(pActive->GetPosition()));
			pGI->SetLocation(LOCATION_WORLD,NULL);
			pGI->SetOwner((Object *)pActive);
			pGI->SetAngle(((float)(rand() %100)  / 100.0f) * PI_MUL_2);
			Valley->AddToUpdate((Object *)pGI);
			return TRUE;
			break;
		case SOURCE_INVENTORY:
			InventoryWin *pInventory;
			pInventory = (InventoryWin *)pWinFrom->GetParent();
			if(PreludeParty.IsMember((Creature *)pInventory->GetOwner()))
			{
				//we're all set
				//drop the item into the world
				Creature *pActive;
				pActive = (Creature *)pInventory->GetOwner();		
				
				GameItem *pGI;
				pGI = (GameItem *)pToReceive;
				pGI->SetPosition(pActive->GetPosition());
				pGI->SetRegionIn(Valley->GetRegion(pActive->GetPosition()));
				pGI->SetLocation(LOCATION_WORLD,NULL);
				pGI->SetOwner((Object *)pActive);
				pGI->SetAngle(((float)(rand() %100)  / 100.0f) * PI_MUL_2);
				Valley->AddToUpdate((Object *)pGI);
				return TRUE;
			}
			return FALSE;
			break;
		default:
			return FALSE;
		}
	}
	return FALSE;
}

void Describe(char *DescribeText)
{
	((ZSMainWindow *)ZSWindow::GetMain())->Describe(DescribeText);
}

void ClearDescribe()
{
	((ZSMainWindow *)ZSWindow::GetMain())->ClearDescribe();
}

int ZSMainWindow::DrawAndUpdate()
{
	FrameLength = (timeGetTime() - LastFrame) + 1;
	LastFrame = timeGetTime();
	if(ShowFrames)		
		sprintf(pFPS->GetText(),"%05.2f",(1000.0f/(float)FrameLength));
	NextFrame = LastFrame + GetFrameRate();

	Draw();
	Engine->Graphics()->Flip();
/*
	if(ShowFrames && !(FrameNum % 30))
	{
		FrameLength =  (timeGetTime() - LastFrame) + 1;
		sprintf(pDrawTime->GetText(),"%06.2f",(1000.0f/(float)FrameLength));
	}

	//get any user input
	Engine->Input()->Update(pInputFocus);

	Engine->Graphics()->AdvanceCursorFrame();
	
	if(Arcing)
	{
		PreludeWorld->ArcCamera(Arcfactor);
	}

	if(Scrolling)
	{
		PreludeWorld->UpdateCameraOffset(ScrollVector);
	//	Valley->LookAt(NULL);
	//	Valley->ChangeCamera();
	}

	if(Rotating)
	{
		PreludeWorld->RotateCamera(RotationFactor);
//			Valley->ChangeCamera();
	}

	if(Zooming)
	{
		Engine->Graphics()->Zoom(ZoomFactor);
		PreludeWorld->ChangeCamera();
	}
*/
	if(SkipFrames)
	{
		PreludeWorld->Update();
		PreludeWorld->Update();
//		Valley->LookAt(NULL);
//		if(PreludeParty.IsMember((Creature *)Valley->GetActiveCombatant()))
//			Valley->LookAt(Valley->GetActiveCombatant());
	}
	else
	{
		PreludeWorld->Update();
	//	Valley->LookAt(NULL);
	//	if(PreludeParty.IsMember((Creature *)Valley->GetActiveCombatant()))
	//		Valley->LookAt(Valley->GetActiveCombatant());
	}
	
	while(timeGetTime() < NextFrame) 
	{
		//do AI updates and what-not here
	}
	FrameNum ++;
	Frame++;
	Frame = Frame % 30;

	return TRUE;

}

void ZSMainWindow::SetTargetString()
{
	if(pTarget)
	{
		if(pTarget->GetObjectType() == OBJECT_CREATURE)
		{
			Creature *pCreature;
			pCreature = (Creature *)pTarget;

			char temp[16];
			if(PreludeParty.IsMember((Creature *)pTarget))
			{
				sprintf(TargetString,"%s  HP: %i/%i  AP: %i/%i", 
					pCreature->GetData(INDEX_NAME).String,
					pCreature->GetData(INDEX_HITPOINTS).Value,
					pCreature->GetData(INDEX_MAXHITPOINTS).Value,

					pCreature->GetData(INDEX_ACTIONPOINTS).Value,
					pCreature->GetData(INDEX_MAXACTIONPOINTS).Value);

				if(pCreature->GetData(INDEX_POWER_OF_FLAME).Value)
				{
					sprintf(temp,"  RP: %i/%i",
						pCreature->GetData(INDEX_RESTPOINTS).Value,
						pCreature->GetData(INDEX_MAXRESTPOINTS).Value);
					strcat(TargetString,temp);
				}

				if(pCreature->GetData(INDEX_GIFTS_OF_GODDESS).Value ||
					pCreature->GetData(INDEX_THAUMATURGY).Value)
				{
					sprintf(temp,"  WP: %i/%i",
						pCreature->GetData(INDEX_WILLPOINTS).Value,
						pCreature->GetData(INDEX_MAXWILLPOINTS).Value);
					strcat(TargetString,temp);
				}
			}
			else
			{
				sprintf(TargetString,"%s  HP: %i/%i", 
					pCreature->GetData(INDEX_NAME).String,
					pCreature->GetData(INDEX_HITPOINTS).Value,
					pCreature->GetData(INDEX_MAXHITPOINTS).Value);
			}
		}
		else
		{
			pTargetString->Hide();
			return;
		}
	}
	else
	{
		pTargetString->Hide();
		return;
	}

	pTargetString->SetText(TargetString);
	pTargetString->Show();
	return;
}

void ZSMainWindow::TabToNextTarget()
{
	if(!PreludeWorld->InCombat())
	{
		return;
	}

	if(!pTarget  || (pTarget->GetObjectType() != OBJECT_CREATURE))
	{
		pTarget = PreludeWorld->GetCombat()->GetCombatants();
	}
	else
	{
		
		pTarget = pTarget->GetNextUpdate();
		if(!pTarget)
		{
			pTarget = PreludeWorld->GetCombat()->GetCombatants();
		}
	}

	if(pTarget)
	{
		PreludeWorld->LookAt((Thing *)pTarget);
		
		RECT *prMouse;
		prMouse = Engine->Input()->GetMouseRect();

		prMouse->left = 398;
		prMouse->top = 245;
		prMouse->right = prMouse->left + MOUSE_CURSOR_WIDTH;
		prMouse->bottom = prMouse->top + MOUSE_CURSOR_HEIGHT;

		long LZ = 0;

		this->MoveMouse(&prMouse->left, &prMouse->top, &LZ);
	}
	



}


