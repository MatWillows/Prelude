//********************************************************************* 
//********************************************************************* 
//**************                worlds.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*    
//*Revision:
//*Revisor:                                               
//*Purpose:                        
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*        			               
//*			
//********************************************************************* 
//*********************************************************************
#include "world.h"
#include "combatmanager.h"
#include "creatures.h"
#include <assert.h>
#include "mainwindow.h"
#include "zsparticle.h"
#include "ZStext.h"
#include "ZSportrait.h"
#include "regions.h"
#include "party.h"
#include "events.h"
#include "flags.h"
#include "items.h"
#include "script.h" //for flags
#include "deathwin.h"
#include "zsmenubar.h"
#include "area.h"
#include "minimap.h" //to unset when entering dungeons
#include "zsdescribe.h"

//for re-seeding random number generator
#include <time.h>


typedef struct
{
	char Name[32];
	int Width;
	int Height;
	int ChunkWidth;
	int ChunkHeight;
	int ChunkOffsets[200*200];
} OLD_STATIC_FILE_HEADER_T;

#define D3D_OVERLOADS
#define DIFFUSE_FACTOR				0.5f
#define AMBIENT_FACTOR				0.5f
#define MAXIMUM_CAMERA_DISTANCE		40.0f

//time pass speed is in millisecond
//one minute of game time passes every 2 seconds.
//one hour = 2 minutes of play time
//one day = 48 minutes of play time

#define TIME_PASS_SPEED		2000	
#define DAILY_EVENT			59

DWORD NextAdvanceTime = 0;

#include <d3d.h>

//************** static Members *********************************

//************** Constructors  ****************************************

//simple constructor
World::World()
{
	DrawNormalChunks = FALSE;
	
	//clear initial help texts
	EdittingEnabled =
	CreationHelp =
	CharacterHelp =
	CombatHelp =
	BarterHelp =
	MapHelp =
	MainHelp = 
	RestHelp = FALSE;

	Difficulty = 1;
	XPConfirmation = TRUE;
	VerboseCombatInfo = FALSE;
	FullBodySelection = FALSE;
	HighlightInformation = TRUE;
	
	pOffScreenCreature = NULL;
	CleanX = 0;
	CleanY = 0;
	
	CurAreaNum = 0;
	
	pCombat = new Combat;

//	ZeroMemory(&RegionHeader,sizeof(RegionHeader));

	int n;

	vCamera.x = 40;
	vCamera.y = 40;
	vCamera.z = 13;

	vLookAt.x = 20;
	vLookAt.y = 20;
	vLookAt.z = 0;

	vWorldUp.x = 0;
	vWorldUp.y = 0;
	vWorldUp.z = 1;

	D3DXMatrixLookAt((D3DXMATRIX*)&matCamera, (D3DXVECTOR3*)&vCamera,
		(D3DXVECTOR3*)&vLookAt, (D3DXVECTOR3*)&vWorldUp);

	Engine->Graphics()->GetD3D()->SetTransform( D3DTRANSFORMSTATE_VIEW, &matCamera );

	CreateLights(DIFFUSE_FACTOR);


	Engine->Graphics()->GetD3D()->SetLight(0,&Light[16]);
	Engine->Graphics()->GetD3D()->LightEnable(0,TRUE);
	

	vCamOffset = _D3DVECTOR(0.0f,0.0f,0.0f);

	CameraAngle = PI_MUL_2 - PI_DIV_4;

	DrawRadius = 2;
	RenderBackTerrain = TRUE;

	ZeroMemory(RegionOffsets,sizeof(int) * MAX_WORLD_REGIONS); //max 1024 regions;
	ZeroMemory(Regions,sizeof(Region *) * MAX_WORLD_REGIONS);

	//pDrawList = NULL;

	//ZeroMemory(UpdateSegments,sizeof(Object *) * WORLD_USEG_WIDTH * WORLD_USEG_HEIGHT);

	//StaticFile = NULL;
	RegionFile = NULL;
	TotalTime = 0;
	SetHour(16);
	ShadowDraw = FALSE;
	pMainObjects = NULL;

	AutosaveRate = 0;
	LastAutosave = timeGetTime();
}

World::World(const char *filename)
{
//	ZeroMemory(&RegionHeader,sizeof(RegionHeader));
	CurAreaNum = 0;
	Difficulty = 1;
	XPConfirmation = TRUE;
	VerboseCombatInfo = FALSE;
	FullBodySelection = FALSE;
	HighlightInformation = TRUE;

	RegionFile = SafeFileOpen("world.rgn","rb");

	if(RegionFile)
	{
		fread(&NumRegions,sizeof(int),1,RegionFile);
		fread(RegionOffsets,sizeof(int),1024,RegionFile);
//		fread(&RegionHeader,sizeof(RegionHeader), 1, RegionFile);
	}

//	UpdateThings = NULL;
//	NoUpdateThings = NULL;

	pMainObjects = NULL;

	int n;

	vCamera.x = 40;
	vCamera.y = 40;
	vCamera.z = 13;

	vLookAt.x = 20;
	vLookAt.y = 20;
	vLookAt.z = 0;

	ScreenX = 20 / CHUNK_TILE_WIDTH;
	ScreenY = 20 / CHUNK_TILE_HEIGHT;

	vWorldUp.x = 0;
	vWorldUp.y = 0;
	vWorldUp.z = 1;

	D3DXMatrixLookAt((D3DXMATRIX*)&matCamera, (D3DXVECTOR3*)&vCamera,
		(D3DXVECTOR3*)&vLookAt, (D3DXVECTOR3*)&vWorldUp);

	Engine->Graphics()->GetD3D()->SetTransform( D3DTRANSFORMSTATE_VIEW, &matCamera );

	float xL;
	float zL;
	//create the lights

	for(n = 0; n < DAY_LENGTH/2; n++)
	{
		ZeroMemory(&Light[n], sizeof(D3DLIGHT7));
		Light[n].dltType = D3DLIGHT_DIRECTIONAL;
		Light[n].dcvDiffuse.r = DIFFUSE_FACTOR * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.g = DIFFUSE_FACTOR * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.b = DIFFUSE_FACTOR * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.r = AMBIENT_FACTOR;
		Light[n].dcvAmbient.g = AMBIENT_FACTOR;
		Light[n].dcvAmbient.b = AMBIENT_FACTOR;
	
	//	Light[n].dcvAmbient.r = AMBIENT_FACTOR * (float)n/(float)(DAY_LENGTH/2);
	//	Light[n].dcvAmbient.g = AMBIENT_FACTOR * (float)n/(float)(DAY_LENGTH/2);
	//	Light[n].dcvAmbient.b = AMBIENT_FACTOR * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvSpecular.r = DIFFUSE_FACTOR;
		Light[n].dcvSpecular.g = DIFFUSE_FACTOR;
		Light[n].dcvSpecular.b = DIFFUSE_FACTOR;
	
		xL = 1.0f - 1.0f * (float)n/(float)(DAY_LENGTH/2);
		zL = -1.0f * (float)n/(float)(DAY_LENGTH/2);

		Light[n].dvDirection = Normalize(D3DVECTOR(xL,0.0f,zL));
	
	}

	for(n = DAY_LENGTH/2; n < DAY_LENGTH; n++)
	{

		ZeroMemory(&Light[n], sizeof(D3DLIGHT7));
		Light[n].dltType = D3DLIGHT_DIRECTIONAL;
		Light[n].dcvDiffuse.r = DIFFUSE_FACTOR - DIFFUSE_FACTOR * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.g = DIFFUSE_FACTOR - DIFFUSE_FACTOR * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.b = DIFFUSE_FACTOR - DIFFUSE_FACTOR * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.r = AMBIENT_FACTOR;
		Light[n].dcvAmbient.g = AMBIENT_FACTOR;
		Light[n].dcvAmbient.b = AMBIENT_FACTOR;
	//	Light[n].dcvAmbient.r = AMBIENT_FACTOR - AMBIENT_FACTOR * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
	//	Light[n].dcvAmbient.g = AMBIENT_FACTOR - AMBIENT_FACTOR * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
	//	Light[n].dcvAmbient.b = AMBIENT_FACTOR - AMBIENT_FACTOR * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvSpecular.r = DIFFUSE_FACTOR;
		Light[n].dcvSpecular.g = DIFFUSE_FACTOR;
		Light[n].dcvSpecular.b = DIFFUSE_FACTOR;
	
		xL = -1.0f * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		zL = -1.0f + 1.0f * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);

		Light[n].dvDirection = Normalize(D3DVECTOR(xL,0.0f,zL));
	}
	
	//nighttime need more blue

	for(n = 0; n < DAY_LENGTH/4; n++)
	{
		Light[n].dcvDiffuse.b *= 1.5f;
		if(Light[n].dcvDiffuse.b > 1.0f)
			Light[n].dcvDiffuse.b = 1.0f;

		Light[n].dcvAmbient.b += 0.1f;
		Light[n].dcvAmbient.b *= 1.5;
		if(Light[n].dcvAmbient.b > 1.0f)
			Light[n].dcvAmbient.b = 1.0f;
	}

	for(n = DAY_LENGTH - DAY_LENGTH/4; n < DAY_LENGTH; n++)
	{
		Light[n].dcvDiffuse.b *= 1.5f;
		if(Light[n].dcvDiffuse.b > 1.0f)
			Light[n].dcvDiffuse.b = 1.0f;

		Light[n].dcvAmbient.b += 0.1f;
		Light[n].dcvAmbient.b *= 1.5f;
		if(Light[n].dcvAmbient.b > 1.0f)
			Light[n].dcvAmbient.b = 1.0f;
		
	}

	
	Engine->Graphics()->GetD3D()->SetLight(0,&Light[16]);
		Engine->Graphics()->GetD3D()->LightEnable(0,TRUE);
	

	vCamOffset = _D3DVECTOR(0.0f,0.0f,0.0f);

	CameraAngle = PI_MUL_2 - PI_DIV_4;

	DrawRadius = 2;
	RenderBackTerrain = TRUE;

	ZeroMemory(Regions,sizeof(Region *) * MAX_WORLD_REGIONS);
	ZeroMemory(RegionOffsets,sizeof(int) * MAX_WORLD_REGIONS); //max 1024 regions;

//	pDrawList = NULL;

//	ZeroMemory(UpdateSegments,sizeof(Object *) * WORLD_USEG_WIDTH * WORLD_USEG_HEIGHT);

	TotalTime = 0;

	SetHour(16);
	
	ShadowDraw = FALSE;
}

//end:  Constructors ***************************************************



//*************** Destructor *******************************************
World::~World()
{
	DEBUG_INFO("Deleting World\n");
	Object *pOb, *pKillOb;

	DEBUG_INFO("Deleting Objects\n");

	pOb = pMainObjects;
	
	while(pOb)
	{
		pKillOb = pOb;
		pOb = pOb->GetNext();

		delete pKillOb;
	}

	DEBUG_INFO("Deleting update things\n");
	
	if(RegionFile)
	{
		fclose(RegionFile);
		RegionFile = NULL;
	}

	if(Regions)
	{
		for(int n = 0; n < NumRegions; n++)
		{
			if(Regions[n])
			{
				delete Regions[n];
			}
			Regions[n] = NULL;
		}
		//delete[] Regions;
		//Regions = NULL;
	}

	for(int an = 0; an < NumAreas; an++)
	{
		delete Areas[an];
		Areas[an] = NULL;
	}

	Valley = NULL;

	if(pCombat)
	{
		delete pCombat;
	}


	DEBUG_INFO("Done Deleting World\n");
}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************
int World::GetHour()
{
	 return (TotalTime / HOUR_LENGTH) % 24;
}


int World::GetDay()
{
	return GetTotalTime() / (DAY_LENGTH * HOUR_LENGTH);
}

int World::GetMinute()
{
	return GetTotalTime() % (DAY_LENGTH * HOUR_LENGTH);

}

unsigned long World::GetTotalTime()
{
	return TotalTime;
}


void World::AdvanceTime(int Minutes)
{
	int StartTime, EndTime, HoursPassed;
	
	if(Minutes <= 0)
	{
		Minutes = 1;
		DEBUG_INFO("Negative or null passed to advancetime");
	}
	
	StartTime = TotalTime;

	EndTime = TotalTime += Minutes - 1;

	IncrementTime();
	
	Valley->UpdateOffscreen();

	HoursPassed = ((EndTime - StartTime) / HOUR_LENGTH);

	if(((StartTime / HOUR_LENGTH) % 24) + HoursPassed >= 24)
	{
		//atleast one day has passed
		PreludeEvents.RunEvent(59);
	}

}

void World::SetHour(int NewHour)
{
	int Day;
	Day = GetDay();

	TotalTime = Day * DAY_LENGTH * HOUR_LENGTH + NewHour * HOUR_LENGTH;
}

D3DLIGHT7 *World::GetLight(int Time)
{
	return &Light[Time];
}

int World::SetLight(D3DLIGHT7 *NewLight)
{
	Engine->Graphics()->GetD3D()->SetLight(0,NewLight);
	Engine->Graphics()->GetD3D()->LightEnable(0,TRUE);

	return TRUE;
}

D3DVECTOR World::GetCamera()
{
	return vCamera;
}

D3DVECTOR World::GetCenterScreen()
{

	return vLookAt;
}


void World::ConvertMouse(int MouseX, int MouseY, float *WorldX, float *WorldY, float *WorldZ)
{
	D3DVECTOR vMouseNear;
	D3DVECTOR vCameraOffset;
	D3DVECTOR vCameraSpace;

	//convert mouse to camera space
	vMouseNear.x = -(1.0f - 2.0f * (float)MouseX/(float)Engine->Graphics()->GetWidth());
	vMouseNear.y = (1.0f - 2.0f * (float)MouseY/(float)(Engine->Graphics()->GetHeight()-100));
	vMouseNear.z = 0.0f;

	vCameraSpace.x = 0;
	vCameraSpace.y = 0;
	vCameraSpace.z = 0;


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
	
	D3DXVec3Transform(&vCameraLocation,(D3DXVECTOR3 *)&vCameraSpace,&matFinal);

	//now we have the start and end points of our line

	D3DXVECTOR3 vLine;
	D3DXVECTOR3 vCamNorm;

	vCameraOffset = vLookAt - (D3DVECTOR)vCameraLocation;

	D3DXVec3Normalize(&vCamNorm,(D3DXVECTOR3 *)&vCameraOffset);

	//get the length of our camera offset

	float Length;

	Length = D3DXVec3Length((D3DXVECTOR3 *)&vCameraOffset);

	Length = Length * (vNear.z / vCameraLocation.z);

	D3DXVec3Scale(&vLine, &vCamNorm, Length);
	
	D3DVECTOR vStart, vEnd;
	vStart = (D3DVECTOR)vNear;

	vLine.x += vNear.x;
	vLine.y += vNear.y;
	vLine.z += vNear.z;
	
	vEnd = vLine;

//	vStart.x = vCameraOffset.x;
//	vStart.y = vCameraOffset.y;
//	vStart.z = vCameraOffset.z;

	//vLine now contains the end point
	//check the polygons surrounding the point of intersection
	int StartX;
	int StartY;
	int EndX;
	int EndY;

	StartX = (int)vLine.x - 1;
	EndX = (int)vLine.x + 1;

	StartY = (int)vLine.y - 1;
	EndY = (int)vLine.y + 1;

	int xn;
	int yn;

	D3DVECTOR vNormA, vNormB;
	D3DVECTOR vSideA;
	D3DVECTOR vSideB;

	float *pVerts;
	D3DVECTOR vL1;
	D3DVECTOR vL2;
	D3DVECTOR vL3;

	D3DVECTOR vIntersect;

	D3DVECTOR vRay = vEnd - vStart;

	float DistToPlane;
	float ProjectedLength;
	float Ratio;

	float AngleTotal;

	xn = vLine.x;
	yn = vLine.y;

	int RunCount = 0;
	int RunSide = 0;
	int RunLength = 1;
	int XOffset = 0;
	int YOffset = -1;
	while(TRUE)
	{
//		pVerts = MapChunks[xn / CHUNK_WIDTH][yn / CHUNK_HEIGHT].GetTile(xn % CHUNK_WIDTH, yn % CHUNK_HEIGHT);
		Chunk *pChunk;
		pChunk = NULL;
		pChunk = Valley->GetChunk(xn/CHUNK_TILE_WIDTH, yn/CHUNK_TILE_HEIGHT);

		if(!pChunk)
		{
			return;
		}
		
		pVerts = pChunk->GetTile((xn % CHUNK_TILE_WIDTH) / 2 , (yn % CHUNK_TILE_HEIGHT) / 2 );		
		
		//uL, uR, lL
		vSideA.x = pVerts[tx1] - pVerts[tx2];
		vSideA.y = pVerts[ty1] - pVerts[ty2];
		vSideA.z = pVerts[tz1] - pVerts[tz2];

		vSideB.x = pVerts[tx3] - pVerts[tx2];
		vSideB.y = pVerts[ty3] - pVerts[ty2];
		vSideB.z = pVerts[tz3] - pVerts[tz2];

		vNormA = Normalize(CrossProduct(vSideA,vSideB));
		
		//vNorm now contains the normal to the upper left triangle of this tile
		vL1.x = pVerts[tx2] - vStart.x;
		vL1.y = pVerts[ty2] - vStart.y;
		vL1.z = pVerts[tz2] - vStart.z;

		DistToPlane = DotProduct(vL1,vNormA);
		
		ProjectedLength = DotProduct((vEnd - vStart),vNormA);
		Ratio = DistToPlane / ProjectedLength;

		vIntersect.x = vStart.x + vRay.x * Ratio;
		vIntersect.y = vStart.y + vRay.y * Ratio;
		vIntersect.z = vStart.z + vRay.z * Ratio;
	
		AngleTotal = 0.0f;
		//now check to see if the interect is in the polygon;
		vL1.x = pVerts[tx1] - vIntersect.x;
		vL1.y = pVerts[ty1] - vIntersect.y;		
		vL1.z = pVerts[tz1] - vIntersect.z;
		vL1 = Normalize(vL1);

		vL2.x = pVerts[tx2] - vIntersect.x;
		vL2.y = pVerts[ty2] - vIntersect.y;		
		vL2.z = pVerts[tz2] - vIntersect.z;
		vL2 = Normalize(vL2);

		vL3.x = pVerts[tx3] - vIntersect.x;
		vL3.y = pVerts[ty3] - vIntersect.y;		
		vL3.z = pVerts[tz3] - vIntersect.z;
		vL3 = Normalize(vL3);

		AngleTotal += acos(DotProduct(vL1,vL2));
		AngleTotal += acos(DotProduct(vL2,vL3));
		AngleTotal += acos(DotProduct(vL3,vL1));

		if(AngleTotal > 6.27f && AngleTotal < 6.29f)
		{
			*WorldX = vIntersect.x;
			*WorldY = vIntersect.y;
			*WorldZ = vIntersect.z;
			memcpy(Engine->Graphics()->GetMouseVerts(),pVerts,sizeof(D3DVERTEX) * 4);
			return;
		}

		//uL, uR, lL
		vSideA.x = pVerts[tx3] - pVerts[tx2];
		vSideA.y = pVerts[ty3] - pVerts[ty2];
		vSideA.z = pVerts[tz3] - pVerts[tz2];

		vSideB.x = pVerts[tx4] - pVerts[tx2];
		vSideB.y = pVerts[ty4] - pVerts[ty2];
		vSideB.z = pVerts[tz4] - pVerts[tz2];

		vNormB = Normalize(CrossProduct(vSideA,vSideB));
		
		//vNorm now contains the normal to the upper left triangle of this tile
		vL1.x = pVerts[tx2] - vStart.x;
		vL1.y = pVerts[ty2] - vStart.y;
		vL1.z = pVerts[tz2] - vStart.z;

		DistToPlane = DotProduct(vL1,vNormB);

		ProjectedLength = DotProduct((vEnd - vStart),vNormB);

		Ratio = DistToPlane / ProjectedLength;

		vIntersect.x = vStart.x + vRay.x * Ratio;
		vIntersect.y = vStart.y + vRay.y * Ratio;
		vIntersect.z = vStart.z + vRay.z * Ratio;
	

		AngleTotal = 0.0f;
		//now check to see if the interect is in the polygon;
		vL1.x = pVerts[tx2] - vIntersect.x;
		vL1.y = pVerts[ty2] - vIntersect.y;		
		vL1.z = pVerts[tz2] - vIntersect.z;
		vL1 = Normalize(vL1);

		vL2.x = pVerts[tx4] - vIntersect.x;
		vL2.y = pVerts[ty4] - vIntersect.y;		
		vL2.z = pVerts[tz4] - vIntersect.z;
		vL2 = Normalize(vL2);

		vL3.x = pVerts[tx3] - vIntersect.x;
		vL3.y = pVerts[ty3] - vIntersect.y;		
		vL3.z = pVerts[tz3] - vIntersect.z;
		vL3 = Normalize(vL3);

		AngleTotal += acos(DotProduct(vL1,vL2));
		AngleTotal += acos(DotProduct(vL2,vL3));
		AngleTotal += acos(DotProduct(vL3,vL1));

		if(AngleTotal > 6.27f && AngleTotal < 6.29f)
		{
			*WorldX = vIntersect.x;
			*WorldY = vIntersect.y;
			*WorldZ = vIntersect.z;
			memcpy(Engine->Graphics()->GetMouseVerts(), pVerts, sizeof(D3DVERTEX) * 4);
			return;
		}
		
		if(RunCount == RunLength)
		{
			RunCount = 0;
			RunSide++;
			switch(RunSide)
			{
				case 1:
					XOffset = 1;
					YOffset = 0;
					break;
				case 2:
					XOffset = 0;
					YOffset = 1;
					RunLength++;
					break;
				case 3:
					XOffset = -1;
					YOffset = 0;
					break;
				default:
					RunSide = 0;
					XOffset = 0;
					YOffset = -1;
					RunLength++;
					break;
			}
		
		}

		xn += XOffset;
		yn += YOffset;
		RunCount++;
		if(yn < 0 || xn < 0)
		{
			return;
		}
	}
	memset(Engine->Graphics()->GetMouseVerts(),0,sizeof(D3DVERTEX) * 4);
}

void World::ConvertMouseTile(int MouseX, int MouseY, float *WorldX, float *WorldY, float *WorldZ)
{
	D3DVECTOR vMouseNear;
	D3DVECTOR vCameraOffset;
	D3DVECTOR vCameraSpace;

	//convert mouse to camera space
	vMouseNear.x = -(1.0f - 2.0f * (float)MouseX/(float)Engine->Graphics()->GetWidth());
	vMouseNear.y = (1.0f - 2.0f * (float)MouseY/(float)(Engine->Graphics()->GetHeight()-100));
	vMouseNear.z = 0.0f;

	vCameraSpace.x = 0;
	vCameraSpace.y = 0;
	vCameraSpace.z = 0;


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
	
	D3DXVec3Transform(&vCameraLocation,(D3DXVECTOR3 *)&vCameraSpace,&matFinal);

	//now we have the start and end points of our line

	D3DXVECTOR3 vLine;
	D3DXVECTOR3 vCamNorm;

	vCameraOffset = vLookAt - (D3DVECTOR)vCameraLocation;

	D3DXVec3Normalize(&vCamNorm,(D3DXVECTOR3 *)&vCameraOffset);

	//get the length of our camera offset

	float Length;

	Length = D3DXVec3Length((D3DXVECTOR3 *)&vCameraOffset);

	Length = Length * (vNear.z / vCameraLocation.z);

	D3DXVec3Scale(&vLine, &vCamNorm, Length);
	
	D3DVECTOR vStart, vEnd;
	vStart = (D3DVECTOR)vNear;

	vLine.x += vNear.x;
	vLine.y += vNear.y;
	vLine.z += vNear.z;
	
	vEnd = vLine;

//	vStart.x = vCameraOffset.x;
//	vStart.y = vCameraOffset.y;
//	vStart.z = vCameraOffset.z;

	//vLine now contains the end point
	//check the polygons surrounding the point of intersection
	int StartX;
	int StartY;
	int EndX;
	int EndY;

	StartX = (int)vLine.x - 1;
	EndX = (int)vLine.x + 1;

	StartY = (int)vLine.y - 1;
	EndY = (int)vLine.y + 1;

	int xn;
	int yn;

	D3DVECTOR vNormA, vNormB;
	D3DVECTOR vSideA;
	D3DVECTOR vSideB;

	float *pVerts;
	D3DVECTOR vL1;
	D3DVECTOR vL2;
	D3DVECTOR vL3;

	D3DVECTOR vRay = vEnd - vStart;

	float DistToPlane;
	float ProjectedLength;
	float Ratio;


	xn = vLine.x;
	yn = vLine.y;

	int RunCount = 0;
	int RunSide = 0;
	int RunLength = 1;
	int XOffset = 0;
	int YOffset = -1;
	
	float fTileHeight;
	D3DVECTOR vNorm(0.0f,0.0f,1.0f);

	float fXIntercept, fYIntercept, fZIntercept;

	while(TRUE)
	{
		Chunk *pChunk;
		pChunk = NULL;
		pChunk = Valley->GetChunk(xn/CHUNK_TILE_WIDTH, yn/CHUNK_TILE_HEIGHT);

		if(!pChunk)
		{
			return;
		}
		
		pVerts = pChunk->GetTile((xn % CHUNK_TILE_WIDTH) / 2 , (yn % CHUNK_TILE_HEIGHT) / 2 );		

		//the height of the four corners is the same
		fTileHeight = Valley->GetTileHeight(xn,yn);

		//uL, uR, lL
		vSideA.x = pVerts[tx1] - pVerts[tx2];
		vSideA.y = pVerts[ty1] - pVerts[ty2];
		vSideA.z = pVerts[tz1] - pVerts[tz2];

		vSideB.x = pVerts[tx3] - pVerts[tx2];
		vSideB.y = pVerts[ty3] - pVerts[ty2];
		vSideB.z = pVerts[tz3] - pVerts[tz2];
		
		//uL, uR, lL
		vNormA = Normalize(CrossProduct(vSideA,vSideB));
		
		//vNorm now contains the normal to the upper left triangle of this tile
		vL1.x = (float)xn - vStart.x;
		vL1.y = (float)yn - vStart.y;
		vL1.z = fTileHeight - vStart.z;

		DistToPlane = DotProduct(vL1, vNorm);
		
		ProjectedLength = DotProduct((vEnd - vStart),vNorm);
		Ratio = DistToPlane / ProjectedLength;

		fXIntercept = vStart.x + vRay.x * Ratio;
		fYIntercept = vStart.y + vRay.y * Ratio;
		fZIntercept = vStart.z + vRay.z * Ratio;
	
		//now check to see if the interect is in the polygon;
		
		if(fXIntercept >= (float)xn && fXIntercept <= (float)(xn+1)
			&& fYIntercept >= (float)yn && fYIntercept <= (float)(yn+1))
		{
			*WorldX = fXIntercept;
			*WorldY = fYIntercept;
			*WorldZ = fZIntercept;
			memcpy(Engine->Graphics()->GetMouseVerts(), pVerts, sizeof(D3DVERTEX) * 4);
			return;
		}

		//uL, uR, lL
				
		if(RunCount == RunLength)
		{
			RunCount = 0;
			RunSide++;
			switch(RunSide)
			{
				case 1:
					XOffset = 1;
					YOffset = 0;
					break;
				case 2:
					XOffset = 0;
					YOffset = 1;
					RunLength++;
					break;
				case 3:
					XOffset = -1;
					YOffset = 0;
					break;
				default:
					RunSide = 0;
					XOffset = 0;
					YOffset = -1;
					RunLength++;
					break;
			}
		}

		xn += XOffset;
		yn += YOffset;
		RunCount++;
		if(yn < 0 || xn < 0 || yn >= Valley->GetHeight() || xn >= Valley->GetWidth() )
		{
			return;
		}
	}
	memset(Engine->Graphics()->GetMouseVerts(),0,sizeof(D3DVERTEX) * 4);
}
	



//returns the to-screen scale factor of a unit length
float World::GetScaling()
{
	D3DMATRIX matView;
	D3DMATRIX matProj;
	D3DMATRIX matWorld;

	D3DXVECTOR3 vBase;
	D3DXVECTOR3 vCenter;

	vBase.x = 0.0f;
	vBase.y = 1.0f;
	vBase.z = 0.0f;
	
	vCenter.x = 0.0f;
	vCenter.y = 0.0f;
	vCenter.z = 0.0f;
	
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

	D3DXVECTOR4 vA;
	D3DXVECTOR4 vB;
	
	D3DXMATRIX matFinal;
	
	D3DXMatrixMultiply(&matFinal,(D3DXMATRIX *)&matProj, (D3DXMATRIX *)&matView);
	
	D3DXVec3Transform(&vA,(D3DXVECTOR3 *)&vBase, &matFinal);
	D3DXVec3Transform(&vB,(D3DXVECTOR3 *)&vCenter, &matFinal);

	D3DVECTOR vFinal;
	vFinal.x = vA.x - vB.x;
	vFinal.y = vA.y - vB.y;
	vFinal.z = vA.z - vB.z;

	return Magnitude(vFinal);
}


void World::ConvertToWorld(int MouseX, int MouseY, D3DVECTOR *vScreen)
{
	D3DVECTOR vMouseNear;
	D3DVECTOR vCameraOffset;
	D3DVECTOR vCameraSpace;

	//convert mouse to camera space
	vMouseNear.x = -(1.0f - 2.0f * (float)MouseX/(float)Engine->Graphics()->GetWidth());
	vMouseNear.y = (1.0f - 2.0f * (float)MouseY/(float)(Engine->Graphics()->GetHeight()-100));
	vMouseNear.z = 0.0f;

	vCameraSpace.x = 0;
	vCameraSpace.y = 0;
	vCameraSpace.z = 0;


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
	
	D3DXVec3Transform(&vCameraLocation,(D3DXVECTOR3 *)&vCameraSpace,&matFinal);

	//now we have the start and end points of our line

	D3DXVECTOR3 vLine;
	D3DXVECTOR3 vCamNorm;

	vCameraOffset =vLookAt - (D3DVECTOR)vCameraLocation;


	D3DXVec3Normalize(&vCamNorm,(D3DXVECTOR3 *)&vCameraOffset);

	//get the length of our camera offset

	float Length;

	Length = D3DXVec3Length((D3DXVECTOR3 *)&vCameraOffset);

	Length = Length * (vNear.z / vCameraLocation.z);

	D3DXVec3Scale(&vLine, &vCamNorm, Length);
	
	D3DXVec3Normalize(&vCamNorm, &vLine);

	D3DXVec3Scale(&vLine, &vCamNorm, 3.0f);
	
	vLine.x += vNear.x;
	vLine.y += vNear.y;
	vLine.z += vNear.z;


	vScreen->x = vLine.x;
	vScreen->y = vLine.y;
	vScreen->z = vLine.z;
}

void World::ConvertToWorld(D3DVECTOR *vScreen)
{
	D3DVECTOR vMouseNear;
	D3DVECTOR vCameraOffset;
	D3DVECTOR vCameraSpace;

	//convert mouse to camera space
	vMouseNear = *vScreen;

	vMouseNear.y *= (float)Engine->Graphics()->GetWidth()/(float)((Engine->Graphics()->GetHeight()-100));
		
	vCameraSpace.x = 0;
	vCameraSpace.y = 0;
	vCameraSpace.z = 0;

	D3DMATRIX matView;
	D3DMATRIX matProj;
	D3DMATRIX matWorld;

	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
	//Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
	matProj = *Engine->Graphics()->GetIdentity();
	
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
	D3DXVec3Transform(&vCameraLocation,(D3DXVECTOR3 *)&vCameraSpace,&matFinal);

	//now we have the start and end points of our line

	D3DXVECTOR3 vNorm,vLine;
	D3DXVECTOR3 vCamNorm;

	vCameraOffset = vLookAt  - (D3DVECTOR)vCameraLocation;
	

	D3DXVec3Normalize(&vCamNorm,(D3DXVECTOR3 *)&vCameraOffset);

	//get the length of our camera offset

	float Length;

	Length = D3DXVec3Length((D3DXVECTOR3 *)&vCameraOffset);

	Length = Length * (vNear.z / vCameraLocation.z);

	D3DXVec3Scale(&vLine, &vCamNorm, Length);
	
	vLine.x += vNear.x;
	vLine.y += vNear.y;
	vLine.z += vNear.z;

	*vScreen = vLine;
}


//end: Accessors *******************************************************

//************ Mutators ************************************************
int World::Load(const char *filename)
{
	LastAutosave = timeGetTime();
	pOffScreenCreature = NULL;
	CleanX = 0;
	CleanY = 0;

	FILE *fp;
	fp = SafeFileOpen(filename,"rb");
	
	fread(&NumAreas,sizeof(int),1,fp);
	int n;
	for(n = 0; n < NumAreas; n++)
	{
		Areas[n] = new Area;
		Areas[n]->SetID(n);
		fread(Areas[n]->Header.Name,sizeof(char),32,fp);
	}
	fread(&NumRegions,sizeof(int),1,fp);
	fread(RegionOffsets,sizeof(int),1024,fp);

	RegionFile = SafeFileOpen("world.rgn","rb");

	fread(&NumRegions,sizeof(int),1,RegionFile);
	fread(RegionOffsets,sizeof(int),1024,RegionFile);
	
	for(n = 0; n < NumAreas; n++)
	{
		if(Areas[n]->Load())
		{
			Areas[n]->LoadRegions();
			Areas[n]->SetBaseTexture(Engine->GetTexture("caveterrain"));
			SetCurrentDirectory(".\\Areas");
			FILE *fp;
			char FileName[64];
			sprintf(FileName,"%s.dyn",Areas[n]->Header.Name);
			fp = SafeFileOpen(FileName, "rb");
			Areas[n]->LoadNonStatic(fp);
			Areas[n]->ClearDynamic(NULL, OBJECT_CREATURE);
			fclose(fp);
			SetCurrentDirectory(Engine->GetRootDirectory());
			DEBUG_INFO(Areas[n]->Header.Name);
			DEBUG_INFO("\n");
		}
		else
		{
		}
	}
	
	fclose(fp);

	Valley = Areas[0];
	Valley->SetBaseTexture(Engine->GetTexture("terrain"));

	LoadTowns();

	return TRUE;
}

int World::LookAt(Thing *pThing)
{
	int NewScreenX;
	int NewScreenY;
	D3DVECTOR *pVector;

	D3DVECTOR vCombined;
	D3DVECTOR vCameraOffset;

	if(pThing)
	{
		pVector = pThing->GetPosition();
		vCamOffset = _D3DVECTOR(0.0f,0.0f,0.0f);
		vCombined = *pVector;
	}

	else
	{
		pVector = PreludeParty.GetLeader()->GetPosition();
		vCombined = *pVector;
	}

	vCameraOffset.x = vCamera.x - vLookAt.x;
	vCameraOffset.y = vCamera.y - vLookAt.y;
	vCameraOffset.z = vCamera.z - vLookAt.z;

	vLookAt = vCombined;

	NewScreenX = (int)(vCombined.x / CHUNK_TILE_WIDTH);
	NewScreenY = (int)(vCombined.y / CHUNK_TILE_HEIGHT);

	UpdateRect.left = ((int)vCombined.x - 36) / UPDATE_SEGMENT_WIDTH;
	UpdateRect.right = ((int)vCombined.x + 36) / UPDATE_SEGMENT_WIDTH;
	UpdateRect.top = ((int)vCombined.y - 36) / UPDATE_SEGMENT_HEIGHT;
	UpdateRect.bottom = ((int)vCombined.y + 36) / UPDATE_SEGMENT_HEIGHT;

	if(UpdateRect.left < 0) 
		UpdateRect.left = 0;
	if(UpdateRect.top < 0) 
		UpdateRect.top = 0;
	if(UpdateRect.bottom >= Valley->UpdateHeight) 
		UpdateRect.bottom = Valley->UpdateHeight - 1;
	if(UpdateRect.right >= Valley->UpdateWidth) 
		UpdateRect.right = Valley->UpdateWidth - 1;

	if(NewScreenX < 0)
		NewScreenX = 0;

	if(NewScreenY < 0)
		NewScreenY = 0;

	//confirm all chunks extant
	if((abs(NewScreenX - ScreenX) + abs(NewScreenY - ScreenY)) > 3)
	{
		int Offset;	
		int StartX;
		int StartY;
		int EndX;
		int EndY;
		int xn, yn;
	
		StartX = NewScreenX - DrawRadius;
		StartY = NewScreenY - DrawRadius;
		
		EndX = NewScreenX + DrawRadius;
		EndY = NewScreenY + DrawRadius;

		if(StartX < 0)
			StartX = 0;

		if(StartY < 0)
			StartY = 0;

		if(EndX >= Valley->ChunkWidth)
			EndX = Valley->ChunkWidth - 1;

		if(EndY >= Valley->ChunkHeight)
			EndY = Valley->ChunkHeight - 1;
		
		Chunk *pChunk;
		//Engine->Graphics()->GetD3D()->BeginScene();
		
		for(yn = StartY; yn <= EndY; yn++)
		{
			Offset = yn * Valley->ChunkWidth;
			for(xn = StartX; xn <= EndX; xn++)
			{
				if(!Valley->BigMap[Offset + xn])
				{
					Valley->LoadChunk(xn,yn);
				}
				pChunk = Valley->GetChunk(xn,yn);
				//if(pChunk && !pChunk->GetTexture()) 
				//	pChunk->CreateTexture(Valley->GetBaseTexture());
				if(pChunk && !pChunk->IsConverted()) 
					pChunk->ConvertTerrain();
			}
		}

		//Engine->Graphics()->GetD3D()->EndScene();
	}

	ScreenX = NewScreenX;
	ScreenY = NewScreenY;

	vCamera.x = vLookAt.x + vCameraOffset.x;
	vCamera.y = vLookAt.y + vCameraOffset.y;
	vCamera.z = vLookAt.z + vCameraOffset.z;

	D3DXMatrixLookAt((D3DXMATRIX*)&matCamera, (D3DXVECTOR3*)&vCamera,
		(D3DXVECTOR3*)&vLookAt, (D3DXVECTOR3*)&vWorldUp);

	Engine->Graphics()->GetD3D()->SetTransform( D3DTRANSFORMSTATE_VIEW, &matCamera );
	
	ChangeCamera();
	return TRUE;
}

int World::Update()
{
	int xn; 
	int yn;
	int Offset;
	DWORD CurTime;
	
	Object *pOb;
	Object *pKill;
	Object *pCreature;
	int NotDone;

	D3DVECTOR *pVector;

	int StartX;
	int StartY;
	int EndX;
	int EndY;

	StartX = ScreenX - DrawRadius;
	StartY = ScreenY - DrawRadius;
	
	EndX = ScreenX + DrawRadius;
	EndY = ScreenY + DrawRadius;

	if(StartX < 0)
		StartX = 0;

	if(StartY < 0)
		StartY = 0;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth - 1;

	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight - 1;
	
	Chunk *pChunk;
	BOOL Loaded = FALSE;
	for(yn = StartY; (yn <= EndY && !Loaded); yn++)
	{
		Offset = yn * Valley->ChunkWidth;
		for(xn = StartX; (xn <= EndX && !Loaded); xn++)
		{
			if(!Valley->BigMap[Offset + xn])
			{
				Valley->LoadChunk(xn,yn);
				Loaded = TRUE;
			}
			else
			{
				pChunk = Valley->GetChunk(xn,yn);
				if(pChunk && !pChunk->IsConverted()) 
				{
					//Engine->Graphics()->GetD3D()->BeginScene();
					//pChunk->CreateTexture(Valley->GetBaseTexture());
					//Engine->Graphics()->GetD3D()->EndScene();
					pChunk->ConvertTerrain();
					Loaded = TRUE;
				}
			}
		}
	}
	
	//done loading a chunk if it's not there
	
	CurTime = timeGetTime();

	pOb = pMainObjects;

	while(pOb)
	{
		NotDone = pOb->AdvanceFrame();

		if(NotDone)
		{
			pOb = pOb->GetNext();
		}
		else
		{
			pKill = pOb;
			pOb = pOb->GetNext();
			RemoveMainObject(pKill);
			delete pKill;
		}
	}

	if(GameState == GAME_STATE_COMBAT)
	{
		//update everything that's in combat
		pCombat->Update();
		if(GameState != GAME_STATE_COMBAT)
			return TRUE;		
			
		//update everything no in combat, adding to combat if necessary
		for(yn = UpdateRect.top; yn <= UpdateRect.bottom; yn++)
		{
			for(xn = UpdateRect.left; xn <= UpdateRect.right; xn++)
			{
				Offset = xn + yn * Valley->UpdateWidth;

				pOb = Valley->UpdateSegments[Offset];

				while(pOb)
				{
					if(pOb->GetObjectType() == OBJECT_CREATURE)
					{
						pCreature = pOb;

						int cx, cy;
						cx = (int)pCreature->GetPosition()->x;
						cy = (int)pCreature->GetPosition()->y;
						if(cx > pCombat->rCombat.left && cy > pCombat->rCombat.top && cx < pCombat->rCombat.right - 1 && cy < pCombat->rCombat.bottom -1)
						{
							pOb = pOb->GetNextUpdate();
							Valley->RemoveFromUpdate(pCreature);
							pCombat->AddToCombat(pCreature);
							((Creature *)pCreature)->EnterCombat();
							((Creature *)pCreature)->StartRound();
							((Creature *)pCreature)->ClearActions();
						}
						
						else
						{
							NotDone = pOb->AdvanceFrame();

							if(NotDone)
							{
								pOb = pOb->GetNextUpdate();
							}
							else
							{
								pKill = pOb;
								pOb = pOb->GetNextUpdate();
								Valley->RemoveFromUpdate(pKill);
								
								if(pKill->GetObjectType() == OBJECT_CREATURE)
								{
									if(((Creature *)pKill)->GetLastPlacedTime() == -1)
									{
										delete pKill;
									}
								}
								else
								{
									delete pKill;
								}
							}	
						}
					}
					else
					{
						NotDone = pOb->AdvanceFrame();

						if(NotDone)
						{
							pOb = pOb->GetNextUpdate();
						}
						else
						{
							pKill = pOb;
							pOb = pOb->GetNextUpdate();
							Valley->RemoveFromUpdate(pKill);
							
							if(pKill->GetObjectType() == OBJECT_CREATURE)
							{
								if(((Creature *)pKill)->GetLastPlacedTime() == -1)
								{
									delete pKill;
								}
							}
							else
							{
								delete pKill;
							}
						}	
					}
				}
		
			}
		}

		return TRUE;
	}
	else
	{
	
		if(CurTime > NextAdvanceTime)
		{
			IncrementTime();
		}
		
		for(yn = UpdateRect.top; yn <= UpdateRect.bottom; yn++)
		{
			for(xn = UpdateRect.left; xn <= UpdateRect.right; xn++)
			{
				Offset = xn + yn * Valley->UpdateWidth;
				
				pOb = Valley->UpdateSegments[Offset];

				while(pOb)
				{
					NotDone = pOb->AdvanceFrame();
					//we may have entered combat, if so, stop updating and exit
					if(NotDone)
					{
						pOb = pOb->GetNextUpdate();
					}
					else
					{
						pKill = pOb;
						pOb = pOb->GetNextUpdate();
						Valley->RemoveFromUpdate(pKill);
						if(pKill->GetObjectType() == OBJECT_CREATURE)
						{
							if(((Creature *)pKill)->GetLastPlacedTime() == -1)
							{
								delete pKill;
							}
						}
						else
						{
							delete pKill;
						}
					}
					
					if(GameState == GAME_STATE_COMBAT)
					{
						return TRUE;
					}
					else
					if(GameState == GAME_STATE_INIT)
					{
						GameState = GAME_STATE_NORMAL;
						return TRUE;
					}

					
				}
				Offset++;
			}
			Offset+= Valley->UpdateWidth - (1 + UpdateRect.right - UpdateRect.left);
		}

		for(int n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			D3DVECTOR *pvLead;
			pvLead = PreludeParty.GetMember(n)->GetPosition();
			int LeadX, LeadY;
			LeadX = (int)pvLead->x / UPDATE_SEGMENT_WIDTH;
			LeadY = (int)pvLead->y / UPDATE_SEGMENT_HEIGHT;
			
			if(LeadX < UpdateRect.left || LeadX > UpdateRect.right ||
			   LeadY < UpdateRect.top || LeadY > UpdateRect.bottom)
			{
				NotDone = PreludeParty.GetMember(n)->AdvanceFrame();
				
				if(NotDone)
				{

				}
				else
				{
					Valley->RemoveFromUpdate(PreludeParty.GetMember(n));
					PreludeParty.RemoveMember(PreludeParty.GetMember(n));
				}
			}
		}
//autosave if neccessary
		if(AutosaveRate)
		{
			if((CurTime - LastAutosave) / 60000 >= AutosaveRate &&
				PreludeWorld->GetGameState() == GAME_STATE_NORMAL)
			{
				LastAutosave = CurTime;
				Describe("Autosaving");
				AutoSave();
			}
		}

		CurTime = GetTotalTime();

		if(!(CurTime % 5))
		{
			PreludeEvents.DoTimed(CurTime);
		}

		if(PreludeParty.HasChangedPosition())
		{
			PreludeParty.Occupy();
		}
	}
	return TRUE;
}

void World::ChangeCamera()
{
	Object *pOb;

	pOb = pMainObjects;
	while(pOb)
	{
		pOb->AdjustCamera();
		pOb = pOb->GetNext();
	}
/*	
	pOb = pDrawList;
	while(pOb)
	{
		pOb->AdjustCamera();
		pOb = pOb->GetNext();
	}
*/

	int xn; 
	int yn;
	int Offset;

	Offset = UpdateRect.left + UpdateRect.top * Valley->UpdateWidth;
	
	for(yn = UpdateRect.top; yn <= UpdateRect.bottom; yn++)
	{
		for(xn = UpdateRect.left; xn <= UpdateRect.right; xn++)
		{
			pOb = Valley->UpdateSegments[Offset];

			while(pOb)
			{
				pOb->AdjustCamera();

				pOb = pOb->GetNextUpdate();
			
			}
			Offset++;
		}
		Offset+= Valley->UpdateWidth - (1 + UpdateRect.right - UpdateRect.left);
	}

	float ViewDim;
	ViewDim = Engine->Graphics()->GetViewDim();

	DrawRadius = (int)(ViewDim/ 10.0f) + 1;	
	if(DrawRadius > 2)
	DrawRadius = 2;

	return;
}


int World::SetCameraPosition(D3DVECTOR NewPosition)
{

	return TRUE;
}

int World::SetCenterScreen(D3DVECTOR NewCenterScreen)
{
	return TRUE;
}

void World::SetCameraAngle(float NewAngle)
{
	RotateCamera(NewAngle - CameraAngle);
	return;
}

int World::RotateCamera(float Angle)
{
	CameraAngle += Angle;
	D3DXMATRIX matRotate;

	D3DXMatrixRotationZ(&matRotate,Angle);
	
	D3DXVECTOR4 vNew;
	D3DXVECTOR3 vOldCam;
	
	vOldCam.x = vCamera.x - vLookAt.x;
	vOldCam.y = vCamera.y - vLookAt.y;
	vOldCam.z = vCamera.z - vLookAt.z;

	D3DXVec3Transform(&vNew,(D3DXVECTOR3 *)&vOldCam,&matRotate);

    vCamera.x = vNew.x + vLookAt.x;
	vCamera.y = vNew.y + vLookAt.y;
	vCamera.z = vNew.z + vLookAt.z;
	
	D3DXMatrixLookAt((D3DXMATRIX*)&matCamera, (D3DXVECTOR3*)&vCamera,
			(D3DXVECTOR3*)&vLookAt, (D3DXVECTOR3*)&vWorldUp);

	Engine->Graphics()->GetD3D()->SetTransform( D3DTRANSFORMSTATE_VIEW, &matCamera );

	ChangeCamera();
	return TRUE;

}

//end: Mutators ********************************************************



//************ Outputs *************************************************
int World::Save(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"wb");
	
	fwrite(&NumAreas,sizeof(int),1,fp);
	int n;
	for(n = 0; n < NumAreas; n++)
	{
		fwrite(Areas[n]->Header.Name,sizeof(char),32,fp);
	}
	fwrite(&NumRegions,sizeof(int),1,fp);
	fwrite(RegionOffsets,sizeof(int),1024,fp);
	
	fclose(fp);
	return TRUE;
}

void World::SaveRegions(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"wb");

	fwrite(&NumRegions,sizeof(int),1,fp);
	fwrite(RegionOffsets,sizeof(int),1024,fp);
	RegionOffsets[0] = 0;
	
	int n;
	for(n = 1; n <= NumRegions; n++)
	{
		if(!Regions[n])
		{
			LoadRegion(n);
		}
		if(Regions[n])
		{
			RegionOffsets[n] = ftell(fp);
			Regions[n]->Save(fp);
		}
		else
		{
			RegionOffsets[n] = 0;
		}
	}
	for(;n < MAX_WORLD_REGIONS; n++)
	{
		RegionOffsets[n] = 0;
	}

//	memcpy(&RegionHeader,&NewHeader,sizeof(RegionHeader));

	fseek(fp,0,SEEK_SET);
	
//	fwrite(&NewHeader,sizeof(NewHeader),1,fp);

	fwrite(&NumRegions,sizeof(int),1,fp);
	fwrite(RegionOffsets,sizeof(int),1024,fp);
	
	fclose(fp);
	if(RegionFile)
	{
		fclose(RegionFile);
		remove("worldreg.old");
		rename("world.rgn","worldreg.old");
	}
	rename(filename,"world.rgn");
	RegionFile = SafeFileOpen("world.rgn","rb");
	return;
}
  
//end: Outputs ********************************************************



//************ Debug ***************************************************
//AddToDrawList
//assumes that the list has not been added to the world draw list

int World::AddMainObject(Object *pToAdd)
{
	pToAdd->SetNext(pMainObjects);
	pMainObjects = pToAdd;
	return TRUE;
}

int World::RemoveMainObject(Object *pToRemove)
{
	Object *pOb, *pLOb;

	pOb = pMainObjects;

	if(pOb == pToRemove)
	{
		pMainObjects = pOb->GetNext();	
		return TRUE;
	}

	while(pOb && pOb != pToRemove)
	{
		pLOb = pOb;
		pOb = pOb->GetNext();
		
	}

	if(!pOb)
	{ 
		return FALSE;
	}
	
	pLOb->SetNext(pOb->GetNext());
	pOb->SetNext(NULL);

	return TRUE;
}

void World::LoadRegion(int Num)
{
	if(!RegionFile)
	{
		return;
	}

	if(Regions[Num])
	{
		delete Regions[Num];
		Regions[Num] = NULL;
	}
	
	//position the file pointer
	int Error;
	if(!RegionOffsets[Num])
	{
		return;
	}
	
	Regions[Num] = new Region;
	
	Error = fseek(RegionFile, RegionOffsets[Num], SEEK_SET);

	assert(!Error);
	Regions[Num]->Load(RegionFile);
	Regions[Num]->CreateDrawList();
	if(!Regions[Num]->IsOccupied())
	{
		Regions[Num]->SetOccupancy(REGION_OCCUPIED);
	}
	else
	{
	}
}

int World::GetNextFreeRegion()
{
	int n; 
	for(n = 1; n < MAX_WORLD_REGIONS; n++)
	{
		if(!RegionOffsets[n])
		{
			return n;
		}
	}
	SafeExit("TOO MANY REGIONS IN WORLD!\n");
	return NULL;
}

void World::AddRegion(Region *ToAdd)
{
	int n;

	if(!ToAdd->GetID())
	{
		//find the next valid region ID;
		n = 1;
		while(RegionOffsets[n])
			n++;
		if(n > NumRegions)
		{
			NumRegions++;
		}
		ToAdd->SetID(n);
		RegionOffsets[n] = 1;
		
	}

	Regions[ToAdd->GetID()] = ToAdd;

	Valley->AddRegion(ToAdd);
}

void World::RemoveRegion(int RemoveID)
{
	RegionOffsets[RemoveID] = 0;
	Region *pRegion;
	pRegion = Regions[RemoveID];
	Regions[RemoveID] = NULL;
	
	Valley->RemoveRegion(pRegion);
	delete pRegion;

}

void World::ClearRegions()
{

}

void World::SaveGame(const char *filename, char *GameID)
{
	this->LastAutosave = timeGetTime();
	unsigned long Pos;
	char blarg[64];
	FILE *fp;
	fp = SafeFileOpen("temp.sav","wb");

	if(GameID)
	{
		strcpy(blarg,GameID);
	}
	else
	{
		sprintf(blarg,"new");
	}

	fwrite(blarg,sizeof(char),64,fp);

	//save date and time
	fwrite(&Hour, sizeof(int),1,fp);
	fwrite(&TotalTime, sizeof(int),1,fp);

	//save our the view matrixes and zoom
	float fViewDim;
	fViewDim = Engine->Graphics()->GetViewDim();

	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)&matView);
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)&matProj);

	fwrite (&matView, sizeof(D3DXMATRIX),1,fp);
	fwrite (&matProj, sizeof(D3DXMATRIX),1,fp);

	fwrite(&fViewDim,sizeof(float),1,fp);
	
	DEBUG_INFO("Savegame creatures\n");
	SaveBinCreatures(fp);
	Pos = ftell(fp);
	sprintf(blarg,"\n%i\n",Pos);
	DEBUG_INFO(blarg);

	DEBUG_INFO("Savegame party\n");
	fwrite(&Pos,sizeof(unsigned long),1,fp);
	PreludeParty.Save(fp);
	Pos = ftell(fp);
	sprintf(blarg,"\n%i\n",Pos);
	DEBUG_INFO(blarg);

	DEBUG_INFO("Savegame flags\n");
	fwrite(&Pos,sizeof(unsigned long),1,fp);
	PreludeFlags.Save(fp);
	Pos = ftell(fp);
	sprintf(blarg,"\n%i\n",Pos);
	DEBUG_INFO(blarg);

	DEBUG_INFO("Savegame events\n");
	fwrite(&Pos,sizeof(unsigned long),1,fp);
	PreludeEvents.Save(fp);
	Pos = ftell(fp);
	sprintf(blarg,"\n%i\n",Pos);
	DEBUG_INFO(blarg);

	DEBUG_INFO("Savegame nonstatic\n");
	fwrite(&Pos,sizeof(unsigned long),1,fp);
	
	this->SaveDynamic(fp);
	
	Pos = ftell(fp);
	
	sprintf(blarg,"\n%i\n",Pos);
	DEBUG_INFO(blarg);
	fwrite(&Pos,sizeof(unsigned long),1,fp);

	DEBUG_INFO("Saving current area\n");

	fwrite(Valley->Header.Name,sizeof(char),32,fp);

	DEBUG_INFO("Saveing Music status\n");
	int MusicOn;
	int MusicNum;
	MusicOn = Engine->Sound()->MusicIsOn();

	fwrite(&MusicOn,sizeof(int),1,fp);
	if(MusicOn)
	{
		MusicNum = Engine->Sound()->GetMusicPlaying();
		fwrite(&MusicNum,sizeof(int),1,fp);
	}

	//count modifiers;
	DEBUG_INFO("Saving modifiers\n");
	int NumMods = 0;
	Object *pOb;
	pOb = this->GetMainObjects();
	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_MODIFIER)
		{
			NumMods++;
		}
		pOb = pOb->GetNext();
	}
	
	int fileloc;
	fileloc = ftell(fp);

	fwrite(&NumMods,sizeof(int),1,fp);
	

	pOb = this->GetMainObjects();
	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_MODIFIER)
		{
			pOb->Save(fp); 
		}
		pOb = pOb->GetNext();
	}

	fwrite(&CreationHelp, sizeof(BOOL),1,fp);
	fwrite(&CharacterHelp, sizeof(BOOL),1,fp);
	fwrite(&CombatHelp, sizeof(BOOL),1,fp);
	fwrite(&BarterHelp, sizeof(BOOL),1,fp);
	fwrite(&MapHelp, sizeof(BOOL),1,fp);
	fwrite(&MainHelp, sizeof(BOOL),1,fp);
	fwrite(&RestHelp, sizeof(BOOL),1,fp);



	fclose(fp);

	fp = fopen(filename,"rb");
	if(fp)
	{
		fclose(fp);
		remove(filename);
	}

	rename("temp.sav",filename);

	DEBUG_INFO("DONE saving game\n");

	GameState = GAME_STATE_INIT;

}

void World::LoadGame(const char *filename)
{
	pOffScreenCreature = NULL;
	CleanX = 0;
	CleanY = 0;

	PreludeEvents.Clear();
	PreludeEvents.UnsetCreaturePointers();
	PreludeWorld->GetCombat()->Kill();
	PreludeWorld->SetGameState(GAME_STATE_NORMAL);
	Engine->Sound()->StopMusic();

	unsigned long Pos, OldPos;
	char blarg[64];
	FILE *fp;
	fp = SafeFileOpen(filename,"rb");

	Object *pOb;
	int Offset = 0;
	int n;

	Creature *pCreature;

	for(n = 0; n < NumAreas; n++)
	{
		Areas[n]->Clear();
	}

	//clear out the party members
	while(PreludeParty.GetNumMembers())
	{
		pCreature = PreludeParty.GetMember(0);
		PreludeParty.RemoveMember(pCreature);
	}
	
	DeleteCreatures();
	
	Object *pMiniMap = NULL;
	
	//clear all current update objects
	//needs to account for active spellsy
	while(pMainObjects)
	{
		pOb = pMainObjects;
		RemoveMainObject(pOb);
		if(pOb->GetObjectType() != OBJECT_MINIMAP)
		{
			delete pOb;
		}
		else
		{
			pMiniMap = pOb;
		}
	}

	//get the file ID
	fread(blarg,sizeof(char),64,fp);

	//get date/time
	fread(&Hour, sizeof(int),1,fp);
	fread(&TotalTime, sizeof(int),1,fp);

	PreludeEvents.SetLastRandomTime(TotalTime);
	

	//save our the view matrixes and zoom
	float fViewDim;

	D3DXMATRIX matView;
	D3DXMATRIX matProj;

	fread (&matView, sizeof(D3DXMATRIX),1,fp);
	fread (&matProj, sizeof(D3DXMATRIX),1,fp);

	fread(&fViewDim,sizeof(float),1,fp);
	
	Engine->Graphics()->SetViewDim(fViewDim);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)&matView);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)&matProj);

	DEBUG_INFO("Loading Creatures\n");
	LoadBinCreatures(fp);
	Pos = ftell(fp);
	sprintf(blarg,"%i\n",Pos);
	DEBUG_INFO(blarg);

	fread(&OldPos,sizeof(unsigned long),1,fp);
	assert(Pos == OldPos);
	
	DEBUG_INFO("Loading Party\n");


	PreludeParty.Load(fp);
		
	Pos = ftell(fp);
	sprintf(blarg,"%i\n",Pos);
	DEBUG_INFO(blarg);

	fread(&OldPos,sizeof(unsigned long),1,fp);
	assert(Pos == OldPos);

	DEBUG_INFO("Loading Flags\n");\
	PreludeFlags.Load(fp);
	Pos = ftell(fp);
	sprintf(blarg,"%i\n",Pos);
	DEBUG_INFO(blarg);
	fread(&OldPos,sizeof(unsigned long),1,fp);
	assert(Pos == OldPos);

	DEBUG_INFO("Loading Eventss\n");
	PreludeEvents.Load(fp);
	PreludeEvents.SetCreaturePointers();
	Pos = ftell(fp);
	
	fread(&OldPos,sizeof(unsigned long),1,fp);
	assert(Pos == OldPos);

	DEBUG_INFO("Loading Non Static.\n");

	LoadDynamic(fp);	
		
	Pos = ftell(fp);
	sprintf(blarg,"%i\n",Pos);
	DEBUG_INFO(blarg);

	fread(&OldPos,sizeof(unsigned long),1,fp);
	assert(Pos == OldPos);

	char CurAreaName[32];

	fread(CurAreaName,sizeof(char),32,fp);

	//count modifiers;
	int NumMods = 0 ;

	DEBUG_INFO("Loading Modifiers\n");
	
	

	this->CurAreaNum = GetAreaNum(CurAreaName);

	Valley = Areas[CurAreaNum];

	//load the area around the party;
	int xStart;
	int yStart;
	int xEnd;
	int yEnd;

	xStart = PreludeParty.GetLeader()->GetPosition()->x / CHUNK_TILE_WIDTH - 1;
	yStart = PreludeParty.GetLeader()->GetPosition()->y / CHUNK_TILE_WIDTH - 1;
	
	xEnd = xStart + 2;
	yEnd = yStart + 2;

	//Engine->Graphics()->GetD3D()->BeginScene();
	
	int xn, yn;
	for(yn = yStart; yn <= yEnd; yn++)
	for(xn = xStart; xn <= xEnd; xn++)
	{
		if(Valley->GetChunk(xn,yn))
		{
			Valley->RemoveChunk(xn,yn);
		}
		Valley->LoadChunk(xn,yn);
		if(Valley->GetChunk(xn,yn))
		{
			//	Valley->GetChunk(xn,yn)->CreateTexture(Valley->GetBaseTexture());
			Valley->GetChunk(xn,yn)->ConvertTerrain();
		}
	}

	//Engine->Graphics()->GetD3D()->EndScene();

	for(n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		//	PreludeParty.GetMember(n)->AddToWorld();
		//create a portrait window for this creature
		//add it to the main window
		ZSPortrait *pPortrait;
		pPortrait = ((ZSMenuBar *)ZSWindow::GetMain()->GetChild(IDC_MAIN_MENUBAR))->GetPortrait(n);
		pPortrait->SetTarget(PreludeParty.GetMember(n));
		pPortrait->Show();
		PreludeParty.GetMember(n)->SetPortrait(pPortrait);
	
		PreludeParty.GetMember(n)->CreateTexture();
		PreludeParty.GetMember(n)->SetEquipObjects();
		PreludeParty.GetMember(n)->InsertAction(ACTION_IDLE,NULL,NULL);
		PreludeParty.GetMember(n)->SetWalkFrames();
		
		if(PreludeParty.GetMember(n)->GetAreaIn() >= 0)
		{
			PreludeParty.GetMember(n)->SetRegionIn(PreludeWorld->GetArea(PreludeParty.GetMember(n)->GetAreaIn())->GetRegion(PreludeParty.GetMember(n)->GetPosition()));
			PreludeWorld->GetArea(PreludeParty.GetMember(n)->GetAreaIn())->AddToUpdate(PreludeParty.GetMember(n));
		}
	}

	((ZSMenuBar *)ZSWindow::GetMain()->GetChild(IDC_MAIN_MENUBAR))->SetPortraits();
	
	((ZSMainWindow *)ZSWindow::GetMain())->SetTarget(NULL);
	((ZSMainWindow *)ZSWindow::GetMain())->ShowInventory();
	
	this->LookAt(PreludeParty.GetLeader());

	DEBUG_INFO("loading Music status\n");
	int MusicOn;
	int MusicNum;
	fread(&MusicOn,sizeof(int),1,fp);
	if(MusicOn)
	{
		fread(&MusicNum,sizeof(int),1,fp);
		Engine->Sound()->PlayMusic(MusicNum);
	}


	DEBUG_INFO("Done loading game\n");
	GameState = GAME_STATE_INIT;
	if(pMiniMap)
	{
	
		AddMainObject(pMiniMap);
		pMiniMap->AdjustCamera();
		((MiniMapObject *)pMiniMap)->Unlock();
	}

	if(!CurAreaNum)
		SetLight(GetLight(GetHour()));
	else
		SetLight(GetLight(12));

	fread(&NumMods,sizeof(int),1,fp);

	if(NumMods && !feof(fp))
	{
		for(n = 0; n < NumMods; n++)
		{
			pOb = LoadObject(fp);
			AddMainObject(pOb);
		}
	}
	else
	{
		DEBUG_INFO("modifiers bypassed\n");
	}

	if(!feof(fp))
	{
		fread(&CreationHelp, sizeof(BOOL),1,fp);
		fread(&CharacterHelp, sizeof(BOOL),1,fp);
		fread(&CombatHelp, sizeof(BOOL),1,fp);
		fread(&BarterHelp, sizeof(BOOL),1,fp);
		fread(&MapHelp, sizeof(BOOL),1,fp);
		fread(&MainHelp, sizeof(BOOL),1,fp);
		fread(&RestHelp, sizeof(BOOL),1,fp);
	}
		
	
	fclose(fp);



	ZSDescribe *pDescribe;

	pDescribe = (ZSDescribe *)ZSWindow::GetMain()->GetChild(IDC_MAIN_MENUBAR)->GetChild(1);

	pDescribe->Clear();
}

void World::IncrementTime()
{ 
	int LastHour;
	int NewHour;
	LastHour = Hour;
	
	NewHour = (TotalTime / HOUR_LENGTH) % 24;

	if(NewHour != LastHour && Valley == Areas[0])
	{
		SetLight(GetLight(NewHour));
		Hour = NewHour;
	}

	if(!NewHour && NewHour != LastHour)
	{
		PreludeEvents.RunEvent(DAILY_EVENT);
	}

	TotalTime++; 
	NextAdvanceTime = timeGetTime() + TIME_PASS_SPEED;
}

void World::UpdateCameraOffset(D3DVECTOR UpdateRay)
{ 
	float CameraLength;
	D3DVECTOR vBase;
	D3DVECTOR vOldLookAt;
	vOldLookAt = vLookAt;
	vBase = *PreludeParty.GetLeader()->GetPosition();

	CameraLength = Magnitude((vLookAt + UpdateRay) - vBase);

	if(CameraLength < MAXIMUM_CAMERA_DISTANCE)
	{
		vLookAt += UpdateRay; 
	}
	else
	{
		vLookAt += UpdateRay;
		vCamOffset = Normalize(vLookAt - vBase);
		vCamOffset *= MAXIMUM_CAMERA_DISTANCE;
		vLookAt = vBase + vCamOffset;
	}

	if(!(vOldLookAt == vLookAt))
	{
		int NewScreenX;
		int NewScreenY;

		D3DVECTOR vCameraOffset;

		vCameraOffset.x = vCamera.x - vOldLookAt.x;
		vCameraOffset.y = vCamera.y - vOldLookAt.y;
		vCameraOffset.z = vCamera.z - vOldLookAt.z;

		NewScreenX = (int)(vLookAt.x / CHUNK_TILE_WIDTH);
		NewScreenY = (int)(vLookAt.y / CHUNK_TILE_HEIGHT);

		UpdateRect.left = ((int)vLookAt.x - 36) / UPDATE_SEGMENT_WIDTH;
		UpdateRect.right = ((int)vLookAt.x + 36) / UPDATE_SEGMENT_WIDTH;
		UpdateRect.top = ((int)vLookAt.y - 36) / UPDATE_SEGMENT_HEIGHT;
		UpdateRect.bottom = ((int)vLookAt.y + 36) / UPDATE_SEGMENT_HEIGHT;

		if(UpdateRect.left < 0)
			UpdateRect.left = 0;

		if(UpdateRect.top < 0)
			UpdateRect.top = 0;

		if(UpdateRect.right >= Valley->UpdateWidth)
			UpdateRect.right = Valley->UpdateWidth - 1;

		if(UpdateRect.bottom >= Valley->UpdateHeight)
			UpdateRect.bottom = Valley->UpdateHeight - 1;

		if(NewScreenX < 0)
			NewScreenX = 0;

		if(NewScreenY < 0)
			NewScreenY = 0;

		int Offset;	
		int StartX;
		int StartY;
		int EndX;
		int EndY;
	
		int OldStartX;
		int OldStartY;
		int OldEndX;
		int OldEndY;

		int xn, yn;
	
		StartX = NewScreenX - DrawRadius;
		StartY = NewScreenY - DrawRadius;
		EndX = NewScreenX + DrawRadius;
		EndY = NewScreenY + DrawRadius;

		if(StartX < 0)
			StartX = 0;

		if(StartY < 0)
			StartY = 0;

		if(EndX >= Valley->ChunkWidth)
			EndX = Valley->ChunkWidth - 1;
		
		if(EndY >= Valley->ChunkHeight)
			EndY = Valley->ChunkHeight - 1;

		if(NewScreenX != ScreenX || NewScreenY != ScreenY)
		{
			int OldStartX;
			int OldStartY;
			int OldEndX;
			int OldEndY;

			int xn, yn;
		
			OldStartX = ScreenX - DrawRadius;
			OldStartY = ScreenY - DrawRadius;
			OldEndX = ScreenX + DrawRadius;
			OldEndY = ScreenY + DrawRadius;

			if(OldStartX < 0)
				OldStartX = 0;

			if(OldStartY < 0)
				StartY = 0;

			if(OldEndX >= Valley->ChunkWidth)
				OldEndX = Valley->ChunkWidth - 1;
			
			if(OldEndY >= Valley->ChunkHeight)
				OldEndY = Valley->ChunkHeight - 1;

			for(yn = OldStartY; yn <= OldEndY; yn++)
			{
				for(xn = OldStartX; xn <= OldEndX; xn++)
				{
					if((xn < StartX || xn > EndX || yn < StartY || yn > EndY)
						)
					{
						Chunk *pChunk;
						pChunk = Valley->GetChunk(xn,yn);
						if(pChunk)
							pChunk->ReleaseTexture();
					}
				}
			}
	
		//confirm all chunks extant
			if((abs(NewScreenX - ScreenX) + abs(NewScreenY - ScreenY)) >= 3)
			{
				for(yn = StartY; yn <= EndY; yn++)
				{
					Offset = yn * Valley->ChunkWidth;
					for(xn = StartX; xn <= EndX; xn++)
					{
						Chunk *pChunk = Valley->BigMap[Offset + xn];
						if(!pChunk)
						{
						//	Engine->Graphics()->GetD3D()->BeginScene();
							Valley->LoadChunk(xn,yn);
							if(Valley->GetChunk(xn,yn))
							{
								Valley->GetChunk(xn,yn)->ConvertTerrain();
							}
								//Valley->GetChunk(xn,yn)->CreateTexture(Valley->GetBaseTexture());
						//	Engine->Graphics()->GetD3D()->EndScene();
						}
						else
						{
							if(!pChunk->IsConverted())
							{
								pChunk->ConvertTerrain();
							}
							/*
							if(!pChunk->GetTexture())
							{
								Engine->Graphics()->GetD3D()->BeginScene();
								pChunk->CreateTexture(Valley->GetBaseTexture());
								Engine->Graphics()->GetD3D()->EndScene();
							}
							*/
						}
					}
				}
			}

		}

		ScreenX = NewScreenX;
		ScreenY = NewScreenY;

		vCamera.x = vLookAt.x + vCameraOffset.x;
		vCamera.y = vLookAt.y + vCameraOffset.y;
		vCamera.z = vLookAt.z + vCameraOffset.z;

		D3DXMatrixLookAt((D3DXMATRIX*)&matCamera, (D3DXVECTOR3*)&vCamera,
			(D3DXVECTOR3*)&vLookAt, (D3DXVECTOR3*)&vWorldUp);

		Engine->Graphics()->GetD3D()->SetTransform( D3DTRANSFORMSTATE_VIEW, &matCamera );
		
		ChangeCamera();
	}
}

void World::ArcCamera(float ArcAmount)
{
	//get the current camera ray
	D3DVECTOR vCameraRay;
	D3DXVECTOR3 vAxis;
	D3DXVECTOR4 vNewCamera;
	D3DXMATRIX matRotate;

	vCameraRay = vCamera - vLookAt;
		
	vAxis.x = -vCameraRay.y;
	vAxis.y = vCameraRay.x;
	vAxis.z = 0.0f;
	//get the current valley angle
	
	D3DXMatrixRotationAxis(&matRotate,&vAxis,ArcAmount);

	D3DXVec3Transform(&vNewCamera,(D3DXVECTOR3 *)&vCameraRay,&matRotate);
	if(vNewCamera.z > 9.0f && vNewCamera.z < 27.0f)
	{
		vCamera.x = vLookAt.x + vNewCamera.x;
		vCamera.y = vLookAt.y + vNewCamera.y;
		vCamera.z = vLookAt.z + vNewCamera.z;

		D3DXMatrixLookAt((D3DXMATRIX*)&matCamera, (D3DXVECTOR3*)&vCamera,
			(D3DXVECTOR3*)&vLookAt, (D3DXVECTOR3*)&vWorldUp);

		Engine->Graphics()->GetD3D()->SetTransform( D3DTRANSFORMSTATE_VIEW, &matCamera );
			
		ChangeCamera();
	}

}

char *World::GetTown(int x, int y)
{
	for(int n = 0; n < NumTowns; n ++)
	{
		if( (x >= TownAreas[n].left && x <= TownAreas[n].right) &&
			(y >= TownAreas[n].top && y <= TownAreas[n].bottom))
		{
			return &TownNames[n][0];
		}
	}
	return NULL;
}

char *World::GetMusic(int x, int y)
{
	for(int n = 0; n < NumTowns; n ++)
	{
		//expand areas around towns for purposes of determining music
		if( (x >= TownAreas[n].left - 128 && x <= TownAreas[n].right + 128) &&
			(y >= TownAreas[n].top - 128 && y <= TownAreas[n].bottom + 128))
		{
			return &TownMusic[n][0];
		}
	}
	return NULL;
}


void World::LoadTowns()
{
	FILE *fp;
	int n;
	fp = fopen ("towns.bin","rb");
	if(fp)
	{
		fread(&NumTowns,sizeof(int),1,fp);
		for(n = 0; n < NumTowns; n++)
		{
			fread(&TownNames[n][0],sizeof(char),32,fp);
			fread(&TownAreas[n],sizeof(RECT),1,fp);
			fread(&TownMusic[n][0],sizeof(char),32,fp);
		}
	
		fclose(fp);
	}
	else
	{	
		fp = SafeFileOpen("towns.txt","rt");
		SeekTo(fp,"NumTowns:");
		NumTowns = GetInt(fp);
		for(n = 0; n < NumTowns; n++)
		{
			SeekTo(fp,"Name:");
			GetString(fp,&TownNames[n][0]);
			SeekTo(fp,"Area:");
			TownAreas[n].left = GetInt(fp);
			TownAreas[n].top = GetInt(fp);
			TownAreas[n].right = GetInt(fp);
			TownAreas[n].bottom = GetInt(fp);
			SeekTo(fp,"Music:");
			GetString(fp,&TownMusic[n][0]);
		}
	

		fclose(fp);
		fp = SafeFileOpen ("towns.bin","wb");
		fwrite(&NumTowns,sizeof(int),1,fp);
		for(n = 0; n < NumTowns; n++)
		{
			fwrite(&TownNames[n][0],sizeof(char),32,fp);
			fwrite(&TownAreas[n],sizeof(RECT),1,fp);
			fwrite(&TownMusic[n][0],sizeof(char),32,fp);
		}
		
		fclose(fp);
	}

	LoadRoads();



	DEBUG_INFO("Towns and road locs loaded\n");
}

int World::PseudoRand(int modifier)
{
	int randreturn = rand();
	return randreturn;
}

Object *World::GetActive()
{
	if(GameState != GAME_STATE_COMBAT)
	{
		return PreludeParty.GetLeader();
	}
	else
	{
		return pCombat->GetActiveCombatant();;
	}
	return NULL;
}

void World::SaveDynamic(FILE *fp)
{
	fwrite(&NumAreas,sizeof(int),1,fp);


	int n;
	for(n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		Valley->RemoveFromUpdate((Object *)PreludeParty.GetMember(n));
	}

	for(n = 0; n < NumAreas; n++)
	{
		Areas[n]->SaveNonStatic(fp);
	}

	for(n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		Valley->AddToUpdate((Object *)PreludeParty.GetMember(n));
	}


}

void World::LoadDynamic(FILE *fp)
{
	fread(&NumAreas,sizeof(int),1,fp);

	for(int n = 0; n < NumAreas; n++)
	{
		Areas[n]->LoadNonStatic(fp);
	}
}

void World::BringUpToDate()
{
	//start with a reimport for the hell of it
	
	//create a new region called the valley
	Area *pValley;
	
	pValley = new Area();

	sprintf(pValley->Header.Name,"valley");

	
	FILE *fp, *fpnew;
	fp = SafeFileOpen("world.bin","rb");
	fpnew = SafeFileOpen("valley.bin","wb");

	OLD_STATIC_FILE_HEADER_T *OldHeader;
	OldHeader = new OLD_STATIC_FILE_HEADER_T;

	fread(OldHeader,sizeof(OLD_STATIC_FILE_HEADER_T), 1, fp);

	//strcpy(pValley->Header.Name,OldHeader->Name);
	pValley->Header.Width = OldHeader->Width;
	pValley->Header.Height = OldHeader->Height;
	pValley->Header.ChunkWidth = OldHeader->ChunkWidth;
	pValley->Header.ChunkHeight = OldHeader->ChunkHeight;

//	fread(pValley->Header.Name,sizeof(char), 32, fp);
//	fread(&pValley->Header.Width,sizeof(int), 1, fp);
//	fread(&pValley->Header.Height,sizeof(int), 1, fp);
//	fread(&pValley->Header.ChunkWidth,sizeof(int), 1, fp);
//	fread(&pValley->Header.ChunkHeight,sizeof(int), 1, fp);

	pValley->Header.Width = pValley->Width = WORLD_CHUNK_WIDTH * CHUNK_TILE_WIDTH;
	pValley->Header.Height = pValley->Height = WORLD_CHUNK_HEIGHT * CHUNK_TILE_HEIGHT;
	pValley->ChunkHeight =pValley->Header.ChunkHeight = WORLD_CHUNK_HEIGHT;
	pValley->ChunkWidth =pValley->Header.ChunkWidth = WORLD_CHUNK_WIDTH;
	pValley->UpdateWidth = WORLD_USEG_WIDTH;
	pValley->UpdateHeight = WORLD_USEG_HEIGHT;
	pValley->Header.ChunkOffsets = new int[WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT];
//	fread(pValley->Header.ChunkOffsets, sizeof(int), WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT, fp);

	memcpy(pValley->Header.ChunkOffsets,OldHeader->ChunkOffsets,sizeof(int) * WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT);

	delete OldHeader;

	fwrite(pValley->Header.Name, sizeof(char), 32, fpnew);
	fwrite(&pValley->Header.Width, sizeof(int), 1, fpnew);
	fwrite(&pValley->Header.Height, sizeof(int), 1, fpnew);
	fwrite(&pValley->Header.ChunkWidth, sizeof(int), 1, fpnew);
	fwrite(&pValley->Header.ChunkHeight, sizeof(int), 1, fpnew);

	fpos_t posOffsets;
	fgetpos(fpnew, &posOffsets);
	fwrite(pValley->Header.ChunkOffsets, sizeof(int), WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT, fpnew);

	Chunk *pChunk;

	for(int n = 0; n < WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT; n++)
	{
		if(pValley->Header.ChunkOffsets[n])
		{
			fseek(fp, pValley->Header.ChunkOffsets[n], SEEK_SET);
			pValley->Header.ChunkOffsets[n] = ftell(fpnew);
			pChunk = new Chunk;
			pChunk->Load(fp);	
			pChunk->Save(fpnew);
			delete pChunk;
		}
	}
	fsetpos(fpnew, &posOffsets);
	fwrite(pValley->Header.ChunkOffsets,sizeof(int),WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT, fpnew);

	fclose(fp);
	fclose(fpnew);

	pValley->Regions = new unsigned short[WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT * NUM_CHUNK_REGIONS];

	FILE *fpreg;

	fp = SafeFileOpen("oldworld.rgn","rb");
	fpreg = SafeFileOpen("world.rgn","wb");

	REGION_FILE_HEADER_T *RegHead;
	RegHead = new REGION_FILE_HEADER_T;

	fread(RegHead,sizeof(REGION_FILE_HEADER_T),1,fp);

	memcpy(pValley->Regions, RegHead->Regions, sizeof(unsigned short) * WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT * NUM_CHUNK_REGIONS);

	NumRegions = RegHead->NumRegions;

	RegionOffsets[0] = 0;
	Region *pRegion;

	fwrite(&NumRegions,sizeof(NumRegions),1,fpreg);

	fgetpos(fpreg, &posOffsets);
	fwrite(RegHead->RegionOffsets, sizeof(int), 1024, fpreg);
	
	for(n = 1; n <= NumRegions; n++)
	{
		if(RegHead->RegionOffsets[n])
		{
			pRegion = new Region;
			fseek(fp,RegHead->RegionOffsets[n],SEEK_SET);
			pRegion->Load(fp);
			RegionOffsets[n] = ftell(fpreg);
			pRegion->Save(fpreg);
			delete pRegion;
			
		}
		else
		{
			RegionOffsets[n] = 0;
		}
	}

	ZeroMemory(Regions,sizeof(Region *)*1024);


	fsetpos(fpreg,&posOffsets);
	fwrite(RegionOffsets,sizeof(int),1024,fpreg);
	fclose(fp);
	fclose(fpreg);

	fp = SafeFileOpen("valley.rgn","wb");

	fwrite(pValley->Regions,sizeof(unsigned short),WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT * NUM_CHUNK_REGIONS, fp);	

	fclose(fp);

	RegionFile = SafeFileOpen("World.rgn","rb");

	delete RegHead;

	fp = SafeFileOpen("dynamic.bin","rb");

	pValley->UpdateSegments = new Object *[WORLD_USEG_WIDTH * WORLD_USEG_HEIGHT];
	ZeroMemory(pValley->UpdateSegments,sizeof(Object *) * (WORLD_USEG_WIDTH * WORLD_USEG_HEIGHT));

	int Offset;
	int NumObjects;
	Object *pOb;
	for(Offset = 0; Offset < (WORLD_USEG_WIDTH * WORLD_USEG_HEIGHT); Offset++)
	{
		NumObjects = 0;
		
		fread(&NumObjects,sizeof(int),1,fp);

		for(n = 0; n < NumObjects; n++)
		{
			pOb = LoadObject(fp);
			if(pOb)
			{
				pValley->AddToUpdate(pOb);
			}
		}
	}

	fclose(fp);

	fp = SafeFileOpen("valley.dyn","wb");

	pValley->SaveNonStatic(fp);

	fclose(fp);

	Areas[0] = pValley;
	NumAreas = 1;

	this->Save("worldbase.bin");

	Valley = pValley;
	Valley->BigMap = new Chunk *[200*200];
	ZeroMemory(Valley->BigMap,sizeof(Chunk *) * 200 * 200);

	Valley->ChunkWidth = 200;
	Valley->ChunkHeight = 200;
	Valley->UpdateWidth = 3200 / 64;
	Valley->UpdateHeight = 3200 / 64;

}

void World::GotoArea(int AreaNum, int x, int y)
{
	CurAreaNum = AreaNum;
	CleanX = 0;
	CleanY = 0;

	if(AreaNum)
	{
		//unset any location on the compass/minimap
		MiniMap *pMinMap;
		pMinMap = (MiniMap *)ZSWindow::GetMain()->GetChild(MINIMAP_ID);

		//fake a button press
		pMinMap->Command(IDC_COMPASS_UNLOCK, COMMAND_BUTTON_CLICKED, 0);

		SetLight(GetLight(12));
	}
	else
	{
		SetLight(GetLight(GetHour()));
	}

	//remove the party from their current location in the current valley
	if(Valley)
	{
		for(int n = 0; n < PreludeParty.GetNumMembers(); n++)
		{
			Valley->RemoveFromUpdate(PreludeParty.GetMember(n));
		}
	}

	Valley = Areas[AreaNum];
	
	//Engine->Graphics()->GetD3D()->BeginScene();
	
	int ChunkX;
	ChunkX = (x / CHUNK_TILE_WIDTH);

	int ChunkY;
	ChunkY = (y / CHUNK_TILE_HEIGHT);

	int xn, yn;
	Chunk *pChunk;

	int XStart;
	int XEnd;
	int YStart;
	int YEnd;

	XStart= ChunkX - 1;
	YStart = ChunkY - 1;

	XEnd = ChunkX + 1;
	YEnd = ChunkY + 1;
	
	if(XStart < 0) 
		XStart = 0;
	
	if(YStart < 0) 
		YStart = 0;
	
	if(XEnd >= Valley->ChunkWidth)
		XEnd = Valley->ChunkWidth - 1;

	if(YEnd >= Valley->ChunkHeight)
		YEnd = Valley->ChunkHeight - 1;
		

	for(yn = YStart; yn <= YEnd; yn++)
	for(xn = XStart; xn <= XEnd; xn++)
	{
		Valley->LoadChunk(xn,yn);
		pChunk = Valley->GetChunk(xn,yn);
		if(pChunk)
		{
		//	pChunk->CreateTexture(Valley->GetBaseTexture());
			pChunk->ConvertTerrain();
		}
	}
	
//	Engine->Graphics()->GetD3D()->EndScene();

	Valley->UpdateOffscreen();
		
	PreludeParty.Teleport(x,y);
	
	Creature::PlaceByLocator();

	//set state to init
	SetGameState(GAME_STATE_INIT);
}

int World::GetAreaNum(char *AreaName)
{
	for(int n = 0; n < NumAreas; n++)
	{
		if(!strcmp(AreaName,Areas[n]->Header.Name))
		{
			return n;
		}
	}
	return -1;
}

int World::GetAreaNum(Area *pArea)
{
	for(int n = 0; n < NumAreas; n++)
	{
		if(pArea == Areas[n])
		{
			return n;
		}
	}
	return -1;
}


Area *World::GetArea(int Num)
{
	if(Num >= NumAreas || Num < 0)
	{
		return NULL;
	}
	return Areas[Num];
}

Area *World::AddArea(char *AreaName)
{
	Area *pArea;
	pArea = Areas[NumAreas] = new Area;
	NumAreas++;
	sprintf(pArea->Header.Name,"%s",AreaName);
	
	//attempt to load the area
	SetCurrentDirectory(".\\Areas");

	char filename[64];
	sprintf(filename,"%s.bin", pArea->Header.Name);
	FILE *fp;
	fp = SafeFileOpen(filename,"rb");
	if(fp)
	{
		SetCurrentDirectory(Engine->GetRootDirectory());
		fclose(fp);
		pArea->Load();
		pArea->LoadRegions();
		sprintf(filename,"%s.dyn", pArea->Header.Name);
		FILE *fp;
		SetCurrentDirectory(".\\Areas");
		fp = SafeFileOpen(filename,"rb");
		pArea->LoadNonStatic(fp);
		fclose(fp);
		SetCurrentDirectory(Engine->GetRootDirectory());
	}
	
	return pArea;
}

void World::CleanAreas()
{
	//get rid of unnecessary areas;
	//start at 1 because 0 is always the valley.
	//assume the first area correct;
	Area *pBase;
	Area *pArea;
	int n, sn, rn;
	BOOL Removed = FALSE;
	for(n = 1; n < NumAreas; n++)
	{
		pBase = GetArea(n);
		if(pBase)
		{
			for(sn = n+1; sn < NumAreas; sn++)
			{
				pArea = GetArea(sn);
				if(!strcmp(pArea->GetName(),pBase->GetName()))
				{
					Removed = TRUE;
					Describe("Removeing Duplicate Area");
					for(rn = sn; rn < NumAreas - 1; rn++)
					{
						Areas[rn] = Areas[rn+1];
						Creature *pCreature;
						Locator *pLoc;
						int NumLoc, ln;
						pCreature = Creature::GetFirst();
						while(pCreature)
						{
							NumLoc = pCreature->GetNumLocators();
							for(ln = 0; ln < NumLoc; ln++)
							{
								pLoc = pCreature->GetLocator(ln);
								if(pLoc->GetArea() == sn)
								{
									pLoc->SetArea(n);
								}
								if(pLoc->GetArea() == rn + 1)
								{
									pLoc->SetArea(rn);
								}
							}
							pCreature = (Creature *)pCreature->GetNext();
						}
					}
					NumAreas--;
					sn--;
				}
			}
		}
	}
	if(Removed)
	{
		this->Save("worldbase.bin");
	}

}

void World::CleanDynamic()
{  
	


}

void World::UpdateOffScreenCreatures()
{

	if(this->GameState != GAME_STATE_NORMAL)
		return;

	if(!pOffScreenCreature)
	{
		pOffScreenCreature = (Object *)Creature::GetFirst();
	}
	Creature *pCreature;
	pCreature = (Creature *)pOffScreenCreature;
	
	//check to see if the creature is actually offscreen;
	int CX;
	int CY;
	CX = pCreature->GetPosition()->x;
	CX /= UPDATE_SEGMENT_WIDTH;

	CY = pCreature->GetPosition()->y;
	CY /= UPDATE_SEGMENT_HEIGHT;

	if(pCreature->GetAreaIn() == this->GetCurAreaNum() 
		&& CX >= UpdateRect.left 
		&& CX <= UpdateRect.right 
		&& CY >= UpdateRect.top 
		&& CY <= UpdateRect.bottom)
	{
		pOffScreenCreature = pOffScreenCreature->GetNext();
	}
	else
	{
		pCreature->UpdateOffScreen();
		pOffScreenCreature = pOffScreenCreature->GetNext();
	}

	//post update clean texture if necessary
	if(pCreature->GetAreaIn() != this->GetCurAreaNum() || CX < UpdateRect.left || CX > UpdateRect.right || CY < UpdateRect.top || CY > UpdateRect.bottom)
	{
		if(pCreature->GetData(INDEX_TYPE).Value == 0)
		{
			if(pCreature->GetTexture())
			{
				delete pCreature->GetTexture();
				pCreature->SetTexture(NULL);
			}
		}
	}

}

void World::CleanOffScreenChunks()
{
	if(!Valley) return;
	if(this->InCombat() == TRUE || this->GameState == GAME_STATE_COMBAT) return;	

	int Top = this->ScreenY - (this->DrawRadius);
	int Left = this->ScreenX - (this->DrawRadius);
	int Bottom = this->ScreenY + (this->DrawRadius);
	int Right = this->ScreenX + (this->DrawRadius);
	
	Chunk *pChunk;
	pChunk = Valley->GetChunk(CleanX, CleanY);

	if(pChunk && (CleanX < Left || CleanX > Right || CleanY < Top || CleanY > Bottom))
	{
	//	Valley->RemoveChunk(CleanX, CleanY);
	}
	
	if(CleanX >= Valley->GetWidth())
	{
		CleanX = 0;
		CleanY++;
		if(CleanY >= Valley->GetHeight())
		{
			CleanY = 0;
		}
	}
	else
	{
		CleanX++;
	}

}

void World::GetNearestRoad(int CurX, int CurY, int *DropX, int *DropY)
{
	int ShortestDistance = 10000;
	static int RoadDirection = 1;
	


	int xn;
	int yn;
	int offset = 1;
	int RoadX, RoadY;
	RoadX = CurX / CHUNK_TILE_WIDTH;
	RoadY = CurY / CHUNK_TILE_HEIGHT;

	if(RoadDirection == 1)
	{
		RoadDirection = -1;

		while(offset < 200)
		{
			yn = RoadY - offset;
			for(xn = RoadX - offset; xn < RoadX + offset; xn++)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}

			
			xn = RoadX + offset;		
			for(yn = RoadY - offset; yn < RoadY + offset; yn++)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}

			yn = RoadY + offset;		
			for(xn = RoadX + offset; xn > RoadX - offset; xn--)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}

			xn = RoadX - offset;
			for(yn = RoadY + offset; yn > RoadY - offset; yn--)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}
			offset++;
		}
		
	}
	else
	{
		RoadDirection = 1;
		while(offset < 200)
		{
			xn = RoadX - offset;
			for(yn = RoadY + offset; yn > RoadY - offset; yn--)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}
			
			yn = RoadY + offset;		
			for(xn = RoadX + offset; xn > RoadX - offset; xn--)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}
		
			xn = RoadX + offset;		
			for(yn = RoadY - offset; yn < RoadY + offset; yn++)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}
		
		
			

			yn = RoadY - offset;
			for(xn = RoadX - offset; xn < RoadX + offset; xn++)
			{
				if(xn >= 0 && xn < 200 && yn >= 0 && yn < 200 && Roads[xn][yn])
				{
					*DropX = xn * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH / 2;
					*DropY = yn * CHUNK_TILE_HEIGHT + CHUNK_TILE_HEIGHT /2;
					return;
				}
			}

			offset++;
		}




	}

	SafeExit("Unable to Find Road");

	return;
}

BOOL World::InCombat()
{
	return pCombat->InCombat();
}


void World::AutoSave()
{
	FILE *fp;
	int n = 0;
	char FileName[64];
	char SaveGameName[64];
	while(TRUE)
	{
		n++;
		sprintf(FileName,"save%i.gam",n);
		fp = fopen(FileName,"rb");
		if(fp)
		{
			fread(SaveGameName,sizeof(char),64,fp);
			fclose(fp);
			if(!strcmp(SaveGameName,"Autosave"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	SaveGame(FileName,"Autosave");
}

void World::QuickSave()
{
	FILE *fp;
	int n = 0;
	char FileName[64];
	char SaveGameName[64];
	while(TRUE)
	{
		n++;
		sprintf(FileName,"save%i.gam",n);
		fp = fopen(FileName,"rb");
		if(fp)
		{
			fread(SaveGameName,sizeof(char),64,fp);
			fclose(fp);
			if(!strcmp(SaveGameName,"Quicksave"))
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	SaveGame(FileName,"Quicksave");
	Describe("Quick Save");
}


int World::GetAutosaveRate()
{
	return AutosaveRate;
}

int World::CreateLights(float DiffuseFactor)
{
	float xL;
	float zL;
	float AmbientFactor;
	AmbientFactor = 1.0f - DiffuseFactor;
	int n;
	//create the lights

	Engine->Graphics()->GetD3D()->LightEnable(0,FALSE);
	
	for(n = 0; n < DAY_LENGTH/2; n++)
	{

		ZeroMemory(&Light[n], sizeof(D3DLIGHT7));
		Light[n].dltType = D3DLIGHT_DIRECTIONAL;
		Light[n].dcvDiffuse.r = DiffuseFactor * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.g = DiffuseFactor * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.b = DiffuseFactor * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.r = AmbientFactor * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.g = AmbientFactor * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.b = AmbientFactor * (float)n/(float)(DAY_LENGTH/2);
		Light[n].dcvSpecular.r = DiffuseFactor;
		Light[n].dcvSpecular.g = DiffuseFactor;
		Light[n].dcvSpecular.b = DiffuseFactor;
	
		xL = 1.0f - 1.0f * (float)n/(float)(DAY_LENGTH/2);
		zL = -1.0f * (float)n/(float)(DAY_LENGTH/2);

		Light[n].dvDirection = Normalize(D3DVECTOR(xL,0.0f,zL));
	
	}

	for(n = DAY_LENGTH/2; n < DAY_LENGTH; n++)
	{

		ZeroMemory(&Light[n], sizeof(D3DLIGHT7));
		Light[n].dltType = D3DLIGHT_DIRECTIONAL;
		Light[n].dcvDiffuse.r = DiffuseFactor - DiffuseFactor * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.g = DiffuseFactor - DiffuseFactor * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvDiffuse.b = DiffuseFactor - DiffuseFactor * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.r = AmbientFactor - AmbientFactor * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.g = AmbientFactor - AmbientFactor * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvAmbient.b = AmbientFactor - AmbientFactor * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		Light[n].dcvSpecular.r = DiffuseFactor;
		Light[n].dcvSpecular.g = DiffuseFactor;
		Light[n].dcvSpecular.b = DiffuseFactor;
	
		xL = -1.0f * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);
		zL = -1.0f + 1.0f * (float)(n - (DAY_LENGTH/2))/(float)(DAY_LENGTH/2);

		Light[n].dvDirection = Normalize(D3DVECTOR(xL,0.0f,zL));
	}
	
	//nighttime need more blue

	for(n = 0; n < DAY_LENGTH/4; n++)
	{
		Light[n].dcvDiffuse.b *= 1.5f;
		if(Light[n].dcvDiffuse.b > 1.0f)
			Light[n].dcvDiffuse.b = 1.0f;

		Light[n].dcvAmbient.b += 0.1f;
		Light[n].dcvAmbient.b *= 1.5;
		if(Light[n].dcvAmbient.b > 1.0f)
			Light[n].dcvAmbient.b = 1.0f;
		Light[n].dvDirection = Light[n + 12].dvDirection;
	}

	for(n = DAY_LENGTH - DAY_LENGTH/4; n < DAY_LENGTH; n++)
	{
		Light[n].dcvDiffuse.b *= 1.5f;
		if(Light[n].dcvDiffuse.b > 1.0f)
			Light[n].dcvDiffuse.b = 1.0f;

		Light[n].dcvAmbient.b += 0.1f;
		Light[n].dcvAmbient.b *= 1.5f;
		if(Light[n].dcvAmbient.b > 1.0f)
			Light[n].dcvAmbient.b = 1.0f;
		Light[n].dvDirection = Light[n - 12].dvDirection;
	}

	Light[0].dcvDiffuse.r =	Light[21].dcvDiffuse.r;
	Light[0].dcvDiffuse.g =	Light[21].dcvDiffuse.g;
	Light[0].dcvDiffuse.b =	Light[21].dcvDiffuse.b;
	Light[0].dcvAmbient.r = Light[21].dcvAmbient.r;
	Light[0].dcvAmbient.g = Light[21].dcvAmbient.g;
	Light[0].dcvAmbient.b = Light[21].dcvAmbient.b;
	

	for(n = 9; n < 20; n++)
	{
		Light[n].dcvDiffuse.r =	Light[12].dcvDiffuse.r;
		Light[n].dcvDiffuse.g =	Light[12].dcvDiffuse.g;
		Light[n].dcvDiffuse.b =	Light[12].dcvDiffuse.b;
		Light[n].dcvAmbient.r = Light[12].dcvAmbient.r;
		Light[n].dcvAmbient.g = Light[12].dcvAmbient.g;
		Light[n].dcvAmbient.b = Light[12].dcvAmbient.b;
	}

	for(n = 1; n < 7; n++)
	{
		Light[n].dcvDiffuse.r =	Light[0].dcvDiffuse.r;
		Light[n].dcvDiffuse.g =	Light[0].dcvDiffuse.g;
		Light[n].dcvDiffuse.b =	Light[0].dcvDiffuse.b;
		Light[n].dcvAmbient.r = Light[0].dcvAmbient.r;
		Light[n].dcvAmbient.g = Light[0].dcvAmbient.g;
		Light[n].dcvAmbient.b = Light[0].dcvAmbient.b;
	}

	for(n = 22; n < 24; n++)
	{
		Light[n].dcvDiffuse.r =	Light[0].dcvDiffuse.r;
		Light[n].dcvDiffuse.g =	Light[0].dcvDiffuse.g;
		Light[n].dcvDiffuse.b =	Light[0].dcvDiffuse.b;
		Light[n].dcvAmbient.r = Light[0].dcvAmbient.r;
		Light[n].dcvAmbient.g = Light[0].dcvAmbient.g;
		Light[n].dcvAmbient.b = Light[0].dcvAmbient.b;
	}	

	Engine->Graphics()->GetD3D()->LightEnable(0,TRUE);
	
	return TRUE;
}



//end: Debug ***********************************************************
World *PreludeWorld;
