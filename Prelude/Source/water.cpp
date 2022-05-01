//static drawlist
#include "water.h"
#include "zsutilities.h"

//cycle time of water in ms
#define WATER_FRAME_RATE 100

static unsigned short WaterDrawList[3 * 4] = { 0,1,2, //1
															  1,4,2,
															  4,3,2,
															  3,0,2 };
														

static float WaterNormals[3*5] = {	0.0f,0.0f,1.0f,
												0.0f,0.0f,1.0f,
												0.0f,0.0f,1.0f,
												0.0f,0.0f,1.0f,
												0.0f,0.0f,1.0f };

ZSTexture *Water::WaterTexture[9] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
D3DMATERIAL7 Water::matWater;

void Water::Expand(float fNorth, float fSouth, float fEast, float fWest)
{
	UV[0] = Verts[0] += fWest;
	UV[1] = Verts[1] += fNorth;

	UV[2] = Verts[3] += fEast;
	UV[3] = Verts[4] += fNorth;

	UV[6] = Verts[9] += fWest;
	UV[7] = Verts[10] += fSouth;

	UV[8] = Verts[12] += fEast;
	UV[9] = Verts[13] += fSouth;

	float xcenter;
	xcenter = (Verts[0] + Verts[9]) / 2.0f;
	float ycenter;
	ycenter = (Verts[1] + Verts[10]) / 2.0f;

	UV[4] = Verts[6] = xcenter;
	UV[5] = Verts[7] = ycenter;

	Verts[8] = (Verts[2] + Verts[5] + Verts[11] + Verts[14]) / 4.0f;

	for(int n = 0; n < 10; n++)
	{
		UV[n] /= 2.0f;
	}

}

void Water::RaiseEdge(float fN, float fS, float fE, float fW)
{
	Verts[2] += fN;
	Verts[2] += fW;

	Verts[5] += fN;
	Verts[5] += fE;

	Verts[11] += fW;
	Verts[11] += fS;

	Verts[14] += fE;
	Verts[14] += fS;

	Verts[8] = (Verts[2] + Verts[5] + Verts[11] + Verts[14]) / 4.0f;
}


void Water::Draw()
{
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
//	Engine->Graphics()->GetD3D()->SetMaterial(&matWater);
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
//	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	HRESULT hr;
	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
										D3DFVF_VERTEX,
										&StridedInfo,
										5,
										WaterDrawList,
										12,
										0);	
	if(FAILED(hr))
	{
		Engine->ReportError(hr);
		SafeExit("Couldn't draw Water\n");
	}
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
//	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	if(timeGetTime() > DrawTime)
	{
		Frame++;
		if(Frame >= 9)
		{
			Frame = 0;
		}
		pTexture = WaterTexture[Frame];
		DrawTime = timeGetTime() + WATER_FRAME_RATE;
	}
	return;
}



BOOL Water::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	// get the bounds
	if(vRayEnd->x > Verts[0] &&
		vRayEnd->x < Verts[3] &&
		vRayEnd->y > Verts[1] &&
		vRayEnd->y < Verts[10])
	{
		return TRUE;
	}
	return FALSE;
}

BOOL Water::RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	return RayIntersect(vRayStart, vRayEnd);

}

void Water::MakeVerts(int ChunkX, int ChunkY, float zNW, float zNE, float zSW, float zSE)
{
	UV[0] = Verts[0] = (float)(ChunkX * CHUNK_TILE_WIDTH);
	UV[1] = Verts[1] = (float)(ChunkY * CHUNK_TILE_HEIGHT);
	Verts[2] = zNW;

	UV[2] = Verts[3] = (float)((ChunkX+1) * CHUNK_TILE_WIDTH);
	UV[3] = Verts[4] = (float)(ChunkY * CHUNK_TILE_HEIGHT);
	Verts[5] = zNE;

	UV[4] = Verts[6] = (float)(ChunkX * CHUNK_TILE_WIDTH + CHUNK_TILE_WIDTH/2);
	UV[5] = Verts[7] = (float)(ChunkY * CHUNK_TILE_WIDTH + CHUNK_TILE_HEIGHT/2);
	Verts[8] = (zNW + zNE + zSW + zSE) /4.0f;

	UV[6] = Verts[9] = (float)(ChunkX * CHUNK_TILE_WIDTH);
	UV[7] = Verts[10] = (float)((ChunkY+1) * CHUNK_TILE_HEIGHT);
	Verts[11] = zSW;

	UV[8] = Verts[12] = (float)((ChunkX+1) * CHUNK_TILE_WIDTH);
	UV[9] = Verts[13] = (float)((ChunkY+1) * CHUNK_TILE_HEIGHT);
	Verts[14] = zSE;
}


void Water::MakeVerts(float Left, float Right, float Top, float Bottom, float Height)
{
	UV[0] = Verts[0] = (Left);
	UV[1] = Verts[1] = (Top);
	Verts[2] = Height;

	UV[2] = Verts[3] = (Right);
	UV[3] = Verts[4] = (Top);
	Verts[5] = Height;

	UV[4] = Verts[6] = ((Right + Left)/2.0f);
	UV[5] = Verts[7] = ((Top + Bottom)/2.0f);
	Verts[8] = Height;

	UV[6] = Verts[9] = (Left);
	UV[7] = Verts[10] = (Bottom);
	Verts[11] = Height;

	UV[8] = Verts[12] = (Right);
	UV[9] = Verts[13] = (Bottom);
	Verts[14] = Height;
	
	for(int n = 0; n < 10; n++)
	{ 
		UV[n] /= 2.0f;
	}
	Position.x = Left;
	Position.y = Top;
	Position.z = Height;

}

int Water::HighLight(COLOR_T Color)
{

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	HRESULT hr;
	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
										D3DFVF_VERTEX,
										&StridedInfo,
										5,
										WaterDrawList,
										12,
										0);	
	if(FAILED(hr))
	{
		SafeExit("Couldn't draw Water\n");
		Engine->ReportError(hr);
	}

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

	return TRUE;
}

BOOL Water::AdvanceFrame()
{
	Frame++;
	if(Frame >= 9)
	{
		Frame = 0;
	}
	pTexture = WaterTexture[Frame];
	return TRUE;
}

void Water::Save(FILE *fp)
{
	OBJECT_T Type;
	Type = GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);
	fwrite(Verts,sizeof(float),3*5,fp);
	fwrite(UV,sizeof(float),2*5,fp);
}


void Water::Load(FILE *fp)
{
	fread(Verts,sizeof(float),3*5,fp);
	fread(UV,sizeof(float),2*5,fp);
	Frame = rand() % 9;
	pTexture = WaterTexture[Frame];

	Position.x = Verts[6];
	Position.y = Verts[7];
	Position.z = Verts[8];
}

Water::Water()
{
	pNext = NULL;
	Angle = 0.0f;
	TextureNum = 0;
	pTexture = NULL;
	Frame = 0;
	Position = D3DVECTOR(0.0f,0.0f,0.0f);
	Scale = 1.0f;
	MeshNum = 0;
	pMesh = NULL;
	pNextUpdate = NULL;
	pPrevUpdate = NULL;

	memset(Verts,0, sizeof(float)*3*5);
	memset(UV,0, sizeof(float)*2*5);
	StridedInfo.position.dwStride = sizeof(float)*3;
	StridedInfo.normal.dwStride = sizeof(float)*3;
	StridedInfo.textureCoords[0].dwStride = sizeof(float)*2;

	StridedInfo.position.lpvData = Verts;
	StridedInfo.normal.lpvData = WaterNormals;
	StridedInfo.textureCoords[0].lpvData = UV;

	if(!WaterTexture[0])
	{
		WaterTexture[0] = Engine->GetTexture("water1");
		WaterTexture[1] = Engine->GetTexture("water2");
		WaterTexture[2] = Engine->GetTexture("water3");
		WaterTexture[3] = Engine->GetTexture("water4");
		WaterTexture[4] = Engine->GetTexture("water5");
		WaterTexture[5] = Engine->GetTexture("water6");
		WaterTexture[6] = Engine->GetTexture("water7");
		WaterTexture[7] = Engine->GetTexture("water8");
		WaterTexture[8] = Engine->GetTexture("water9");

		matWater.specular.r = 1.0f;
		matWater.specular.g = 1.0f;
		matWater.specular.b = 1.0f;
		matWater.specular.a = 1.0f;

		matWater.emissive.r = 0.0f;
		matWater.emissive.g = 0.0f;
		matWater.emissive.b = 0.0f;
		matWater.emissive.a = 0.0f;
		
		matWater.power = 1.0f;
		matWater.ambient.r = matWater.diffuse.r = 1.0f;
		matWater.ambient.g = matWater.diffuse.g = 1.0f;
		matWater.ambient.b = matWater.diffuse.b = 1.0f;
		matWater.ambient.a = matWater.diffuse.a = 0.5f;
	}
	Frame = rand() % 9;
	pTexture = WaterTexture[Frame];
	DrawTime = 0;
}

void Water::MoveVerts(D3DVECTOR *vNW, D3DVECTOR *vNE, D3DVECTOR *vSW, D3DVECTOR *vSE)
{
	if(vNW)
	{
		Verts[0] += vNW->x;
		Verts[1] += vNW->y;
		Verts[2] += vNW->z;
		UV[0] = Verts[0];
		UV[1] = Verts[1];
	}

	if(vNE)
	{
		Verts[3] += vNE->x;
		Verts[4] += vNE->y;
		Verts[5] += vNE->z;
		UV[2] = Verts[3];
		UV[3] = Verts[4];
	}

	if(vSW)
	{
		Verts[9] += vSW->x;
		Verts[10] += vSW->y;
		Verts[11] += vSW->z;
		UV[6] = Verts[9];
		UV[7] = Verts[10];
	}
	if(vSE)
	{
		Verts[12] += vSE->x;
		Verts[13] += vSE->y;
		Verts[14] += vSE->z;
		UV[8] = Verts[12];
		UV[9] = Verts[13];
	}

	Verts[6] = (Verts[0] + Verts[3] + Verts[9] + Verts[12]) / 4.0f;
	Verts[7] = (Verts[1] + Verts[4] + Verts[10] + Verts[13]) / 4.0f;
	Verts[8] = (Verts[2] + Verts[5] + Verts[11] + Verts[14]) / 4.0f;
	UV[4] = Verts[6];
	UV[5] = Verts[7];
}

#ifndef NDEBUG
	extern BYTE Watered[200][200];
#else
	extern BYTE Watered[1][1];
#endif



void Water::AdjustWater(Chunk *pChunk)
{
	int xn, yn;
	xn = pChunk->GetX();
	yn = pChunk->GetY();

	int North, South, East, West;

	North = South = East = West = FALSE;

	
	if(yn && !Watered[xn][yn-1]) North = TRUE;
	if(!Watered[xn][yn+1]) South = TRUE;
	if(!Watered[xn+1][yn]) East = TRUE;
	if(xn && !Watered[xn-1][yn]) West = TRUE;

	float zn;
	if(North || West)
	{
		zn = pChunk->GetHeight(0,0);
		
		if(Verts[2] > (zn - 3.0f) || Verts[2] < (zn - 4.0))
		{
			Verts[2] = zn - 3.0f;
		}
	
	}
	if(North || East)
	{
		zn = pChunk->GetHeight(15,0);
		
		if(Verts[5] > (zn - 3.0f) || Verts[5] < (zn - 4.0))
		{
			Verts[5] = zn - 3.0f;
		}
	
	}

	if(South || West)
	{
		zn = pChunk->GetHeight(0,15);
		
		if(Verts[11] > (zn - 3.0f) || Verts[11] < (zn - 4.0))
		{
			Verts[11] = zn - 3.0f;
		}
	
	}
	if(South || East)
	{
		zn = pChunk->GetHeight(15,15);
		
		if(Verts[14] > (zn - 3.0f) || Verts[14] < (zn - 4.0))
		{
			Verts[14] = zn - 3.0f;
		}
	
	}

	Verts[8] = (Verts[2] + Verts[5] + Verts[11] + Verts[14]) / 4.0f;
}

void Water::Reset()
{
	UV[0] = Verts[0];
	UV[1] = Verts[1];

	UV[2] = Verts[3];
	UV[3] = Verts[4];

	UV[6] = Verts[9];
	UV[7] = Verts[10];

	UV[8] = Verts[12];
	UV[9] = Verts[13];

	Verts[6] = (Verts[0] + Verts[3] + Verts[9] + Verts[12]) / 4.0f;
	Verts[7] = (Verts[1] + Verts[4] + Verts[10] + Verts[13]) / 4.0f;
	Verts[8] = (Verts[2] + Verts[5] + Verts[11] + Verts[14]) / 4.0f;
	UV[4] = Verts[6];
	UV[5] = Verts[7];

	for(int n = 0; n < 10; n++)
	{ 
		UV[n] /= 2.0f;
	}
	Position.x = Verts[6];
	Position.y = Verts[7];
	Position.z = Verts[8];

}


