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
#include "area.h"
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
#include "combatmanager.h"
#include "gameitem.h"
#include "cavewall.h"

#define D3D_OVERLOADS
#define DIFFUSE_FACTOR				0.5f
#define AMBIENT_FACTOR				0.5f
#define MAXIMUM_CAMERA_DISTANCE		48.0f

typedef struct
{
	char Name[32];
	int Width;
	int Height;
	int ChunkWidth;
	int ChunkHeight;
	int ChunkOffsets[200*200];
} OLD_FILE_HEADER_T;

//time pass speed is in millisecond
//one minute of game time passes every 2 seconds.
//one hour = 2 minutes of play time
//one day = 48 minutes of play time

#define TIME_PASS_SPEED		2000	

#include <d3d.h>

//************** static Members *********************************

//************** Constructors  ****************************************

//simple constructor
Area::Area()
{
	AreaID = 0;
	BigMap = NULL;
	Width = Height = 0;

	TextureNum = 0;

	ZeroMemory(&Header,sizeof(Header));

	HightLightTextureCoordinates[0] = 0.0f;
	HightLightTextureCoordinates[1] = 0.0f;
	HightLightTextureCoordinates[2] = 1.0f;
	HightLightTextureCoordinates[3] = 0.0f;
	HightLightTextureCoordinates[4] = 0.0f;
	HightLightTextureCoordinates[5] = 1.0f;
	HightLightTextureCoordinates[6] = 1.0f;
	HightLightTextureCoordinates[7] = 1.0f;

	HighLightDrawList[0] = 0;
	HighLightDrawList[1] = 1;
	HighLightDrawList[2] = 2;
	HighLightDrawList[3] = 1;
	HighLightDrawList[4] = 3;
	HighLightDrawList[5] = 2;
	

	//set the strides.  (!!!!THis can be moved to the constructor to save time).	
	HighLightDataInfo.position.dwStride = sizeof(float)*6;
	HighLightDataInfo.normal.dwStride = sizeof(float)*6;
	HighLightDataInfo.textureCoords[0].dwStride = sizeof(float)*2;

	//set the data
	HighLightDataInfo.textureCoords[0].lpvData = HightLightTextureCoordinates;

	Regions = NULL;

	UpdateSegments = NULL;

	StaticFile = NULL;

	UpdateWidth = 0;
	UpdateHeight = 0;
	Width = 0;
	Height = 0;
	ChunkWidth = 0;
	ChunkHeight = 0;

}

Area::Area(const char *filename)
{
	BigMap = NULL;
	ZeroMemory(&Header,sizeof(Header));

	HightLightTextureCoordinates[0] = 0.0f;
	HightLightTextureCoordinates[1] = 0.0f;
	HightLightTextureCoordinates[2] = 1.0f;
	HightLightTextureCoordinates[3] = 0.0f;
	HightLightTextureCoordinates[4] = 0.0f;
	HightLightTextureCoordinates[5] = 1.0f;
	HightLightTextureCoordinates[6] = 1.0f;
	HightLightTextureCoordinates[7] = 1.0f;

	HighLightDrawList[0] = 0;
	HighLightDrawList[1] = 1;
	HighLightDrawList[2] = 2;
	HighLightDrawList[3] = 1;
	HighLightDrawList[4] = 3;
	HighLightDrawList[5] = 2;
	
	//set the strides. 
	HighLightDataInfo.position.dwStride = sizeof(float)*6;
	HighLightDataInfo.normal.dwStride = sizeof(float)*6;
	HighLightDataInfo.textureCoords[0].dwStride = sizeof(float)*2;

	//set the data
	HighLightDataInfo.textureCoords[0].lpvData = HightLightTextureCoordinates;

	StaticFile = SafeFileOpen(filename,"rb");

	LoadHeader(StaticFile);

	Width = Header.Width;
	Height = Header.Height;

	Regions = new unsigned short[(Header.Width / this->ChunkWidth) * (Header.Height / this->ChunkWidth) * NUM_CHUNK_REGIONS];

	memset(Regions,0,sizeof(unsigned short) * (Header.Width / this->ChunkWidth) * (Header.Height / this->ChunkWidth) * NUM_CHUNK_REGIONS);

	UpdateSegments = new Object *[(Header.Width / UPDATE_SEGMENT_WIDTH) * (Header.Height / UPDATE_SEGMENT_HEIGHT)];

	memset(UpdateSegments,0, sizeof(Object *) * (Header.Width / UPDATE_SEGMENT_WIDTH) * (Header.Height / UPDATE_SEGMENT_HEIGHT));
}

//end:  Constructors ***************************************************



//*************** Destructor *******************************************
void Area::Clear()
{
	int Offset = 0;	
	int n;
	int maxn;
	
	//clear out the static data that is still loaded
	maxn = this->Header.ChunkWidth * this->Header.ChunkHeight;
	for(n = 0; n < maxn; n++)
	{
		if(BigMap[n])
		{
			delete BigMap[n];
			BigMap[n] = NULL;
		}
	}
	Object *pOb;

	int xn,yn;

	for(yn = 0; yn < this->UpdateHeight; yn++)
	for(xn = 0; xn < this->UpdateWidth; xn++)
	{
		while(this->GetUpdateSegment(xn,yn))
		{
			pOb = this->GetUpdateSegment(xn,yn);
			RemoveFromUpdate(pOb);
			if(pOb->GetObjectType() != OBJECT_CREATURE)
				delete pOb;
		}
		Offset++;
	}
	DEBUG_INFO("Area Cleared");
}

Area::~Area()
{
	if(StaticFile)
	{
		fclose(StaticFile);
		StaticFile = NULL;
	}

	if(BigMap)
	{
		Clear();
		delete[] BigMap;
	}

	if(UpdateSegments)
	{
		delete[] UpdateSegments;
		UpdateSegments = NULL;
	}
	
	if(Header.ChunkOffsets)
	{
		delete[] Header.ChunkOffsets;
		Header.ChunkOffsets = NULL;
	}
	
	DEBUG_INFO("Done Deleting Area\n");

}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************
int Area::GetWidth()
{
	return Width;
}

int Area::GetHeight()
{

	return Height;
}

float Area::GetTileHeight(int TileX, int TileY)
{
	if(TileX < 0 || TileY < 0 || TileX >= this->Width || TileY >= this->Height)
		return PreludeParty.GetLeader()->GetPosition()->z; 

	int MapOffset;
	MapOffset = TileX / CHUNK_TILE_WIDTH + TileY/CHUNK_TILE_WIDTH * this->ChunkWidth;
	if(BigMap[MapOffset])
	{
		return BigMap[MapOffset]->GetHeight(TileX % CHUNK_TILE_WIDTH, TileY % CHUNK_TILE_HEIGHT);
//		float *pVerts;
//		pVerts = BigMap[TileX / CHUNK_WIDTH + TileY/CHUNK_WIDTH * Width]->GetTile(TileX % CHUNK_WIDTH, TileY % CHUNK_HEIGHT);
//		return pVerts[2];
	}
	else
	{
		LoadChunk(TileX/CHUNK_TILE_WIDTH,TileY/CHUNK_TILE_WIDTH);
		if(BigMap[MapOffset])
		{
			float zval;
			zval = BigMap[MapOffset]->GetHeight(TileX % CHUNK_TILE_WIDTH, TileY % CHUNK_TILE_HEIGHT);
			delete BigMap[MapOffset];
			BigMap[TileX / CHUNK_TILE_WIDTH + TileY/CHUNK_TILE_WIDTH * this->ChunkWidth] = NULL;
			return zval;
		}
		else
		{
			return PreludeParty.GetLeader()->GetPosition()->z;
		}
	}
}

float Area::GetTileHeight(int TileX, int TileY, DIRECTION_T Side)
{
	if(TileX < 0 || TileY < 0 || TileX >= this->Width || TileY >= this->Height)
		return PreludeParty.GetLeader()->GetPosition()->z;

	if(BigMap[TileX / CHUNK_TILE_WIDTH + TileY / CHUNK_TILE_HEIGHT * this->ChunkWidth])
	{
		return BigMap[TileX / CHUNK_TILE_WIDTH + TileY/CHUNK_TILE_WIDTH * this->ChunkWidth]->GetHeight(TileX % CHUNK_TILE_WIDTH, TileY % CHUNK_TILE_HEIGHT);
	/*	float *pVerts;
		pVerts = BigMap[TileX / CHUNK_TILE_WIDTH + TileY/CHUNK_TILE_WIDTH * this->ChunkWidth]->GetTile(TileX % CHUNK_TILE_WIDTH, TileY % CHUNK_TILE_HEIGHT);
		switch(Side)
		{
		case NORTH:
			return (pVerts[2] + pVerts[8]) / 2.0f;
		case SOUTH:
			return (pVerts[14] + pVerts[20]) / 2.0f;
		case EAST:
			return (pVerts[8] + pVerts[20]) / 2.0f;
		case WEST:
			return (pVerts[2] + pVerts[14]) / 2.0f;
		case NORTHEAST:
			return pVerts[8];
		case SOUTHEAST:
			return pVerts[20];
		case SOUTHWEST:
			return pVerts[14];
		case NORTHWEST:
			return pVerts[2];
		default:
			BOOL Flipped;
			Flipped = BigMap[TileX / CHUNK_TILE_WIDTH + TileY/CHUNK_TILE_WIDTH * this->ChunkWidth]->IsFlipped(TileX % CHUNK_TILE_WIDTH, TileY % CHUNK_TILE_HEIGHT);
			if(Flipped)
			{
				return (pVerts[2] + pVerts[20]) / 2.0f;
			}
			else
			{
				return (pVerts[8] + pVerts[14]) / 2.0f;
			}
		}
	*/
	}
	else
	{
		return PreludeParty.GetLeader()->GetPosition()->z;
	}
}

Chunk *Area::GetChunk(int x, int y)
{ 
	if(x >= 0 && y >= 0 && x < ChunkWidth && y < ChunkHeight)
		return BigMap[x + (y * ChunkWidth)]; 
	return NULL;
};


float Area::GetZ(float x, float y)
{
	int TileX;
	TileX = (int)x;

	int TileY;

	TileY = (int)y;

	Chunk *pChunk;

	pChunk = GetChunk(TileX / CHUNK_TILE_WIDTH,TileY / CHUNK_TILE_HEIGHT);

	if(!pChunk)
	{
		LoadChunk(TileX / CHUNK_TILE_WIDTH,TileY / CHUNK_TILE_HEIGHT);
		pChunk = GetChunk(TileX / CHUNK_TILE_WIDTH,TileY / CHUNK_TILE_HEIGHT);
	}

	int ChunkTileX;
	int ChunkTileY;
	ChunkTileX = (TileX % CHUNK_TILE_WIDTH) / 2;
	ChunkTileY = (TileY % CHUNK_TILE_HEIGHT)/ 2;

	//return pChunk->GetHeight(TileX % CHUNK_TILE_WIDTH, TileY % CHUNK_TILE_HEIGHT);
		float *pVerts;
		pVerts = pChunk->GetTile(ChunkTileX, ChunkTileY);
	
		unsigned short *DrawList;
		unsigned short MyDraw[6];
		
		DrawList = pChunk->GetTileDraw(ChunkTileX,ChunkTileY);
		
		if(!DrawList[0] && !DrawList[1]) 
			return 0.0f;

		memcpy(MyDraw,DrawList,6 * sizeof(unsigned short));

		int Offset;
		Offset = (ChunkTileX + (ChunkTileY * CHUNK_WIDTH)) * 4;

		MyDraw[0] -= Offset;
		MyDraw[1] -= Offset;
		MyDraw[2] -= Offset;
		MyDraw[3] -= Offset;
		MyDraw[4] -= Offset;
		MyDraw[5] -= Offset;
		
		//check the first triangle
		float AngleTotal = 0.0f;
		
		D3DVECTOR vA;
		D3DVECTOR vB;
		D3DVECTOR vC;
		
		D3DVECTOR Side1;
		D3DVECTOR Side2;

		D3DVECTOR vNorm;
	
		vA.x = pVerts[MyDraw[0]*6] - x;
		vA.y = pVerts[MyDraw[0]*6+1] - y;
		vA.z = 0.0f;

		vB.x = pVerts[MyDraw[1]*6] - x;
		vB.y = pVerts[MyDraw[1]*6+1] - y;
		vB.z = 0.0f;

		vC.x = pVerts[MyDraw[2]*6] - x;
		vC.y = pVerts[MyDraw[2]*6+1] - y;
		vC.z = 0.0f;

		AngleTotal += GetAngle(&vA,&vB);
		AngleTotal += GetAngle(&vB,&vC);
		AngleTotal += GetAngle(&vC,&vA);
		
		if(AngleTotal > 6.265f && AngleTotal < 6.295f)
		{
			vA.x = pVerts[MyDraw[0]*6];
			vA.y = pVerts[MyDraw[0]*6+1];
			vA.z = pVerts[MyDraw[0]*6+2];

			vB.x = pVerts[MyDraw[1]*6];
			vB.y = pVerts[MyDraw[1]*6+1];
			vB.z = pVerts[MyDraw[1]*6+2];

			vC.x = pVerts[MyDraw[2]*6];
			vC.y = pVerts[MyDraw[2]*6+1];
			vC.z = pVerts[MyDraw[2]*6+2];
		}
		else
		{
			//use the second triangle
			vA.x = pVerts[MyDraw[3]*6];
			vA.y = pVerts[MyDraw[3]*6+1];
			vA.z = pVerts[MyDraw[3]*6+2];

			vB.x = pVerts[MyDraw[4]*6];
			vB.y = pVerts[MyDraw[4]*6+1];
			vB.z = pVerts[MyDraw[4]*6+2];

			vC.x = pVerts[MyDraw[5]*6];
			vC.y = pVerts[MyDraw[5]*6+1];
			vC.z = pVerts[MyDraw[5]*6+2];
		}

		Side1 = Normalize(vB - vA);
		Side2 = Normalize(vC - vA);

		vNorm = CrossProduct(Side1, Side2);

		float a,b,c,d,zn;

		a = vNorm.x;
		b = vNorm.y;
		c = vNorm.z;
		d = -1 * (a * vA.x +
					 b * vA.y +
					 c * vA.z);

		zn = -1 * (a * x + b * y + d)/c;

		return zn;
	
}

int Area::GetTileBlocking(int TileX, int TileY)
{
	return EMPTY_MASK;
}

BOOL Area::IsClear(int x, int y, Object *pTester)
{
	if(x < 0 || y < 0 || x >= this->Width || y >= this->Width)
	{
		return FALSE;
	}

	if(GetBlocking(x,y))
	{
		return FALSE;
	}
	//needs to be optimized to search only the area immediately around the x/y coordinates
	//also needs code to check for things which have a radius of greater than one
	Object *pOb;

	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		pOb = PreludeWorld->GetCombat()->Combatants;
		while(pOb)
		{
			if(pOb->TileBlock(x,y))
			{
				if(pOb == pTester)
				{
					pOb = pOb->GetNextUpdate();
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				pOb = pOb->GetNextUpdate();
			}
		}
	}

	if(x >= 0 && x < this->Width && y >= 0 && y < this->Height)
	{
		pOb = UpdateSegments[x / UPDATE_SEGMENT_WIDTH + (y/UPDATE_SEGMENT_HEIGHT)*this->UpdateWidth];

		while(pOb)
		{
			if(pOb->TileBlock(x,y))
			{
				if(pOb == pTester)
				{
					pOb = pOb->GetNextUpdate();
				}
				else
				{
					return FALSE;
				}
			}
			else
			{
				pOb = pOb->GetNextUpdate();
			}
		}

		return TRUE;
	}
	else
	{
		return FALSE;
	}


}
	
Object *Area::FindThing(int x, int y)
{
	//needs to be optimized to search only the area immediately around the x/y coordinates
	//also needs code to check for things which have a radius of greater than one
	int SegX;
	int SegY;


	SegX = x/UPDATE_SEGMENT_WIDTH;
	SegY = y/UPDATE_SEGMENT_HEIGHT;

	Object *pOb;

	pOb = UpdateSegments[SegX + SegY * this->UpdateWidth];
	D3DVECTOR *pPosition;
	
	while(pOb)
	{
		pPosition = pOb->GetPosition();

		if((int)pPosition->x == x 
			&& (int)pPosition->y == y)
		{
			return pOb;
		}
		pOb = pOb->GetNextUpdate();
	}

	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		pOb = PreludeWorld->GetCombat()->GetCombatants();

		while(pOb)
		{
			pPosition = pOb->GetPosition();

			if((int)pPosition->x == x 
				&& (int)pPosition->y == y)
			{
				return pOb;
			}
			pOb = pOb->GetNextUpdate();
		}
	}
	return NULL;
}

void Area::ConvertMouse(int MouseX, int MouseY, float *WorldX, float *WorldY, float *WorldZ)
{
/*	D3DVECTOR vMouseNear;
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
	DIRECTION_T RunDirection;

	while(TRUE)
	{
//		pVerts = MapChunks[xn / CHUNK_WIDTH][yn / CHUNK_HEIGHT].GetTile(xn % CHUNK_WIDTH, yn % CHUNK_HEIGHT);
		if(BigMap[xn/CHUNK_TILE_WIDTH + (yn/CHUNK_TILE_HEIGHT) * Width])
		{
			pVerts = BigMap[xn/CHUNK_TILE_WIDTH + (yn/CHUNK_TILE_HEIGHT) * this->ChunkWidth]->GetTile((xn % CHUNK_TILE_WIDTH) / 2 , (yn %CHUNK_TILE_HEIGHT) / 2 );		
		}
		else
		{
			return;
		}
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
*/
}

void Area::ConvertToWorld(int MouseX, int MouseY, D3DVECTOR *vScreen)
{
/*	D3DVECTOR vMouseNear;
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
*/
}

void Area::ConvertToWorld(D3DVECTOR *vScreen)
{
/*	D3DVECTOR vMouseNear;
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
*/
}


//end: Accessors *******************************************************



//************  Display Functions  *************************************
void Area::DrawTiles()
{
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->ClearTexture();
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));

	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;
	
	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;

	
	int xn,yn;

	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth;
		for(xn = StartX; xn <= EndX; xn++)
		{
			if(BigMap[Offset + xn])
			{
				BigMap[Offset + xn]->DrawTiles();
			}
		}
	}
	

	Offset = 0;
	
	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
	
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	StartX *= CHUNK_TILE_WIDTH;
	StartY *= CHUNK_TILE_HEIGHT;
	EndX = (PreludeWorld->ScreenX + PreludeWorld->DrawRadius) * CHUNK_TILE_HEIGHT;
	if(EndX > this->GetWidth()) EndX = this->GetWidth();
	
	EndY = (PreludeWorld->ScreenY + PreludeWorld->DrawRadius) * CHUNK_TILE_WIDTH;
	if(EndY > this->GetHeight()) EndY = this->GetHeight();
	
	//show ourblocking
	ZSModelEx *pMesh;
	pMesh = Engine->GetMesh("selectbox");
	for(yn = StartY; yn < EndY; yn++)
	{
		for(xn = StartX; xn < EndX; xn++)
		{
			if(GetBlocking(xn,yn))
			{
				pMesh->Draw(Engine->Graphics()->GetD3D(),(float)xn + 0.5f,(float)yn + 0.5f, GetTileHeight(xn,yn),0.0f,0.9f,0.9f,0.9f,0);
			}
			else
			{
				if(!IsClear(xn,yn))
				{
					pMesh->Draw(Engine->Graphics()->GetD3D(),(float)xn + 0.5f,(float)yn + 0.5f, GetTileHeight(xn,yn), 0.0f, 0.9f, 0.9f, 0.9f, 0);
				}
			}
		}
	}




	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);

}

void Area::DrawTerrain()
{
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->GetD3D()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		               0x00000000, 1.0f, 0L );
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_POINT);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_POINT);
	
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;
	
	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;

	int xn, yn;

	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth;
		for(xn = StartX; xn <= EndX; xn++)
		{
			if(BigMap[Offset + xn])
			{
				BigMap[Offset + xn]->Draw();
			}
		}
	}
	
}



void Area::Draw()
{
	int xn;
	int yn;

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->GetD3D()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		               0x00000000, 1.0f, 0L );
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	
	//should be able to eliminate this
	/*
	if(Engine->Graphics()->GetFilterState() == FILTER_NONE)
	{
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_POINT);
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_POINT);
	}
	else
	{
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_LINEAR);
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_LINEAR);
	}
	*/

	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;

	//draw terrain
	if(!PreludeParty.Inside())
	{
		for(yn = StartY; yn <= EndY; yn++)
		{
			Offset = yn * this->ChunkWidth;
			for(xn = StartX; xn <= EndX; xn++)
			{
				if(BigMap[Offset + xn])
				{
					BigMap[Offset + xn]->Draw();
				}
			}
		}
	}
	Object *pOb;
	
	//draw objects
	if(!PreludeParty.Inside())
	{
		for(yn = StartY; yn <= EndY; yn++)
		{
			Offset = yn * this->ChunkWidth;
			for(xn = StartX; xn <= EndX; xn++)
			{
				if(BigMap[Offset + xn])
					BigMap[Offset + xn]->DrawObjects();
			}
		}
	}


	//draw combatants if necessary
	if(PreludeWorld->GetGameState() != GAME_STATE_NORMAL)
	{
		PreludeWorld->GetCombat()->Draw();
	}
	
	//clear transform
	if(FAILED(Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity())))
	{
		DEBUG_INFO("Failed to clear Transform\n");
		exit(1);
	}
	//draw any regions around

	DrawRegions();

	//draw updating objects
	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			pOb = UpdateSegments[Offset];

			while(pOb)
			{
				pOb->Draw();

				pOb = pOb->GetNextUpdate();
			}
			Offset++;
		}
		Offset+= this->UpdateWidth - (1 + PreludeWorld->UpdateRect.right - PreludeWorld->UpdateRect.left);
	}

	//clear transform
	if(FAILED(Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity())))
	{
		DEBUG_INFO("Failed to clear Transform\n");
		exit(1);
	}



	//draw any special objects
	
	pOb = PreludeWorld->GetMainObjects();

	while(pOb)
	{
		Engine->Graphics()->SetTexture(pOb->GetTexture());
		pOb->Draw();
		pOb = pOb->GetNext();
	}

		//draw combatants if necessary
	if(PreludeWorld->InCombat())
	{
		PreludeWorld->GetCombat()->DrawMoveArea();
	}

	//set filtering
	/*
	if(Engine->Graphics()->GetFilterState() != FILTER_BOTH)
	{
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_POINT);
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_POINT);
	}
	else
	{
	}
	
	if(PreludeWorld->RenderBackTerrain)
	{
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
		Engine->Graphics()->SetTexture(NULL);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_BLACK));

		for(yn = StartY; yn <= EndY; yn++)
		{
			Offset = yn * this->ChunkWidth;
			for(xn = StartX; xn <= EndX; xn++)
			{
				if(BigMap[Offset + xn])
				{
					BigMap[Offset + xn]->DrawBacksides();
				}
			}
		}
		
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	}
	*/


//	UnsetRegions();

	/*
	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth;
		for(xn = StartX; xn <= EndX; xn++)
		{
			if(BigMap[Offset + xn])
			{
				BigMap[Offset + xn]->UnsetRegions();
			}
		}
	}
	*/

	DrawShadows();

	return;
}

void Area::DrawAltHighlight()
{
	int xn, yn;
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;


	Object *pOb;
	
	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			pOb = UpdateSegments[Offset];

			while(pOb)
			{
				pOb->AltHighLight();

				pOb = pOb->GetNextUpdate();
			}
			Offset++;
		}
		Offset+= this->UpdateWidth - (1 + PreludeWorld->UpdateRect.right - PreludeWorld->UpdateRect.left);
	}
	if(PreludeWorld->InCombat())
	{
		PreludeWorld->GetCombat()->DrawAltHighlight();
	}

}


void Area::HighlightTile(int x, int y)
{
	if(x < 0 || x >= Width || y < 0 || y >= Height)
	{
		return;
	}
	D3DVERTEX Verts[5];
	Verts[0].x = x;
	Verts[0].y = y;
	Verts[1].x = x + 1;
	Verts[1].y = y;
	Verts[2].x = x + 1;
	Verts[2].y = y + 1;
	Verts[3].x = x;
	Verts[3].y = y + 1;
	Verts[4].x = x;
	Verts[4].y = y;

	for(int n = 0; n < 5; n++)
	{
		Verts[n].nx = 0;
		Verts[n].ny = 0;
		Verts[n].nz = 1.0f;
	}

//	float *pVerts;
	float zVal;

	zVal = GetTileHeight(x,y) + 0.1f;

	Verts[0].z = Verts[1].z = Verts[2].z = Verts[3].z = Verts[4].z = zVal;

	HRESULT hr;
	
	if(BigMap[x / CHUNK_TILE_WIDTH + (y / CHUNK_TILE_HEIGHT) * this->ChunkWidth])
	{
		hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP,D3DFVF_VERTEX,Verts,5,0);

		
/*		pVerts = BigMap[x / CHUNK_TILE_WIDTH + (y / CHUNK_TILE_HEIGHT) * this->ChunkWidth]->GetTile((x % CHUNK_TILE_WIDTH)/2, (y % CHUNK_TILE_HEIGHT)/2);
		//set the data
		HighLightDataInfo.position.lpvData = &pVerts[0];
		HighLightDataInfo.normal.lpvData = &pVerts[3];

		hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
													D3DFVF_VERTEX,
													&this->HighLightDataInfo,
													4,
													this->HighLightDrawList,
													6,
													0);
*/	

		if(hr != D3D_OK)
		{
			zVal = 500.0f;
		}
  	}
}

void Area::CombatHighlightTile(int x, int y)
{
	if(x < 0 || x >= (Width-1) || y < 0 || y >= (Height - 1))
	{
		return;
	}
	D3DVERTEX Verts[5];
	Verts[0].x = x;
	Verts[0].y = y;
	Verts[1].x = x + 1;
	Verts[1].y = y;
	Verts[2].x = x + 1;
	Verts[2].y = y + 1;
	Verts[3].x = x;
	Verts[3].y = y + 1;
	Verts[4].x = x;
	Verts[4].y = y;

	for(int n = 0; n < 5; n++)
	{
		Verts[n].nx = 0;
		Verts[n].ny = 0;
		Verts[n].nz = 1.0f;
	}

	float TileZ;
	TileZ = Valley->GetTileHeight(x,y) + 0.1f;

	float tne, tn, tnw, tw, tsw, ts, tse, te;

	tne = Valley->GetTileHeight(x + 1, y - 1) + 0.1f;
	tn =  Valley->GetTileHeight(x    , y - 1) + 0.1f;
	tnw = Valley->GetTileHeight(x - 1, y - 1) + 0.1f;
	tw =  Valley->GetTileHeight(x - 1, y    ) + 0.1f;
	tsw = Valley->GetTileHeight(x - 1, y + 1) + 0.1f;
	ts =  Valley->GetTileHeight(x    , y + 1) + 0.1f;
	tse = Valley->GetTileHeight(x + 1, y + 1) + 0.1f;
	te =  Valley->GetTileHeight(x + 1, y    ) + 0.1f;

	float fNW;
	float fNE;
	float fSE;
	float fSW;
	
	fNW = tnw;
	if(tn > fNW) fNW = tn;
	if(tw > fNW) fNW = tw;
	if(TileZ > fNW) fNW = TileZ;

	fNE = tne;
	if(tn > fNE) fNE = tn;
	if(te > fNE) fNE = te;
	if(TileZ > fNE) fNE = TileZ;

	fSW = tsw;
	if(ts > fSW) fSW = ts;
	if(tw > fSW) fSW = tw;
	if(TileZ > fSW) fSW = TileZ;

	fSE = tse;
	if(ts > fSE) fSE = ts;
	if(te > fSE) fSE = te;
	if(TileZ > fSE) fSE = TileZ;

	Verts[0].z = fNW;
	Verts[1].z = fNE;
	Verts[2].z = fSE;
	Verts[3].z = fSW;
	Verts[4].z = fNW;

	HRESULT hr;
	
	if(BigMap[x / CHUNK_TILE_WIDTH + (y / CHUNK_TILE_HEIGHT) * this->ChunkWidth])
	{
		hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP,D3DFVF_VERTEX,Verts,5,0);

		
/*		pVerts = BigMap[x / CHUNK_TILE_WIDTH + (y / CHUNK_TILE_HEIGHT) * this->ChunkWidth]->GetTile((x % CHUNK_TILE_WIDTH)/2, (y % CHUNK_TILE_HEIGHT)/2);
		//set the data
		HighLightDataInfo.position.lpvData = &pVerts[0];
		HighLightDataInfo.normal.lpvData = &pVerts[3];

		hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
													D3DFVF_VERTEX,
													&this->HighLightDataInfo,
													4,
													this->HighLightDrawList,
													6,
													0);
*/	

		if(hr != D3D_OK)
		{
			TileZ = 500.0f;
		}
  	}
}


void Area::CircleTile(int x, int y, COLOR_T Color)
{
	if(x < 0 || x >= (Width * CHUNK_TILE_WIDTH) || y < 0 || y >= (Height * CHUNK_TILE_HEIGHT))
	{
		return;
	}

	float *pVerts;

	HRESULT hr;
	
	if(BigMap[x / CHUNK_TILE_WIDTH + (y / CHUNK_TILE_HEIGHT) * this->ChunkWidth])
	{
		Engine->Graphics()->SetTexture(Engine->Graphics()->GetCircleTexture());
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(Color));
		pVerts = BigMap[x / CHUNK_TILE_WIDTH + (y / CHUNK_TILE_HEIGHT) * this->ChunkWidth]->GetTile(x % CHUNK_TILE_WIDTH, y % CHUNK_TILE_HEIGHT);
		
		//set the data
		HighLightDataInfo.position.lpvData = &pVerts[0];
		HighLightDataInfo.normal.lpvData = &pVerts[3];

		hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
													D3DFVF_VERTEX,
													&this->HighLightDataInfo,
													4,
													this->HighLightDrawList,
													6,
													0);
		if(hr != D3D_OK)
		{

		}
  	}

}

ZSTexture *pShadowTexture = NULL;

void Area::DrawShadows()
{
	if(!PreludeWorld->ShadowDraw) return;
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);

	//equivalent to above
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_DESTCOLOR);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO);

	//use pure texture w/o materials.
	if(FAILED(Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1)))
	{
		DEBUG_INFO("Could not set texture stage state.\n");
		exit(1);
	}

	if(FAILED(Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE)))
	{
		DEBUG_INFO("Could not set texture stage state.\n");
		exit(1);
	}
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTOP_DIFFUSE);

//	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);

//	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

	if(FAILED(Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE)))
	{
		DEBUG_INFO("Could not set texture stage state.\n");
		exit(1);
	}

	if(!pShadowTexture)
	{
		pShadowTexture = Engine->GetTexture("shadow");
	}

	Engine->Graphics()->SetTexture(pShadowTexture);
	
	int xn, yn;
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;

	Object *pOb;
	
	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			pOb = UpdateSegments[Offset];

			while(pOb)
			{
				pOb->Shadow();

				pOb = pOb->GetNextUpdate();
			}
			Offset++;
		}
		Offset+= this->UpdateWidth - (1 + PreludeWorld->UpdateRect.right - PreludeWorld->UpdateRect.left);
	}

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);

}

//end: Display functions ***********************************************



//************ Mutators ************************************************

int Area::Load()
{
	SetCurrentDirectory(".\\Areas");
	
	char filename[64];
	sprintf(filename,"%s.bin", Header.Name);

	StaticFile = SafeFileOpen(filename,"rb");
	
	if(!StaticFile) 
		return FALSE;
	
	LoadHeader(StaticFile);

	if(BigMap)
	{
		delete[] BigMap;
	}

	BigMap = new Chunk *[this->ChunkWidth * this->ChunkHeight];
	ZeroMemory(BigMap, sizeof(Chunk *) * this->ChunkWidth * this->ChunkHeight);
	
	SetCurrentDirectory(Engine->GetRootDirectory());

	if(!strcmp(this->Header.Name,"valley"))
	{
		pBaseTexture = Engine->GetTexture("terrain");
	}
	else
	{
		pBaseTexture = Engine->GetTexture("caveterrain");
	}

	return TRUE;
}

void Area::LoadRegions()
{
	SetCurrentDirectory(".\\Areas");

	char filename[64];

	FILE *fp;
	sprintf(filename,"%s.rgn", Header.Name);

	fp = SafeFileOpen(filename,"rb");

	if(Regions)
	{
		delete Regions;
	}
	
	Regions = new unsigned short[this->ChunkWidth * this->ChunkHeight * NUM_CHUNK_REGIONS];

	fread(Regions,sizeof(unsigned short),(this->ChunkWidth * this->ChunkHeight * NUM_CHUNK_REGIONS),fp);

	fclose(fp);

	SetCurrentDirectory(Engine->GetRootDirectory());
}

void Area::SaveRegions()
{
	SetCurrentDirectory(".\\Areas");

	char filename[64];

	FILE *fp;
	sprintf(filename,"%s.rgn", Header.Name);

	fp = fopen(filename,"wb");

	if(fp)
	{
		fwrite(Regions, sizeof(unsigned short),(this->ChunkWidth * this->ChunkHeight * NUM_CHUNK_REGIONS),fp);
		fclose(fp);
	}
	else
	{
		Describe("Couldn't save <area>.rgn");
	}

	SetCurrentDirectory(Engine->GetRootDirectory());
}



void Area::AddToUpdate(Object *pToAdd)
{
	
	AddToUpdate(pToAdd,(int)pToAdd->GetPosition()->x,(int)pToAdd->GetPosition()->y);

	return;
}

void Area::AddToUpdate(Object *pToAdd, int xUpdate, int yUpdate)
{

	int SegX;
	int SegY;

	if(pToAdd->GetPrevUpdate() || pToAdd->GetNextUpdate())
	{
		DEBUG_INFO("Adding object on update to update\n");
		this->RemoveFromUpdate(pToAdd);
		DEBUG_INFO("Should have been removed.\n");
	}

	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT &&
		pToAdd->GetObjectType() == OBJECT_CREATURE)
	{
		return;
	}

	Object *CurList;

	SegX = xUpdate / UPDATE_SEGMENT_WIDTH;
	SegY = yUpdate / UPDATE_SEGMENT_HEIGHT;

	CurList = UpdateSegments[SegX + SegY * this->UpdateWidth];

//check to see if the obejct is already on the update list
	while(CurList)
	{
		if(CurList == pToAdd)
		{
			DEBUG_INFO("Object still on update list\n");
			return;
		}
		else
		{
			CurList = CurList->GetNextUpdate();
		}
	}

	CurList = UpdateSegments[SegX + SegY * this->UpdateWidth];
	
	if(CurList)
	{
		CurList->SetPrevUpdate(pToAdd);
	}
	
	pToAdd->SetNextUpdate(CurList);

	UpdateSegments[SegX + SegY * this->UpdateWidth] = pToAdd;
	
	pToAdd->SetPrevUpdate(NULL);

	if(pToAdd->GetObjectType() == OBJECT_CREATURE)
	{
		((Creature *)pToAdd)->SetAreaIn(this->AreaID);
	}
	else
	if(pToAdd->GetObjectType() == OBJECT_ITEM)
	{
		((GameItem *)pToAdd)->SetRegionIn(this->GetRegion(pToAdd->GetPosition()));
	}

	return;
}

void Area::RemoveRegion(Region *ToRemove)
{
	RECT Bounds;

	ToRemove->GetBounds(&Bounds);
	int RemoveID = ToRemove->GetID();

	int xn, yn, n, Offset;
	BOOL Found = FALSE;

	for(yn = Bounds.top / CHUNK_TILE_HEIGHT; yn <= Bounds.bottom / CHUNK_TILE_HEIGHT; yn++)
	{
		Offset = yn * (Width / CHUNK_TILE_WIDTH) * NUM_CHUNK_REGIONS + (Bounds.left / CHUNK_TILE_WIDTH) * NUM_CHUNK_REGIONS;
		for(xn = Bounds.left / CHUNK_TILE_WIDTH; xn <= Bounds.right / CHUNK_TILE_WIDTH; xn++)
		{
			for(n = 0; n < NUM_CHUNK_REGIONS; n++)
			{
				if(Regions[Offset + n] == RemoveID)
				{
					for(int sn = n; sn < NUM_CHUNK_REGIONS-1; sn++)
					{
						Regions[Offset + sn] = Regions[Offset + sn+1];
					}
					Regions[Offset + NUM_CHUNK_REGIONS-1] = 0;
					break;
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}

}

void Area::SetRegionsUnchecked(int xn, int yn)
{
	int n, Offset;

	Offset = yn * (this->Width / CHUNK_TILE_WIDTH) * NUM_CHUNK_REGIONS + xn * NUM_CHUNK_REGIONS;
	for(n = 0; n < NUM_CHUNK_REGIONS; n++)
	{
		if(Regions[Offset + n])
		{
			PreludeWorld->GetRegion(Regions[Offset + n])->ClearCheckedLOS();
		}
		else
		{
			break;
		}
	}
}

BOOL Area::CheckChunkLOS(int xn, int yn, D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd)
{

	int n, Offset;

	Offset = xn * NUM_CHUNK_REGIONS + yn * (Width / CHUNK_TILE_WIDTH) * NUM_CHUNK_REGIONS;
	for(n = 0; n < NUM_CHUNK_REGIONS; n++)
	{
		if(Regions[Offset + n])
		{
			if(PreludeWorld->GetRegion(Regions[Offset + n])->LineIntersect(vLineStart, vLineEnd))
			{
				return FALSE;
			}
		}
		else
		{
			break;
		}
	}

	Chunk *pChunk;
	pChunk = GetChunk(xn,yn);
	if(pChunk)
	{
		return pChunk->CheckLOS(vLineStart,vLineEnd);
	}

	return TRUE;
}


BOOL Area::CheckLOS(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd)
{
	//calculate the area to check
	RECT rLOS;
	if(vLineStart->x < vLineEnd->x)
	{
		rLOS.left = vLineStart->x;
		rLOS.right = vLineEnd->x;
	}
	else
	{
		rLOS.right = vLineStart->x;
		rLOS.left = vLineEnd->x;
	}

	if(vLineStart->y < vLineEnd->y)
	{
		rLOS.top = vLineStart->y;
		rLOS.bottom = vLineEnd->y;
	}
	else
	{
		rLOS.bottom = vLineStart->y;
		rLOS.top = vLineEnd->y;
	}

	rLOS.top /= CHUNK_TILE_HEIGHT;
	rLOS.bottom /= CHUNK_TILE_HEIGHT;
	rLOS.left /= CHUNK_TILE_WIDTH;
	rLOS.right /= CHUNK_TILE_WIDTH;
	
	int xn; 
	int yn;

	for(yn = rLOS.top; yn <= rLOS.bottom; yn++)
	for(xn = rLOS.left; xn <= rLOS.right; xn++)
	{
		SetRegionsUnchecked(xn,yn);
	}

	for(yn = rLOS.top; yn <= rLOS.bottom; yn++)
	for(xn = rLOS.left; xn <= rLOS.right; xn++)
	{
		if(!CheckChunkLOS(xn,yn,vLineStart,vLineEnd))
			return FALSE;
	}

	return TRUE;
}


void Area::RemoveFromUpdate(Object *pToRemove)
{

	RemoveFromUpdate(pToRemove,(int)pToRemove->GetPosition()->x,(int)pToRemove->GetPosition()->y);

	return;
/*

	if(!UpdateSegments) return;
	
	if(!pToRemove->GetPrevUpdate())
	{
		D3DVECTOR *pPosition;
		pPosition = pToRemove->GetPosition();
		int SegX;
		int SegY;

		SegX = ((int)pPosition->x)/UPDATE_SEGMENT_WIDTH;
		SegY = ((int)pPosition->y)/UPDATE_SEGMENT_HEIGHT;

		if(UpdateSegments[SegX + SegY * this->UpdateWidth] == pToRemove)
		{
			UpdateSegments[SegX + SegY * this->UpdateWidth] = pToRemove->GetNextUpdate();
		}
		else
		{
			DEBUG_INFO("Attempted to remove object not on update\n");
			int Offset;
			Offset = 0;
			for(Offset = 0; Offset < this->UpdateWidth * this->UpdateHeight; Offset++)
			{
				if(UpdateSegments[Offset] == pToRemove)
				{
					DEBUG_INFO("Found in other update list\n");
					UpdateSegments[Offset] = pToRemove->GetNextUpdate();
					break;
				}
			}
	
			if(pToRemove->GetNextUpdate())
				pToRemove->GetNextUpdate()->SetPrevUpdate(pToRemove->GetPrevUpdate());
				
			pToRemove->SetPrevUpdate(NULL);
			pToRemove->SetNextUpdate(NULL);

			return;
		}
	}
	else
	{
		pToRemove->GetPrevUpdate()->SetNextUpdate(pToRemove->GetNextUpdate());
	}
		
	if(pToRemove->GetNextUpdate())
	{
		pToRemove->GetNextUpdate()->SetPrevUpdate(pToRemove->GetPrevUpdate());
	}

	pToRemove->SetPrevUpdate(NULL);
	pToRemove->SetNextUpdate(NULL);
*/
}

void Area::RemoveFromUpdate(Object *pToRemove, int xUpdate, int yUpdate)
{
	if(!UpdateSegments) return;
	if(!pToRemove->GetPrevUpdate())
	{
		D3DVECTOR *pPosition;
		pPosition = pToRemove->GetPosition();
		int SegX;
		int SegY;

		SegX = xUpdate/UPDATE_SEGMENT_WIDTH;
		SegY = yUpdate/UPDATE_SEGMENT_HEIGHT;

		if(UpdateSegments[SegX + SegY * this->UpdateWidth] == pToRemove)
		{
			UpdateSegments[SegX + SegY * this->UpdateWidth] = pToRemove->GetNextUpdate();
		}
		else
		{
			if(PreludeWorld->InCombat())
			{
				return;
			}
//			DEBUG_INFO("Attempted to remove object not on update\n");
			int Offset;
			Offset = 0;
			for(Offset = 0; Offset < this->UpdateWidth * this->UpdateHeight; Offset++)
			{
				if(UpdateSegments[Offset] == pToRemove)
				{
//					DEBUG_INFO("Found in other update list\n");
					UpdateSegments[Offset] = pToRemove->GetNextUpdate();
					break;
				}
			}
	
			if(pToRemove->GetNextUpdate())
				pToRemove->GetNextUpdate()->SetPrevUpdate(pToRemove->GetPrevUpdate());
				
			pToRemove->SetPrevUpdate(NULL);
			pToRemove->SetNextUpdate(NULL);

			return;
		}
	}
	else
	{
		pToRemove->GetPrevUpdate()->SetNextUpdate(pToRemove->GetNextUpdate());
	}
		
	if(pToRemove->GetNextUpdate())
	{
		pToRemove->GetNextUpdate()->SetPrevUpdate(pToRemove->GetPrevUpdate());
	}

	pToRemove->SetPrevUpdate(NULL);
	pToRemove->SetNextUpdate(NULL);

}

int Area::Update()
{
//	DWORD CurTime;
//	CurTime = timeGetTime();

//	if(CurTime > NextAdvanceTime)
//	{
//		IncrementTime();
//	}
	
	Object *pOb;
	Object *pKill;

	pOb = PreludeWorld->GetMainObjects();

	int NotDone;

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
			PreludeWorld->RemoveMainObject(pKill);
			delete pKill;
		}
	}

	int xn; 
	int yn;
	int Offset;

	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			pOb = UpdateSegments[Offset];

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
					RemoveFromUpdate(pKill);
					if(pKill->GetObjectType() == OBJECT_CREATURE)
					{

					}
					else
					{
						delete pKill;
					}
				}
				
				if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
				{
					return TRUE;
				}
				else
				if(PreludeWorld->GetGameState() == GAME_STATE_INIT)
				{
					PreludeWorld->SetGameState(GAME_STATE_NORMAL);
					return TRUE;
				}

				
			}
			Offset++;
		}
		Offset+= this->UpdateWidth - (1 + PreludeWorld->UpdateRect.right - PreludeWorld->UpdateRect.left);
	}

	for(int n = 0; n < PreludeParty.GetNumMembers(); n++)
	{
		D3DVECTOR *pvLead;
		pvLead = PreludeParty.GetMember(n)->GetPosition();
		int LeadX, LeadY;
		LeadX = (int)pvLead->x / UPDATE_SEGMENT_WIDTH;
		LeadY = (int)pvLead->y / UPDATE_SEGMENT_HEIGHT;
		
		if(LeadX < PreludeWorld->UpdateRect.left || LeadX > PreludeWorld->UpdateRect.right ||
		   LeadY < PreludeWorld->UpdateRect.top || LeadY > PreludeWorld->UpdateRect.bottom)
		{
			NotDone = PreludeParty.GetMember(n)->AdvanceFrame();
			
			if(NotDone)
			{

			}
			else
			{
				RemoveFromUpdate(PreludeParty.GetMember(n));
				PreludeParty.RemoveMember(PreludeParty.GetMember(n));
			}
		}
	}

	if(PreludeParty.HasChangedPosition())
	{
		PreludeParty.Occupy();
	}
	return TRUE;
}


void Area::RaiseTileHeight(int TileX, int TileY, float NewHeight)
{
	Chunk *pChunk[9];
	
	pChunk[0] = GetChunk((TileX-1) / CHUNK_TILE_WIDTH, (TileY-1) / CHUNK_TILE_HEIGHT);
	pChunk[1] = GetChunk((TileX) / CHUNK_TILE_WIDTH, (TileY-1) / CHUNK_TILE_HEIGHT);
	pChunk[2] = GetChunk((TileX+1) / CHUNK_TILE_WIDTH, (TileY-1) / CHUNK_TILE_HEIGHT);
	pChunk[3] = GetChunk((TileX-1) / CHUNK_TILE_WIDTH, (TileY) / CHUNK_TILE_HEIGHT);
	pChunk[4] = GetChunk((TileX) / CHUNK_TILE_WIDTH, (TileY) / CHUNK_TILE_HEIGHT);
	pChunk[5] = GetChunk((TileX+1) / CHUNK_TILE_WIDTH, (TileY) / CHUNK_TILE_HEIGHT);
	pChunk[6] = GetChunk((TileX-1) / CHUNK_TILE_WIDTH, (TileY+1) / CHUNK_TILE_HEIGHT);
	pChunk[7] = GetChunk((TileX) / CHUNK_TILE_WIDTH, (TileY+1) / CHUNK_TILE_HEIGHT);
	pChunk[8] = GetChunk((TileX+1) / CHUNK_TILE_WIDTH, (TileY+1) / CHUNK_TILE_HEIGHT);

	float *pVerts;
	pVerts = pChunk[0]->GetTile((TileX-1) % CHUNK_TILE_WIDTH,(TileY-1) % CHUNK_TILE_HEIGHT);
	pVerts[tz4] += NewHeight;
	pChunk[0]->ResetTile((TileX-1) % CHUNK_TILE_WIDTH,(TileY-1) % CHUNK_TILE_HEIGHT);

	pVerts = pChunk[1]->GetTile((TileX) % CHUNK_TILE_WIDTH,(TileY-1) % CHUNK_TILE_HEIGHT);
	pVerts[tz3] += NewHeight;
	pVerts[tz4] += NewHeight;
	pChunk[1]->ResetTile((TileX) % CHUNK_TILE_WIDTH,(TileY-1) % CHUNK_TILE_HEIGHT);
	
	pVerts = pChunk[2]->GetTile((TileX+1) % CHUNK_TILE_WIDTH,(TileY-1) % CHUNK_TILE_HEIGHT);
	pVerts[tz3] += NewHeight;
	pChunk[2]->ResetTile((TileX+1) % CHUNK_TILE_WIDTH,(TileY-1) % CHUNK_TILE_HEIGHT);
	
	pVerts = pChunk[3]->GetTile((TileX-1) % CHUNK_TILE_WIDTH,(TileY) % CHUNK_TILE_HEIGHT);
	pVerts[tz2] += NewHeight;
	pVerts[tz4] += NewHeight;
	pChunk[3]->ResetTile((TileX-1) % CHUNK_TILE_WIDTH,(TileY) % CHUNK_TILE_HEIGHT);
	
	pVerts = pChunk[4]->GetTile((TileX) % CHUNK_TILE_WIDTH,(TileY) % CHUNK_TILE_HEIGHT);
	pVerts[tz1] += NewHeight;
	pVerts[tz2] += NewHeight;
	pVerts[tz3] += NewHeight;
	pVerts[tz4] += NewHeight;
	pChunk[4]->ResetTile((TileX) % CHUNK_TILE_WIDTH,(TileY) % CHUNK_TILE_HEIGHT);
	
	pVerts = pChunk[5]->GetTile((TileX+1) % CHUNK_TILE_WIDTH,(TileY) % CHUNK_TILE_HEIGHT);
	pVerts[tz1] += NewHeight;
	pVerts[tz3] += NewHeight;
	pChunk[5]->ResetTile((TileX+1) % CHUNK_TILE_WIDTH,(TileY) % CHUNK_TILE_HEIGHT);

	pVerts = pChunk[6]->GetTile((TileX-1) % CHUNK_TILE_WIDTH,(TileY+1) % CHUNK_TILE_HEIGHT);
	pVerts[tz2] += NewHeight;
	pChunk[6]->ResetTile((TileX-1) % CHUNK_TILE_WIDTH,(TileY+1) % CHUNK_TILE_HEIGHT);

	pVerts = pChunk[7]->GetTile((TileX) % CHUNK_TILE_WIDTH,(TileY+1) % CHUNK_TILE_HEIGHT);
	pVerts[tz1] += NewHeight;
	pVerts[tz2] += NewHeight;
	pChunk[7]->ResetTile((TileX) % CHUNK_TILE_WIDTH,(TileY+1) % CHUNK_TILE_HEIGHT);

	pVerts = pChunk[8]->GetTile((TileX+1) % CHUNK_TILE_WIDTH,(TileY+1) % CHUNK_TILE_HEIGHT);
	pVerts[tz1] += NewHeight;
	pChunk[8]->ResetTile((TileX+1) % CHUNK_TILE_WIDTH,(TileY+1) % CHUNK_TILE_HEIGHT);
}

void Area::SetTileHeight(int TileX, int TileY, float NewHeight)
{
	Chunk *pChunk[9];
	int GraphTileX;
	GraphTileX = TileX / 2;
	int GraphTileY;
	GraphTileY = TileY / 2;
	
	pChunk[0] = GetChunk((GraphTileX-1) / CHUNK_WIDTH, (GraphTileY-1) / CHUNK_HEIGHT);
	pChunk[1] = GetChunk((GraphTileX) / CHUNK_WIDTH, (GraphTileY-1) / CHUNK_HEIGHT);
	pChunk[2] = GetChunk((GraphTileX+1) / CHUNK_WIDTH, (GraphTileY-1) / CHUNK_HEIGHT);
	pChunk[3] = GetChunk((GraphTileX-1) / CHUNK_WIDTH, (GraphTileY) / CHUNK_HEIGHT);
	pChunk[4] = GetChunk((GraphTileX) / CHUNK_WIDTH, (GraphTileY) / CHUNK_HEIGHT);
	pChunk[5] = GetChunk((GraphTileX+1) / CHUNK_WIDTH, (GraphTileY) / CHUNK_HEIGHT);
	pChunk[6] = GetChunk((GraphTileX-1) / CHUNK_WIDTH, (GraphTileY+1) / CHUNK_HEIGHT);
	pChunk[7] = GetChunk((GraphTileX) / CHUNK_WIDTH, (GraphTileY+1) / CHUNK_HEIGHT);
	pChunk[8] = GetChunk((GraphTileX+1) / CHUNK_WIDTH, (GraphTileY+1) / CHUNK_HEIGHT);

	float *pVerts;
	if(pChunk[0])
	{
		pVerts = pChunk[0]->GetTile((GraphTileX-1) % CHUNK_WIDTH ,(GraphTileY-1) % CHUNK_HEIGHT );
		pVerts[tz4] = NewHeight;
		pChunk[0]->ResetTile((GraphTileX-1) % CHUNK_WIDTH ,(GraphTileY-1) % CHUNK_HEIGHT );
	}
	if(pChunk[1])
	{
		pVerts = pChunk[1]->GetTile((GraphTileX) % CHUNK_WIDTH ,(GraphTileY-1) % CHUNK_HEIGHT );
		pVerts[tz3] = NewHeight;
		pVerts[tz4] = NewHeight;
		pChunk[1]->ResetTile((GraphTileX) % CHUNK_WIDTH ,(GraphTileY-1) % CHUNK_HEIGHT );
	}
	if(pChunk[2])
	{
		pVerts = pChunk[2]->GetTile((GraphTileX+1) % CHUNK_WIDTH ,(GraphTileY-1) % CHUNK_HEIGHT );
		pVerts[tz3] = NewHeight;
		pChunk[2]->ResetTile((GraphTileX+1) % CHUNK_WIDTH ,(GraphTileY-1) % CHUNK_HEIGHT );
	}
	if(pChunk[3])
	{
		pVerts = pChunk[3]->GetTile((GraphTileX-1) % CHUNK_WIDTH ,(GraphTileY) % CHUNK_HEIGHT );
		pVerts[tz2] = NewHeight;
		pVerts[tz4] = NewHeight;
		pChunk[3]->ResetTile((GraphTileX-1) % CHUNK_WIDTH ,(GraphTileY) % CHUNK_HEIGHT );
	}
	if(pChunk[4])
	{
		pVerts = pChunk[4]->GetTile((GraphTileX) % CHUNK_WIDTH ,(GraphTileY) % CHUNK_HEIGHT );
		pVerts[tz1] = NewHeight;
		pVerts[tz2] = NewHeight;
		pVerts[tz3] = NewHeight;
		pVerts[tz4] = NewHeight;
		pChunk[4]->ResetTile((GraphTileX) % CHUNK_WIDTH ,(GraphTileY) % CHUNK_HEIGHT );
	}
	if(pChunk[5])
	{
		pVerts = pChunk[5]->GetTile((GraphTileX+1) % CHUNK_WIDTH ,(GraphTileY) % CHUNK_HEIGHT );
		pVerts[tz1] = NewHeight;
		pVerts[tz3] = NewHeight;
		pChunk[5]->ResetTile((GraphTileX+1) % CHUNK_WIDTH ,(GraphTileY) % CHUNK_HEIGHT );
	}
	if(pChunk[6])
	{
		pVerts = pChunk[6]->GetTile((GraphTileX-1) % CHUNK_WIDTH ,(GraphTileY+1) % CHUNK_HEIGHT );
		pVerts[tz2] = NewHeight;
		pChunk[6]->ResetTile((GraphTileX-1) % CHUNK_WIDTH ,(GraphTileY+1) % CHUNK_HEIGHT );
	}
	if(pChunk[7])
	{
		pVerts = pChunk[7]->GetTile((GraphTileX) % CHUNK_WIDTH ,(GraphTileY+1) % CHUNK_HEIGHT );
		pVerts[tz1] = NewHeight;
		pVerts[tz2] = NewHeight;
		pChunk[7]->ResetTile((GraphTileX) % CHUNK_WIDTH ,(GraphTileY+1) % CHUNK_HEIGHT );
	}
	if(pChunk[8])
	{
		pVerts = pChunk[8]->GetTile((GraphTileX+1) % CHUNK_WIDTH ,(GraphTileY+1) % CHUNK_HEIGHT );
		pVerts[tz1] = NewHeight;
		pChunk[8]->ResetTile((GraphTileX+1) % CHUNK_WIDTH ,(GraphTileY+1) % CHUNK_HEIGHT );
	}

}

int Area::AddObject(Object *ToAdd)
{
	Region *pRegion;
	pRegion = GetRegion(ToAdd->GetPosition());

	if(pRegion)
	{
		pRegion->AddObject(ToAdd);
		return TRUE;
	}
	
	
	int ChunkX = (int)(ToAdd->GetPosition()->x / (float)CHUNK_TILE_WIDTH);
	int ChunkY = (int)(ToAdd->GetPosition()->y / (float)CHUNK_TILE_HEIGHT);

	Chunk *pChunk;
	pChunk = GetChunk(ChunkX,ChunkY);

	if(pChunk)
	{
		pChunk->AddObject(ToAdd);
	}
	else
	{
		LoadChunk(ChunkX,ChunkY);
		pChunk = GetChunk(ChunkX,ChunkY);
		if(pChunk)
			pChunk->AddObject(ToAdd);
	}



  return TRUE;
}
//end: Mutators ********************************************************



//************ Outputs *************************************************
int Area::SaveBrief()
{
	fclose(StaticFile);
	char filename[64];
	sprintf(filename,"%s.bin",Header.Name);
	StaticFile = SafeFileOpen(filename,"rb+");
	int xn, yn;
	Chunk *pChunk;

	for(yn = 0; yn < this->ChunkHeight; yn++)
	{
		for(xn = 0; xn < this->ChunkWidth; xn++)
		{
			pChunk = GetChunk(xn, yn);
			if(pChunk)
			{
				//position the file pointer
				int Error;

				Error = fseek(StaticFile, Header.ChunkOffsets[xn + yn * this->ChunkWidth], SEEK_SET);

				assert(!Error);
				pChunk->SaveBrief(StaticFile);
			}
			else
			{

			}
		}
	}
	
	fclose(StaticFile);

	StaticFile = SafeFileOpen(filename,"rb");



	char blarg[64];
	sprintf(blarg,"saved brief %s",Header.Name);
	Describe(blarg);

	return TRUE;

}

int Area::Save()
{
	char filename[64];
	sprintf(filename,"%s.bin",Header.Name);

	FILE *fp;
	fp = SafeFileOpen("temp.bin","wb");

	int xn;
	int yn;

	Chunk *pChunk;
	
	SaveHeader(fp);
	
	for(yn = 0; yn < this->ChunkHeight; yn++)
	{
		for(xn = 0; xn < this->ChunkWidth; xn++)
		{
			pChunk = GetChunk(xn, yn);
			if(!pChunk)
			{
				LoadChunk(xn,yn);
				pChunk = GetChunk(xn, yn);
			}
			if(pChunk)
			{
				Header.ChunkOffsets[xn + yn * this->ChunkWidth] = ftell(fp);
				pChunk->Save(fp);
				delete pChunk;
				pChunk = NULL;
			}
			else
			{
				Header.ChunkOffsets[xn + yn * this->ChunkWidth] = NULL;
			}
			BigMap[xn + yn*this->ChunkWidth] = NULL;
		}
	}

	fseek(fp,0,SEEK_SET);
	
	SaveHeader(fp);
	
	fclose(fp);
	
	fclose(StaticFile);
	remove(filename);
	rename("temp.bin",filename);
	
	StaticFile = SafeFileOpen(filename,"rb");
	char blarg[64];
	sprintf(blarg,"saved %s",Header.Name);
	Describe(blarg);
	return TRUE;
}

void Area::SaveHeader(FILE *fp)
{
	fwrite(Header.Name,sizeof(char),  32,fp);
	fwrite(&Header.Width,sizeof(int),  1,fp);
	fwrite(&Header.Height,sizeof(int), 1,fp);
	fwrite(&Header.ChunkWidth,sizeof(int),  1,fp);
	fwrite(&Header.ChunkHeight,sizeof(int), 1,fp);
	fwrite(Header.ChunkOffsets,sizeof(int), Header.ChunkWidth * Header.ChunkHeight,fp);
}

void Area::LoadHeader(FILE *fp)
{
	fread(Header.Name,sizeof(char),  32,fp);
	fread(&Header.Width,sizeof(int),  1,fp);
	fread(&Header.Height,sizeof(int), 1,fp);
	fread(&Header.ChunkWidth,sizeof(int),  1,fp);
	fread(&Header.ChunkHeight,sizeof(int), 1,fp);
	if(Header.ChunkOffsets)
	{
		delete[] Header.ChunkOffsets;
	}

	Header.ChunkOffsets = new int[Header.ChunkWidth * Header.ChunkHeight];

	fread(Header.ChunkOffsets, sizeof(int), Header.ChunkWidth * Header.ChunkHeight,fp);

	this->Height = Header.Height;
	this->Width = Header.Width;
	
	this->ChunkWidth = Header.ChunkWidth;
	this->ChunkHeight = Header.ChunkHeight;

	this->UpdateWidth = this->Width / UPDATE_SEGMENT_WIDTH;
	this->UpdateHeight = this->Height / UPDATE_SEGMENT_HEIGHT;

}

  
//end: Outputs ********************************************************



//************ Debug ***************************************************
int Area::Smooth()
{
	return TRUE;
}

int Area::AddOverlay(int x, int y, int num)
{
	if(x < 0 || y < 0)
	{
		return FALSE;
	}

	if(num != -1)
	{
		if(BigMap[x/CHUNK_WIDTH + (y/CHUNK_HEIGHT * this->ChunkWidth)])
			BigMap[x/CHUNK_WIDTH + (y/CHUNK_HEIGHT * this->ChunkWidth)]->AddOverlay(x%CHUNK_WIDTH,y%CHUNK_HEIGHT,num);
	}
	else
	{
		//
		//add the overlays
		int ter, overlaynum;
		overlaynum = 0;
		ter = Valley->GetTerrain(x,y);
		ter = ter - (ter%64);
		if(ter == TerrainIndex[TER_GRASS_ONE] || ter == TerrainIndex[TER_GRASS_TWO])
		{
			switch(rand() % 6)
			{
			case 0:
				overlaynum = 3;
				break;
			case 1:
				overlaynum = 7;
				break;
			case 2:
				overlaynum = 8;
				break;
			case 3:
				overlaynum = 9;
				break;
			case 4:
				overlaynum = 10;
				break;
			case 5:
				overlaynum = 11;
				break;
			}
		}
		else
		if(ter == TerrainIndex[TER_GRASS_THREE])
		{
			switch(rand() % 7)
			{
			case 0:
				overlaynum = 2;
				break;
			case 1:
				overlaynum = 4;
				break;
			case 2:
				overlaynum = 5;
				break;
			case 3:
				overlaynum = 6;
				break;
			case 4:
				overlaynum = 9;
				break;
			case 5:
				overlaynum = 10;
				break;
			case 6:
				overlaynum = 11;
				break;
			}
		}
		else
		if(ter == TerrainIndex[TER_GRASS_FOUR])
		{
			switch(rand() % 6)
			{
			case 0:
				overlaynum = 2;
				break;
			case 1:
				overlaynum = 4;
				break;
			case 2:
				overlaynum = 5;
				break;
			case 3:
				overlaynum = 6;
				break;
			case 4:
				overlaynum = 9;
				break;
			case 5:
				overlaynum = 10;
				break;
			}
		}
		else
		if(ter ==TerrainIndex[TER_SAND])
		{
			switch(rand() % 5)
			{
			case 0:
				overlaynum =  3;
				break;
			case 1:
				overlaynum = 7;
				break;
			case 2:
				overlaynum = 8;
				break;
			case 3:
				overlaynum = 9;
				break;
			case 4:
				overlaynum = 10;
				break;
			}
		}
		else
		if(ter == TerrainIndex[TER_MOSS] ||
		   ter == TerrainIndex[TER_DIRT])
		{
			switch(rand() % 4)
			{
			case 0:
				overlaynum = 2;
				break;
			case 1:
				overlaynum = 4;
				break;
			case 2:
				overlaynum = 5;
				break;
			case 3:
				overlaynum = 6;
				break;
			}
		}
				
		if(overlaynum)
		{
			overlaynum = TerrainIndex[TER_MOSS_ONE + (overlaynum - 1)] + (rand()%4)*4;

			if(BigMap[x/CHUNK_WIDTH + (y/CHUNK_HEIGHT * this->ChunkWidth)])
				BigMap[x/CHUNK_WIDTH + (y/CHUNK_HEIGHT * this->ChunkWidth)]->AddOverlay(x%CHUNK_WIDTH,y%CHUNK_HEIGHT,overlaynum);
		}
	}
	return TRUE;
}

void Area::CreateChunks()
{
	int xn,yn;
	for(yn = 1200/CHUNK_TILE_HEIGHT - 1; yn <= 1200/CHUNK_TILE_HEIGHT + 1; yn++)
	for(xn = 1200/CHUNK_TILE_WIDTH - 1; xn <= 1200/CHUNK_TILE_WIDTH + 1; xn++)
	{
		LoadChunk(xn,yn);
		if(GetChunk(xn,yn))
			GetChunk(xn,yn)->CreateTexture(pBaseTexture);
	}
}

Object *Area::FindNextThing(Object *pThing, int x, int y)
{
	int SegX;
	int SegY;

	SegX = x/UPDATE_SEGMENT_WIDTH;
	SegY = y/UPDATE_SEGMENT_HEIGHT;

	Object *pOb;

	pOb = UpdateSegments[SegX + SegY * this->UpdateWidth];

	while(pOb && pOb != pThing)
	{
		pOb = pOb->GetNextUpdate();
	}

	if(pOb)
		pOb = pOb->GetNextUpdate();

	D3DVECTOR *pPosition;

	while(pOb)
	{
		pPosition = pOb->GetPosition();

		if((int)pPosition->x == x 
			&& (int)pPosition->y == y)
		{
			return pOb;
		}
		pOb = pOb->GetNextUpdate();
	}

	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		pOb = PreludeWorld->GetCombat()->GetCombatants();

		while(pOb && pOb != pThing)
		{
			pOb = pOb->GetNextUpdate();
		}

		if(pOb)
			pOb = pOb->GetNextUpdate();

		while(pOb)
		{
			pPosition = pOb->GetPosition();

			if((int)pPosition->x == x 
				&& (int)pPosition->y == y)
			{
				return pOb;
			}
			pOb = pOb->GetNextUpdate();
		}
	}

	return NULL;
}


void Area::Damage(float Radius, D3DVECTOR Center, int Min, int Max, DAMAGE_T Type, Thing *pSource)
{

	int xn;
	int yn;

	int xstart;
	int ystart;
	int xend;
	int yend;

	xstart = (int)(Center.x - Radius + 0.5f);
	ystart = (int)(Center.y - Radius + 0.5f);

	xend = (int)(Center.x + Radius + 0.5f);
	yend = (int)(Center.y + Radius + 0.5f);
	
	D3DVECTOR vAt;
	
	for(xn = xstart; xn <= xend; xn++)
	for(yn = ystart; yn <= yend; yn++)
	{
		vAt.x = (float)xn;
		vAt.y = (float)yn;
		vAt.z = 0.0f;

		if(GetDistance(&vAt, &Center) <= Radius)
		{
			DamageTile(xn,yn,Min,Max,Type, pSource);
		}
	}
}

void Area::DamageTile(int x, int y, int Min, int Max, DAMAGE_T Type, Thing *pSource)
{
	Object *pOb;

	pOb = FindThing(x,y);

	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_CREATURE)
		{
			int Damage;
			Damage = rand()%(Max+1-Min) + Min;
			((Creature *)pOb)->TakeDamage(pSource, Damage, Type);
		}
		pOb = FindNextThing(pOb,x,y);
	}

}
	
void Area::LoadChunk(int x, int y)
{
	if(!StaticFile)
	{
		return;
	}
	
	if(BigMap[y * this->ChunkWidth + x])
	{
		return;
	}

	if(!Header.ChunkOffsets[x + y * this->ChunkWidth])
		return;

	//allocate space for the new chunk
	Chunk *pChunk;

	pChunk = new Chunk;

	//position the file pointer
	int Error;

	Error = fseek(StaticFile, Header.ChunkOffsets[x + y * this->ChunkWidth], SEEK_SET);

	assert(!Error);

	pChunk->Load(StaticFile);


	BigMap[y*this->ChunkWidth + x] = pChunk;
	
	//pChunk->OutPutDebugInfo("chunk.txt");

}

void Area::SetTerrain(int x, int y, int NewVal)
{
	Chunk *pChunk;
	pChunk = GetChunk(x / CHUNK_TILE_WIDTH, y / CHUNK_TILE_HEIGHT);

	if(pChunk)
	{
		pChunk->SetTerrain((x%CHUNK_TILE_WIDTH)/2, (y%CHUNK_TILE_HEIGHT)/2,NewVal);
	}
}

int Area::GetTerrain(int x,int y)
{
	Chunk *pChunk;
	pChunk = GetChunk(x / CHUNK_WIDTH, y / CHUNK_HEIGHT);

	if(pChunk)
	{
		return pChunk->GetTerrain(x%CHUNK_WIDTH, y%CHUNK_HEIGHT);
	}

	return 0;
}

void Area::ClearTile(int x, int y)
{
	Chunk *pChunk;
	pChunk = GetChunk(x / CHUNK_TILE_WIDTH, y / CHUNK_TILE_HEIGHT);

	if(pChunk)
	{
		pChunk->ClearTile(x, y);
	}

	//now do the update list
	Object *pOb;

	pOb = UpdateSegments[(x/UPDATE_SEGMENT_WIDTH) + (y /UPDATE_SEGMENT_WIDTH) * this->UpdateWidth];

	while(pOb)
	{
		D3DVECTOR *pPosition;
		pPosition = pOb->GetPosition();
		if((int)pPosition->x == x && (int)pPosition->y == y)
		{
			RemoveFromUpdate(pOb);
			pOb = UpdateSegments[(x/UPDATE_SEGMENT_WIDTH) + (y /UPDATE_SEGMENT_WIDTH) * this->UpdateWidth];
		}
		else
		{
			pOb = pOb->GetNextUpdate();
		}
	}
}

BYTE Area::GetBlocking(int x, int y)
{
/* not necessary for now
	if(x <= 0 || y <= 0 || x >= this->Width || y >= this->Width)
	{
		return 1;
	}
*/
	Chunk *pChunk;
	pChunk = GetChunk(x / CHUNK_TILE_WIDTH, y / CHUNK_TILE_HEIGHT);

	if(!pChunk)
	{
		return 0;
	}
	else
	{
		return pChunk->GetBlocking(x % CHUNK_TILE_WIDTH, y % CHUNK_TILE_HEIGHT);
	}
}

void Area::ClearBlocking(int x, int y)
{
	if(x <= 0 || y <= 0 || x >= this->Width || y >= this->Width)
	{
		return;
	}
	
	Chunk *pChunk;
	pChunk = GetChunk(x / CHUNK_TILE_WIDTH, y / CHUNK_TILE_HEIGHT);

	if(!pChunk)
	{
		return;
	}
	else
	{
		pChunk->RemoveBlocking(x % CHUNK_TILE_WIDTH, y % CHUNK_TILE_HEIGHT);
	}
}

void Area::SetBlocking(int x, int y)
{
	if(x <= 0 || y <= 0 || x >= this->Width || y >= this->Width)
	{
		return;
	}
	
	Chunk *pChunk;
	pChunk = GetChunk(x / CHUNK_TILE_WIDTH, y / CHUNK_TILE_HEIGHT);

	if(!pChunk)
	{
		LoadChunk(x / CHUNK_TILE_WIDTH, y / CHUNK_TILE_HEIGHT);
		pChunk = GetChunk(x / CHUNK_TILE_WIDTH, y / CHUNK_TILE_HEIGHT);
	}

	pChunk->SetBlocking(x % CHUNK_TILE_WIDTH, y % CHUNK_TILE_HEIGHT);
}


//used whenever the blocking information for an area of the game has been lost (usually due to editting)
//resets the tile blocking in a given area based on the regions in that area
void Area::ReBlock(RECT *rArea)
{
	int xn,yn,n;
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = rArea->top/CHUNK_TILE_HEIGHT;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = rArea->left/CHUNK_TILE_WIDTH;

	if(StartX < 0)
		StartX = 0;

	EndX = rArea->right/CHUNK_TILE_WIDTH;
	EndY = rArea->bottom/CHUNK_TILE_HEIGHT;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;


	Region *pRegion;

	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth * NUM_CHUNK_REGIONS + StartX * NUM_CHUNK_REGIONS;
		for(xn = StartX; xn <= EndX; xn++)
		{
			for(n = 0; n < NUM_CHUNK_REGIONS; n++)
			{
				if(Regions[Offset + n])
				{
					pRegion = PreludeWorld->GetRegion(Regions[Offset + n]);
				    pRegion->ReBlock();
				}
				else
				{
					break;
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}
}

void Area::DrawRegions()
{
	int xn,yn,n;
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;

	if(EndX >= ChunkWidth)
		EndX = ChunkWidth -1;
	if(EndY >= ChunkHeight)
		EndY = ChunkHeight -1;


	Region *pRegion;

	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth * NUM_CHUNK_REGIONS + StartX * NUM_CHUNK_REGIONS;
		for(xn = StartX; xn <= EndX; xn++)
		{
			for(n = 0; n < NUM_CHUNK_REGIONS; n++)
			{
				if(Regions[Offset + n])
				{
					pRegion = PreludeWorld->GetRegion(Regions[Offset + n]);
					if(pRegion)
					{
						pRegion->Draw();
					}
				}
				else
				{
					break;
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}
	
	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth * NUM_CHUNK_REGIONS + StartX * NUM_CHUNK_REGIONS;
		for(xn = StartX; xn <= EndX; xn++)
		{
			for(n = 0; n < NUM_CHUNK_REGIONS; n++)
			{
				if(Regions[Offset + n])
				{
					pRegion =  PreludeWorld->GetRegion(Regions[Offset + n]);
					if(pRegion)
					{
						pRegion->UnDraw();
					}
				//	else
				//	{
				//		LoadRegion(Regions[Offset + n]);
				//	}
				}
				else
				{
					break;
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}

}

Region *Area::GetRegion(int x, int y)
{
	D3DVECTOR vAt;
	vAt.x = (float)x;
	vAt.y = (float)y;
	vAt.z = 0.0f;
	return GetRegion(&vAt);
}

Region *Area::GetRegion(D3DVECTOR *vAt)
{
	int xn, yn;
	
	xn = (int)vAt->x / CHUNK_TILE_WIDTH;
	yn = (int)vAt->y / CHUNK_TILE_HEIGHT;

	if(xn >= this->ChunkWidth || yn >= this->ChunkHeight)
	{
		DEBUG_INFO("Get region out of bounds\n");
		return NULL;
	}

	Region *pRegion;
	int Offset;
	Offset = xn * NUM_CHUNK_REGIONS + yn * (Width / CHUNK_TILE_WIDTH) * NUM_CHUNK_REGIONS;
	
	for(int n = 0; n < NUM_CHUNK_REGIONS; n++)
	{
		if(Regions[Offset + n])
		{
			pRegion = PreludeWorld->GetRegion(Regions[Offset + n]);
			if(pRegion)
			{
				Region *pSub;
				pSub = pRegion->PointIntersect(vAt);
				if(pSub)
				{
					return pSub;
				}
			}
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}

Object *Area::GetTarget(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	Object *pOb;
	int xn, yn, Offset;

	if(PreludeWorld->InCombat())
	{
		pOb = PreludeWorld->GetCombat()->Combatants;

		while(pOb)
		{
			if(pOb->RayIntersect(vRayStart,vRayEnd))
			{
				return pOb;
			}
			pOb = pOb->GetNextUpdate();
		}
	 }
	
	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			pOb = UpdateSegments[Offset];

			while(pOb)
			{
				if(pOb->RayIntersect(vRayStart,vRayEnd))
				{
					return pOb;
				}

				pOb = pOb->GetNextUpdate();
			}
			Offset++;
		}
		Offset+= this->UpdateWidth - (1 + PreludeWorld->UpdateRect.right - PreludeWorld->UpdateRect.left);
	}

	pOb = PreludeWorld->GetMainObjects();

	while(pOb)
	{
		if(pOb->GetObjectType() != OBJECT_STATIC)
		{
			if(pOb->RayIntersect(vRayStart,vRayEnd))
			{
				return pOb;
			}
		}
		pOb = pOb->GetNext();
	}

	return NULL;
}

Object *Area::GetStaticTarget(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	Object *pOb;
	int xn, yn, Offset;
	int EndY;
	int EndX;
	
	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	pOb = PreludeWorld->GetMainObjects();

	while(pOb)
	{
		if(pOb->RayIntersect(vRayStart,vRayEnd))
		{
			return pOb;
		}
		pOb = pOb->GetNext();
	}

	int StartY;
	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
	
	int StartX;
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;


	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth;
		for(xn = StartX; xn <= EndX; xn++)
		{
			if(BigMap[Offset + xn])
			{
				pOb = BigMap[Offset + xn]->GetObjects();
				while(pOb)
				{
					if(pOb->RayIntersectAlwaysCheck(vRayStart,vRayEnd))
					{
						return pOb;
					}
					pOb = pOb->GetNext();
				}

			}
		}
	}

	//check objects in regions;
	Region *pRegion;

	pRegion = GetRegion(vRayEnd);

	if(pRegion)
	{
		pOb = pRegion ->GetObjects();
		while(pOb)
		{
			if(pOb->RayIntersectAlwaysCheck(vRayStart,vRayEnd))
			{
				return pOb;
			}
			pOb = pOb->GetNext();
		}
	}
	

   return NULL;
}

void Area::LoadNonStatic(FILE *fp)
{
	FILE *myfp;

	myfp = fp;

	if(!myfp)
	{
		char blarg[64];
		sprintf(blarg,"%s.dyn",Header.Name);
		myfp = SafeFileOpen(blarg,"rb");
	}

	long ToNext;
	
	fread(Header.Name,sizeof(char),32,myfp);
	fread(&ToNext,sizeof(long),1,myfp);
	
	int NumObjects;
	Object *pOb;
	int Offset = 0;
	int MaxOffset;
	int n;

	MaxOffset = this->UpdateWidth * this->UpdateHeight;

	if(UpdateSegments)
	{
		//clear all current update objects
		for(Offset = 0; Offset < MaxOffset; Offset++)
		{
			while(UpdateSegments[Offset])
			{
				pOb = UpdateSegments[Offset];
				RemoveFromUpdate(pOb);
				if(pOb->GetObjectType() != OBJECT_CREATURE)
					delete pOb;
			}
		}
	}
	else
	{
		UpdateSegments = new Object *[this->UpdateWidth * this->UpdateHeight];
	}

	memset(UpdateSegments,0, sizeof(Object *) * this->UpdateWidth * this->UpdateHeight);

	for(Offset = 0; Offset < MaxOffset; Offset++)
	{
		UpdateSegments[Offset] = NULL;

		NumObjects = 0;
		
		fread(&NumObjects,sizeof(int),1,myfp);

		for(n = 0; n < NumObjects; n++)
		{
			pOb = LoadObject(myfp);
			if(pOb)
			{
				pOb->AddToArea(this);
				//AddToUpdate(pOb);
			}
		}
	}
	
	if(!fp)
	{
		fclose(myfp);
	}

	return;
}


void Area::SaveNonStatic(FILE *fp)
{
	FILE *myfp;
	 
	myfp = fp;

	if(!myfp)
	{
		char blarg[64];
		sprintf(blarg,"%s.dyn",Header.Name);
		myfp = SafeFileOpen(blarg,"wb");
	}

	long ToNext;
	fpos_t NextPos;
	fpos_t EndPos;
	
	ToNext = 0;

	fwrite(Header.Name,sizeof(char),32,myfp);
	
	fgetpos(myfp, &NextPos);
	fwrite(&ToNext,sizeof(long),1,myfp);

	int NumObjects;
	Object *pOb;
	int Offset = 0;
	//remove the party, they are handled seperately
	
	int MaxOffset;

	MaxOffset = this->UpdateWidth * this->UpdateHeight;
	
	for(Offset = 0; Offset < MaxOffset; Offset++)
	{
		NumObjects = 0;
		pOb = UpdateSegments[Offset];

		//save the objects in each segment
		//objects can be
		//items
		//people
		//portals
		//events

		while(pOb)
		{
			NumObjects++;
			pOb = pOb->GetNextUpdate();
		}
		fwrite(&NumObjects,sizeof(int),1,myfp);

		pOb = UpdateSegments[Offset];

		while(pOb)
		{
			pOb->Save(myfp);
			pOb = pOb->GetNextUpdate();
		}
	}

	fgetpos(myfp,&EndPos);
	
	ToNext = ftell(myfp);

	fseek(myfp, NextPos, SEEK_SET);
	
	fwrite(&ToNext,sizeof(long),1,myfp);
	
	fseek(myfp, EndPos, SEEK_SET);

	//add the party back to the update

	if(!fp)
	{
		fclose(myfp);
	}

}

Object *Area::FindOtherObject(int x, int y, OBJECT_T oType, Object *pExclude)
{
	Object *pOb;
	
	pOb = UpdateSegments[x / UPDATE_SEGMENT_WIDTH + (y/UPDATE_SEGMENT_HEIGHT)*(Width / UPDATE_SEGMENT_WIDTH)];

	while(pOb)
	{
		if(pOb != pExclude && pOb->GetObjectType() == oType && pOb->TileIntersect(x,y))
		{
			return pOb;
		}
		else
		{
			pOb = pOb->GetNextUpdate();
		}
	}

	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		pOb = PreludeWorld->GetCombat()->GetCombatants();
		while(pOb)
		{
			if(pOb != pExclude && pOb->GetObjectType() == oType && pOb->TileIntersect(x,y))
			{
				return pOb;
			}
			else
			{
				pOb = pOb->GetNextUpdate();
			}
		}
	}

	return NULL;



}

Object *Area::FindObject(int x, int y, OBJECT_T oType)
{
	Object *pOb;
	
	pOb = UpdateSegments[x / UPDATE_SEGMENT_WIDTH + (y/UPDATE_SEGMENT_HEIGHT)*(Width / UPDATE_SEGMENT_WIDTH)];

	while(pOb)
	{
		if(pOb->GetObjectType() == oType && pOb->TileIntersect(x,y))
		{
			return pOb;
		}
		else
		{
			pOb = pOb->GetNextUpdate();
		}
	}

	if(PreludeWorld->GetGameState() == GAME_STATE_COMBAT)
	{
		pOb = PreludeWorld->GetCombat()->GetCombatants();
		while(pOb)
		{
			if(pOb->GetObjectType() == oType && pOb->TileIntersect(x,y))
			{
				return pOb;
			}
			else
			{
				pOb = pOb->GetNextUpdate();
			}
		}
	}

	return NULL;
}

void Area::UnOccupyRegions()
{
	int xn,yn,n;
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = PreludeWorld->ScreenY - PreludeWorld->DrawRadius;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = PreludeWorld->ScreenX - PreludeWorld->DrawRadius;

	if(StartX < 0)
		StartX = 0;

	EndX = PreludeWorld->ScreenX + PreludeWorld->DrawRadius;
	EndY = PreludeWorld->ScreenY + PreludeWorld->DrawRadius;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;



	Region *pRegion;

	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth * NUM_CHUNK_REGIONS + StartX * NUM_CHUNK_REGIONS;
		for(xn = StartX; xn <= EndX; xn++)
		{
			for(n = 0; n < NUM_CHUNK_REGIONS; n++)
			{
				if(Regions[Offset + n])
				{
					pRegion = PreludeWorld->GetRegion(Regions[Offset + n]);
					if(pRegion)
						pRegion->UnOccupy();
				}
				else
				{
					break;
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}
}


Object *Area::GetEvent(float x, float y)
{
	D3DVECTOR Check;
	Check.x = x;
	Check.y = y;
	Check.z = Valley->GetTileHeight(x,y);

	int xn; 
	int yn;
	int Offset;
	Object *pOb;

	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			pOb = UpdateSegments[Offset];

			while(pOb)
			{
				if(pOb->GetObjectType() == OBJECT_EVENT)
				{
					Event *pEvent;
					pEvent = (Event *)pOb;
					if(GetDistance(&Check,pEvent->GetPosition()) < pEvent->GetRadius() + 0.1f)
					{
						return pOb;
					}
				}
				pOb = pOb->GetNextUpdate();
			}
			Offset++;
		}
		Offset+= this->UpdateWidth - (1 + PreludeWorld->UpdateRect.right - PreludeWorld->UpdateRect.left);
	}
	return NULL;
}

void Area::UnRegionObjects(RECT *rArea)
{
	int xn,yn,n;
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = rArea->top/CHUNK_TILE_HEIGHT;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = rArea->left/CHUNK_TILE_WIDTH;

	if(StartX < 0)
		StartX = 0;

	EndX = rArea->right/CHUNK_TILE_WIDTH;
	EndY = rArea->bottom/CHUNK_TILE_HEIGHT;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;


	Region *pRegion;

	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth * NUM_CHUNK_REGIONS + StartX * NUM_CHUNK_REGIONS;
		for(xn = StartX; xn <= EndX; xn++)
		{
			for(n = 0; n < NUM_CHUNK_REGIONS; n++)
			{
				if(Regions[Offset + n])
				{
					pRegion = PreludeWorld->GetRegion(Regions[Offset + n]);
				   pRegion->UnGatherObjects();
				}
				else
				{
					break;
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}

}

void Area::RegionObjects(RECT *rArea)
{
	int xn,yn,n;
	int Offset;
	int StartY;
	int EndY;
	int StartX;
	int EndX;

	StartY = rArea->top/CHUNK_TILE_HEIGHT;
	
	if(StartY < 0)
		StartY = 0;
		
	StartX = rArea->left/CHUNK_TILE_WIDTH;

	if(StartX < 0)
		StartX = 0;

	EndX = rArea->right/CHUNK_TILE_WIDTH;
	EndY = rArea->bottom/CHUNK_TILE_HEIGHT;

	if(EndX >= Valley->ChunkWidth)
		EndX = Valley->ChunkWidth-1;
	if(EndY >= Valley->ChunkHeight)
		EndY = Valley->ChunkHeight-1;

	Region *pRegion;

	for(yn = StartY; yn <= EndY; yn++)
	{
		Offset = yn * this->ChunkWidth * NUM_CHUNK_REGIONS + StartX * NUM_CHUNK_REGIONS;
		for(xn = StartX; xn <= EndX; xn++)
		{
			for(n = 0; n < NUM_CHUNK_REGIONS; n++)
			{
				if(Regions[Offset + n])
				{
					pRegion = PreludeWorld->GetRegion(Regions[Offset + n]);
				   pRegion->GatherObjects();
				}
				else
				{
					break;
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}
}


void Area::Cut(RECT *rArea)
{
	int xn;
	int yn;

	for(yn = 0; yn < (rArea->top / CHUNK_TILE_WIDTH); yn++)
	for(xn = 0; xn < ChunkWidth; xn++)
	{
		Header.ChunkOffsets[yn * ChunkWidth + xn] = 0;
		if(BigMap[yn * this->ChunkWidth + xn])
		{
			delete BigMap[yn * this->ChunkWidth + xn];
			BigMap[yn * this->ChunkWidth + xn] = NULL;
		}
	
	}

	for(yn = (rArea->top / CHUNK_TILE_HEIGHT); yn <= (rArea->bottom / CHUNK_TILE_HEIGHT); yn++)
	for(xn = 0; xn < (rArea->left / CHUNK_TILE_WIDTH); xn++)
	{
		Header.ChunkOffsets[yn * ChunkWidth + xn] = 0;
		if(BigMap[yn * this->ChunkWidth + xn])
		{
			delete BigMap[yn * this->ChunkWidth + xn];
			BigMap[yn * this->ChunkWidth + xn] = NULL;
		}
	}
	
	for(yn = (rArea->top / CHUNK_TILE_HEIGHT); yn <= (rArea->bottom / CHUNK_TILE_HEIGHT); yn++)
	for(xn = (rArea->right / CHUNK_TILE_WIDTH) + 1; xn < ChunkWidth; xn++)
	{
		Header.ChunkOffsets[yn * ChunkWidth + xn] = 0;
		if(BigMap[yn * this->ChunkWidth + xn])
		{
			delete BigMap[yn * this->ChunkWidth + xn];
			BigMap[yn * this->ChunkWidth + xn] = NULL;
		}
	}

	for(yn = (rArea->bottom / CHUNK_TILE_HEIGHT) + 1; yn < ChunkHeight; yn++)
	for(xn = 0; xn < ChunkWidth; xn++)
	{
		Header.ChunkOffsets[yn * ChunkWidth + xn] = 0;
		if(BigMap[yn * this->ChunkWidth + xn])
		{
			delete BigMap[yn * this->ChunkWidth + xn];
			BigMap[yn * this->ChunkWidth + xn] = NULL;
		}
	}
}

void  Area::ImportStuff(FILE *fpImport)
{
	SetCurrentDirectory(".\\Areas");

	FILE *newfp;

	newfp = SafeFileOpen("worldnew.bin","wb");

	SaveHeader(newfp);

	OLD_FILE_HEADER_T MainHeader;
	
	fread(&MainHeader, sizeof(OLD_FILE_HEADER_T), 1, fpImport);

	int xn, yn;
	Chunk *pChunk;
	pChunk = NULL;
	Chunk *pImportChunk;
	int cxn, cyn, con;

	for(yn = 0; yn < ChunkHeight; yn++)
	{
		for(xn = 0; xn < ChunkWidth; xn++)
		{
			pChunk = GetChunk(xn, yn);
			if(!pChunk)
			{
				LoadChunk(xn,yn);
				pChunk = GetChunk(xn,yn);
			}

			if(MainHeader.ChunkOffsets[xn + yn * ChunkWidth])
			{
				fseek(fpImport,MainHeader.ChunkOffsets[xn + yn * ChunkWidth], SEEK_SET);
	
				pImportChunk = new Chunk;
				pImportChunk->Load(fpImport);
			
				//now copy terrain and objects to the chunk

				//objects, just switch the pointers
				Object *pOb;
				pOb = pChunk->GetObjects();
				
				pChunk->pObjectList = pImportChunk->GetObjects();
				pImportChunk->pObjectList = pOb;

				pOb = pChunk->GetObjects();

				while(pOb)
				{
					float ZDiff;

					ZDiff = pOb->GetPosition()->z - pImportChunk->GetHeight((int)pOb->GetPosition()->x % CHUNK_TILE_WIDTH, (int)pOb->GetPosition()->y % CHUNK_TILE_HEIGHT); 
					
					D3DVECTOR *pPosition;
					pPosition = pOb->GetPosition();
					pPosition->z = pChunk->GetHeight((int)pOb->GetPosition()->x % CHUNK_TILE_WIDTH, (int)pOb->GetPosition()->y % CHUNK_TILE_HEIGHT) + ZDiff;
				
					pOb = pOb->GetNext();

				}


				//now copy terrain
				for(cyn = 0; cyn < CHUNK_WIDTH; cyn++)
				for(cxn = 0; cxn < CHUNK_HEIGHT; cxn++)
				{
					pChunk->Terrain[cxn][cyn] = pImportChunk->Terrain[cxn][cyn];
					for(con = 0; con < NUM_OVERLAYS; con++)
					{
						pChunk->Overlays[cxn][cyn][con] = pImportChunk->Overlays[cxn][cyn][con];
					}
				}

				//now blocking info
				for(con = 0; con < CHUNK_HEIGHT * 2; con++)
				{
					pChunk->Blocking[con] = pImportChunk->Blocking[con];
				}
				
				delete pImportChunk;
				pImportChunk = NULL;
			}
				
			if(pChunk)
			{
				Header.ChunkOffsets[xn + yn * this->ChunkWidth] = ftell(newfp);
				pChunk->Save(newfp);
				delete pChunk;
				pChunk = NULL;
			}
			else
			{
				DEBUG_INFO("Should have Found Chunk\n");
				Describe("problem during getstuff.");
				Header.ChunkOffsets[xn + yn * this->ChunkWidth] = NULL;
			}
			BigMap[xn + yn* this->ChunkWidth] = NULL;
		}
	}

	//memcpy(&Header,&NewHeader,sizeof(Header));

	fseek(newfp,0,SEEK_SET);
	
	SaveHeader(newfp);
	
	fclose(newfp);
	
	fclose(StaticFile);
	
	char filename[64];
	sprintf(filename,"%s.bin",Header.Name);

	rename(filename,"worldold.bin");
	rename("worldnew.bin",filename);
	
	StaticFile = SafeFileOpen(filename,"rb");
	Describe("Done Combined Saving");

	SetCurrentDirectory(Engine->GetRootDirectory());
	return;
}


void Area::Combine(FILE *fpImport)
{
	SetCurrentDirectory(".\\Areas");

	FILE *newfp;

	newfp = SafeFileOpen("worldnew.bin","wb");

	SaveHeader(newfp);

	OLD_FILE_HEADER_T MainHeader;
	
	fread(&MainHeader, sizeof(OLD_FILE_HEADER_T), 1, fpImport);

	int xn, yn;
	Chunk *pChunk;
	pChunk = NULL;
	Chunk *pImportChunk;
	int cxn, cyn, con;

	for(yn = 0; yn < ChunkHeight; yn++)
	{
		for(xn = 0; xn < ChunkWidth; xn++)
		{
			pChunk = GetChunk(xn, yn);
			if(!pChunk)
			{
				LoadChunk(xn,yn);
				pChunk = GetChunk(xn,yn);
			}

			if(MainHeader.ChunkOffsets[xn + yn * ChunkWidth])
			{
				fseek(fpImport,MainHeader.ChunkOffsets[xn + yn * ChunkWidth], SEEK_SET);
	
				pImportChunk = new Chunk;
				pImportChunk->Load(fpImport);
			
				//now copy terrain and objects to the chunk

				//objects, just switch the pointers
				Object *pOb;
				pOb = pChunk->GetObjects();
				
				pChunk->pObjectList = pImportChunk->GetObjects();
				pImportChunk->pObjectList = pOb;

				pOb = pChunk->GetObjects();
				memcpy(pChunk->Verts, pImportChunk->Verts, sizeof(float) * NUM_CHUNK_VERTS * 6);
				memcpy(pChunk->TileHeights, pImportChunk->TileHeights, sizeof(float) * CHUNK_TILE_HEIGHT * CHUNK_TILE_WIDTH);

				while(pOb)
				{
					float ZDiff;

					ZDiff = pOb->GetPosition()->z - pImportChunk->GetHeight((int)pOb->GetPosition()->x % CHUNK_TILE_WIDTH, (int)pOb->GetPosition()->y % CHUNK_TILE_HEIGHT); 
					
					D3DVECTOR *pPosition;
					pPosition = pOb->GetPosition();
					pPosition->z = pChunk->GetHeight((int)pOb->GetPosition()->x % CHUNK_TILE_WIDTH, (int)pOb->GetPosition()->y % CHUNK_TILE_HEIGHT) + ZDiff;
				
					pOb = pOb->GetNext();

				}


				//now copy terrain
				for(cyn = 0; cyn < CHUNK_WIDTH; cyn++)
				for(cxn = 0; cxn < CHUNK_HEIGHT; cxn++)
				{
					pChunk->Terrain[cxn][cyn] = pImportChunk->Terrain[cxn][cyn];
					for(con = 0; con < NUM_OVERLAYS; con++)
					{
						pChunk->Overlays[cxn][cyn][con] = pImportChunk->Overlays[cxn][cyn][con];
					}
				}

				

				//now blocking info
				for(con = 0; con < CHUNK_HEIGHT * 2; con++)
				{
					pChunk->Blocking[con] = pImportChunk->Blocking[con];
				}
				
				delete pImportChunk;
				pImportChunk = NULL;
			}
				
			if(pChunk)
			{
				Header.ChunkOffsets[xn + yn * this->ChunkWidth] = ftell(newfp);
				pChunk->Save(newfp);
				delete pChunk;
				pChunk = NULL;
			}
			else
			{
				DEBUG_INFO("Should have Found Chunk\n");
				Describe("problem during getstuff.");
				Header.ChunkOffsets[xn + yn * this->ChunkWidth] = NULL;
			}
			BigMap[xn + yn* this->ChunkWidth] = NULL;
		}
	}

	//memcpy(&Header,&NewHeader,sizeof(Header));

	fseek(newfp,0,SEEK_SET);
	
	SaveHeader(newfp);
	
	fclose(newfp);
	
	fclose(StaticFile);
	
	char filename[64];
	sprintf(filename,"%s.bin",Header.Name);

	rename(filename,"worldold.bin");
	rename("worldnew.bin",filename);
	
	StaticFile = SafeFileOpen(filename,"rb");
	Describe("Done Combined Saving");

	SetCurrentDirectory(Engine->GetRootDirectory());
	return;
}

Object *Area::GetUpdateSegment(int SegX, int SegY)
{
	if(SegX < 0 || SegY < 0 || SegX >= this->UpdateWidth || SegY >= this->UpdateHeight)
	{
		return NULL;
	}

	int Offset;

	Offset = SegX + SegY * this->UpdateWidth;
	
	return UpdateSegments[Offset];
}

void Area::IllegalActivity(Object *pLawBreaker)
{
	int x;
	int y;
	x = pLawBreaker->GetPosition()->x;
	y = pLawBreaker->GetPosition()->y;

	int xn;
	int yn;
	int xStart;
	int yStart;
	int xEnd;
	int yEnd;
	Object *pOb;

	if(!PreludeWorld->GetCurAreaNum())
	{
	for(int n = 0; n < PreludeWorld->NumTowns; n ++)
	{
		if( (x >= PreludeWorld->TownAreas[n].left && x <= PreludeWorld->TownAreas[n].right) &&
			(y >= PreludeWorld->TownAreas[n].top && y <= PreludeWorld->TownAreas[n].bottom))
		{
			xStart = PreludeWorld->TownAreas[n].left / UPDATE_SEGMENT_WIDTH;
			yStart = PreludeWorld->TownAreas[n].top / UPDATE_SEGMENT_HEIGHT;
			xEnd = PreludeWorld->TownAreas[n].right / UPDATE_SEGMENT_WIDTH;
			yEnd = PreludeWorld->TownAreas[n].bottom / UPDATE_SEGMENT_HEIGHT;

			for(yn = yStart; yn <= yEnd; yn++)
			for(xn = xStart; xn <= xEnd; xn++)
			{
				pOb = UpdateSegments[xn + yn * this->UpdateWidth];
				while(pOb)
				{
					if(pOb->GetObjectType() == OBJECT_CREATURE)
					{
						x = pOb->GetPosition()->x;
						y = pOb->GetPosition()->y;
						if((x >= PreludeWorld->TownAreas[n].left && x <= PreludeWorld->TownAreas[n].right) &&
								(y >= PreludeWorld->TownAreas[n].top && y <= PreludeWorld->TownAreas[n].bottom))
						{
							Creature *pCreature;

							pCreature = (Creature *)pOb;
							if(!PreludeParty.IsMember(pCreature))
							{
								pCreature->SetData(INDEX_BATTLESIDE,1);
								pCreature->SetData(INDEX_AICODE,1);
							}
						}
					}
					pOb = pOb->GetNextUpdate();
				}
			}
			return;
		}
	}
	}

	xStart = (x - 32) / UPDATE_SEGMENT_WIDTH;
	yStart = (y - 32) / UPDATE_SEGMENT_HEIGHT;
	xEnd = (x + 32) / UPDATE_SEGMENT_WIDTH;
	yEnd = (y + 32) / UPDATE_SEGMENT_HEIGHT;

	if(xStart < 0) xStart = 0;
	if(yStart < 0) yStart = 0;
	if(xEnd >= this->UpdateWidth) xEnd = this->UpdateWidth-1;
	if(yEnd >= this->UpdateHeight) yEnd = this->UpdateHeight-1;

	for(yn = yStart; yn <= yEnd; yn++)
	for(xn = xStart; xn <= xEnd; xn++)
	{
		pOb = UpdateSegments[xn + yn * this->UpdateWidth];
		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_CREATURE)
			{
				Creature *pCreature;
				pCreature = (Creature *)pOb;
				if(!PreludeParty.IsMember(pCreature))
					pCreature->SetData(INDEX_BATTLESIDE,1);
			}
			pOb = pOb->GetNextUpdate();
		}
	}
	return;
}

int Area::UpdateOffscreen()
{

	Creature *pCreature;


	pCreature = pCreature->GetFirst();

	while(pCreature)
	{
		pCreature->UpdateOffScreen();
		pCreature = (Creature *)pCreature->GetNext();
	}
/*
	Offset = PreludeWorld->UpdateRect.left + PreludeWorld->UpdateRect.top * this->UpdateWidth;
	
	for(yn = PreludeWorld->UpdateRect.top; yn <= PreludeWorld->UpdateRect.bottom; yn++)
	{
		for(xn = PreludeWorld->UpdateRect.left; xn <= PreludeWorld->UpdateRect.right; xn++)
		{
			pOb = UpdateSegments[Offset];

			while(pOb)
			{
				if(pOb->GetObjectType() == OBJECT_CREATURE)
				{
					pCreature = (Creature *)pOb;
					pCreature->UpdateOffScreen();
				}
				pOb = pOb->GetNext();
			}
			Offset++;
		}
		Offset+= this->UpdateWidth - (1 + PreludeWorld->UpdateRect.right - PreludeWorld->UpdateRect.left);
	}

	return TRUE;

	//update everything as needed
	int xn,yn;
	for(yn = 0; yn < this->UpdateRect.top; yn++)
	for(xn = 0; xn < this->Width; xn++)
	{
		

	}
	for(yn = this->UpdateRect.top; yn < this->UpdateRect.bottom; yn++)
	for(xn = 0; xn < UpdateRect.left; xn++)
	{
		

	}
	for(yn = this->UpdateRect.top; yn < this->UpdateRect.bottom; yn++)
	for(xn = UpdateRect.right; xn < this->Width; xn++)
	{
		

	}
	for(yn = this->UpdateRect.bottom; yn < this->Height; yn++)
	for(xn = 0; xn < this->Width; xn++)
	{
		

	}
*/

	return TRUE;
}

void Area::AddRegion(Region *ToAdd)
{
	RECT Bounds;
	int xn, yn, n;
	int Offset;

	ToAdd->GetBounds(&Bounds);
	BOOL Found = FALSE;
	for(yn = Bounds.top / CHUNK_TILE_HEIGHT; yn <= Bounds.bottom / CHUNK_TILE_HEIGHT; yn++)
	{
		Offset = yn * (Width / CHUNK_TILE_HEIGHT) * NUM_CHUNK_REGIONS + (Bounds.left / CHUNK_TILE_WIDTH) * NUM_CHUNK_REGIONS;
		for(xn = Bounds.left / CHUNK_TILE_WIDTH; xn <= Bounds.right / CHUNK_TILE_WIDTH; xn++)
		{
			Found = FALSE;
			for( n = 0; n < NUM_CHUNK_REGIONS; n++ )
			{
				if(this->Regions[Offset + n] == ToAdd->GetID())
				{
					Found = TRUE;
					break;
				}
			}

			if(!Found)
			{
				for( n = 0; n < NUM_CHUNK_REGIONS; n++ )
				{
					if(!this->Regions[Offset + n])
					{
						this->Regions[Offset + n] = ToAdd->GetID();
						break;
					}
				}
			}
			Offset += NUM_CHUNK_REGIONS;
		}
	}
}

void Area::SetEventHeights()
{
	int xn;
	int yn;
	Object *pOb;

	for(yn = 0; yn < UpdateHeight; yn++)
	for(xn = 0; xn < UpdateWidth; xn++)
	{
		pOb = UpdateSegments[xn + yn * UpdateWidth];

		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_EVENT)
			{
				D3DVECTOR *pPosition;
				pPosition = pOb->GetPosition();

				pPosition->z = GetTileHeight((int)pPosition->x,(int)pPosition->y);

			}
			pOb = pOb->GetNextUpdate();
		}
	}
}

void Area::LoadSubDynamic(RECT *rArea, const char *fname, OBJECT_T ToLoad)
{
	FILE *fp;
	RECT rPArea;
	rPArea.left = 0;
	rPArea.right = this->Width - 1;
	rPArea.top = 0;
	rPArea.bottom = this->Height - 1;

	ClearDynamic(&rPArea, OBJECT_CREATURE);

	if(!fname) return;

	fp = fopen(fname,"rb");
	if(!fp)
	{
		Describe("couldn't import sub dynamic");
		return;
	}

	Object *pOb;

	ClearDynamic(rArea, ToLoad);

	long ToNext;
	
	fread(Header.Name,sizeof(char),32,fp);
	fread(&ToNext,sizeof(long),1,fp);
	
	int NumObjects;
	int Offset = 0;
	int MaxOffset;
	int n;

	MaxOffset = this->UpdateWidth * this->UpdateHeight;

	for(Offset = 0; Offset < MaxOffset; Offset++)
	{
		NumObjects = 0;
		
		fread(&NumObjects,sizeof(int),1,fp);

		for(n = 0; n < NumObjects; n++)
		{
			pOb = LoadObject(fp);
			if(ToLoad)
			{
				if(pOb && 
				  pOb->GetObjectType() == ToLoad &&
				  pOb->GetPosition()->x >= rArea->left &&
				  pOb->GetPosition()->x <= rArea->right &&
				  pOb->GetPosition()->y >= rArea->top &&
				  pOb->GetPosition()->y <= rArea->bottom)
				{
					pOb->AddToArea(this);
				}
				else
				{
				//	delete pOb;
				}
			}
			else
			{
				if(pOb &&
				  pOb->GetObjectType() != OBJECT_CREATURE &&
				  pOb->GetPosition()->x >= rArea->left &&
				  pOb->GetPosition()->x <= rArea->right &&
				  pOb->GetPosition()->y >= rArea->top &&
				  pOb->GetPosition()->y <= rArea->bottom)
				{
					pOb->AddToArea(this);
				}
				else
				{
					delete pOb;
				}
			
			}
		}
	}

	fclose(fp);
	
	return;
}

void Area::ClearDynamic(RECT *pArea, OBJECT_T ToClear)
{
	RECT rArea;

	if(pArea)
	{
		rArea = *pArea;
	}
	else
	{
		rArea.left = 0;
		rArea.right = this->Width - 1;
		rArea.top = 0;
		rArea.bottom = this->Height - 1;
	}

	int xn;
	int yn;
	Object *pOb;
	int xEnd;
	xEnd = (rArea. right / UPDATE_SEGMENT_WIDTH) ; 
	int yEnd;
	yEnd = (rArea. bottom / UPDATE_SEGMENT_HEIGHT) ; 
	

	for(yn = rArea. top / UPDATE_SEGMENT_WIDTH; yn < yEnd; yn++)
	for(xn = rArea. left / UPDATE_SEGMENT_HEIGHT; xn < xEnd; xn++)
	{
		pOb = UpdateSegments[xn + yn * this->UpdateWidth];
			
		while(pOb)
		{
			if(ToClear)
			{
				if(pOb->GetObjectType() == ToClear)
				{
					RemoveFromUpdate(pOb,xn,yn);
					if(pOb->GetObjectType() != OBJECT_CREATURE)
					{
						//delete pOb;
					}
					pOb = UpdateSegments[xn + yn * this->UpdateWidth];
				}
				else
				{
					pOb = pOb->GetNextUpdate();
				}
			}
			else
			{
				RemoveFromUpdate(pOb,xn,yn);
				if(pOb->GetObjectType() != OBJECT_CREATURE)
				{
					//delete pOb;
				}
				pOb = UpdateSegments[xn + yn * this->UpdateWidth];
			}
		}
	}
}

void Area::ResetUpdate(int x, int y)
{
	UpdateSegments[x + y * this->UpdateWidth] = NULL;
}

void Area::FixHeightMap()
{
	int xn, yn;
	Chunk *pChunk;
	for(yn = 0; yn < this->ChunkHeight; yn++)
	for(xn = 0; xn < this->ChunkWidth; xn++)
	{
		LoadChunk(xn,yn);
		pChunk = GetChunk(xn,yn);
		pChunk->CreateHeightMap();
	}
	Describe("height map fixed");
}


void Area::ClearWalls(RECT *pArea)
{



}

void Area::FlattenWalls(RECT *pArea)
{
	int xn;
	int yn;
	Chunk *pChunk;
	Object *pOb;
	
	for(yn = pArea->top / CHUNK_TILE_HEIGHT; yn <= pArea->bottom / CHUNK_TILE_HEIGHT; yn++)
	for(xn = pArea->left / CHUNK_TILE_WIDTH; xn <= pArea->right / CHUNK_TILE_HEIGHT; xn++)
	{
		pChunk = GetChunk(xn, yn);
		if(pChunk)
		{
			pOb = pChunk->GetObjects();
			while(pOb)
			{
				if(pOb->GetObjectType() == OBJECT_BARRIER)
				{
					((CaveWall *)pOb)->Flatten();
				}
				pOb = pOb->GetNext();
			}
		}
	}
	




}

void Area::PaintWalls(RECT *pArea, ZSTexture *pTexture)
{

	if(!pTexture)
	{
		Describe("No texture found to paint walls with");
		return;
	}

	int xn;
	int yn;
	int ChunkWidth;
	ChunkWidth = this->GetWidth() / CHUNK_TILE_WIDTH;
	int ChunkHeight;
	ChunkHeight = this->GetHeight() / CHUNK_TILE_HEIGHT;

	Chunk *pChunk;
	Object *pOb;
	
	for(xn = 0; xn < ChunkWidth; xn++)
	for(yn = 0; yn < ChunkHeight; yn++)
	{
		pChunk = GetChunk(xn, yn);
		if(!pChunk)
		{
			LoadChunk(xn,yn);
			pChunk = GetChunk(xn, yn);
		}
		pOb = pChunk->GetObjects();
		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_BARRIER)
			{
				pOb->SetTexture(pTexture);
			}
			pOb = pOb->GetNext();
		}
	}

}

void Area::CleanDynamic()
{
	int MyID;
	MyID = PreludeWorld->GetAreaNum(this);

	
	
	Object *pOb;
	int xn, yn;

	for(yn = 0; yn < UpdateHeight; yn++)
	for(xn = 0; xn < UpdateWidth; xn++)
	{
		pOb = this->GetUpdateSegment(xn,yn);
		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_CREATURE)
			{
				if(((Creature *)pOb)->GetAreaIn() != MyID)
				{
					this->RemoveFromUpdate(pOb,xn,yn);
					DEBUG_INFO("creature found in wrong area\n");
					DEBUG_INFO(((Creature *)pOb)->GetData(INDEX_NAME).String);
					DEBUG_INFO("\n");
					Describe("Creature found in wrong Area");
					Describe(this->Header.Name);
					Describe(((Creature *)pOb)->GetData(INDEX_NAME).String);
				}
			}
			pOb = pOb->GetNextUpdate();
		}
	}
}

void Area::DungeonBlock()
{
	int xn;
	int yn;
	for(xn = 0; xn < this->ChunkWidth; xn++)
	for(yn = 0; yn < this->ChunkHeight; yn++)
	{
		Chunk *pChunk;
		LoadChunk(xn,yn);
		pChunk = GetChunk(xn,yn);
		pChunk->DungeonBlock();
	}
}

void Area::RemoveChunk(int x, int y)
{
	Chunk *pChunk;
	pChunk = GetChunk(x, y);
	
	if(pChunk)
	{
		delete pChunk;
		BigMap[x + (y * ChunkWidth)] = NULL;
	}
	else
	{
		DEBUG_INFO("Attempted to remove chunk not in memory\n");
	}
}




//end: Debug ***********************************************************

Area *Valley;
