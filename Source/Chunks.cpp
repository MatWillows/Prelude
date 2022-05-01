//********************************************************************* 
//********************************************************************* 
//**************               Chunks.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:                                                                 
//*Revisor:                                               
//*Purpose:                        
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*        			               
//*			
//********************************************************************* 
//*********************************************************************
#include "chunks.h"
#include "world.h"
#include "zsengine.h"
#include "zsutilities.h"
#include <assert.h>
#include "regions.h"
#include "area.h"

float TempBuf[9 * 9];

#define TDIR_N	44
#define TDIR_NE 48
#define TDIR_E	32
#define TDIR_SE 52
#define TDIR_S	36
#define TDIR_SW 56
#define TDIR_W	40
#define TDIR_NW 60

//************** static Members *********************************
D3DTLVERTEX Chunk::vTerrain[16*4*4*4];
float Chunk::TextureCoordinates[2 * NUM_CHUNK_VERTS];
TextureGroup Chunk::ChunkTextures(CHUNK_TEXTURE_WIDTH, CHUNK_TEXTURE_HEIGHT);
//************** Constructors  ****************************************

//simple constructor
Chunk::Chunk()
{
	pTerrainTexture = NULL;

	//create the texture for the chunk
	pObjectList = NULL;
	pObjectList2 = NULL;
	pObjectListEnd = NULL;
	int xn, yn, on;

	for(yn = 0; yn < CHUNK_WIDTH; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		Terrain[xn][yn] = 0;
	}

	for(yn = 0; yn < CHUNK_WIDTH; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	for(on = 0; on < NUM_OVERLAYS; on++)
	{
		Overlays[xn][yn][on] = 0;
	}

	//set the strides.  (!!!!THis can be moved to the constructor to save time).	
	stridedDataInfo.position.dwStride = sizeof(float)*6;
	stridedDataInfo.normal.dwStride = sizeof(float)*6;
	stridedDataInfo.textureCoords[0].dwStride = sizeof(float)*2;

	//set the data
	stridedDataInfo.position.lpvData = &Verts[0];
	stridedDataInfo.normal.lpvData = &Verts[3];
	stridedDataInfo.textureCoords[0].lpvData = TextureCoordinates;

	memset(Blocking,0, CHUNK_WIDTH * CHUNK_HEIGHT);
//	ZeroMemory(Regions,sizeof(unsigned short) * NUM_CHUNK_REGIONS);

}

//copy constructor


//end:  Constructors ***************************************************



//*************** Destructor *******************************************
Chunk::~Chunk()
{
	//destroy the texture
	if(pTerrainTexture)
	{
		ChunkTextures.FreeTexture(pTerrainTexture);
	}

	pTerrainTexture = NULL;

	if(pObjectListEnd)
	pObjectListEnd->SetNext(NULL);
	
	//destroy all the objects on the object list
	Object *pObject, *pLastObject;
	pLastObject = pObject = pObjectList;
	while(pObject)
	{
		pLastObject = pObject;
		pObject = pObject->GetNext();
		delete pLastObject;
	}
	pObjectList = NULL;

}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************
float *Chunk::GetTile(int x, int y)
{
	return &Verts[(x + y * CHUNK_WIDTH) * 24]; 
}

unsigned short *Chunk::GetTileDraw(int x, int y)
{
	return &DrawList[(x + y * CHUNK_WIDTH) * 6];
}



//end: Accessors *******************************************************



//************  Display Functions  *************************************
void Chunk::Draw()
{
	HRESULT hr;
		
	Engine->Graphics()->SetTexture(pTerrainTexture);
	
	//hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, Verts, (CHUNK_WIDTH * CHUNK_HEIGHT * 4), DrawList, (CHUNK_WIDTH*CHUNK_HEIGHT * 6), D3DDP_WAIT);

	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
												D3DFVF_VERTEX,
													&this->stridedDataInfo,
													NUM_CHUNK_VERTS,
													this->DrawList,
													CHUNK_DRAW_LENGTH,
													0);
  
	if(FAILED(hr))
	{
		SafeExit("Failed To Draw Chunk");
	}

}

void Chunk::DrawBacksides()
{
	HRESULT hr;

	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
													D3DFVF_VERTEX,
													&this->stridedDataInfo,
													NUM_CHUNK_VERTS,
													this->DrawList,
													CHUNK_DRAW_LENGTH,
													0);
  
	if(FAILED(hr))
	{
		SafeExit("failed to draw chunk backsides");
	}

}

void Chunk::DrawTiles()
{
	HRESULT hr;

//	hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINELIST, D3DFVF_VERTEX, Verts, (CHUNK_WIDTH * CHUNK_HEIGHT * 4), NULL);

	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
													D3DFVF_VERTEX,
													&this->stridedDataInfo,
													NUM_CHUNK_VERTS,
													this->DrawList,
													CHUNK_DRAW_LENGTH,
													0);
  
	if(FAILED(hr))
	{
		SafeExit("failed to draw chunk tiles");
	}


}


void Chunk::DrawObjects()
{
	
	Object *pObject;
	pObject = pObjectList;

	while(pObject)
	{
		Engine->Graphics()->SetTexture(pObject->GetTexture());
		pObject->Draw();
		pObject = pObject->GetNext();
	}
}

//end: Display functions ***********************************************



//************ Mutators ************************************************
int Chunk::AddObject(Object *pAddObject)
{
	if(!pObjectList)
	{
		pObjectListEnd = pAddObject;
	}
	pAddObject->SetNext(pObjectList);
	pObjectList = pAddObject;
	D3DVECTOR *Position;

	Position = pAddObject->GetPosition();

//	if(pAddObject->GetMesh())
//	{
//		AddBlocking(((int)Position->x) % CHUNK_WIDTH, ((int)Position->y) % CHUNK_HEIGHT,pAddObject->GetMesh()->GetBlocking());
//	}
	return TRUE;
}

void Chunk::ReleaseTexture()
{
	if(pTerrainTexture)
	{
		ChunkTextures.FreeTexture(pTerrainTexture);
		pTerrainTexture = NULL;
	}
}

int Chunk::CreateTexture(ZSTexture *pBaseTexture)
{
	HRESULT hr;
	//create the terrain bitmap for a given area
	if(!pTerrainTexture)
	{
		pTerrainTexture = ChunkTextures.GetTexture();
	}
	
	if(!pTerrainTexture)
	{
		SafeExit("Failed to create Texture for Chunk Terrain");
	}

	int x;
	int y;
	int n;
	x = X * CHUNK_WIDTH;
	y = Y * CHUNK_HEIGHT;
	int DrawY = 0;
	int DrawX = 0;

	RECT rFrom;
	rFrom.top = 0;
	rFrom.bottom = CHUNK_TEXTURE_HEIGHT;
	rFrom.left = 0;
	rFrom.right = CHUNK_TEXTURE_WIDTH;
	
	D3DTLVERTEX *vp;

	//clear the back buffer and the zbuffer
	Engine->Graphics()->GetD3D()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		               0x00000000, 1.0f, 0L );

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);          
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_POINT);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_POINT); 
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
	Engine->Graphics()->GetD3D()->SetRenderState(D3DRENDERSTATE_ALPHAREF, 0x00000000);
	Engine->Graphics()->GetD3D()->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATER);

	Engine->Graphics()->SetTexture(pBaseTexture);
	
	float left, right, top, bottom;
	top = -0.5f;
	bottom = TILE_HEIGHT + 0.5f;
	for(y = 0; y < CHUNK_HEIGHT; y++)
	{
		left = -0.5f;
		right = TILE_WIDTH + 0.5f;
		for(x = 0; x < CHUNK_WIDTH; x++)
		{
			vp = &vTerrain[Terrain[x][y]];
			//vp = &vTerrain[Valley->GetTerrain(x+(X*CHUNK_WIDTH),y+(Y*CHUNK_HEIGHT))];
			vp[0].dvSX = (float)left;
			vp[0].dvSY = (float)top;
			vp[0].dvSZ = 0;
			vp[1].dvSX = (float)right;
			vp[1].dvSY = (float)top;
			vp[1].dvSZ = 0;
			vp[2].dvSX = (float)left;
			vp[2].dvSY = (float)bottom;
			vp[2].dvSZ = 0;
			vp[3].dvSX = (float)right;
			vp[3].dvSY = (float)bottom;
			vp[3].dvSZ = 0;
		
			hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0);

			if(FAILED(hr))
			{
				SafeExit("Problem in create chunk texture");
			}

			left += (float)TILE_WIDTH;
			right += (float)TILE_WIDTH;
		}
		top += (float)TILE_HEIGHT;
		bottom += (float)TILE_HEIGHT;
		
	}

	top = 0;
	bottom = TILE_HEIGHT;

	unsigned short *pOverlays;
	
	for(y = 0; y < CHUNK_HEIGHT; y++)
	{
		left = 0;
		right = TILE_WIDTH;
		for(x = 0; x < CHUNK_WIDTH; x++)
		{
			n = 0;

			pOverlays = &Overlays[x][y][0];
			//Overlays = Valley->GetOverlays(x+(X*CHUNK_WIDTH),y+(Y*CHUNK_HEIGHT));
			while(pOverlays[n] && n < NUM_OVERLAYS)
			{
				vp = &vTerrain[pOverlays[n]];
				vp[0].dvSX = (float)left;
				vp[0].dvSY = (float)top;
				vp[0].dvSZ = 0;
				vp[1].dvSX = (float)right;
				vp[1].dvSY = (float)top;
				vp[1].dvSZ = 0;
				vp[2].dvSX = (float)left;
				vp[2].dvSY = (float)bottom;
				vp[2].dvSZ = 0;
				vp[3].dvSX = (float)right;
				vp[3].dvSY = (float)bottom;
				vp[3].dvSZ = 0;
				
				if(FAILED(Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, 0)))
				{
					SafeExit("problem in create chunk texture");
				}
				n++;
			}
			left += TILE_WIDTH;
			right += TILE_WIDTH;
		}
		top += TILE_HEIGHT;
		bottom += TILE_HEIGHT;
		
	}

	hr = D3DXLoadTextureFromSurface(Engine->Graphics()->GetD3D(), pTerrainTexture->GetSurface(), NULL, Engine->Graphics()->GetBBuffer(),&rFrom,NULL,D3DX_FT_POINT);
	
	if(hr != DD_OK)
	{
		SafeExit("problem in create chunk texture: load texture from surface");
	}
	
	//clear the back buffer and the zbuffer
	Engine->Graphics()->GetD3D()->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
		               0x00000000, 1.0f, 0L );
	
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTFG_LINEAR);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTFG_LINEAR);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE); 
//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	
	return TRUE;

}

void Chunk::ResetTile(int x, int y)
{
	float *pVerts;

	pVerts = GetTile(x, y);

	float LengthAB;
	float LengthCD;

	D3DVECTOR vNorm, vNormA, vNormB, vNormC, vNormD, vA, vB, vC, vD;

	int DrawIndex;
	int VertexIndex;

	DrawIndex = (x + y*CHUNK_WIDTH) * 6;
	VertexIndex = (x + y * CHUNK_WIDTH) * 4;

	if(!DrawList[DrawIndex] && !DrawList[DrawIndex + 1])
		return;

	vA.x = pVerts[tx1] - pVerts[tx2];
	vC.x = pVerts[tx3] - pVerts[tx2];
	vD.x = pVerts[tx4] - pVerts[tx2];

	vA.y = pVerts[ty1] - pVerts[ty2];
	vC.y = pVerts[ty3] - pVerts[ty2];
	vD.y = pVerts[ty4] - pVerts[ty2];

	vA.z = pVerts[tz1] - pVerts[tz2];
	vC.z = pVerts[tz3] - pVerts[tz2];
	vD.z = pVerts[tz4] - pVerts[tz2];

	vNormA = Normalize(CrossProduct(vC,vA));
	vNormB = Normalize(CrossProduct(vD,vC));

	vA = vA + vNormA;
	vD = vD + vNormB;

	LengthAB = GetDistance(&vA,&vD);

	vB.x = pVerts[tx2] - pVerts[tx1];
	vC.x = pVerts[tx3] - pVerts[tx1];
	vD.x = pVerts[tx4] - pVerts[tx1];

	vB.y = pVerts[ty2] - pVerts[ty1];
	vC.y = pVerts[ty3] - pVerts[ty1];
	vD.y = pVerts[ty4] - pVerts[ty1];

	vB.z = pVerts[tz2] - pVerts[tz1];
	vC.z = pVerts[tz3] - pVerts[tz1];
	vD.z = pVerts[tz4] - pVerts[tz1];

	vNormC = Normalize(CrossProduct(vB,vD));
	vNormD = Normalize(CrossProduct(vD,vC));

	vB = vB + vNormC;
	vC = vC + vNormD;

	LengthCD = GetDistance(&vB,&vC);

	if(LengthAB > LengthCD)
	{
		vNorm = Normalize(vNormA + vNormB);

		pVerts[ntx1] = vNormA.x;
		pVerts[nty1] = vNormA.y;
		pVerts[ntz1] = vNormA.z;
		
		pVerts[ntx2] = vNorm.x;
		pVerts[nty2] = vNorm.y;
		pVerts[ntz2] = vNorm.z;
		
		pVerts[ntx3] = vNorm.x;
		pVerts[nty3] = vNorm.y;
		pVerts[ntz3] = vNorm.z;
		
		pVerts[ntx4] = vNormB.x;
		pVerts[nty4] = vNormB.y;
		pVerts[ntz4] = vNormB.z;

		DrawList[DrawIndex] = VertexIndex;
		DrawList[DrawIndex+1] = VertexIndex + 1;
		DrawList[DrawIndex+2] = VertexIndex + 2;
		DrawList[DrawIndex+3] = VertexIndex + 1;
		DrawList[DrawIndex+4] = VertexIndex + 3;
		DrawList[DrawIndex+5] = VertexIndex + 2;
//		Blocking[x + y * CHUNK_WIDTH] = Blocking[x + y * CHUNK_WIDTH] & ~DRAW_FLIPPED;
	}
	else
	{
		vNorm = Normalize(vNormC + vNormD);

		pVerts[ntx1] = vNorm.x;
		pVerts[nty1] = vNorm.y;
		pVerts[ntz1] = vNorm.z;
		
		pVerts[ntx2] = vNormA.x;
		pVerts[nty2] = vNormA.y;
		pVerts[ntz2] = vNormA.z;
		
		pVerts[ntx3] = vNormB.x;
		pVerts[nty3] = vNormB.y;
		pVerts[ntz3] = vNormB.z;
		
		pVerts[ntx4] = vNorm.x;
		pVerts[nty4] = vNorm.y;
		pVerts[ntz4] = vNorm.z;

		DrawList[DrawIndex] = VertexIndex;
		DrawList[DrawIndex+1] = VertexIndex + 1;
		DrawList[DrawIndex+2] = VertexIndex + 3;
		DrawList[DrawIndex+3] = VertexIndex + 0;
		DrawList[DrawIndex+4] = VertexIndex + 3;
		DrawList[DrawIndex+5] = VertexIndex + 2;
//		Blocking[x + y * CHUNK_WIDTH] = Blocking[x + y * CHUNK_WIDTH] | DRAW_FLIPPED;
	}
	
}

void Chunk::InitTerrain()
{
	//set up all the various vertex arrays
	float uoff;
	float voff;

	uoff = voff = 32.0f / 256.0f;

	float left,right,top,bottom;
	//first set up the basic terrain's
	int xn, yn;

	for(yn = 0; yn < 16*64; yn++)
	{
		vTerrain[yn].color = D3DRGB(1.0f,1.0f,1.0f);
		vTerrain[yn].rhw = 1.0f;
		vTerrain[yn].specular = D3DRGB(0.5f,0.5f,0.5f);
	
	}
	for(yn = 0; yn < 16*64; yn += 4)
	{
		vTerrain[yn].tu = 0;
		vTerrain[yn].tv = 1.0f;
		vTerrain[yn+1].tu = 1.0f;
		vTerrain[yn+1].tv = 1.0f;
		vTerrain[yn+2].tu = 0;
		vTerrain[yn+2].tv = 0;
		vTerrain[yn+3].tu = 1.0f;
		vTerrain[yn+3].tv = 0;
	}
		
	for( yn = 0; yn < 16; yn++)
	{
		top = ((float)(yn%8)*voff) + 0.005f;
		bottom = top + voff - 0.010f;
		for(xn = 0; xn < 4; xn++)
		{
			left = (float)((yn/8)*4)*uoff + (float)xn*uoff + 0.005f;
			right = left + uoff-0.010f;
			//base
			vTerrain[yn*64+xn*16].tu = left;
			vTerrain[yn*64+xn*16].tv = top;
			vTerrain[yn*64+xn*16+1].tu = right;
			vTerrain[yn*64+xn*16+1].tv = top;
			vTerrain[yn*64+xn*16+2].tu = left;
			vTerrain[yn*64+xn*16+2].tv = bottom;
			vTerrain[yn*64+xn*16+3].tu = right;
			vTerrain[yn*64+xn*16+3].tv = bottom;
			//rotate90
			vTerrain[yn*64+xn*16+4].tu = left;
			vTerrain[yn*64+xn*16+4].tv = bottom;
			vTerrain[yn*64+xn*16+5].tu = left;
			vTerrain[yn*64+xn*16+5].tv = top;
			vTerrain[yn*64+xn*16+6].tu = right;
			vTerrain[yn*64+xn*16+6].tv = bottom;
			vTerrain[yn*64+xn*16+7].tu = right;
			vTerrain[yn*64+xn*16+7].tv = top;
			//rotate180
			vTerrain[yn*64+xn*16+8].tu = right;
			vTerrain[yn*64+xn*16+8].tv = bottom;
			vTerrain[yn*64+xn*16+9].tu = left;
			vTerrain[yn*64+xn*16+9].tv = bottom;
			vTerrain[yn*64+xn*16+10].tu = right;
			vTerrain[yn*64+xn*16+10].tv = top;
			vTerrain[yn*64+xn*16+11].tu = left;
			vTerrain[yn*64+xn*16+11].tv = top;
			//rotate270
			vTerrain[yn*64+xn*16+12].tu = right;
			vTerrain[yn*64+xn*16+12].tv = top;
			vTerrain[yn*64+xn*16+13].tu = right;
			vTerrain[yn*64+xn*16+13].tv = bottom;
			vTerrain[yn*64+xn*16+14].tu = left;
			vTerrain[yn*64+xn*16+14].tv = top;
			vTerrain[yn*64+xn*16+15].tu = left;
			vTerrain[yn*64+xn*16+15].tv = bottom;
		}
	}
	//done

	//fill the static terrain UV list
	for(yn = 0; yn < CHUNK_HEIGHT; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		int VertOffset = (yn * CHUNK_WIDTH + xn) * 8;
		TextureCoordinates[VertOffset] = (float)xn * (1.0 / (float)CHUNK_WIDTH);
		TextureCoordinates[VertOffset+1] = (float)yn * (1.0 / (float)CHUNK_HEIGHT);

		TextureCoordinates[VertOffset+2] = (float)(xn+1) * (1.0 / (float)CHUNK_WIDTH);
		TextureCoordinates[VertOffset+3] = (float)yn * (1.0 / (float)CHUNK_HEIGHT);

		TextureCoordinates[VertOffset+4] = (float)xn * (1.0 / (float)CHUNK_WIDTH);
		TextureCoordinates[VertOffset+5] = (float)(yn+1) * (1.0 / (float)CHUNK_HEIGHT);

		TextureCoordinates[VertOffset+6] = (float)(xn+1) * (1.0 / (float)CHUNK_WIDTH);
		TextureCoordinates[VertOffset+7] = (float)(yn+1) * (1.0 / (float)CHUNK_HEIGHT);
	}
}


//end: Mutators ********************************************************



//************ Outputs *************************************************

//end: Outputs ********************************************************



//************ Debug ***************************************************

//end: Debug ***********************************************************

void Chunk::Load(FILE *fp)
{
	fread(&X,sizeof(X),1,fp);
	fread(&Y,sizeof(Y),1,fp);
//	fread(Regions,sizeof(unsigned short) * NUM_CHUNK_REGIONS,1,fp);
	
	int xn,yn;
	int VertOffset = 0;
	int XOffset;
	int YOffset;
	XOffset = X * CHUNK_WIDTH * 2;
	YOffset = Y * CHUNK_HEIGHT * 2;

	int n, overlayn;
	for(n = 0; n < CHUNK_WIDTH; n++)
	{
		fread(&Terrain[n][0],sizeof(int),CHUNK_HEIGHT,fp);
		for(overlayn = 0; overlayn < CHUNK_HEIGHT; overlayn++)
		{
			fread(&Overlays[n][overlayn][0],sizeof(int),NUM_OVERLAYS,fp);
		}
	}

	//blocking
	fread(Blocking,sizeof(unsigned short), CHUNK_TILE_HEIGHT,fp);

	fread(TileHeights, sizeof(float),CHUNK_TILE_WIDTH * CHUNK_TILE_HEIGHT,fp);
	
	fread(TempBuf,sizeof(float),9*9,fp);

	for(yn = 0; yn < CHUNK_TILE_HEIGHT; yn += 2)
	for(xn = 0; xn < CHUNK_TILE_WIDTH; xn += 2)
	{
		Verts[VertOffset + tx1] = (float)xn + XOffset;
		Verts[VertOffset + ty1] = (float)yn + YOffset;
		Verts[VertOffset + tz1] = TempBuf[(xn/2) + (yn/2) * 9];
		
		Verts[VertOffset + tx2] = (float)xn + XOffset + 2;
		Verts[VertOffset + ty2] = (float)yn + YOffset;
		Verts[VertOffset + tz2] = TempBuf[(xn/2) + 1 + (yn/2) * 9];

		Verts[VertOffset + tx3] = (float)xn + XOffset;
		Verts[VertOffset + ty3] = (float)yn + YOffset + 2;
		Verts[VertOffset + tz3] = TempBuf[(xn/2) + ((yn/2)+1) * 9];

		Verts[VertOffset + tx4] = (float)xn + XOffset + 2;
		Verts[VertOffset + ty4] = (float)yn + YOffset + 2;
		Verts[VertOffset + tz4] = TempBuf[(xn/2) + 1 + ((yn/2)+1) * 9];

		VertOffset += 24;
	}

	fread(DrawList,sizeof(unsigned short), CHUNK_DRAW_LENGTH, fp);

	fread(&NumObjects,sizeof(int),1,fp);

	assert(NumObjects < 8*8*4);

	Object *pOb;

	//note this inverts the order from the save order
	//should not matter.
	for(n = 0; n < NumObjects; n++)
	{
	//	pOb = new Object;
	//	pOb->Load(fp);

		pOb = LoadObject(fp);
		AddObject(pOb);
	}

	for(yn = 0; yn < CHUNK_HEIGHT; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		ResetTile(xn,yn);
	}
}

void Chunk::SaveBrief(FILE *fp)
{
	fwrite(&X,sizeof(X),1,fp);
	fwrite(&Y,sizeof(Y),1,fp);
	
//	fwrite(Regions,sizeof(unsigned short) * NUM_CHUNK_REGIONS,1,fp);
	
	int n, overlayn;
	for(n = 0; n < CHUNK_WIDTH; n++)
	{
		fwrite(&Terrain[n][0],sizeof(int),CHUNK_HEIGHT,fp);
		for(overlayn = 0; overlayn < CHUNK_HEIGHT; overlayn++)
		{
			fwrite(&Overlays[n][overlayn][0],sizeof(int),NUM_OVERLAYS,fp);
		}
	}

	//blocking
	fwrite(Blocking,sizeof(unsigned short), CHUNK_TILE_HEIGHT,fp);
	
	fwrite(TileHeights, sizeof(float),CHUNK_TILE_WIDTH * CHUNK_TILE_HEIGHT,fp);
}

void Chunk::Save(FILE *fp)
{
	//ensure that water is saved last for alpha blend purposes
	SortObjects();

	fwrite(&X,sizeof(X),1,fp);
	fwrite(&Y,sizeof(Y),1,fp);
	
//	fwrite(Regions,sizeof(unsigned short) * NUM_CHUNK_REGIONS,1,fp);
	
	int n, overlayn;
	for(n = 0; n < CHUNK_WIDTH; n++)
	{
		fwrite(&Terrain[n][0],sizeof(int),CHUNK_HEIGHT,fp);
		for(overlayn = 0; overlayn < CHUNK_HEIGHT; overlayn++)
		{
			fwrite(&Overlays[n][overlayn][0],sizeof(int),NUM_OVERLAYS,fp);
		}
	}

	//blocking
	fwrite(Blocking,sizeof(unsigned short), CHUNK_TILE_HEIGHT,fp);
	
	fwrite(TileHeights, sizeof(float),CHUNK_TILE_WIDTH * CHUNK_TILE_HEIGHT,fp);
	
	int VertOffset = 2;
	int EightStart;

	for(n = 0; n < 8; n++)
	{
		//row 1
		EightStart = VertOffset;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//1

		VertOffset += 24;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//2
		
		VertOffset += 24;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//3
		
		VertOffset += 24;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//4
		
		VertOffset += 24;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//5
		
		VertOffset += 24;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//6
		
		VertOffset += 24;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//7
		
		VertOffset += 24;
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//8
		VertOffset += 6;
		
		fwrite(&Verts[VertOffset],sizeof(float),1,fp);//9
		VertOffset += 18;
	}

	//row 9
	VertOffset = EightStart + 12;
	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 24;
	
	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 24;
	
	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 24;
	
	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 24;
	
	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 24;
	
	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 24;
	
	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 24;

	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	VertOffset += 6;

	fwrite(&Verts[VertOffset],sizeof(float),1,fp);
	
	fwrite(DrawList,sizeof(unsigned short),CHUNK_DRAW_LENGTH, fp);

	NumObjects = 0;

	Object *pOb;

	pOb = pObjectList;

	while(pOb)
	{
		NumObjects++;
		if(pOb == pObjectListEnd)
		{
			pOb = NULL;
		}
		else
		{
			pOb = pOb->GetNext();
		}
	}

	assert(NumObjects < 8*8*4);

	fwrite(&NumObjects,sizeof(int),1,fp);

	pOb = pObjectList;
	while(pOb)
	{
		pOb->Save(fp);
		if(pOb == pObjectListEnd)
		{
			pOb = NULL;
		}
		else
		{
			pOb = pOb->GetNext();
		}
	}



}

void Chunk::Build()
{
	int xn;
	int yn;

	for(yn = 0; yn < CHUNK_WIDTH; yn++)
	for(xn = 0; xn < CHUNK_HEIGHT; xn++)
	{
		ResetTile(xn,yn);
	}
}

int Chunk::GetSize()
{
	int Obsize = 0;
	Obsize += sizeof(D3DVECTOR);
	Obsize += sizeof(int);
	Obsize += sizeof(long);	//currently unused
	Obsize += sizeof(float);
	Obsize += sizeof(float);
	Obsize += sizeof(float);
	Obsize += sizeof(int);
	Obsize += sizeof(int);

	int MySize = 0;
	MySize = sizeof(Chunk);
	MySize -= sizeof(ZSTexture *);
	MySize -= sizeof(Object *);
	MySize -= sizeof(Object *);

	Object *pOb;
	pOb = pObjectList;
	while(pOb)
	{
		MySize += Obsize;
		pOb = pOb->GetNext();
	}
	return MySize;
}

void Chunk::OutPutDebugInfo(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename, "wt+");
	
	fprintf(fp,"x: %i  y: %i\n",X,Y);
	fprintf(fp,"NumOb: %i\n",NumObjects);
	for(int yn = 0; yn < CHUNK_TILE_HEIGHT; yn++)
	{
		for(int xn = 0; xn < CHUNK_TILE_WIDTH; xn++)
		{
//			fprintf(fp,"(%f,%f,%f) ",(double)Verts[(yn*CHUNK_WIDTH + xn)*4].x,(double)Verts[(yn*CHUNK_WIDTH + xn)*4].y,(double)Verts[(yn*CHUNK_WIDTH + xn)*4].z);
			if(GetBlocking(xn,yn))
			{
				fprintf(fp,"1");
			}
			else
			{
				fprintf(fp,"0");
			}
		}
		fprintf(fp,"\n");
	}

	fclose(fp);
}

void Chunk::Smooth()
{
	int x, y;
	int ter;

	for(y = 0; y < CHUNK_HEIGHT; y++)
	for(x = 0; x < CHUNK_WIDTH; x++)
	{
		ZeroMemory(&Overlays[x][y][0],sizeof(unsigned short)*NUM_OVERLAYS);
	}

	for(y = (Y * CHUNK_HEIGHT) - 1; y <= (Y * CHUNK_HEIGHT) + CHUNK_HEIGHT; y++) 
	{
	for(x = (X * CHUNK_WIDTH) - 1; x <= (X * CHUNK_WIDTH) + CHUNK_WIDTH; x++) 
	{
		//add the overlays
		ter = Valley->GetTerrain(x,y);
		ter = ter - (ter%64);
		if(ter <= TerrainIndex[TER_DIRT])
		{	
			
			if(Valley->GetTerrain(x,y-1) - (Valley->GetTerrain(x,y-1)%64) > ter)
			{
				Valley->AddOverlay(x,y-1, ter+44);
			}
			if(Valley->GetTerrain(x+1,y-1) - (Valley->GetTerrain(x+1,y-1)%64) > ter)
			{
				Valley->AddOverlay(x+1,y-1, ter+48);
			}
			if(Valley->GetTerrain(x+1,y) - (Valley->GetTerrain(x+1,y)%64) > ter)
			{
				Valley->AddOverlay(x+1, y, ter+32);
			}
			if(Valley->GetTerrain(x+1,y+1) - (Valley->GetTerrain(x+1,y+1)%64)> ter)
			{
				Valley->AddOverlay(x+1, y+1, ter+52);
			}
			if(Valley->GetTerrain(x,y+1) - (Valley->GetTerrain(x,y+1)%64)> ter)
			{
				Valley->AddOverlay(x, y+1, ter+36);
			}
			if(Valley->GetTerrain(x-1,y+1) - (Valley->GetTerrain(x-1,y+1)%64)> ter)
			{
				Valley->AddOverlay(x-1, y+1, ter+56);
			}
			if(Valley->GetTerrain(x-1,y) - (Valley->GetTerrain(x-1,y)%64)> ter)
			{
				Valley->AddOverlay(x-1, y, ter+40);
			}
			if(Valley->GetTerrain(x-1,y-1) - (Valley->GetTerrain(x+1,y-1)%64)> ter)
			{
				Valley->AddOverlay(x-1, y-1, ter+60);
			}
		}
	}
	}
	return;
}

void Chunk::AddOverlay(int x, int y, int num)
{
	if(x < 0 || y < 0 || x >= CHUNK_WIDTH || y >= CHUNK_HEIGHT) 
		return;
	int o;
	int n = 0;
	
	while(Overlays[x][y][n] && n < 8)
	{
		if(Overlays[x][y][n] == num)
			return;
		n++;
	}
	if(n >= 8) 
	{
		return;
	}
	
	Overlays[x][y][n] = num;
	
	//sort overlays[x][y]
	n = 0;
	while(Overlays[x][y][n] && n < 7)
	{
		if(Overlays[x][y][n] < Overlays[x][y][n+1])
		{
			o = Overlays[x][y][n];
			Overlays[x][y][n] = Overlays[x][y][n+1];
			Overlays[x][y][n+1] = o;
			n = 0;
		}
		else
		{
			n++;
		}

	}

	return;
}

void Chunk::ClearTile(int x, int y)
{
	Object *pOb;
	pOb = pObjectList;
	float Left;
	float Right;
	float Top;
	float Bottom;

	Left = (float)x - 0.05f;
	Right = (float)x + 1.05f;
	Top = (float)y - 0.05f;
	Bottom = (float)y + 1.05f;
	
	D3DVECTOR *vPos;

	while(pOb)
	{
		vPos = pOb->GetPosition();
		if(vPos->x > Left && vPos->x < Right && vPos->y > Top && vPos->y < Bottom)
		{
			RemoveObject(pOb);
			delete pOb;
			pOb = pObjectList;
		}
		else
		{
			pOb = pOb->GetNext();
		}
	}

}

int Chunk::RemoveObject(Object *pToRemove)
{
	Object *pOb, *pLOb;

	pOb = pObjectList;

	if(pObjectList == pToRemove)
	{
		pObjectList = pOb->GetNext();
		return TRUE;
	}

	while(pOb)
	{
		if(pOb == pToRemove)
		{
			pLOb->SetNext(pOb->GetNext());
			if(pOb == pObjectListEnd)
			{
				pObjectListEnd = pLOb;
			}
			return TRUE;
		}
		pLOb = pOb;
		pOb = pOb->GetNext();
	}

	return FALSE;

}

void Chunk::ResetBlocking()
{
	
	memset(Blocking,0, CHUNK_HEIGHT * 2);

	return;

	Object *pOb;
	D3DVECTOR *Position;
	
	pOb = pObjectList;

	while(pOb )
	{

		Position = pOb->GetPosition();

		if(pOb->GetMesh())
		{
//			AddBlocking(((int)Position->x) % CHUNK_WIDTH, ((int)Position->y) % CHUNK_HEIGHT,pOb->GetMesh()->GetBlocking());
		}

		if(pOb == pObjectListEnd)
		{
			pOb = NULL;
		}
		else
		{
			pOb = pOb->GetNext();
		}
	}
}

static unsigned short BaseBlock = 1;

void Chunk::SetBlocking(int x, int y)
{
	Blocking[y] = Blocking[y] | (BaseBlock << x);
}

void Chunk::RemoveBlocking(int x, int y)
{
	Blocking[y] = Blocking[y] ^ (BaseBlock << x);
}

BOOL Chunk::GetBlocking(int x, int y)
{
	if(Blocking[y] & (BaseBlock << x))
		return TRUE;
	else
		return FALSE;
}

float Chunk::GetHeight(int x, int y)
{
	return TileHeights[x + y * CHUNK_TILE_WIDTH];
}

void Chunk::CreateHeightMap()
{
	//use the largest height value in a given tile
	//at this point we have the draw and tile list
	//height
	
	//first get the height values for each corner of the tile.

	float fNW, fN, fNE, fS, fSW, fE, fSE, fW, fMid;
	float *pVerts;
	//unsigned short *DrawList;


	int xn, yn;

	for(yn = 0; yn < CHUNK_HEIGHT; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		pVerts = GetTile(xn, yn);
		
		fNW = pVerts[tz1];
		fNE = pVerts[tz2];
		fSW = pVerts[tz3];
		fSE = pVerts[tz4];
		
		fN = (fNW + fNE) / 2.0f;
		fS = (fSW + fSE) / 2.0f;
		fE = (fNE + fSE) / 2.0f;
		fW = (fNW + fSW) / 2.0f;

		fMid = (fNE + fSW) / 2.0f;
		if( ((fNW + fSE) / 2.0f) > fMid)
		{
			fMid = (fNW + fSE) / 2.0f;
		}
		
		//NW tile
		if(fMid > fNW && fMid > fN && fMid > fW)
		{
			TileHeights[xn*2 + (yn*2)*(2*CHUNK_WIDTH)] = fMid;
		}
		else
		if(fNW > fN && fNW > fW)
		{
			TileHeights[xn*2 + (yn*2)*(2*CHUNK_WIDTH)] = fNW;
		}
		else
		if(fN > fW)
		{
			TileHeights[xn*2 + (yn*2)*(2*CHUNK_WIDTH)] = fN;
		}
		else
		{
			TileHeights[xn*2 + (yn*2)*(2*CHUNK_WIDTH)] = fW;
		}
	
		//NE tile
		if(fMid > fNE && fMid > fN && fMid > fE)
		{
			TileHeights[xn*2 + 1 + (yn*2)*(2*CHUNK_WIDTH)] = fMid;
		}
		else
		if(fNE > fN && fNE > fE)
		{
			TileHeights[xn*2 + 1 + (yn*2)*(2*CHUNK_WIDTH)] = fNE;
		}
		else
		if(fN > fE)
		{
			TileHeights[xn*2 + 1 + (yn*2)*(2*CHUNK_WIDTH)] = fN;
		}
		else
		{
			TileHeights[xn*2 + 1 + (yn*2)*(2*CHUNK_WIDTH)] = fE;
		}
		
		//sw
		if(fMid > fSW && fMid > fW && fMid > fS)
		{
			TileHeights[xn*2 + 1 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fMid;
		}
		else
		if(fSW > fS && fSW > fW)
		{
			TileHeights[xn*2 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fSW;
		}
		else
		if(fS > fW)
		{
			TileHeights[xn*2 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fS;
		}
		else
		{
			TileHeights[xn*2 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fW;
		}
		
		//se
		if(fMid > fSE && fMid > fS && fMid > fE)
		{
			TileHeights[xn*2 + 1 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fMid;
		}
		else
		if(fSE > fS && fSE > fE)
		{
			TileHeights[xn*2 + 1 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fSE;
		}
		else
		if(fS > fE)
		{
			TileHeights[xn*2 + 1 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fS;
		}
		else
		{
			TileHeights[xn*2 + 1 + (yn*2 + 1)*(2*CHUNK_WIDTH)] = fE;
		}
	}
}

void Chunk::RemoveTile(int x, int y)
{
	int DrawOffset;
	DrawOffset = (x + y*CHUNK_WIDTH) * 6;
	DrawList[DrawOffset] = 0;//DrawList[DrawOffset + 2];
	DrawList[DrawOffset+1] = 0;//DrawList[DrawOffset + 2];
	DrawList[DrawOffset+2] = 0;//DrawList[DrawOffset + 2];
	DrawList[DrawOffset+3] = 0;//DrawList[DrawOffset + 2];
	DrawList[DrawOffset+4] = 0;//DrawList[DrawOffset + 2];
	DrawList[DrawOffset+5] = 0;//DrawList[DrawOffset + 2];
}

void Chunk::SetObjectHeights()
{
	float zFloor;

	Object *pOb;
	
	pOb = pObjectList;

	while(pOb)
	{
		zFloor = Valley->GetZ(pOb->GetPosition()->x,pOb->GetPosition()->y);
		pOb->GetPosition()->z = zFloor;
		pOb = pOb->GetNext();
	}
}

void Chunk::FixTerrainHeights()
{
	int xn,yn;
	float MaxHeight;
	float MinHeight;
	float Slope;
	float fNW, fNE, fSW, fSE;

	float *pVerts;

	int Ter;

	for(yn = 0; yn < CHUNK_HEIGHT; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		MaxHeight = 0.0f;
		MinHeight = 1024.0f;
		
		//unsigned short *DrawList;
		pVerts = GetTile(xn, yn);
			
		fNW = pVerts[tz1];
		fNE = pVerts[tz2];
		fSW = pVerts[tz3];
		fSE = pVerts[tz4];
		
		if(fNW < MinHeight) MinHeight = fNW;
		if(fNW > MaxHeight) MaxHeight = fNW;

		if(fNE < MinHeight) MinHeight = fNE;
		if(fNE > MaxHeight) MaxHeight = fNE;

		if(fSW < MinHeight) MinHeight = fSW;
		if(fSW > MaxHeight) MaxHeight = fSW;

		if(fSE < MinHeight) MinHeight = fSE;
		if(fSE > MaxHeight) MaxHeight = fSE;

		//check our terrain;
		Ter = GetTerrain(xn,yn);
		Ter = Ter - (Ter%64);

		Slope = MaxHeight - MinHeight;

		int ChunkX;
		int ChunkY;

		if(Slope > 1.99f)
		{
			if(Ter < TerrainIndex[TER_STONE_ONE])
			{
				this->Terrain[xn][yn] = TerrainIndex[TER_STONE_ONE + rand() % 4] + (rand() % 4)*4;
			}
			ChunkX = this->X * CHUNK_TILE_WIDTH;
			ChunkY = this->Y * CHUNK_TILE_HEIGHT;

			this->ClearTile(ChunkX + xn*2,		ChunkY + yn*2);
			this->ClearTile(ChunkX + xn*2+1,	ChunkY + yn*2);
			this->ClearTile(ChunkX + xn*2,		ChunkY + yn*2+1);
			this->ClearTile(ChunkX  + xn*2+1,	ChunkY + yn*2+1);
			
		}
		else
		{
			if(Ter >= TerrainIndex[TER_STONE_ONE] && Ter <= TerrainIndex[TER_STONE_FOUR]) 
			{
				this->Terrain[xn][yn] = TerrainIndex[TER_GRASS_ONE] + (rand() % 4)*4;
			}
		}
	}
}

void Chunk::Smooth(BYTE *pTerrains, int Pitch)
{
	int xn;
	int yn;
	int XStart;
	int YStart;
	int XEnd;
	int YEnd;
	int H, N, S,E,W,NE,SE,SW,NW;
	int Offset;

	for(yn = 0; yn < CHUNK_HEIGHT; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		ZeroMemory(&Overlays[xn][yn][0],sizeof(unsigned short)*NUM_OVERLAYS);
	}


	XStart = X * CHUNK_WIDTH;
	YStart = Y * CHUNK_HEIGHT;
	XEnd = XStart + CHUNK_WIDTH;
	YEnd = YStart + CHUNK_HEIGHT;
	
	int xat = 0;
	int yat = 0;
	for(yn = YStart; yn < YEnd; yn++)
	{
		xat = 0;
	for(xn = XStart; xn < XEnd; xn++)
	{
		Offset = xn + yn * Pitch;
		Terrain[xat][yat] = TerrainIndex[pTerrains[Offset]] + (rand() %4) * 4;
		
		H = pTerrains[Offset];
		N = pTerrains[Offset - Pitch];
		S = pTerrains[Offset + Pitch];
		NE = pTerrains[Offset + 1 - Pitch];
		NW = pTerrains[Offset - 1 - Pitch];
		SE = pTerrains[Offset + 1 + Pitch];
		SW = pTerrains[Offset - 1 + Pitch];
		E = pTerrains[Offset + 1];
		W = pTerrains[Offset - 1];
		
		if(N < H)
		{
			AddOverlay(xat,yat,TerrainIndex[N] + TDIR_S);
		}
		if(S < H)
		{
			AddOverlay(xat,yat,TerrainIndex[S] + TDIR_N);
		}
		if(E < H)
		{
			AddOverlay(xat,yat,TerrainIndex[E] + TDIR_W);
		}
		if(W < H)
		{
			AddOverlay(xat,yat,TerrainIndex[W] + TDIR_E);
		}
		if(NE < H)
		{
			AddOverlay(xat,yat,TerrainIndex[NE] + TDIR_SW);

		}
		if(NW < H)
		{
			AddOverlay(xat,yat,TerrainIndex[NW] + TDIR_SE);
		}
		if(SE < H)
		{
			AddOverlay(xat,yat,TerrainIndex[SE] + TDIR_NW);
		}
		if(SW < H)
		{
			AddOverlay(xat,yat,TerrainIndex[SW] + TDIR_NE);
		}
		xat++;
	}
		yat++;
	}



}

void Chunk::SetTileHeight(int x, int y, float NewHeight)
{
	TileHeights[x + y * CHUNK_TILE_WIDTH] = NewHeight;
}

void Chunk::BlockBySlope(float fTestSlope)
{

	//use the largest height value in a given tile
	//at this point we have the draw and tile list
	//height
	
	//first get the height values for each corner of the tile.

	float fNW, fN, fNE, fS, fSW, fE, fSE, fW, fMid;
	float *pVerts;
	//unsigned short *DrawList;
	float fHighest;
	float fLowest;
	float fSlope;

	int xn, yn;

	for(yn = 0; yn < CHUNK_HEIGHT; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		pVerts = GetTile(xn, yn);
		
		fNW = pVerts[tz1];
		fNE = pVerts[tz2];
		fSW = pVerts[tz3];
		fSE = pVerts[tz4];
		
		fN = (fNW + fNE) / 2.0f;
		fS = (fSW + fSE) / 2.0f;
		fE = (fNE + fSE) / 2.0f;
		fW = (fNW + fSW) / 2.0f;

		fMid = (fNE + fSW) / 2.0f;
		if( ((fNW + fSE) / 2.0f) > fMid)
		{
			fMid = (fNW + fSE) / 2.0f;
		}
		
		//NW tile
		fHighest = fNW;
		fLowest = fNW;
		if(fN > fHighest) fHighest = fN;
		if(fMid > fHighest) fHighest = fMid;
		if(fW > fHighest) fHighest = fW;

		if(fN < fLowest) fLowest = fN;
		if(fMid < fLowest) fLowest = fMid;
		if(fW < fLowest) fLowest = fW;

		fSlope = fHighest - fLowest;
		if(fSlope >= fTestSlope)
		{
			SetBlocking(xn*2,yn*2);
		}

		//NE tile
		fHighest = fNE;
		fLowest = fNE;
		if(fN > fHighest) fHighest = fN;
		if(fMid > fHighest) fHighest = fMid;
		if(fE > fHighest) fHighest = fE;

		if(fN < fLowest) fLowest = fN;
		if(fMid < fLowest) fLowest = fMid;
		if(fE < fLowest) fLowest = fE;

		fSlope = fHighest - fLowest;
		if(fSlope >= fTestSlope)
		{
			SetBlocking(xn*2 + 1,yn*2);
		}
		
		//sw
		fHighest = fSW;
		fLowest = fSW;
		if(fS > fHighest) fHighest = fS;
		if(fMid > fHighest) fHighest = fMid;
		if(fW > fHighest) fHighest = fW;

		if(fS < fLowest) fLowest = fS;
		if(fMid < fLowest) fLowest = fMid;
		if(fW < fLowest) fLowest = fW;

		fSlope = fHighest - fLowest;
		if(fSlope >= fTestSlope)
		{
			SetBlocking(xn*2,yn*2 + 1);
		}
		
		//se
		fHighest = fSE;
		fLowest = fSE;
		if(fS > fHighest) fHighest = fS;
		if(fMid > fHighest) fHighest = fMid;
		if(fE > fHighest) fHighest = fE;

		if(fS < fLowest) fLowest = fS;
		if(fMid < fLowest) fLowest = fMid;
		if(fE < fLowest) fLowest = fE;

		fSlope = fHighest - fLowest;
		if(fSlope >= fTestSlope)
		{
			SetBlocking(xn*2 + 1,yn*2 + 1);
		}
	}
}

void Chunk::DungeonBlock()
{
	//first block all "flipped" tiles
	int xn;
	int yn;
	int DrawOffset;
	int TileX;
	int TileY;
	for (yn = 0; yn < CHUNK_HEIGHT; yn++)
	for (xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		DrawOffset = (xn + yn*CHUNK_WIDTH) * 6;
		if(!DrawList[DrawOffset] && !DrawList[DrawOffset + 1])
		{
			TileX = this->X * CHUNK_TILE_WIDTH + xn * 2;
			TileY = this->Y * CHUNK_TILE_HEIGHT + yn * 2;

			this->SetBlocking(xn*2,yn*2);
			this->SetBlocking(xn*2+1,yn*2);
			this->SetBlocking(xn*2+1,yn*2+1);
			this->SetBlocking(xn*2,yn*2+1);

			if(xn > 0)
			{
				this->SetBlocking(xn*2 - 1,yn*2);
				this->SetBlocking(xn*2 - 1,yn*2+1);
			}
			else
			{
				if(TileX > 0)
				{
					Valley->SetBlocking(TileX - 1, TileY);
					Valley->SetBlocking(TileX - 1, TileY + 1); 
				}
			}

			if(xn < CHUNK_HEIGHT - 1)
			{
				this->SetBlocking(xn*2 + 2,yn*2);
				this->SetBlocking(xn*2 + 2,yn*2+1);
			}
			else
			{
				Valley->SetBlocking(TileX + 2, TileY);
				Valley->SetBlocking(TileX + 2, TileY + 1);
			}

			if(yn > 0)
			{
				this->SetBlocking(xn*2,yn*2 - 1);
				this->SetBlocking(xn*2+1,yn*2 - 1);
			}
			else
			{
				if(TileY > 0)
				{
					Valley->SetBlocking(TileX, TileY - 1);
					Valley->SetBlocking(TileX +1, TileY - 1); 
				}
			}

			if(yn < CHUNK_HEIGHT - 1)
			{
				this->SetBlocking(xn*2+1,yn*2+2);
				this->SetBlocking(xn*2,yn*2+2);
			}
			else
			{
				Valley->SetBlocking(TileX + 1, TileY + 2);
				Valley->SetBlocking(TileX, TileY + 2);
			}
		}
	}
}

void Chunk::SortObjects()
{
	Object *pOb;

	pOb = this->GetObjects();
	
	BOOL WaterFound = NULL;

	while(pOb)
	{
		if(pOb->GetObjectType() != OBJECT_WATER)
		{
			if(WaterFound == TRUE)
			{	
				this->RemoveObject(pOb);
				this->AddObject(pOb);
				pOb = this->GetObjects();
				WaterFound = FALSE;
			}
			else
			{
				pOb = pOb->GetNext();
			}
		}
		else
		{
			pOb = pOb->GetNext();
			WaterFound = TRUE;
		}
	}
}


BOOL Chunk::CheckLOS(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd)
{

	return TRUE;	
}

BOOL Chunk::IsRoadChunk()
{
	//first check for a road terrain
	int RoadTerrainCount = 0;

	int xn,yn, terrain;
	for(yn = 0; yn < CHUNK_HEIGHT; yn++)
	for(xn = 0; xn < CHUNK_WIDTH; xn++)
	{
		terrain = GetTerrain(xn,yn);
		terrain = terrain - (terrain%64);
		if(Terrain[xn][yn] == TerrainIndex[TER_SAND])
		{
			RoadTerrainCount++;
		}
	}

	if(RoadTerrainCount < 7) return FALSE;

	//check for water
	Object *pOb;

	pOb = GetObjects();

	while(pOb)
	{
		if(pOb->GetObjectType() == OBJECT_WATER)
		{
			return FALSE;
		}
		pOb = pOb->GetNext();
	}

	for(xn = 0; xn < CHUNK_TILE_WIDTH; xn++)
	for(yn = 0; yn < CHUNK_TILE_HEIGHT; yn++)
	{
		if(GetBlocking(xn,yn)) return FALSE;
	}
	
	char blarg[64];
	sprintf(blarg,"road at %i %i\n",this->X,this->Y);
	DEBUG_INFO(blarg);

	return TRUE;

}









