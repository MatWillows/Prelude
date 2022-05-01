#include "world.h"
#include "zsengine.h"
#include "forest.h"
#include "objects.h"
#include "zswindow.h"
#include "water.h"
#include <assert.h>
#include "editregion.h"
#include "cavewall.h"

#define HEIGHT_RANGE		256.0f //must limit to 64 in order to have a tile variance of .25f 

#define CAVE_HEIGHT_RANGE	16.0f;
#define TERRAIN_DEPTH   32

#define SMOOTH_FACTOR	2

#define WORLD_DIM		1600

#define ID_X_WIN		41415
#define ID_Y_WIN		41414
typedef struct
{
	char Name[32];
	int Width;
	int Height;
	int ChunkWidth;
	int ChunkHeight;
	int ChunkOffsets[200*200];
} OLD_FILE_HEADER_T;




#ifndef NDEBUG
float StartHeightLevels[1600][1600];
float HeightLevels[1600][1600];
static int Terrain[1600][1600];
static int ForestDensity[1600][1600];
static BYTE Road[3200][3200];
static BYTE BaseRoad[3200][3200];
BYTE Watered[200][200];
static WORD ForestTypes[32][32];	
static float CornerHeights[200][200];
static BYTE Locations[1600][1600];
#else
float StartHeightLevels[1][1];
float HeightLevels[1][1];
static int Terrain[1][1];
static int ForestDensity[1][1];
static BYTE Road[1][1];
static BYTE BaseRoad[1][1];
BYTE Watered[1][1];
static WORD ForestTypes[1][1];	
static float CornerHeights[1][1];
static BYTE Locations[1][1];
#endif


DWORD dwBSHL=0;
DWORD dwRSHL=0;
DWORD dwGSHL=0;

DWORD dwRMask = 0;
DWORD dwGMask = 0;
DWORD dwBMask = 0;

Forest Forests[8];
WORD FColors[8];

static int Highest = 0;
static int Lowest = 255;

/*
class WaterBand
{
	BOOL Vert;
	int xNW;
	int xNE;
	int xSE;
	int xSW;
	int yNW;
	int yNE;
	int ySE;
	int ySW;

	WaterBand()
	{
		Vert = FALSE;
		xNW = 0;
		xNE = 0;
		xSE = 0;
		xSW = 0;
		yNW = 0;
		yNE = 0;
		ySE = 0;
		ySW = 0;
	}

	void Expand();
	void AddToWorld();
};

WoidaterBand::Expand()
{
	if(Vert)
	{}
	else
	{
		//left
		float fzTemp;
		float fz;
		fz = Valley->GetTileHeight(xNW,yNW);
		fzTemp = Valley->GetTileHeight(xNW - 1, yNW);
		while(fzTemp - fz > 2.0f)
		{
			xNW--;
			fz = Valley->GetTileHeight(xNW,yNW);
			fzTemp = Valley->GetTileHeight(xNW - 1, yNW);
		}

		fz = Valley->GetTileHeight(xNE,yNE);
		fzTemp = Valley->GetTileHeight(xNE + 1, yNE);
		while(fzTemp - fz > 2.0f)
		{
			xNE++;
			fz = Valley->GetTileHeight(xNE,yNE);
			fzTemp = Valley->GetTileHeight(xNW - 1, yNE);
		}
	
		fz = Valley->GetTileHeight(xSW,ySW);
		fzTemp = Valley->GetTileHeight(xSW - 1, ySW);
		while(fzTemp - fz > 2.0f)
		{
			xSW--;
			fz = Valley->GetTileHeight(xSW,ySW);
			fzTemp = Valley->GetTileHeight(xSW - 1, ySW);
		}

		fz = Valley->GetTileHeight(xSE,ySE);
		fzTemp = Valley->GetTileHeight(xSE + 1, ySE);
		while(fzTemp - fz > 2.0f)
		{
			xSE++;
			fz = Valley->GetTileHeight(xSE,ySE);
			fzTemp = Valley->GetTileHeight(xNW - 1, ySE);
		}
	
		
	}

}
*/


Water *CreateWater(int ChunkX, int ChunkY)
{
	assert(ChunkX < 200);
	assert(ChunkY < 200);
	Water *pNewWater;
	pNewWater = new Water;

	int left;
	int right;
	int top;
	int bottom;

	left = ChunkX;
	right = ChunkX + 1;
	if(right >= 200) right = ChunkX;

	top = ChunkY;
	bottom = ChunkY + 1;
	if(bottom >= 200) bottom = ChunkY;

	pNewWater->MakeVerts(ChunkX,ChunkY,CornerHeights[left][top],
													CornerHeights[right][top],
													CornerHeights[left][bottom],
													CornerHeights[right][bottom]);
	return pNewWater;
}

void FillTerrain()
{
	//fills the basic terrain
	int xn;
	int yn;
	int n;
	RECT rArea;

	//fill basic grass
	for(yn = 0; yn < 1600; yn++)
	for(xn = 0; xn < 1600; xn++)
	{
		n = rand() % 5;
	
		switch(n)
		{
			case 0:
			case 1:
			case 2:
			case 3:
				Terrain[xn][yn] = TerrainIndex[TER_GRASS_ONE] + (n%4)*4;
				break;
			case 4:
			default:
				Terrain[xn][yn] = TerrainIndex[TER_GRASS_TWO] + (n%4)*4;
				break;
		}
	}

	int subx;
	int suby;
	int Index;

	//add patches
	for(yn = 0; yn < 1600; yn += 2)
	for(xn = 0; xn < 1600; xn += 2)
	{
		n = rand() % 100;
		if(n < 10)
		{
			n = rand() % 9;

			switch(n)
			{
				case 0:
				case 1:
					Index = TER_DIRT;
					break;
				case 2:
					Index = TER_MOSS;
					break;
				case 3:
				case 4:
					Index = TER_GRASS_TWO;
					break;
				case 5:
				case 6:
					Index = TER_GRASS_THREE;
					break;
				case 7:
				case 8:
					Index = TER_GRASS_FOUR;
					break;
				default:
					Index = TER_DIRT;
					break;
			}

			rArea.left = xn - (rand() % 6);
			if(rArea.left < 0) 
				rArea.left = 0;

			rArea.right = xn + (rand() % 6);
			if(rArea.right > 1600) 
				rArea.right = 1600;
			
			rArea.top = yn - (rand() % 6);
			if(rArea.top < 0) 
				rArea.top = 0;
			
			rArea.bottom = yn + (rand() % 6);
			if(rArea.bottom > 1600) 
				rArea.bottom = 1600;
			
			for(suby = rArea.top; suby < rArea.bottom; suby++)
			for(subx = rArea.left; subx < rArea.right; subx++)
			{
				n = rand() % 100;
				if(n < 66)
				{
					Terrain[subx][suby] = TerrainIndex[Index] + (n%4) * 4;			
				}
			}
		}


	}
}


unsigned short GetBaseTerrain(int TLevel)
{
	int n;
	n = rand();

	switch(TLevel)
	{
	case 0:
		switch(n%6)
		{
			case 5:
				return TerrainIndex[TER_DIRT] + (n%4) * 4;
			case 0:
			case 1:
			case 2:
			case 3:
				return TerrainIndex[TER_GRASS_ONE] + (n%4)*4;
			case 4:
			default:
				return TerrainIndex[TER_GRASS_TWO]+ (n%4)*4;
		}
		break;
	case 1:
		switch(n%4)
		{
		case 4:
				return TerrainIndex[TER_DIRT] + (n%4) * 4;
		case 0:
		case 1:
			return TerrainIndex[TER_GRASS_ONE] + (n%4)*4;
		case 2:
		default:
			return TerrainIndex[TER_GRASS_TWO] + (n%4)*4;
		}
		break;
	case 2:
		return TerrainIndex[TER_GRASS_ONE + n%2] + (n%4)*4;
	case 3:
		return TerrainIndex[TER_GRASS_TWO + n%2] + (n%4)*4;
	case 4:
		return TerrainIndex[TER_GRASS_THREE + n%2] + (n%4)*4;
	case 5:
		return TerrainIndex[TER_GRASS_FOUR + n%2] + (n%4)*4;
	case 6:
		return TerrainIndex[TER_GRASS_FOUR + n%3] + (n%4)*4;
	case 7:
		if(n%2)
		{
			return TerrainIndex[TER_DIRT] + (n%4)*4;
		}
		else
		{
			return TerrainIndex[TER_STONE_ONE + n%4] + (rand()%4)*4;	
		}
	case 8:
		return TerrainIndex[TER_STONE_ONE + n%4] + (rand()%4)*4;	
	default:
		return 11*64;
		break;
	}
}

void Area::SmoothBaseTerrain()
{
	ZSWindow *pXWin;
	ZSWindow *pYWin;

	pXWin = new ZSWindow(WINDOW_TEXT, ID_X_WIN, 100,100,64,16);
	pXWin->Show();
	ZSWindow::GetMain()->AddTopChild(pXWin);

	pYWin = new ZSWindow(WINDOW_TEXT, ID_Y_WIN, 164,100,64,16);
	pYWin->Show();
	ZSWindow::GetMain()->AddTopChild(pYWin);

	SetCurrentDirectory(".\\Areas");
	
	if(StaticFile)
		fclose(StaticFile);
	
	StaticFile = SafeFileOpen("valley.bin","r+b");
	
	int xn;
	int yn;
	for(yn = 1; yn < this->ChunkHeight - 1; yn++)
	{
		pYWin->SetText(yn);
		for(xn = 1; xn < this->ChunkWidth - 1; xn++)
		{
			pXWin->SetText(xn);
			ZSWindow::GetMain()->Draw();
			Engine->Graphics()->Flip();

			if(!GetChunk(xn,yn))
				LoadChunk(xn,yn);
			if(!GetChunk(xn+1,yn-1))
				LoadChunk(xn+1,yn-1);
			if(!GetChunk(xn+1,yn))
				LoadChunk(xn+1,yn);
			if(!GetChunk(xn+1,yn+1))
				LoadChunk(xn+1,yn+1);
			if(!GetChunk(xn,yn+1))
				LoadChunk(xn,yn+1);
			if(!GetChunk(xn-1,yn+1))
				LoadChunk(xn-1,yn+1);
			if(!GetChunk(xn-1,yn))
				LoadChunk(xn-1,yn);
			if(!GetChunk(xn-1,yn-1))
				LoadChunk(xn-1,yn-1);
			if(!GetChunk(xn,yn-1))
				LoadChunk(xn,yn-1);
			
			GetChunk(xn,yn)->Smooth();

			int SubX, SubY;
			for(SubY = yn * CHUNK_HEIGHT; SubY < (yn+1)* CHUNK_HEIGHT; SubY++)
			for(SubX = xn * CHUNK_WIDTH; SubX < (xn+1)* CHUNK_WIDTH; SubX++)
			{
				if(!(rand() % 30))
				{
					AddOverlay(SubX,SubY, - 1);
				}
			}

			if(!fseek(StaticFile,Header.ChunkOffsets[xn + yn*ChunkWidth],SEEK_SET))
			{
				GetChunk(xn,yn)->Save(StaticFile);
			}
			else
			{ 
				DEBUG_INFO("Could not seek during smoothing.\n");
			}

			delete BigMap[xn-1 + (yn-1) * ChunkWidth];
			BigMap[xn-1 + (yn-1) * ChunkWidth ] = NULL;
		}
	}

	ZSWindow::GetMain()->RemoveChild(pXWin);
	ZSWindow::GetMain()->RemoveChild(pYWin);

	fclose(StaticFile);
	StaticFile = SafeFileOpen("valley.bin","rb");
	
	SetCurrentDirectory(Engine->GetRootDirectory());

}

void Area::CleanUpChunks()
{
	int xn;
	int yn;

	for(yn = 0; yn < this->Height; yn++)
	for(xn = 0; xn < this->Width; xn++)
	{
		if(BigMap[xn + yn * Width])
		{
			delete BigMap[xn + yn * Width];
			BigMap[xn + yn * Width ] = NULL;
		}
	}
}

int GetForestValue(int x, int y)
{
	WORD Color;
	Color = ForestTypes[x/50][y/50];

	for(int n = 0; n < 8; n++)
	{
		if(FColors[n] == Color)
		{
			return n;
		}
	}
	return 2;
}

void GetShiftValues(LPDIRECTDRAWSURFACE7 Surface)
{
	DDSURFACEDESC2 TempDescription;
	TempDescription.dwSize = sizeof(DDSURFACEDESC2);
	
	ZeroMemory( &TempDescription, sizeof( TempDescription ));  
	TempDescription.dwSize = sizeof( TempDescription );  
 
	HRESULT hr;
	hr = Surface->Lock( NULL, &TempDescription, DDLOCK_WAIT, NULL );  
	if(hr != DD_OK)
	{
		return;
	}
	Surface->Unlock( NULL );

	dwBSHL=0;
	dwRSHL=0;
	dwGSHL=0;

	dwBMask = TempDescription.ddpfPixelFormat.dwBBitMask;
	
	dwRMask = TempDescription.ddpfPixelFormat.dwRBitMask;
	
	dwGMask = TempDescription.ddpfPixelFormat.dwGBitMask;

	while((dwBMask & 1)==0)
	{
		dwBMask=dwBMask >> 1;
		dwBSHL++;
	}

	while((dwGMask & 1)==0)
	{
		dwGMask=dwGMask >> 1;
		dwGSHL++;
	}

	while((dwRMask & 1)==0)
	{
		dwRMask=dwRMask >> 1;
		dwRSHL++;
	}

}


int GetLevel(WORD* SurfPtr, int x, int y, int pitch)
{
	WORD pixel;

	pixel = SurfPtr[pitch * y + x];
	
	DWORD Red;
	DWORD Green;
	DWORD Blue;

	Red = pixel & dwRMask;
	Red = Red >> dwRSHL;
	
	Green = pixel & dwGMask;
	Green = Green >> dwGSHL;
	
	Blue = pixel & dwBMask;
	Blue = Blue >> dwBSHL;

	return (Red + Green + Blue);
}

void Area::MakeWater(BOOL Vert)
{
	//open the water bitmap
	LPDIRECTDRAWSURFACE7 WaterSurface = NULL;

	WORD *WaterPtr;

	DDSURFACEDESC2  ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	HRESULT hr;

	WaterSurface = Engine->Graphics()->CreateSurfaceFromFile("Water.bmp",800,800,NULL,0);
	assert(WaterSurface);

	int Level;

	GetShiftValues(WaterSurface);

	int xn, yn;

	hr = WaterSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	
	if(FAILED(hr))
	{
		SafeExit("could not lock Water surface\n");
	}

	WaterPtr = (WORD *)ddsd.lpSurface;

	int xo, yo;
	
	//scan to get the height scale range;
	for(yn = 0; yn < 800; yn ++)
	for(xn = 0; xn < 800; xn ++)
	{
		yo = yn*2;
		xo = xn*2;

		Level = GetLevel(WaterPtr,xn,yn,800);
		if(Level < 16)
		{
			Watered[xn/4][yn/4] = 1;
			Terrain[xo][yo] = 1;		
			Terrain[xo][yo+1] = 1;		
			Terrain[xo+1][yo] = 1;		
			Terrain[xo+1][yo+1] = 1;		
		}


	}

	WaterSurface->Unlock(NULL);
	WaterSurface->Release();

	
	Chunk *pChunk;
	Object *pOb;

	for(yn = 0; yn < ChunkHeight; yn++)
	{
		for(xn = 0; xn < ChunkWidth; xn++)
		{

			if(Watered[xn][yn])
			{
				LoadChunk(xn,yn);
				pChunk = GetChunk(xn,yn);
				pOb = pChunk->GetObjects();
				while(pOb)
				{
					if(pOb->GetObjectType() == OBJECT_WATER)
					{
						pChunk->RemoveObject(pOb);
						pOb = pChunk->GetObjects();
					}
					else
					{
						pOb = pOb->GetNext();
					}
				}
			}
		}
	}


	D3DVECTOR vNW,vNE,vSW,vSE;
	float zTop;
	float zBottom;
	Water *pWater;
	int xNW, yNW, xNE, yNE, xSW, ySW, xSE, ySE, zNW,zNE,zSW,zSE;
	int sx, sy;

	BOOL Done;
	int ChunkX, ChunkY;
	char blarg[256];

	//search until we find an edge;
	for(yn = 0; yn < 1600; yn ++)
	for(xn = 0; xn < 1600; xn ++)
	{
		if(Terrain[xn][yn])
		{
			xNW = xn;
			yNW = yn;
			while(Terrain[xn][yn])
			{
				if(!GetChunk(xn * 2 / CHUNK_TILE_WIDTH, yn * 2/ CHUNK_TILE_HEIGHT))
				{
					LoadChunk(xn * 2 / CHUNK_TILE_WIDTH, yn * 2/ CHUNK_TILE_HEIGHT);
				}

				xn++;
			}
			xNE = xn - 1;
			yNE = yn;
			
			//create bottom left;
			sx = xNW;
			sy = yn + 1;
			while(!Terrain[sx][sy]) { sx++;  if( sx >=1600) { sx = xNW; break; }}
			while(Terrain[sx][sy]) { sx--; if(sx < 0) {sx = xNW; break; }}
			sx++;
			xSW = sx;
			ySW = sy;

			while(Terrain[sx][sy])
			{
				sx++;
				if( sx >= 1600) 
				{ 
					sx = xNE; 
					break; 
				};
			}
			
			xSE = sx - 1;
			ySE = sy;

			for(sx = xSW - 3; sx < xSE + 3; sx++)
			{
				if(!GetChunk(sx * 2 / CHUNK_TILE_WIDTH, sy * 2/ CHUNK_TILE_HEIGHT))
				{
					LoadChunk(sx * 2 / CHUNK_TILE_WIDTH, sy * 2/ CHUNK_TILE_HEIGHT);
				}
			}

			xNW *= 2;
			yNW *= 2;
			xNE *= 2;
			yNE *= 2;
			xSW *= 2;
			ySW *= 2;
			xSE *= 2;
			ySE *= 2;
	
			Done = FALSE;

		
			//left
			float fzTemp;
			float fz;
			fz = Valley->GetTileHeight(xNW,yNW);
			fzTemp = Valley->GetTileHeight(xNW - 1, yNW);
			while(fzTemp - fz > 1.4f)
			{
				xNW--;
				fz = Valley->GetTileHeight(xNW,yNW);
				fzTemp = Valley->GetTileHeight(xNW - 1, yNW);
			}
			zNW = fz;

			fz = Valley->GetTileHeight(xNE,yNE);
			fzTemp = Valley->GetTileHeight(xNE + 1, yNE);
			while(fzTemp - fz > 1.4f)
			{
				xNE++;
				fz = Valley->GetTileHeight(xNE,yNE);
				fzTemp = Valley->GetTileHeight(xNE + 1, yNE);
			}
			zNE = fz;
		
			fz = Valley->GetTileHeight(xSW,ySW);
			fzTemp = Valley->GetTileHeight(xSW - 1, ySW);
			while(fzTemp - fz > 1.4f)
			{
				xSW--;
				fz = Valley->GetTileHeight(xSW,ySW);
				fzTemp = Valley->GetTileHeight(xSW - 1, ySW);
			}
			zSW = fz;

			fz = Valley->GetTileHeight(xSE,ySE);
			fzTemp = Valley->GetTileHeight(xSE + 1, ySE);
			while(fzTemp - fz > 1.4f)
			{
				xSE++;
				fz = Valley->GetTileHeight(xSE,ySE);
				fzTemp = Valley->GetTileHeight(xSE + 1, ySE);
			}
			zSE = fz;
			
			if(zNE > zNW) { zTop = zNW;  } else { zTop = zNE; }
			if(zSE > zSW) { zBottom = zSW;  } else { zBottom = zSE; }
			
			while(!Done)
			{
				ChunkY = yNW / CHUNK_TILE_HEIGHT;
				ChunkX = xNW / CHUNK_TILE_WIDTH;
				pWater = new Water();
				pWater->Verts[0] = xNW;
				pWater->Verts[1] = yNW;
				pWater->Verts[2] = zTop;
				pWater->Verts[9] = xSW;
				pWater->Verts[10] = ySW;
				pWater->Verts[11] = zBottom;
				if(xNE / CHUNK_TILE_HEIGHT != ChunkX)
				{
					pWater->Verts[3] = (ChunkX + 1) * CHUNK_TILE_WIDTH;
					pWater->Verts[4] = yNE;
					pWater->Verts[5] = zTop;
					pWater->Verts[12] = (ChunkX + 1) * CHUNK_TILE_WIDTH;;
					pWater->Verts[13] = ySE;
					pWater->Verts[14] = zBottom;
					xNW = (ChunkX + 1) * CHUNK_TILE_WIDTH;
					xSW = (ChunkX + 1) * CHUNK_TILE_WIDTH;
				}
				else
				{
					pWater->Verts[3] = xNE;
					pWater->Verts[4] = yNE;
					pWater->Verts[5] = zTop;
					pWater->Verts[12] = xSE;
					pWater->Verts[13] = ySE;
					pWater->Verts[14] = zBottom;
					Done = TRUE;
				}

				sprintf(blarg, "added water:\n  NW %i,%i    NE %i,%i\n  SW %i,%i    SE %i, %i",
							(int)pWater->Verts[0], 
							(int)pWater->Verts[1], 
							(int)pWater->Verts[3], 
							(int)pWater->Verts[4], 
							(int)pWater->Verts[9], 
							(int)pWater->Verts[10], 
							(int)pWater->Verts[12], 
							(int)pWater->Verts[13]);
				DEBUG_INFO(blarg);

				pWater->Reset();
				pChunk = this->GetChunk(ChunkX, ChunkY);
				if(pChunk)
				pChunk->AddObject(pWater);
			}	
			
		}
	}

	Object *pOb2;
	Water *pSWat;
	Water *pNWat;
	Water *pWat2;
	float zRaise;
	float zNorth;
	BOOL Fixed;
	Fixed = FALSE;
	int nFixed;
	int Count;
	Count = 0;

	while(!Fixed && Count < 256)
	{
		Count++;
		DEBUG_INFO("\n water fixing pass\n");

		Fixed = TRUE;
		nFixed = 0;
		for(yn = 0; yn < this->ChunkHeight; yn ++)
		for(xn = 0; xn < this->ChunkWidth; xn ++)
		{
			pChunk = GetChunk(xn, yn);
			if(pChunk)
			{
				pOb = pChunk->GetObjects();
				while(pOb)
				{
					if(pOb->GetObjectType() == OBJECT_WATER)
					{
						//examine our z's
						pWater = (Water *)pOb;
						if(pWater->Verts[2] < pWater->Verts[11])
						{
							Fixed = FALSE;
							nFixed++;
							pNWat = NULL;
							pSWat = NULL;
							//find water to the north
							if(pWater->Verts[1] == yn * CHUNK_TILE_WIDTH)
							{
								pChunk = GetChunk(xn, yn-1);
							}
							else
							{
								pChunk = GetChunk(xn, yn);
							}

							if(pChunk)
							{
								pOb2 = pChunk->GetObjects();
								while(pOb2)
								{
									if(pOb2->GetObjectType() == OBJECT_WATER)
									{
										pWat2 = (Water *)pOb2;
										if(pWat2->Verts[10] == pWater->Verts[1])
										{
											pNWat = pWat2;
										}
									}
									pOb2 = pOb2->GetNext();
								}
							}

							zRaise = pWater->Verts[11];

							if(pNWat)
							{
							
								zNorth = pWater->Verts[2];
								if(zNorth < zRaise)
								{
									zRaise = zNorth;
								}
													
								pNWat->Verts[11] = zRaise;
								pNWat->Verts[14] = zRaise;
								pNWat->Reset();
							}

							if(pWater->Verts[1] == yn * CHUNK_TILE_WIDTH)
							{
								if(pWater->Verts[0] == xn * CHUNK_TILE_WIDTH)
								{
									pChunk = GetChunk(xn - 1, yn-1);
								}
								else
								{
									pChunk = GetChunk(xn, yn-1);
								}
							}
							else
							{
								if(pWater->Verts[0] == xn * CHUNK_TILE_WIDTH)
								{
									pChunk = GetChunk(xn - 1, yn);
								}
								else
								{
									pChunk = GetChunk(xn, yn);
								}
							}

							if(pChunk)
							{
								pOb2 = pChunk->GetObjects();
								while(pOb2)
								{
									if(pOb2->GetObjectType() == OBJECT_WATER)
									{
										pWat2 = (Water *)pOb2;
										if(pWat2->Verts[10] == pWater->Verts[1])
										{
											pNWat = pWat2;
										}
									}
									pOb2 = pOb2->GetNext();
								}
							}

							if(pNWat)
							{
								pNWat->Verts[11] = zRaise;
								pNWat->Verts[14] = zRaise;
								pNWat->Reset();
							}

							if(pWater->Verts[1] == yn * CHUNK_TILE_WIDTH)
							{
								if(pWater->Verts[0] == (xn+1) * CHUNK_TILE_WIDTH)
								{
									pChunk = GetChunk(xn + 1, yn-1);
								}
								else
								{
									pChunk = GetChunk(xn, yn-1);
								}
							}
							else
							{
								if(pWater->Verts[0] == (xn+1) * CHUNK_TILE_WIDTH)
								{
									pChunk = GetChunk(xn + 1, yn);
								}
								else
								{
									pChunk = GetChunk(xn, yn);
								}
							}

							if(pChunk)
							{
								pOb2 = pChunk->GetObjects();
								while(pOb2)
								{
									if(pOb2->GetObjectType() == OBJECT_WATER)
									{
										pWat2 = (Water *)pOb2;
										if(pWat2->Verts[10] == pWater->Verts[1])
										{
											pNWat = pWat2;
										}
									}
									pOb2 = pOb2->GetNext();
								}
							}

							if(pNWat)
							{
								pNWat->Verts[11] = zRaise;
								pNWat->Verts[14] = zRaise;
								pNWat->Reset();
							}




							pWater->Verts[2] = zRaise;
							pWater->Verts[5] = zRaise;
							pWater->Reset();

							if(pWater->Verts[11] > zRaise)
							{
								pWater->Verts[11] = zRaise;
								pWater->Verts[14] = zRaise;
								if(pWater->Verts[10] == (yn+1)* CHUNK_TILE_WIDTH)
								{
									pChunk = GetChunk(xn, yn+1);
								}
								else
								{
									pChunk = GetChunk(xn, yn);
								}

								if(pChunk)
								{
									pOb2 = pChunk->GetObjects();
									while(pOb2)
									{
										if(pOb2->GetObjectType() == OBJECT_WATER)
										{
											pWat2 = (Water *)pOb2;
											if(pWat2->Verts[1] == pWater->Verts[10])
											{
												pSWat = pWat2;
											}
										}
										pOb2 = pOb2->GetNext();
									}
								}
								
								if(pSWat)
								{
									pSWat->Verts[2] = pWater->Verts[11];
									pSWat->Verts[5] = pWater->Verts[14];
									pSWat->Reset();
								}

								if(pWater->Verts[1] == (yn+1) * CHUNK_TILE_WIDTH)
								{
									if(pWater->Verts[0] == (xn+1) * CHUNK_TILE_WIDTH)
									{
										pChunk = GetChunk(xn + 1, yn+1);
									}
									else
									{
										pChunk = GetChunk(xn, yn + 1);
									}
								}
								else
								{
									if(pWater->Verts[0] == (xn + 1) * CHUNK_TILE_WIDTH)
									{
										pChunk = GetChunk(xn + 1, yn);
									}
									else
									{
										pChunk = GetChunk(xn, yn);
									}
								}

								if(pChunk)
								{
									pOb2 = pChunk->GetObjects();
									while(pOb2)
									{
										if(pOb2->GetObjectType() == OBJECT_WATER)
										{
											pWat2 = (Water *)pOb2;
											if(pWat2->Verts[1] == pWater->Verts[10])
											{
												pSWat = pWat2;
											}
										}
										pOb2 = pOb2->GetNext();
									}
								}
								
								if(pSWat)
								{
									pSWat->Verts[2] = pWater->Verts[11];
									pSWat->Verts[5] = pWater->Verts[14];
									pSWat->Reset();
								}

							if(pWater->Verts[1] == (yn+1) * CHUNK_TILE_WIDTH)
							{
								if(pWater->Verts[0] == (xn * CHUNK_TILE_WIDTH))
								{
									pChunk = GetChunk(xn - 1, yn+1);
								}
								else
								{
									pChunk = GetChunk(xn, yn-1);
								}
							}
							else
							{
								if(pWater->Verts[0] == (xn * CHUNK_TILE_WIDTH))
								{
									pChunk = GetChunk(xn - 1, yn);
								}
								else
								{
									pChunk = GetChunk(xn, yn);
								}
							}

								if(pChunk)
								{
									pOb2 = pChunk->GetObjects();
									while(pOb2)
									{
										if(pOb2->GetObjectType() == OBJECT_WATER)
										{
											pWat2 = (Water *)pOb2;
											if(pWat2->Verts[1] == pWater->Verts[10])
											{
												pSWat = pWat2;
											}
										}
										pOb2 = pOb2->GetNext();
									}
								}
								
								if(pSWat)
								{
									pSWat->Verts[2] = pWater->Verts[11];
									pSWat->Verts[5] = pWater->Verts[14];
									pSWat->Reset();
								}
							}
						}
					}
					pOb = pOb->GetNext();
				}
			}
		}
		sprintf(blarg,"fixed: %i\n",nFixed);
		DEBUG_INFO(blarg);
		sprintf(blarg,"pass # %i\n",Count);
		DEBUG_INFO(blarg);
	}

	//now fix the water heights.
	
	int ChunkN;
	int ChunkS;
	Count = 0;

	Fixed = FALSE;
	while(!Fixed)
	{
		Count++;
		DEBUG_INFO("\n water fixing pass\n");

		Fixed = TRUE;
		for(yn = 0; yn < this->ChunkHeight; yn ++)
		for(xn = 0; xn < this->ChunkWidth; xn ++)
		{
			pChunk = GetChunk(xn, yn);
			if(pChunk)
			{
				pOb = pChunk->GetObjects();
				while(pOb)
				{
					if(pOb->GetObjectType() == OBJECT_WATER)
					{
						pWater = (Water *)pOb;
						if(pWater->Verts[1] == yn * CHUNK_TILE_HEIGHT)
						{
							ChunkN = yn - 1;
						}
						else
						{
							ChunkN = yn;
						}
						
						if(pWater->Verts[10] == (yn+1) * CHUNK_TILE_HEIGHT)
						{
							ChunkS = yn + 1;
						}
						else
						{
							ChunkS = yn;
						}
	
						//northwest;
						pChunk = GetChunk(xn-1,ChunkN);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[13] == pWater->Verts[1] &&
										pWat2->Verts[12] == pWater->Verts[0])
									{
										if(pWater->Verts[2] > pWat2->Verts[14])
										{
											Fixed = FALSE;
											pWater->Verts[2] = pWater->Verts[5] = pWat2->Verts[14];
											pWater->Reset();
										}
										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}

						//north
						pChunk = GetChunk(xn,ChunkN);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[13] == pWater->Verts[4] &&
										pWat2->Verts[12] == pWater->Verts[3])
									{
										if(pWater->Verts[2] > pWat2->Verts[14])
										{
											Fixed = FALSE;
											pWater->Verts[2] = pWater->Verts[5] = pWat2->Verts[14];
											pWater->Reset();
										}
										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}

						//northeast
						pChunk = GetChunk(xn + 1,ChunkN);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[10] == pWater->Verts[4] &&
										pWat2->Verts[9] == pWater->Verts[3])
									{
										if(pWater->Verts[2] > pWat2->Verts[14])
										{
											Fixed = FALSE;
											pWater->Verts[2] = pWater->Verts[5] = pWat2->Verts[14];
											pWater->Reset();
										}
										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}

						//west;
						pChunk = GetChunk(xn-1,yn);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[13] == pWater->Verts[10] &&
										pWat2->Verts[12] == pWater->Verts[9])
									{
										if(pWater->Verts[2] > pWat2->Verts[2])
										{
											Fixed = FALSE;
											pWater->Verts[2] = pWater->Verts[5] = pWat2->Verts[2];
											pWater->Reset();
										}

										if(pWater->Verts[11] > pWat2->Verts[14])
										{
											Fixed = FALSE;
											pWater->Verts[11] = pWater->Verts[14] = pWat2->Verts[14];
											pWater->Reset();
										}

										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}

						//east
						pChunk = GetChunk(xn + 1,yn);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[10] == pWater->Verts[13] &&
										pWat2->Verts[9] == pWater->Verts[2])
									{
										if(pWater->Verts[2] > pWat2->Verts[2])
										{
											Fixed = FALSE;
											pWater->Verts[2] = pWater->Verts[5] = pWat2->Verts[2];
											pWater->Reset();
										}

										if(pWater->Verts[11] > pWat2->Verts[14])
										{
											Fixed = FALSE;
											pWater->Verts[11] = pWater->Verts[14] = pWat2->Verts[14];
											pWater->Reset();
										}
										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}

						//South;
						pChunk = GetChunk(xn-1,ChunkS);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[4] == pWater->Verts[10] &&
										pWat2->Verts[3] == pWater->Verts[9])
									{
										if(pWater->Verts[11] > pWat2->Verts[5])
										{
											Fixed = FALSE;
											pWater->Verts[11] = pWater->Verts[14] = pWat2->Verts[5];
											pWater->Reset();
										}
										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}

						//south
						pChunk = GetChunk(xn,ChunkS);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[1] == pWater->Verts[10] &&
										pWat2->Verts[0] == pWater->Verts[9])
									{
										if(pWater->Verts[11] > pWat2->Verts[2])
										{
											Fixed = FALSE;
											pWater->Verts[11] = pWater->Verts[14] = pWat2->Verts[2];
											pWater->Reset();
										}
										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}

						//southeast
						pChunk = GetChunk(xn + 1,ChunkS);
						if(pChunk)
						{
							pOb2 = pChunk->GetObjects();
							while(pOb2)
							{
								if(pOb2->GetObjectType() == OBJECT_WATER)
								{
									pWat2 = (Water *)pOb2;
									if(pWat2->Verts[1] == pWater->Verts[13] &&
										pWat2->Verts[0] == pWater->Verts[12])
									{
										if(pWater->Verts[14] > pWat2->Verts[2])
										{
											Fixed = FALSE;
											pWater->Verts[11] = pWater->Verts[14] = pWat2->Verts[2];
											pWater->Reset();
										}
										break;
									}
								}
								pOb2 = pOb2->GetNext();
							}
						}
	
					}
					pOb = pOb->GetNext();
				}
			}
		}
		sprintf(blarg,"smooth pass # %i\n",Count);
		DEBUG_INFO(blarg);

	}


	DEBUG_INFO("\ndone watering\n");
	Describe("Done Watering World.");
}

void CreateBaseHeights()
{
	ZSModelEx *pMesh;
	pMesh = Engine->GetMesh("valley");
	assert(pMesh);

	DEBUG_INFO("Filling height levels\n");
/*  float foffset;
	float x;
	float y;
	foffset = 1.0f / 1600.0f;

	x = 0.0f;
	y = 0.0f;

	for(int yn = 0; yn < 1600; yn ++)
	{
		x = 0;
		for(int xn = 0; xn < 1600; xn ++)
		{
			x+= foffset;
			StartHeightLevels[xn][yn] = (pMesh->GetZ(x,y) * HEIGHT_RANGE);
		}
		y += foffset;
	}
*/
	pMesh->GetZ(0.0f,0.0f);
	DEBUG_INFO("Done Filling base height levels\n");

}

void Area::GenerateTerrain(RECT *pArea)
{
	BYTE *Ter;
	RECT rArea;
	int Pitch;
	int XStart,YStart,XEnd,YEnd;
	XStart = pArea->left / 2;
	XEnd = pArea->right / 2;
	YStart = pArea->top / 2;
	YEnd = pArea->bottom / 2;

	RECT rChunk;
	rChunk.left = pArea->left / CHUNK_TILE_WIDTH;
	rChunk.right = pArea->right / CHUNK_TILE_WIDTH;
	rChunk.bottom = pArea->bottom / CHUNK_TILE_HEIGHT;
	rChunk.top = pArea->top / CHUNK_TILE_HEIGHT;

	Pitch = this->Width / 2;
	Ter = new BYTE[(this->Width / 2) * (this->Height / 2)];
	memset(Ter, TER_GRASS_ONE, sizeof(BYTE) * (this->Width / 2) * (this->Height / 2));

	Forests[0].Load("Maple");
	Forests[1].Load("Aspen");
	Forests[2].Load("Default");
	Forests[3].Load("IronWood");
	Forests[4].Load("Swamp");
	Forests[5].Load("Dead");
	Forests[6].Load("SparseOne");
	Forests[7].Load("SparseTwo");

	LPDIRECTDRAWSURFACE7 TreeSurface = NULL;
	LPDIRECTDRAWSURFACE7 ForestSurface = NULL;
	LPDIRECTDRAWSURFACE7 RoadSurface = NULL;

	WORD *TreePtr;
	WORD *RoadPtr;

	DDSURFACEDESC2  ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	HRESULT hr;

	TreeSurface = Engine->Graphics()->CreateSurfaceFromFile("trees.bmp",800,800,NULL,123456);
	assert(TreeSurface);
	
	ForestSurface = Engine->Graphics()->CreateSurfaceFromFile("Foresttypes.bmp",32,33,NULL,123132);
	assert(ForestSurface);

	RoadSurface = Engine->Graphics()->CreateSurfaceFromFile("roads.bmp",800,800,NULL,0);
	assert(RoadSurface);

	int Level;

	GetShiftValues(RoadSurface);

	hr = TreeSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		TreeSurface->Release();
		ForestSurface->Release();
		SafeExit("could not lock trees\n");
	}

	TreePtr = (WORD *) ddsd.lpSurface;

	hr = ForestSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		TreeSurface->Unlock(NULL);
		TreeSurface->Release();
		ForestSurface->Release();
		SafeExit("could not lock tree types\n");
	}

	WORD *ForestPtr;

	ForestPtr = (WORD *) ddsd.lpSurface;
	for(int cn = 0; cn < 8; cn++)
	{
		FColors[cn] = ForestPtr[32*32+cn];
	}

	int xn, yn;

	for(yn = 0; yn < 32; yn++)
	for(xn = 0; xn < 32; xn++)
	{
		ForestTypes[xn][yn] = ForestPtr[xn + yn*32];//ddsd.lPitch];
	}

	hr = RoadSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	
	if(FAILED(hr))
	{
		ForestSurface->Unlock(NULL);
		TreeSurface->Unlock(NULL);
		TreeSurface->Release();
		ForestSurface->Release();
		RoadSurface->Release();
		SafeExit("could not lock ROAD surface\n");
	}

	RoadPtr = (WORD *)ddsd.lpSurface;

	int MaxDense = 0;
	int MinDense = 255;
	
	for(yn = YStart; yn < YEnd; yn ++)
	{
		memset(&Road[yn][0],0,1600);	
	}

	//scan to get the height scale range;
	for(yn = YStart/2; yn < YEnd/2; yn ++)
	for(xn = XStart/2; xn < XEnd/2; xn ++)
	{
		ForestDensity[xn*2][yn*2] = Level = GetLevel(TreePtr,xn,yn,800);
		ForestDensity[xn*2][yn*2+1] = Level;
		ForestDensity[xn*2+1][yn*2] = Level;
		ForestDensity[xn*2+1][yn*2+1] = Level;

		if(Level < MinDense)
		{
			MinDense = Level;
		}
		if(Level > MaxDense)
		{
			MaxDense = Level;
		}

		Level = GetLevel(RoadPtr,xn,yn,800);

		if(Level < 16)
		{
			Road[xn * 2-1][yn * 2-1] = 1;			
			Road[xn * 2][yn * 2-1] = 1;			
			Road[xn * 2+1][yn * 2-1] = 1;			
			Road[xn * 2-1][yn * 2] = 1;			
			Road[xn * 2][yn * 2] = 1;			
			Road[xn * 2+1][yn * 2] = 1;			
			Road[xn * 2-1][yn * 2+1] = 1;			
			Road[xn * 2][yn * 2+1] = 1;			
			Road[xn * 2+1][yn * 2+1] = 1;			
		}			
	}
	
	RoadSurface->Unlock(NULL);
	TreeSurface->Unlock(NULL);
	ForestSurface->Unlock(NULL);

	RoadSurface->Release();
	TreeSurface->Release();
	ForestSurface->Release();

	int RoadTotal;

	for(yn = YStart + 1; yn < YEnd - 1; yn ++)
	for(xn = XStart + 1; xn < XEnd - 1; xn ++)
	{
		RoadTotal = 0;
		if(Road[xn-1][yn]) RoadTotal++;

		if(Road[xn][yn+1]) RoadTotal++;
		
		if(Road[xn][yn-1]) RoadTotal++;
		
		if(Road[xn+1][yn]) RoadTotal++;
		
		if(Road[xn][yn] || RoadTotal >= 2)
		{
			BaseRoad[xn][yn] = 1;
		}
	}

	for(yn = YStart; yn < YEnd; yn ++)
	{
		memset(&Road[yn][0],0,1600);	
	}
	
	for(yn = YStart; yn < YEnd; yn++)
	for(xn = XStart; xn < XEnd; xn++)
	{
		ForestDensity[xn][yn] = 100 - ((ForestDensity[xn][yn] * 100) / MaxDense);
	}
	
	//0 = nothing;
	//1 = tree
	//2 = shrub
	ZSModelEx *Tree;

	int xo = 0;
	int yo = 0;
	int n;
	//first tree pass;
	int NumTrees = 0;
	int NumShrubs = 0;
	
	for(yn = YStart; yn < YEnd; yn++)
	{
		for(xn = XStart; xn < XEnd; xn++)
		{
			n = rand() % 350;

			if(n < ForestDensity[xn][yn])
			{
				NumTrees++;
				Road[xn][yn] = 1;
			}
			else
			{
				Road[xn][yn] = 0;
			}
		}
	}	

	char blarg[64];
	sprintf(blarg,"Pass one trees: %i\n",NumTrees);
	DEBUG_INFO(blarg);

	int Adjacent;
	
	//pass one, add trees
	for(yn = YStart + 1; yn < YEnd - 1; yn++)
	for(xn = XStart + 1; xn < XEnd - 1; xn++)
	{
		Adjacent = 0;
		Adjacent += Road[xn-1][yn+1];
		Adjacent += Road[xn-1][yn];
		Adjacent += Road[xn-1][yn+1];
		Adjacent += Road[xn+1][yn-1];
		Adjacent += Road[xn+1][yn];
		Adjacent += Road[xn+1][yn+1];
		Adjacent += Road[xn][yn-1];
		Adjacent += Road[xn][yn];
		Adjacent += Road[xn][yn+1];
	
		n = rand() % 17;
		if(n < Adjacent)
		{
			NumTrees++;
			Road[xn][yn] = 1;
		}
	}

	sprintf(blarg,"Pass two trees: %i\n",NumTrees);
	DEBUG_INFO(blarg);

	for(yn = YStart; yn < YEnd; yn++)
	{
		for(xn = XStart; xn < XEnd; xn++)
		{
			if(!Road[xn][yn])
			{
				n = rand() % 600;
				
				if(n < ForestDensity[xn][yn])
				{
					NumShrubs++;
					Road[xn][yn] = 2;
				}
				else
				{
					Road[xn][yn] = 0;
				}
			}
		}
	}
	sprintf(blarg,"Shrubbery: %i\n",NumShrubs);
	DEBUG_INFO(blarg);

	//the basic terrain is layered in
	//see the secondary terrain
	//add patches
	int Offset = 0;
	int subx,suby;
	int Index;
	for(yn = YStart; yn < YEnd; yn ++)
	for(xn = XStart; xn < XEnd; xn ++)
	{
		Offset = xn + yn * Pitch;
		n = rand() % 100;
		if(n < 3)
		{
			n = rand() % 9;

			switch(n)
			{
				case 0:
					Index = TER_SAND;
					break;
				case 2:
					Index = TER_MOSS;
					break;
				case 3:
				case 4:
				case 1:
					Index = TER_GRASS_TWO;
					break;
				case 5:
				case 6:
				case 7:
					Index = TER_GRASS_THREE;
					break;
				case 8:
					Index = TER_GRASS_FOUR;
					break;
				default:
					Index = TER_DIRT;
					break;
			}

			rArea.left = xn - (rand() % 5);
			if(rArea.left < 0) 
				rArea.left = 0;

			rArea.right = xn + (rand() % 5);
			if(rArea.right > this->Width / 2) 
				rArea.right = this->Width / 2;
			
			rArea.top = yn - (rand() % 5);
			if(rArea.top < 0) 
				rArea.top = 0;
			
			rArea.bottom = yn + (rand() % 5);
			if(rArea.bottom > this->Width / 2) 
				rArea.bottom = this->Width / 2;
			
			for(suby = rArea.top; suby < rArea.bottom; suby++)
			for(subx = rArea.left; subx < rArea.right; subx++)
			{
				n = rand() % 100;
				if(n < 50)
				{
					Ter[subx + suby * Pitch] = Index;			
				}
			}
		}
	}

	int H,N,E,S,W;
	int HDown;

	//pass two, remove trees
	for(yn = YStart + 2; yn < YEnd - 2; yn++)
	for(xn = XStart + 2; xn < XEnd - 2; xn++)
	{
		if(Road[xn][yn])
		{
			Adjacent = 0;
			for(yo = yn - 2; yo <= yn + 2; yo++)
			for(xo = xn - 2; xo <= xn + 2; xo++)
			{
				Adjacent += Road[xo][yo];
			}

			n = rand() % 8;
			if(n > Adjacent)
			{
				NumTrees--;
				Road[xn][yn] = 0;
			}
			
			if(Adjacent >= 8)
			{
				Ter[xn + yn * Pitch] = TER_DIRT;
			}

		}
	}

	sprintf(blarg,"Post removal trees: %i\n",NumTrees);
	DEBUG_INFO(blarg);

	BOOL Modified = TRUE;
	while(Modified)
	{
		Modified = FALSE;
		//now we have to smooth
		for(yn = YStart + 1; yn < YEnd - 1; yn++)
		for(xn = XStart + 1; xn < XEnd - 1; xn++)
		{
			Offset = xn + yn * Pitch;
					
			H = Ter[Offset];

			if(H > TER_GRASS_TWO)
			{
				switch(H)
				{
					case TER_DIRT:
						HDown = TER_MOSS;
						break;
					case TER_SAND:
						HDown = TER_GRASS_THREE;
						break;
					default:
						HDown = H - 1;
						break;
				}

				N = Ter[Offset - Pitch];
				S = Ter[Offset + Pitch];
				E = Ter[Offset + 1];
				W = Ter[Offset - 1];
				
				if(N < HDown)
				{
					Ter[Offset - Pitch] = HDown;
					Modified = TRUE;
				}

				if(S < HDown)
				{
					Ter[Offset + Pitch] = HDown;
					Modified = TRUE;
				}

				if(E < HDown)
				{
					Ter[Offset + 1] = HDown;
					Modified = TRUE;
				}

				if(W < HDown)
				{
					Ter[Offset - 1] = HDown;
					Modified = TRUE;
				}
			}
		}

	}

	//set up roads

	for(yn = YStart; yn < YEnd; yn++)
	for(xn = XStart; xn < XEnd; xn++)
	{
		if(BaseRoad[xn][yn])
		{
			Ter[xn + yn * Pitch] = TER_SAND;
			Road[xn][yn] = 0;
		}
	}

	//NowActually set the terrain

//	fclose(StaticFile);

	char FileName[64];
	sprintf(FileName,"%s.bin",Header.Name);
		
	SetCurrentDirectory(".\\Areas");
	
	FILE *fp;
	fp = SafeFileOpen("temp.bin","wb");
	SaveHeader(fp);
		
	int *NewOff;
	NewOff = new int[ChunkWidth * ChunkHeight];
	
	int ChunkX;
	int ChunkY;
	int xat,yat;
	XStart /= CHUNK_WIDTH;
	YStart /= CHUNK_HEIGHT;
	XEnd /= CHUNK_WIDTH;
	YEnd /= CHUNK_HEIGHT;
	
	Chunk *pChunk;
	Object *Op;
	float ScaleOffset;
	float Angle;
	D3DVECTOR Position;

	NumTrees = 0;
	NumShrubs = 0;
	
	for(ChunkY = 0; ChunkY < ChunkWidth; ChunkY++)
	for(ChunkX = 0; ChunkX < ChunkHeight; ChunkX++)
	{
		pChunk = GetChunk(ChunkX,ChunkY);
		if(!pChunk)
		{
			LoadChunk(ChunkX,ChunkY);
			pChunk = GetChunk(ChunkX, ChunkY);
		}

	if(ChunkX > rChunk.left && ChunkY > rChunk.top && ChunkX < rChunk.right && ChunkY < rChunk.bottom)
	{
		
		while(pChunk->GetObjects())
		{
			Op = pChunk->GetObjects();
			pChunk->RemoveObject(Op);
			delete Op;
		}

		int LeftOff;
		int TopOff;

		for(yn = 0; yn < CHUNK_HEIGHT; yn++)
		for(xn = 0; xn < CHUNK_WIDTH; xn++)
		{
			xat = ChunkX * CHUNK_WIDTH + xn;
			yat = ChunkY * CHUNK_HEIGHT + yn;
			pChunk->SetTerrain(xn, yn,TerrainIndex[Ter[xat + yat * Pitch]] + rand()%4 * 4);
			if(Road[xat][yat] == 1)
			{
				Tree = Forests[GetForestValue(xat, yat)].GetTree();
				NumTrees++;
			}
			else
			if(Road[xat][yat] == 2)
			{
				Tree = Forests[GetForestValue(xat, yat)].GetShrub();
				NumShrubs++;
			}
			else
				Tree = NULL;

			if(Tree)
			{
				LeftOff = rand() % 2;
				TopOff = rand() % 2;
				
				ScaleOffset = (float)rand() / (float)RAND_MAX;
				ScaleOffset *= 0.5f;
				Op = new Object;
				Position.x = ((float)xat*2) + LeftOff + 0.5f;
				Position.y = ((float)yat*2) + TopOff + 0.5f;
				Position.z = pChunk->GetHeight((xn*2)+LeftOff, (yn*2)+TopOff) - 0.01f;
				Op->SetPosition(&Position);
				Op->SetMesh(Tree);
				Op->SetMeshNum(Engine->GetMeshNum(Tree));
				Op->SetScale(0.75f + ScaleOffset);
				Angle = ((float)rand() /(float)RAND_MAX) * PI_MUL_2;
				Op->SetAngle(Angle);
				Op->SetTexture(Tree->GetTexture());
				Op->SetTextureNum(Engine->GetTextureNum(Tree->GetTexture()));
				Op->SetNext(pChunk->pObjectList);
				pChunk->pObjectList = Op;
			}
		}
		
		pChunk->FixTerrainHeights();
		pChunk->Smooth(Ter,Pitch);
		//add overlays
		for(yn = 0; yn < CHUNK_HEIGHT; yn++)
		for(xn = 0; xn < CHUNK_WIDTH; xn++)
		{
			xat = ChunkX * CHUNK_WIDTH + xn;
			yat = ChunkY * CHUNK_HEIGHT + yn;
			if(!(rand() % 32))
			{
				Valley->AddOverlay(xat,yat, -1);
			}
		
		}
	}	
		NewOff[ChunkX + ChunkY * this->ChunkWidth] = ftell(fp);
		pChunk->Save(fp);
		delete pChunk;
		BigMap[ChunkX + ChunkY * this->ChunkWidth] = NULL;
	}

	fseek(fp,0,SEEK_SET);

	delete[] Header.ChunkOffsets;
	Header.ChunkOffsets = NewOff;

	SaveHeader(fp);

	fclose(fp);

	fclose(StaticFile);

	remove(FileName);
	rename("temp.bin",FileName);

	StaticFile = SafeFileOpen(FileName,"rb");

	SetCurrentDirectory(Engine->GetRootDirectory());

	delete[] Ter;
	return;
}

int Area::GenerateBase(RECT *rArea)
{
	RECT *pArea;
	RECT pAll;
	pAll.right = WORLD_DIM;
	pAll.bottom = WORLD_DIM;
	pAll.top = 0;
	pAll.left = 0;
	if(!rArea)
	{
		pArea = &pAll;
	}
	else
	{
		pArea = rArea;
	}

	pArea->left -= pArea->left % CHUNK_WIDTH;
	pArea->right -= pArea->right % CHUNK_WIDTH;
	pArea->top -= pArea->top % CHUNK_HEIGHT;
	pArea->bottom -= pArea->bottom % CHUNK_HEIGHT;


	if(StaticFile)
		fclose(StaticFile);

//	CleanUpChunks();

	Chunk TempChunk;

	Forests[0].Load("Maple");
	Forests[1].Load("Aspen");
	Forests[2].Load("Default");
	Forests[3].Load("IronWood");
	Forests[4].Load("Swamp");
	Forests[5].Load("Dead");
	Forests[6].Load("SparseOne");
	Forests[7].Load("SparseTwo");

	int ChunkSize;
	ChunkSize = sizeof(Chunk); //sansitemlist
	Header.ChunkWidth = CHUNK_WIDTH;
	Header.ChunkHeight = CHUNK_HEIGHT;
	this->Height = Header.Height = (pArea->right/CHUNK_WIDTH) - (pArea->left/CHUNK_WIDTH);
	this->Width = Header.Width = (pArea->bottom/CHUNK_HEIGHT) - (pArea->top/CHUNK_HEIGHT);
	//RegionHeader.NumRegions = 0;
	strcpy(Header.Name,"TestWorld");
	
	LPDIRECTDRAWSURFACE7 ElevationSurface = NULL;
	LPDIRECTDRAWSURFACE7 TreeSurface = NULL;
	LPDIRECTDRAWSURFACE7 ForestSurface = NULL;
	LPDIRECTDRAWSURFACE7 RoadSurface = NULL;
	LPDIRECTDRAWSURFACE7 WaterSurface = NULL;

	WORD *TreePtr;
	WORD *ElevationPtr;
	WORD *RoadPtr;
	WORD *WaterPtr;

	DDSURFACEDESC2  ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	HRESULT hr;

	ElevationSurface = Engine->Graphics()->CreateSurfaceFromFile("elevation.bmp",800,800,NULL,123456);
	assert(ElevationSurface);

	TreeSurface = Engine->Graphics()->CreateSurfaceFromFile("trees.bmp",800,800,NULL,123456);
	assert(TreeSurface);
	
	ForestSurface = Engine->Graphics()->CreateSurfaceFromFile("Foresttypes.bmp",32,33,NULL,123132);
	assert(ForestSurface);

	RoadSurface = Engine->Graphics()->CreateSurfaceFromFile("roads.bmp",800,800,NULL,0);
	assert(RoadSurface);

	WaterSurface = Engine->Graphics()->CreateSurfaceFromFile("Water.bmp",800,800,NULL,0);
	assert(WaterSurface);

	int Level;

	GetShiftValues(ElevationSurface);

	hr = ElevationSurface->Lock( NULL, &ddsd, DDLOCK_WAIT, NULL );  
 
	if(hr != DD_OK)
	{
		SafeExit("could not lock elevation\n");
	}

	ElevationPtr = (WORD *) ddsd.lpSurface;

	hr = TreeSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		ElevationSurface->Unlock(NULL);
		ElevationSurface->Release();
		TreeSurface->Release();
		ForestSurface->Release();
		SafeExit("could not lock trees\n");
	}

	TreePtr = (WORD *) ddsd.lpSurface;

	hr = ForestSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		TreeSurface->Unlock(NULL);
		ElevationSurface->Unlock(NULL);
		ElevationSurface->Release();
		TreeSurface->Release();
		ForestSurface->Release();
		SafeExit("could not lock tree types\n");
	}

	WORD *ForestPtr;

	ForestPtr = (WORD *) ddsd.lpSurface;
	for(int cn = 0; cn < 8; cn++)
	{
		FColors[cn] = ForestPtr[32*32+cn];
	}

	int xn, yn;

	for(yn = 0; yn < 32; yn++)
	for(xn = 0; xn < 32; xn++)
	{
		ForestTypes[xn][yn] = ForestPtr[xn + yn*32];//ddsd.lPitch];
	}


	hr = RoadSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	
	if(FAILED(hr))
	{
		ForestSurface->Unlock(NULL);
		TreeSurface->Unlock(NULL);
		ElevationSurface->Unlock(NULL);
		ElevationSurface->Release();
		TreeSurface->Release();
		ForestSurface->Release();
		RoadSurface->Release();
		SafeExit("could not lock ROAD surface\n");
	}

	RoadPtr = (WORD *)ddsd.lpSurface;

	hr = WaterSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	
	if(FAILED(hr))
	{
		ForestSurface->Unlock(NULL);
		TreeSurface->Unlock(NULL);
		ElevationSurface->Unlock(NULL);
		ElevationSurface->Release();
		TreeSurface->Release();
		ForestSurface->Release();
		RoadSurface->Release();
		SafeExit("could not lock ROAD surface\n");
	}

	WaterPtr = (WORD *)ddsd.lpSurface;

	
	int MaxDense = 0;
	int MinDense = 255;


	for(yn = 0; yn < 800; yn ++)
	{
		memset(&Road[yn][0],0,1600);	
		memset(&Watered[yn][0],0,1600);	
	}

	int xo, yo;
	float HLevel;
	
//	CreateBaseHeights();

	//scan to get the height scale range;
	for(yn = 0; yn < 800; yn ++)
	for(xn = 0; xn < 800; xn ++)
	{
		yo = yn*2;
		xo = xn*2;

		HLevel = (float)GetLevel(ElevationPtr, xn, yn,800);
		HLevel = HLevel * (HLevel / 2.0f);
		HeightLevels[xo][yo] = HLevel;
		HeightLevels[xo+1][yo] = HLevel;
		HeightLevels[xo][yo+1] = HLevel;
		HeightLevels[xo+1][yo+1] = HLevel;
		
		if(HLevel < (float)Lowest)
		{
			Lowest = (int)HLevel;
		}
		if(HLevel > (float)Highest)
		{
			Highest = (int)HLevel;
		}

		Level = GetLevel(TreePtr,xn,yn,800);

		ForestDensity[xn][yn] = Level = GetLevel(TreePtr,xn,yn,800);

		if(Level < MinDense)
		{
			MinDense = Level;
		}
		if(Level > MaxDense)
		{
			MaxDense = Level;
		}

		Level = GetLevel(RoadPtr,xn,yn,800);

		if(Level < 16)
		{
			BaseRoad[xo-1][yo-1] = 1;			
			BaseRoad[xo][yo-1] = 1;			
			BaseRoad[xo+1][yo-1] = 1;			
			BaseRoad[xo-1][yo] = 1;			
			BaseRoad[xo][yo] = 1;			
			BaseRoad[xo+1][yo] = 1;			
			BaseRoad[xo-1][yo+1] = 1;			
			BaseRoad[xo][yo+1] = 1;			
			BaseRoad[xo+1][yo+1] = 1;			
		}

		Level = GetLevel(WaterPtr,xn,yn,800);
		if(Level < 16)
		{
			Watered[xo-1][yo-1] = 1;			
			Watered[xo][yo-1] = 1;			
			Watered[xo+1][yo-1] = 1;			
			Watered[xo-1][yo] = 1;			
			Watered[xo][yo] = 1;			
			Watered[xo+1][yo] = 1;			
			Watered[xo-1][yo+1] = 1;			
			Watered[xo][yo+1] = 1;			
			Watered[xo+1][yo+1] = 1;			
		
		}

	}

	int RoadTotal;
	int WaterTotal;

	for(yn = 1; yn < 1599; yn ++)
	for(xn = 1; xn < 1599; xn ++)
	{
		RoadTotal = 0;
		if(BaseRoad[xn-1][yn]) RoadTotal++;

		if(BaseRoad[xn][yn+1]) RoadTotal++;
		
		if(BaseRoad[xn][yn-1]) RoadTotal++;
		
		if(BaseRoad[xn+1][yn]) RoadTotal++;
		
		if(BaseRoad[xn][yn] || RoadTotal >= 2)
		{
			Road[xn][yn] = 1;
		}

		WaterTotal = 0;
		if(Watered[xn-1][yn] == 1) WaterTotal++;

		if(Watered[xn][yn+1] == 1) WaterTotal++;
		
		if(Watered[xn][yn-1] == 1) WaterTotal++;
		
		if(Watered[xn+1][yn] == 1) WaterTotal++;
		
		if(WaterTotal >= 2 && !Watered[xn][yn])
		{
			Watered[xn][yn] = 2;
		}
	}

	int yoffset;
	int xoffset;
	float Percent;

	for(yn = 0; yn < 1600; yn ++)
	for(xn = 0; xn < 1600; xn ++)
	{
		Percent = HeightLevels[xn][yn] / (float)Highest;
		HeightLevels[xn][yn] = Percent * HEIGHT_RANGE;
	}
	
	yoffset = 0;
	for(yn = 0; yn < 200; yn++)
	{
		xoffset = 0;
		for(xn = 0; xn < 200; xn++)
		{
			CornerHeights[xn][yn] = StartHeightLevels[xoffset][yoffset] - 3.0f;
			xoffset += CHUNK_WIDTH;
		}
		yoffset += CHUNK_HEIGHT;
	}

	for(yn = 1; yn < 1599; yn ++)
	for(xn = 1; xn < 1599; xn ++)
	{
		if(Watered[xn][yn])
		{
			HeightLevels[xn][yn] -= 24.0f;
		}
	}	

	for(yn = 1; yn < 1599; yn ++)
	for(xn = 1; xn < 1599; xn ++)
	{
		StartHeightLevels[xn][yn] =
			(HeightLevels[xn][yn] + 
			 HeightLevels[xn+1][yn] + 
			 HeightLevels[xn-1][yn] + 
			 HeightLevels[xn][yn+1] + 
			 HeightLevels[xn+1][yn-1] + 
			 HeightLevels[xn-1][yn-1] + 
			 HeightLevels[xn+1][yn+1] + 
			 HeightLevels[xn-1][yn+1] + 
			 HeightLevels[xn][yn-1]) / 9.0f;
	}

	for(yn = 1; yn < 1599; yn ++)
	for(xn = 1; xn < 1599; xn ++)
	{
		HeightLevels[xn][yn] =
			(StartHeightLevels[xn][yn] + 
			 StartHeightLevels[xn+1][yn] + 
			 StartHeightLevels[xn-1][yn] + 
			 StartHeightLevels[xn][yn+1] + 
			 StartHeightLevels[xn+1][yn-1] + 
			 StartHeightLevels[xn-1][yn-1] + 
			 StartHeightLevels[xn+1][yn+1] + 
			 StartHeightLevels[xn-1][yn+1] + 
			 StartHeightLevels[xn][yn-1]) / 9.0f;
	}

	//expand our watered tiles
	for(yn = 1599; yn > 0; yn --)
	for(xn = 1599; xn > 0; xn --)
	{
		if(Watered[xn][yn] && !Watered[xn+1][yn])
		{
			Watered[xn][yn] = 3;
		}
		/*
		if(!Watered[xn][yn] && Watered[xn+1][yn])
		{
			Watered[xn][yn] = 2;
		}
		*/
	}
/*
	for(yn = 1599; yn > 0; yn --)
	for(xn = 1599; xn > 0; xn --)
	{
		
		if(Watered[xn][yn] && !Watered[xn+1][yn])
		{
			Watered[xn][yn] = 3;
		}
		
		if(!Watered[xn][yn] && Watered[xn+1][yn])
		{
			Watered[xn][yn] = 2;
		}
	}
*/
	
	for(yn = 1599; yn > 0; yn --)
	for(xn = 1599; xn > 0; xn --)
	{
		WaterTotal = 0;
		if(Watered[xn-1][yn+1] == 1 ||
			Watered[xn-1][yn+1] == 2) WaterTotal++;
		
		if(Watered[xn][yn+1] == 1 ||
			Watered[xn][yn+1] == 2) WaterTotal++;
			
		if(Watered[xn][yn-1] == 1 ||
			Watered[xn][yn-1] == 2) WaterTotal++;
			
		if(Watered[xn+1][yn] == 1 ||
			Watered[xn+1][yn] == 2) WaterTotal++;
		
		if(WaterTotal && !Watered[xn][yn])
		{
			Watered[xn][yn] = 3;
		}
	}

	ForestSurface->Unlock(NULL);
	ElevationSurface->Unlock(NULL);
	TreeSurface->Unlock(NULL);
	RoadSurface->Unlock(NULL);
	WaterSurface->Unlock(NULL);

	WaterSurface->Release();
	ElevationSurface->Release();
	TreeSurface->Release();
	RoadSurface->Release();
	ForestSurface->Release();


	FillTerrain();

	ZSModelEx *Tree;
	ZSModelEx *Shrub;

	float Height;

	int ChunkX;
	int ChunkY;

	D3DVECTOR Position;
	float Angle;

	int VertOffset;
	Object *Op;

	FILE *fp;

	fp = SafeFileOpen("world.bin","wb");
	long Offset = 0;
	Offset += sizeof(STATIC_FILE_HEADER_T);

	fwrite(&Header,sizeof(Header),1,fp);
	int DI;
	int TerrainLevel;
	float HHighest;
	float HLowest;

	TempChunk.pObjectList = NULL;
	TempChunk.pObjectList2 = NULL;
	TempChunk.NumObjects = 0;
	int LeftOffset;
	int TopOffset;

	LeftOffset = pArea->left; // * CHUNK_WIDTH;
	TopOffset = pArea->top; // * CHUNK_HEIGHT;
	Water *TempWater;
	TempWater = NULL;
	BOOL WaterFound;

	for(ChunkY = (pArea->top /CHUNK_HEIGHT); ChunkY < (pArea->bottom/CHUNK_HEIGHT); ChunkY++)
	{
		for(ChunkX = (pArea->left/CHUNK_WIDTH); ChunkX < (pArea->right/CHUNK_WIDTH); ChunkX++)
		{
			TempChunk.X = ChunkX - (pArea->left / CHUNK_WIDTH);
			TempChunk.Y = ChunkY - (pArea->top / CHUNK_HEIGHT);
			DI = 0;
			yoffset = ChunkY * CHUNK_HEIGHT;
			WaterFound = FALSE;
			for(yn = 0; yn < CHUNK_HEIGHT; yn ++)
			{	
				xoffset = ChunkX * CHUNK_WIDTH;
				for(xn = 0; xn < CHUNK_WIDTH; xn ++)
				{
					if(Watered[xoffset][yoffset])
					{
						WaterFound = TRUE;
					}
					HHighest = 0.0f;
					HLowest = 256.0f;
					//each tile in a chunk must have the height of each corner
					Percent = HLevel/(float)Highest;
					Height = (HeightLevels[xoffset][yoffset]);
					if(Height > HHighest) HHighest = Height;
					if(Height < HLowest) HLowest = Height;

					TerrainLevel = (int)Height;

					VertOffset = (xn + yn * CHUNK_WIDTH)*24;
					
					TempChunk.Verts[VertOffset] = (xoffset - LeftOffset)*2;
					TempChunk.Verts[VertOffset+1] = (yoffset - TopOffset)*2;
					TempChunk.Verts[VertOffset+2] = Height;
					
					Height = (HeightLevels[xoffset+1][yoffset]);
					if(Height > HHighest) HHighest = Height;
					if(Height < HLowest) HLowest = Height;

					TempChunk.Verts[VertOffset+6] = (xoffset+1 - LeftOffset)*2;
					TempChunk.Verts[VertOffset+7] = yoffset - TopOffset;
					TempChunk.Verts[VertOffset+8] = Height;
					
					Height = (HeightLevels[xoffset][yoffset+1]);
					if(Height > HHighest) HHighest = Height;
					if(Height < HLowest) HLowest = Height;

					TempChunk.Verts[VertOffset+12] = (xoffset - LeftOffset)*2;
					TempChunk.Verts[VertOffset+13] = (yoffset+1 - TopOffset)*2;
					TempChunk.Verts[VertOffset+14] = Height;
				
					Height = (HeightLevels[xoffset+1][yoffset+1]);
					if(Height > HHighest) HHighest = Height;
					if(Height < HLowest) HLowest = Height;

					TempChunk.Verts[VertOffset+18] = (xoffset + 1 - LeftOffset)*2;
					TempChunk.Verts[VertOffset+19] = (yoffset + 1 - TopOffset)*2;
					TempChunk.Verts[VertOffset+20] = Height;
				
					Level = MaxDense - (ForestDensity[xoffset/2][yoffset/2]);
					Percent = ((float)Level/(float)MaxDense);
					
					Tree = Forests[GetForestValue(xoffset,yoffset)].GetTree(Percent,xoffset,yoffset);

					if(!Tree)
						Shrub = Forests[GetForestValue(xoffset,yoffset)].GetShrub(Percent,xoffset,yoffset);

					TempChunk.DrawList[DI] = VertOffset;
					TempChunk.DrawList[DI + 1] = VertOffset + 1;
					TempChunk.DrawList[DI + 2] = VertOffset + 2;
					TempChunk.DrawList[DI + 3] = VertOffset + 1;	
					TempChunk.DrawList[DI + 4] = VertOffset + 3;
					TempChunk.DrawList[DI + 5] = VertOffset + 2;
					
					DI += 6;

					float ScaleOffset;
					
					//set up the base terrain
					TerrainLevel = TerrainLevel / TERRAIN_DEPTH;

					if(Road[xoffset][yoffset] || Watered[xoffset][yoffset])
					{
						TempChunk.Terrain[xn][yn] = TerrainIndex[TER_SAND] + (rand()%4)*4;
					}
					else
					{
						TempChunk.Terrain[xn][yn] = Terrain[xoffset][yoffset];				
						int Chance;
						Chance = rand() % 10;
						if(Percent > 0.4f)
						{
							switch(Chance)
							{
								case 7:
									TempChunk.Terrain[xn][yn] = TerrainIndex[TER_GRASS_FOUR] + (rand()%4) * 4;
									break;
								case 1:
								case 2:
								case 3:
								case 4:
								case 5:
								case 6:
									TempChunk.Terrain[xn][yn] = TerrainIndex[TER_DIRT] + (rand()%4) * 4;
									break;
								case 8:
								case 9:
									TempChunk.Terrain[xn][yn] = TerrainIndex[TER_MOSS] + (rand()%4) * 4;
									break;
								default:
									break;
							}
						}
						
						if(Tree)
						{
							ScaleOffset = (float)rand() / (float)RAND_MAX;
							ScaleOffset *= 0.5f;
							Op = new Object;
							Position.x = ((float)xoffset - LeftOffset) * 2.0f + 0.5f;
							Position.y = ((float)yoffset - TopOffset) * 2.0f + 0.5f;
							Position.z = ((HHighest + HLowest) / 2.0f) - .05f;
							Op->SetPosition(&Position);
							Op->SetMesh(Tree);
							Op->SetMeshNum(Engine->GetMeshNum(Tree));
							Op->SetScale(0.75f + ScaleOffset);
							Angle = ((float)rand() /(float)RAND_MAX) * PI_MUL_2;
							Op->SetAngle(Angle);
							Op->SetTexture(Tree->GetTexture());
							Op->SetTextureNum(Engine->GetTextureNum(Tree->GetTexture()));
							Op->SetNext(TempChunk.pObjectList);
							TempChunk.pObjectList = Op;
//							TempChunk.SetBlocking(xn,yn,Tree->GetBlocking());
							switch(Chance)
							{
								case 7:
									TempChunk.Terrain[xn][yn] = TerrainIndex[TER_GRASS_FOUR] + (rand()%4) * 4;
									break;
								case 1:
								case 2:
								case 3:
								case 4:
								case 5:
								case 6:
									TempChunk.Terrain[xn][yn] = TerrainIndex[TER_DIRT] + (rand()%4) * 4;
									break;
								case 8:
								case 9:
									TempChunk.Terrain[xn][yn] = TerrainIndex[TER_MOSS] + (rand()%4) * 4;
									break;
								default:
									break;
							}
						}
						else
						if(Shrub)
						{
							ScaleOffset = (float)rand() / (float)RAND_MAX;
							ScaleOffset *= 0.5;
							Op = new Object;
							float ShrubX;
							float ShrubY;
							ShrubX = (float)rand() / (float)RAND_MAX;
							ShrubY = (float)rand() / (float)RAND_MAX;
							Position.x = ((float)xoffset + ShrubX - LeftOffset) * 2.0f;
							Position.y = ((float)yoffset + ShrubY - TopOffset) * 2.0f;
							Position.z = ((HHighest + HLowest) / 2.0f) - .05f;
							
							Op->SetPosition(&Position);
							Op->SetMesh(Shrub);
							Op->SetMeshNum(Engine->GetMeshNum(Shrub));
							Op->SetScale(0.75f + ScaleOffset);
							Angle = ((float)rand() /(float)RAND_MAX) * PI_MUL_2;
							Op->SetAngle(Angle);
							Op->SetTexture(Shrub->GetTexture());
							Op->SetTextureNum(Engine->GetTextureNum(Shrub->GetTexture()));
							Op->SetNext(TempChunk.pObjectList);

//							TempChunk.SetBlocking(xn, yn, Shrub->GetBlocking());
							TempChunk.pObjectList = Op;
						}
						
						if((HHighest - HLowest) >= 1.1f)
						{
							TempChunk.Terrain[xn][yn] = TerrainIndex[TER_STONE_ONE + rand()%4] + (rand()%4) * 4; 
						}
					}
					xoffset++;
				}
				yoffset++;;
			}
			if(WaterFound)
			{
				TempWater = CreateWater(TempChunk.X, TempChunk.Y);
				TempChunk.AddObject(TempWater);
			}
			Header.ChunkOffsets[TempChunk.X + TempChunk.Y * Header.Width] = ftell(fp);
			TempChunk.Build();
			TempChunk.CreateHeightMap();
			if(WaterFound)
			{
				yoffset = TempChunk.Y * CHUNK_HEIGHT;
				for(yn = 0; yn < CHUNK_HEIGHT; yn ++)
				{	
					xoffset = TempChunk.X * CHUNK_WIDTH;
					for(xn = 0; xn < CHUNK_WIDTH; xn ++)
					{
						if(Watered[xoffset][yoffset] == 1 ||
							Watered[xoffset][yoffset] == 2)
						{
							TempChunk.RemoveTile(xn,yn);
						}
						xoffset++;
					}
					yoffset++;
				}
			}
			TempChunk.Save(fp);
			TempChunk.pObjectList = NULL;
			TempChunk.pObjectList2 = NULL;
			TempChunk.NumObjects = 0;
			if(TempWater)
			{
				delete TempWater;
				TempWater = NULL;
			}
		}
	}

	


	fseek(fp,0,SEEK_SET);

	fwrite(&Header,sizeof(Header),1,fp);

	fclose(fp);

	fp = SafeFileOpen("world.bin","rb");
	StaticFile = fp;

	CleanUpChunks();
//	SmoothBaseTerrain();
	return TRUE;
}

void World::DefaultRegions(RECT *rArea)
{
	//clear current regions
//	ZeroMemory(&RegionHeader,sizeof(RegionHeader));

	LPDIRECTDRAWSURFACE7 BuildingSurface;
	WORD *BuildingPtr;

	BuildingSurface = Engine->Graphics()->CreateSurfaceFromFile("buildings.bmp",1600,1600,NULL,0);
	assert(BuildingSurface);
	
	DDSURFACEDESC2  ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	HRESULT hr;

	hr = BuildingSurface->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		BuildingSurface->Release();
		SafeExit("failed to lock building surface");
	}

	BuildingPtr = (WORD *) ddsd.lpSurface;

	int xn,yn;
	int yoffset = 0;
	for(yn = 0; yn < 1600; yn++)
	{
		for(xn = 0; xn < 1600; xn++)
		{
			if(!BuildingPtr[xn + yoffset]) 
			{
				Locations[xn][yn] = 1;
			}
			else
			{
				Locations[xn][yn] = 0;
			}
		}
		yoffset += 1600;
	}

	BuildingSurface->Unlock(NULL);
	BuildingSurface->Release();
	
	RECT rBuilding;
	int subx, suby;

	int NumAdded = 0;
	for(yn = rArea->top; yn <= rArea->bottom; yn++)
	{
		for(xn = rArea->left; xn <= rArea->right; xn++)
		{
			if(Locations[xn][yn])
			{
				rBuilding.left = rBuilding.right = xn*2;
				rBuilding.top = rBuilding.bottom = yn*2;
				//get the right
				while(Locations[rBuilding.right/2][yn])
				{
					rBuilding.right+= 2;
				}
				//get the bottom
				while(Locations[xn][rBuilding.bottom/2])
				{
					rBuilding.bottom+= 2;
				}
				//clear the locations
				for(suby = rBuilding.top/2; suby <= rBuilding.bottom/2; suby++)
				for(subx = rBuilding.left/2; subx <= rBuilding.right/2; subx++)
				{
					Locations[subx][suby] = 0;
				}
				AddRegion(&rBuilding);
				NumAdded++;
			}
		}
	}

	SaveRegions("temp.rgn");
}

void World::AddRegion(RECT *rArea)
{
	//load all the chunk encompassed by rArea into memory
	int xn,yn;
	Chunk *pChunk;
		
	for(yn = rArea->top/CHUNK_TILE_HEIGHT; yn <= rArea->bottom/CHUNK_TILE_HEIGHT; yn++)
	for(xn = rArea->left/CHUNK_TILE_WIDTH; xn <= rArea->right/CHUNK_TILE_WIDTH; xn++)
	{
		pChunk = Valley->GetChunk(xn,yn);
		if(!pChunk)
		{
			Valley->LoadChunk(xn,yn);
			pChunk = Valley->GetChunk(xn,yn);
			Engine->Graphics()->GetD3D()->BeginScene();
			pChunk->CreateTexture(Valley->GetBaseTexture());
			Engine->Graphics()->GetD3D()->EndScene();
		}

		for(int suby = 0; suby < CHUNK_TILE_HEIGHT; suby++)
		for(int subx = 0; subx < CHUNK_TILE_WIDTH; subx++)
		{
			pChunk->ClearTile(subx,suby);
		}
	}

	float Height;
	int HeightCount;
	Height = 0.0f;
	HeightCount = 0;
	for(yn = rArea->top; yn <= rArea->bottom; yn++)
	for(xn = rArea->left; xn <= rArea->right; xn++)
	{
		Height += Valley->GetZ(xn,yn);
		HeightCount++;
	}
	Height = Height/ (float)HeightCount;
	for(yn = rArea->top; yn <= rArea->bottom; yn++)
	for(xn = rArea->left; xn <= rArea->right; xn++)
	{
		Valley->SetTileHeight(xn,yn,Height);
	}


	Region *pRegion;

	EditRegion *pER;
	
	pER = new EditRegion;
	
	EditPoint *pA, *pB, *pC, *pD;

	D3DVECTOR vA;

	float zVal;
	
	vA.x = (float)rArea->left + 0.5f;
	vA.y = (float)rArea->top + 0.5f;
	
	zVal = vA.z = Valley->GetZ(vA.x,vA.y);

	pA = new EditPoint(&vA);

	vA.x = (float)rArea->right + 0.5f;
	vA.y = (float)rArea->top + 0.5f;

	pB = new EditPoint(&vA);

	vA.x = (float)rArea->right + 0.5f;
	vA.y = (float)rArea->bottom + 0.5f;

	pC = new EditPoint(&vA);

	vA.x = (float)rArea->left + 0.5f;
	vA.y = (float)rArea->bottom + 0.5f;

	pD = new EditPoint(&vA);

	pER->AddPoint(pA);
	pER->AddPoint(pB);
	pER->AddPoint(pC);
	pER->AddPoint(pD);

	EditWall *pEWall;

	pEWall = new EditWall(pA,pB);

	pER->AddWall(pEWall);

	pEWall = new EditWall(pB,pC);

	pER->AddWall(pEWall);

	pEWall = new EditWall(pC,pD);

	pER->AddWall(pEWall);

	pEWall = new EditWall(pD,pA);

	pER->AddWall(pEWall);

	pER->CreateInterior();

	pRegion = pER->CreateGameRegion(NULL,NULL,Engine->GetTexture("floor"),NULL);
	pRegion->SetOccupancy(REGION_OCCUPIED);
	pRegion->SetID(0);

	AddRegion(pRegion);

	delete pER;
}

WallSegmentList *FindWall(BYTE *WallAreas,BYTE *Visited,int xfrom, int yfrom, int XStart,int YStart,int XEnd,int YEnd, int TileWidth);

void Area::BuildCavern(char *filename, int TileWidth, int TileHeight)
{

	SetCurrentDirectory(".\\Areas");

	sprintf(this->Header.Name,"%s",filename);
	
	this->Width = this->Header.Width = TileWidth;
	this->Height = this->Header.Height = TileHeight;

	this->ChunkWidth = this->Header.ChunkWidth = TileWidth / CHUNK_TILE_WIDTH;
	this->ChunkHeight = this->Header.ChunkHeight = TileHeight / CHUNK_TILE_HEIGHT;

	this->UpdateWidth = TileWidth / WORLD_USEG_WIDTH;
	this->UpdateHeight = TileHeight / WORLD_USEG_HEIGHT;

	if(this->Header.ChunkOffsets)
		delete[] this->Header.ChunkOffsets;

	this->Header.ChunkOffsets = new int[this->ChunkWidth * this->ChunkHeight];
	ZeroMemory(this->Header.ChunkOffsets,sizeof(int) * this->ChunkWidth * this->ChunkHeight);

	if(this->UpdateSegments)
		delete[] this->UpdateSegments;
	
	this->UpdateSegments = new Object *[this->UpdateWidth * this->UpdateHeight];
	ZeroMemory(this->UpdateSegments, sizeof(Object *) * this->UpdateWidth * this->UpdateHeight);

	if(this->Regions)
		delete[] this->Regions;

	this->Regions = new unsigned short[this->ChunkWidth * this->ChunkHeight * NUM_CHUNK_REGIONS];
	ZeroMemory(this->Regions, sizeof(unsigned short) * this->ChunkWidth * this->ChunkHeight * NUM_CHUNK_REGIONS);

	if(this->BigMap)
		delete[] this->BigMap;
	this->BigMap = new Chunk *[this->ChunkWidth * this->ChunkHeight];
	ZeroMemory(this->BigMap,sizeof(Chunk *) * this->ChunkWidth * this->ChunkHeight);

	float *Elevations;
	float *BaseElevations;
	BYTE *WallAreas;
	BYTE *Visited;
	
	Elevations = new float[TileWidth * TileHeight];
	BaseElevations = new float[TileWidth * TileHeight];
	WallAreas = new BYTE[TileWidth * TileHeight];
	Visited = new BYTE[TileWidth * TileHeight];
	ZeroMemory(Visited,sizeof(BYTE) * TileWidth * TileHeight);


	LPDIRECTDRAWSURFACE7 lpddHeights;
	LPDIRECTDRAWSURFACE7 lpddWalls;
	char WallFileName[64];
	char HeightFileName[64];
	sprintf(WallFileName,   "%swalls.bmp", filename);
	sprintf(HeightFileName, "%selevations.bmp", filename);

	lpddHeights = Engine->Graphics()->CreateSurfaceFromFile(HeightFileName,TileWidth,TileHeight,NULL,COLOR_KEY_FROM_FILE);
	lpddWalls = Engine->Graphics()->CreateSurfaceFromFile(WallFileName,TileWidth,TileHeight,NULL,COLOR_KEY_FROM_FILE);
	
	WORD *pHeights;
	WORD *pWalls;

	DDSURFACEDESC2  ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	HRESULT hr;

	GetShiftValues(lpddHeights);

	hr = lpddHeights->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		lpddHeights->Unlock(NULL);
		lpddHeights->Release();
		lpddWalls->Release();
		SafeExit("could not lock heights\n");
	}

	pHeights = (WORD *) ddsd.lpSurface;

	hr = lpddWalls->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		lpddHeights->Unlock(NULL);
		lpddHeights->Release();
		lpddWalls->Unlock(NULL);
		lpddWalls->Release();
		SafeExit("could not lock walls\n");
	}

	pWalls = (WORD *) ddsd.lpSurface;
	
	int xn;
	int yn;
	int Height = 0;
	int MaxHeight = 0;
	int MinHeight = 9999;
	for(yn = 0; yn < TileHeight; yn+=2)
	for(xn = 0; xn < TileHeight; xn+=2)
	{
		//get the heightvalue
		Height = GetLevel(pHeights,xn,yn,TileWidth);
		if(Height > MaxHeight) MaxHeight = Height;
		if(Height < MinHeight) MinHeight = Height;
		BaseElevations[xn + yn * TileWidth] = (float)Height;
		BaseElevations[xn + (yn+1) * TileWidth] = (float)Height;
		BaseElevations[xn+1 + yn * TileWidth] = (float)Height;
		BaseElevations[xn+1 + (yn+1) * TileWidth] = (float)Height;
				
		if(GetLevel(pWalls, xn, yn, TileWidth) > 8)
		{
			WallAreas[xn + yn * TileWidth] = 1;
			WallAreas[xn + (yn + 1) * TileWidth] = 1;
			WallAreas[xn+1 + yn * TileWidth] = 1;
			WallAreas[xn+1 + (yn + 1) * TileWidth] = 1;
		}
		else
		{
			WallAreas[xn + yn * TileWidth] = 0;
			WallAreas[xn + (yn + 1) * TileWidth] = 0;
			WallAreas[xn+1 + yn * TileWidth] = 0;
			WallAreas[xn+1 + (yn + 1) * TileWidth] = 0;
		}
	}

	lpddHeights->Unlock(NULL);
	lpddWalls->Unlock(NULL);

	lpddHeights->Release();
	lpddWalls->Release();

	float HeightRange;
	HeightRange = MaxHeight - MinHeight;
		
	for(yn = 0; yn < TileHeight; yn++)
	for(xn = 0; xn < TileHeight; xn++)
	{
		BaseElevations[xn + yn * TileWidth] -= (float)MinHeight;
		BaseElevations[xn + yn * TileWidth] /= HeightRange;
		BaseElevations[xn + yn * TileWidth] *= CAVE_HEIGHT_RANGE;
		Elevations[xn + yn * TileWidth] = BaseElevations[xn + yn * TileWidth];
	}
	int numborders;

	for(yn = 1; yn < (TileHeight - 1); yn++)
	for(xn = 1; xn < (TileHeight - 1); xn++)
	{
		Elevations[xn + yn*TileWidth] =
			(BaseElevations[xn + yn*TileWidth] +
			 BaseElevations[xn-1 + (yn - 1)*TileWidth] +
			 BaseElevations[xn+1 + (yn - 1)*TileWidth] +
			 BaseElevations[xn-1 + (yn + 1)*TileWidth] +
			 BaseElevations[xn+1 + (yn + 1)*TileWidth] +
			 BaseElevations[xn+1 + yn*TileWidth] +
			 BaseElevations[xn-1 + yn*TileWidth] +
			 BaseElevations[xn + (yn-1)*TileWidth]+
			 BaseElevations[xn + (yn+1)*TileWidth]) / 9.0f;
	}

	for(yn = 2; yn < (TileHeight - 2); yn+=2)
	for(xn = 2; xn < (TileHeight - 2); xn+=2)
	{
		if(WallAreas[xn + yn * TileWidth])
		{
			numborders = 0;
			if(!WallAreas[xn + (yn - 2) * TileWidth])
				numborders++;
			if(!WallAreas[xn + (yn + 2) * TileWidth])
				numborders++;
			if(!WallAreas[xn - 2 + yn * TileWidth])
				numborders++;
			if(!WallAreas[xn + 2 + yn * TileWidth])
				numborders++;
			if(numborders >=3)
			{
				DEBUG_INFO("found bad bordered tile\n");
				WallAreas[xn + yn * TileWidth] = 0;
				WallAreas[xn + (yn + 1) * TileWidth] = 0;
				WallAreas[xn+1 + yn * TileWidth] = 0;
				WallAreas[xn+1 + (yn + 1) * TileWidth] = 0;
				xn = 2;
				yn = 2;
			}
		}
	}


	delete[] BaseElevations;

	//use only stone
	int ChunkX;
	int ChunkY;
	int XStart;
	int XEnd;
	int YStart;
	int YEnd;

	Chunk TempChunk;

	FILE *fp;
	char AreaFileName[64];
	sprintf(AreaFileName,"%s.bin",this->Header.Name);
	fp = SafeFileOpen(AreaFileName,"wb");

	this->SaveHeader(fp);
	int VertOffset;
	int DI = 0;
	float HHighest;
	float HLowest;
	float fHeight1, fHeight2, fHeight3, fHeight4;
	
	for(ChunkY = 0; ChunkY < this->ChunkHeight; ChunkY++)
	for(ChunkX = 0; ChunkX < this->ChunkWidth; ChunkX++)
	{
		TempChunk.X = ChunkX;
		TempChunk.Y = ChunkY;
		this->Header.ChunkOffsets[ChunkX + ChunkY * this->ChunkWidth] = ftell(fp);

		XStart = ChunkX * CHUNK_TILE_WIDTH;
		XEnd = XStart + CHUNK_TILE_WIDTH;
		YStart = ChunkY * CHUNK_TILE_HEIGHT;
		YEnd = YStart + CHUNK_TILE_HEIGHT;
		
		DI = 0;

		for(yn = YStart; yn < YEnd; yn+=2)
		for(xn = XStart; xn < XEnd; xn+=2)
		{		
			HHighest = 0.0f;
			HLowest = 256.0f;
			//each tile in a chunk must have the height of each corner
			fHeight1 = Elevations[xn + yn*TileWidth];
			fHeight2 = Elevations[xn+2 + yn*TileWidth];
			fHeight3 = Elevations[xn + (yn+2)*TileWidth];
			fHeight4 = Elevations[xn+2 + (yn+2)*TileWidth];
			if(fHeight1 > HHighest) HHighest = Height;
			if(fHeight1 < HLowest) HLowest = Height;

			VertOffset = (((xn%16)/2) + ((yn%16)/2) * CHUNK_WIDTH)*24;
			
			TempChunk.Verts[VertOffset] = xn;
			TempChunk.Verts[VertOffset+1] = yn;
			TempChunk.Verts[VertOffset+2] = fHeight1;
		
			TempChunk.Verts[VertOffset+6] = xn+2;
			TempChunk.Verts[VertOffset+7] = yn;
			TempChunk.Verts[VertOffset+8] = fHeight2;
			
			TempChunk.Verts[VertOffset+12] = xn;
			TempChunk.Verts[VertOffset+13] = yn +2;
			TempChunk.Verts[VertOffset+14] = fHeight3;
		
			TempChunk.Verts[VertOffset+18] = xn + 2;
			TempChunk.Verts[VertOffset+19] = yn + 2;
			TempChunk.Verts[VertOffset+20] = fHeight4;

			TempChunk.DrawList[DI] = VertOffset;
			TempChunk.DrawList[DI + 1] = VertOffset + 1;
			TempChunk.DrawList[DI + 2] = VertOffset + 2;
			TempChunk.DrawList[DI + 3] = VertOffset + 1;	
			TempChunk.DrawList[DI + 4] = VertOffset + 3;
			TempChunk.DrawList[DI + 5] = VertOffset + 2;
					
			DI += 6;

			TempChunk.SetTerrain((xn % 16)/2, (yn % 16)/2, TerrainIndex[TER_STONE_ONE + rand() %4]); 
		}

		TempChunk.Build();
		TempChunk.CreateHeightMap();
		for(yn = YStart; yn < YEnd; yn += 2)
		for(xn = XStart; xn < XEnd; xn += 2)
		{
			if(!WallAreas[xn + yn * TileWidth])
			{
				TempChunk.RemoveTile((xn %16)/2, (yn%16)/2);
			}
		}

		

		TempChunk.Save(fp);
		while(TempChunk.GetObjects())
		{
			Object *pOb;
			pOb = TempChunk.GetObjects();
			TempChunk.RemoveObject(pOb);
			delete pOb;
		}
		ZeroMemory(&TempChunk,sizeof(Chunk));
	}

	fseek(fp,0, SEEK_SET);

	SaveHeader(fp);

	fclose(fp);

	StaticFile = SafeFileOpen(AreaFileName,"rb");

	SetCurrentDirectory(Engine->GetRootDirectory());

	SaveRegions();

	SetCurrentDirectory(".\\Areas");

	sprintf(AreaFileName,"%s.dyn",this->Header.Name);

	fp = SafeFileOpen(AreaFileName,"wb");

	SaveNonStatic(fp);

	fclose(fp);

	SetCurrentDirectory(Engine->GetRootDirectory());

	delete[] Elevations;
	delete[] WallAreas;
	delete[] Visited;
	


	return;
}

DIRECTION_T GetWallTileAngle(BYTE *WallAreas, int xn, int yn, int TileWidth);

WallSegmentList *FindWall(BYTE *WallAreas,BYTE *Visited,int xfrom, int yfrom, int XStart,int YStart,int XEnd,int YEnd, int TileWidth)
{
	BYTE North;
	BYTE South;
	BYTE East;
	BYTE West;
	BYTE NumBorders;
	//examine our starting point.
	//looknorth
	North = WallAreas[xfrom + (yfrom - 2)*TileWidth];
	//looksouth
	South = WallAreas[xfrom + (yfrom + 2)*TileWidth];
	//lookeast
	East = WallAreas[xfrom + 2 + yfrom*TileWidth];
	//lookwest
	West = WallAreas[xfrom - 2 + yfrom*TileWidth];

	NumBorders = North + South + East + West;

	Visited[xfrom + yfrom * TileWidth] = TRUE;
	//check for a valid startpoint
	if(NumBorders == 0 || NumBorders == 4 || (!North && !South) || (!East && !West))
	{
		return NULL;
	}

	//else we've found a valid start point

	int xn;
	int yn;
	xn = xfrom;
	yn = yfrom;

	int xto;
	int yto;
	int xlast;
	int ylast;

	WallSegmentList *pSL;
	WALL_SEGMENT_T WSeg;
	WALL_SEGMENT_T BSeg; // link between squares;
	WallSegmentList *pCur;
	pSL = new WallSegmentList();
	pSL->pStart = pSL;

	pCur = pSL;
	
	DIRECTION_T Next;
	DIRECTION_T CurAngle;

	WallSegmentList *pLast = NULL;

	while(TRUE)
	{
	Visited[xn + yn * TileWidth] = TRUE;

	CurAngle = GetWallTileAngle(WallAreas,xn,yn,TileWidth);

	WSeg.ThisAngle = CurAngle;

	
	switch(CurAngle)
	{
	case NORTH:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn - 2;
		yto = yn - 2;
		Next = NORTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto += 2;
			Next = NORTH;
		}
			xlast = xn - 2;
			ylast = yn + 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = NORTH;
			}
			else
			{
				WSeg.FromAngle = NORTHEAST;
			}
		break;
	case SOUTH:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn + 2;
		yto = yn + 2;
		Next = SOUTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto -= 2;
			Next = SOUTH;
		}
			xlast = xn + 2;
			ylast = yn - 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = SOUTH;
			}
			else
			{
				WSeg.FromAngle = SOUTHWEST;
			}
		break;
	case EAST:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn + 2;
		yto = yn - 2;
		Next = NORTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto += 2;
			Next = EAST;
		}
		xlast = xn - 2;
		ylast = yn - 2;
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = EAST;
		}
		else
		{
			WSeg.FromAngle = SOUTHEAST;
		}
		break;
	case WEST:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn - 2;
		yto = yn + 2;
		Next = SOUTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto -= 2;
			Next = WEST;
		}
		xlast = xn + 2;
		ylast = yn + 2;
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = WEST;
		}
		else
		{
			WSeg.FromAngle = NORTHWEST;
		}
		break;
	case NORTHEAST:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn + 2;
		yto = yn - 2;
		Next = NORTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto += 2;
			Next = EAST;
		}
			xlast = xn - 2;
			ylast = yn + 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = NORTH;
			}
			else
			{
				WSeg.FromAngle = NORTHEAST;
			}
		break;
	case NORTHWEST:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn - 2;
		yto = yn - 2;
		Next = NORTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto += 2;
			Next = NORTH;
		}
		xlast = xn + 2;
		ylast = yn + 2;
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = WEST;
		}
		else
		{
			WSeg.FromAngle = NORTHWEST;
		}
		break;
	case SOUTHEAST:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn + 2;
		yto = yn + 2;
		Next = SOUTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto -= 2;
			Next = SOUTH;
		}

		xlast = xn - 2;
		ylast = yn - 2;
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = EAST;
		}
		else
		{
			WSeg.FromAngle = SOUTHEAST;
		}
		break;
	case SOUTHWEST:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn - 2;
		yto = yn + 2;
		Next = SOUTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto -= 2;
			Next = WEST;
		}

		xlast = xn + 2;
		ylast = yn - 2;
		
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = SOUTH;
		}
		else
		{
			WSeg.FromAngle = SOUTHWEST;
		}
		break;
	default:
		SafeExit("Bad something in create walls!\n");
		break;
	}

	if(pLast)
	{
		if(WSeg.FromAngle != pLast->Seg.ThisAngle ||
			pLast->Seg.ToAngle != WSeg.ThisAngle)
		{
			DEBUG_INFO("Wseg from not match pLast->To\n");
		}

		WSeg.FromAngle = pLast->Seg.ThisAngle;
		pLast->Seg.ToAngle = CurAngle;
	}
	
	BSeg.vEnd = BSeg.vStart;
		
	switch(Next)
	{
	case NORTH:
		BSeg.vEnd.y -= 1.0f;
		break;
	case SOUTH:
		BSeg.vEnd.y += 1.0f;
		break;
	case EAST:
		BSeg.vEnd.x += 1.0f;
		break;
	case WEST:
		BSeg.vEnd.x -= 1.0f;
		break;
	case NORTHEAST:
		BSeg.vEnd.x += 1.0f;
		BSeg.vEnd.y -= 1.0f;
		break;
	case NORTHWEST:
		BSeg.vEnd.x -= 1.0f;
		BSeg.vEnd.y -= 1.0f;
		break;
	case SOUTHEAST:
		BSeg.vEnd.x += 1.0f;
		BSeg.vEnd.y += 1.0f;
		break;
	case SOUTHWEST:
		BSeg.vEnd.x -= 1.0f;
		BSeg.vEnd.y += 1.0f;
		break;
	}

	pCur->Seg = WSeg;
	pCur->Seg.ToAngle = Next;
	pCur->Seg.ThisAngle = CurAngle;
	
	pCur->pNext = new WallSegmentList;

	pCur = pCur->pNext;
	
	pCur->Seg = BSeg;
	pCur->Seg.FromAngle = CurAngle;
	pCur->Seg.ThisAngle = Next;
	pCur->Seg.ToAngle = GetWallTileAngle(WallAreas,xto,yto,TileWidth);

	pLast = pCur;


	if(Visited[xto + yto*TileWidth] ||
		xto >= XEnd || yto >= YEnd || xto < XStart || yto < YStart)
	{
		//we've reached a visitted square or a square which is outside the current chunk
		break;
	}
	
	pCur->pNext = new WallSegmentList;

	pCur = pCur->pNext;

	pCur->Seg.FromAngle = Next;

	xn = xto;
	yn = yto;

	}

	pLast = NULL;

	//now we have to trace the reverse order;
	WallSegmentList *pNew;

	pCur = pSL->pStart;

	xn = xfrom;
	yn = yfrom;

while(TRUE)
{
	xto = xn;
	yto = yn;
	switch(pCur->Seg.FromAngle)
	{
	case NORTH:
		yto += 2;
		break;
	case SOUTH:
		yto -= 2;
		break;
	case EAST:
		xto -= 2;
		break;
	case WEST:
		xto += 2;
		break;
	case NORTHEAST:
		xto -= 2;
		yto += 2;
		break;
	case SOUTHEAST:
		xto -= 2;
		yto -= 2;
		break;
	case SOUTHWEST:
		xto += 2;
		yto -= 2;
		break;
	case NORTHWEST:
		xto += 2;
		yto += 2;
		break;
	}
	if(Visited[xto + yto*TileWidth] ||
	xto >= XEnd || yto >= YEnd || xto < XStart || yto < YStart)
	{
		//we've reached a visitted square or a square which is outside the current chunk
		break;
	}

	xn = xto;
	yn = yto;
	
	Visited[xn + yn * TileWidth] = 1;

	CurAngle = GetWallTileAngle(WallAreas,xn,yn,TileWidth);

		switch(CurAngle)
	{
	case NORTH:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn - 2;
		yto = yn - 2;
		Next = NORTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto += 2;
			Next = NORTH;
		}
			xlast = xn - 2;
			ylast = yn + 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = NORTH;
			}
			else
			{
				WSeg.FromAngle = NORTHEAST;
			}
		break;
	case SOUTH:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn + 2;
		yto = yn + 2;
		Next = SOUTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto -= 2;
			Next = SOUTH;
		}
			xlast = xn + 2;
			ylast = yn - 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = SOUTH;
			}
			else
			{
				WSeg.FromAngle = SOUTHWEST;
			}
		break;
	case EAST:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn + 2;
		yto = yn - 2;
		Next = NORTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto += 2;
			Next = EAST;
		}
			xlast = xn - 2;
			ylast = yn - 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = EAST;
			}
			else
			{
				WSeg.FromAngle = SOUTHEAST;
			}
		
		break;
	case WEST:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn - 2;
		yto = yn + 2;
		Next = SOUTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto -= 2;
			Next = WEST;
		}
		xlast = xn + 2;
		ylast = yn + 2;
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = WEST;
		}
		else
		{
			WSeg.FromAngle = NORTHWEST;
		}

		break;
	case NORTHEAST:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn + 2;
		yto = yn - 2;
		Next = NORTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto += 2;
			Next = EAST;
		}
			xlast = xn - 2;
			ylast = yn + 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = NORTH;
			}
			else
			{
				WSeg.FromAngle = NORTHEAST;
			}
		break;
	case NORTHWEST:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 1.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 0.5;
		xto = xn - 2;
		yto = yn - 2;
		Next = NORTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto += 2;
			Next = NORTH;
		}
		xlast = xn + 2;
		ylast = yn + 2;
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = WEST;
		}
		else
		{
			WSeg.FromAngle = NORTHWEST;
		}
		break;
	case SOUTHEAST:
		WSeg.vStart.x = xn + 0.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 1.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn + 2;
		yto = yn + 2;
		Next = SOUTHEAST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			xto -= 2;
			Next = SOUTH;
		}
			xlast = xn - 2;
			ylast = yn - 2;
			if(!WallAreas[xlast + ylast * TileWidth])
			{
				WSeg.FromAngle = EAST;
			}
			else
			{
				WSeg.FromAngle = SOUTHEAST;
			}
		break;
	case SOUTHWEST:
		WSeg.vStart.x = xn + 1.5;
		WSeg.vStart.y = yn + 0.5;
		BSeg.vStart.x = WSeg.vEnd.x = xn + 0.5;
		BSeg.vStart.y = WSeg.vEnd.y = yn + 1.5;
		xto = xn - 2;
		yto = yn + 2;
		Next = SOUTHWEST;
		if(!WallAreas[xto + yto * TileWidth])
		{
			yto -= 2;
			Next = WEST;
		}

		xlast = xn + 2;
		ylast = yn - 2;
		
		if(!WallAreas[xlast + ylast * TileWidth])
		{
			WSeg.FromAngle = SOUTH;
		}
		else
		{
			WSeg.FromAngle = SOUTHWEST;
		}
		
		break;
	default:
		SafeExit("Bad something in create walls!\n");
		break;
	}

	
	
	BSeg.vEnd = BSeg.vStart;
		
	switch(Next)
	{
	case NORTH:
		BSeg.vEnd.y -= 1.0f;
		break;
	case SOUTH:
		BSeg.vEnd.y += 1.0f;
		break;
	case EAST:
		BSeg.vEnd.x += 1.0f;
		break;
	case WEST:
		BSeg.vEnd.x -= 1.0f;
		break;
	case NORTHEAST:
		BSeg.vEnd.x += 1.0f;
		BSeg.vEnd.y -= 1.0f;
		break;
	case NORTHWEST:
		BSeg.vEnd.x -= 1.0f;
		BSeg.vEnd.y -= 1.0f;
		break;
	case SOUTHEAST:
		BSeg.vEnd.x += 1.0f;
		BSeg.vEnd.y += 1.0f;
		break;
	case SOUTHWEST:
		BSeg.vEnd.x -= 1.0f;
		BSeg.vEnd.y += 1.0f;
		break;
	}

	pNew = new WallSegmentList;
	pNew->Seg = WSeg;
	pNew->Seg.ThisAngle = CurAngle;
	pNew->Seg.ToAngle = Next;

	pNew->pNext = new WallSegmentList;
	pNew->pNext->Seg = BSeg;
	pNew->pNext->Seg.ToAngle = pCur->Seg.ThisAngle;
	pNew->pNext->Seg.ThisAngle = Next;
	pNew->pNext->Seg.FromAngle = CurAngle;

	pNew->pNext->pNext = pCur;
	pNew->pStart = pNew;
	
	pCur = pNew;

	}

	return pSL;
	
	return NULL;
}

DIRECTION_T GetWallTileAngle(BYTE *WallAreas, int xn, int yn, int TileWidth)
{
	BYTE North;
	BYTE South;
	BYTE East;
	BYTE West;
	BYTE NumBorders;
	
	DIRECTION_T CurAngle;

	//looknorth
	North = WallAreas[xn + (yn - 2)*TileWidth];

	//looksouth
	South = WallAreas[xn + (yn + 2)*TileWidth];
	//lookeast
	East = WallAreas[xn + 2 + yn*TileWidth];
	//lookwest
	West = WallAreas[xn - 2 + yn*TileWidth];

	NumBorders = 0;

	if(!North)
	{
		NumBorders++;
	}
	if(!South)
	{
		NumBorders++;
	}
	if(!East)
	{
		NumBorders++;
	}
	if(!West)
	{
		NumBorders++;
	}
	
	if(!North)
	{	
		if(NumBorders > 1)
		{
			if(!West)
			{
				CurAngle = NORTHEAST;
			}
			else
			if(!East)
			{
				CurAngle = SOUTHEAST;
			}
			else
			{
				DEBUG_INFO("Tripple...\n");
				CurAngle = EAST;
			}
		}
		else
		{
			CurAngle = EAST;
		}
	}
	//south
	else
	if(!South)
	{
		if(NumBorders > 1)
		{
			if(!West)
			{
				CurAngle = NORTHWEST;
			}
			else
			if(!East)
			{
				CurAngle = SOUTHWEST;
			}
			else
			{
				DEBUG_INFO("Tripple...\n");
				CurAngle = WEST;
			}
		}
		else
		{
			CurAngle = WEST;
		}
	}
	else
	if(!West)
	{
		CurAngle = NORTH;
	}
	else
	if(!East)
	{
		CurAngle = SOUTH;
	}
	else
	{
		DEBUG_INFO("Bad something in GetWallTileAngle walls!\n");
		return NORTH;
		//	exit(1);
	}
	
	return CurAngle;
}

void Area::AddCaveWalls(char *Texture, int WallHeight, int Type)
{
	SetCurrentDirectory(".\\Areas");
	BYTE *WallAreas;
	BYTE *Visited;
	
	WallAreas = new BYTE[this->Width * this->Height];
	Visited = new BYTE[this->Width * this->Height];
	ZeroMemory(Visited,sizeof(BYTE) * this->Width * this->Height);


	LPDIRECTDRAWSURFACE7 lpddWalls;
	char WallFileName[64];
	sprintf(WallFileName, "%swalls.bmp", Header.Name);

	lpddWalls = Engine->Graphics()->CreateSurfaceFromFile(WallFileName,this->Width,this->Height,NULL,COLOR_KEY_FROM_FILE);
	
//	WORD *pHeights;
	WORD *pWalls;

	DDSURFACEDESC2  ddsd;
	memset(&ddsd,0,sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd);

	HRESULT hr;

	GetShiftValues(lpddWalls);
	
	hr = lpddWalls->Lock(NULL, &ddsd, DDLOCK_WAIT, NULL);
	if(hr != DD_OK)
	{
		lpddWalls->Unlock(NULL);
		lpddWalls->Release();
		SafeExit("could not lock walls\n");
	}

	pWalls = (WORD *) ddsd.lpSurface;
	
	int xn;
	int yn;
	
	for(yn = 0; yn < this->Height; yn+=2)
	for(xn = 0; xn < this->Height; xn+=2)
	{
		if(GetLevel(pWalls, xn, yn, this->Width) > 8)
		{
			WallAreas[xn + yn * this->Width] = 1;
			WallAreas[xn + (yn + 1) * this->Width] = 1;
			WallAreas[xn+1 + yn * this->Width] = 1;
			WallAreas[xn+1 + (yn+1) * this->Width] = 1;
		}
		else
		{
			WallAreas[xn + yn * this->Width] = 0;
			WallAreas[xn + (yn + 1) * this->Width] = 0;
			WallAreas[xn+1 + yn * this->Width] = 0;
			WallAreas[xn+1 + (yn + 1) * this->Width] = 0;
		}
	}

	lpddWalls->Unlock(NULL);

	lpddWalls->Release();

	int numborders;

	for(yn = 2; yn < (this->Height - 2); yn+=2)
	for(xn = 2; xn < (this->Height - 2); xn+=2)
	{
		if(WallAreas[xn + yn * this->Width])
		{
			numborders = 0;
			if(!WallAreas[xn + (yn - 2) * this->Width])
				numborders++;
			if(!WallAreas[xn + (yn + 2) * this->Width])
				numborders++;
			if(!WallAreas[xn - 2 + yn * this->Width])
				numborders++;
			if(!WallAreas[xn + 2 + yn * this->Width])
				numborders++;
			if(numborders >=3)
			{
				DEBUG_INFO("found bad bordered tile\n");
				WallAreas[xn + yn * this->Width] = 0;
				WallAreas[xn + (yn + 1) * this->Width] = 0;
				WallAreas[xn+1 + yn * this->Width] = 0;
				WallAreas[xn+1 + (yn + 1) * this->Width] = 0;
				xn = 2;
				yn = 2;
			}
		}
	}
	
	//use only stone
	int ChunkX;
	int ChunkY;
	int XStart;
	int XEnd;
	int YStart;
	int YEnd;

	Chunk *pChunk;

	FILE *fp;
	char AreaFileName[64];
	sprintf(AreaFileName,"%s.bin",this->Header.Name);
	fp = SafeFileOpen("temp.bin","wb");

	this->SaveHeader(fp);
//	int VertOffset;
	int DI = 0;
//	float HHighest;
//	float HLowest;

	int *NewOffsets;
	NewOffsets = new int[ChunkWidth * ChunkHeight];
	ZeroMemory(NewOffsets,sizeof(int)*ChunkWidth*ChunkHeight);

	for(ChunkY = 0; ChunkY < this->ChunkHeight; ChunkY++)
	for(ChunkX = 0; ChunkX < this->ChunkWidth; ChunkX++)
	{
		NewOffsets[ChunkX + ChunkY * ChunkWidth] = ftell(fp);

		pChunk = GetChunk(ChunkX,ChunkY);
		if(!pChunk)
		{
			LoadChunk(ChunkX,ChunkY);
			pChunk = GetChunk(ChunkX,ChunkY);
		}

		Object *pOb;
		pOb = pChunk->GetObjects();
		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_BARRIER)
			{
				pChunk->RemoveObject(pOb);
				pOb = pChunk->GetObjects();
			}
			else
			{
				pOb = pOb->GetNext();
			}
		}
		
		//now add walls
		WallSegmentList *pSeg;

		XStart = ChunkX * CHUNK_TILE_WIDTH;
		XEnd = XStart + CHUNK_TILE_WIDTH;
		YStart = ChunkY * CHUNK_TILE_HEIGHT;
		YEnd = YStart + CHUNK_TILE_HEIGHT;
		
		DI = 0;

	//	if(ChunkX <= 2 && ChunkY <= 2)
		for(yn = YStart; yn < YEnd; yn += 2)
		for(xn = XStart; xn < XEnd; xn += 2)
		{
			if(!WallAreas[xn + yn * this->Width])
			{
				Visited[xn + yn * this->Width] = 1;
			}
			else
			{
				if(!Visited[xn + yn * this->Width])
				{
					pSeg = FindWall(WallAreas,Visited,xn,yn,XStart,YStart,XEnd,YEnd, this->Width);
					if(pSeg)
					{
						//setzvalues
						WallSegmentList *pCur;
						pCur = pSeg->pStart;
						while(pCur)
						{
							pCur->Seg.vStart.z = GetZ(pCur->Seg.vStart.x, pCur->Seg.vStart.y) - 0.4f;

							pCur->Seg.vEnd.z = GetZ(pCur->Seg.vEnd.x, pCur->Seg.vEnd.y) - 0.4f;

							pCur = pCur->pNext;
						}

						CaveWall *pCaveWall;
						pCaveWall = new CaveWall;
						pCaveWall->SetType(Type);
						pCaveWall->SetHeight((float)WallHeight);
						pCaveWall->Build(pSeg);
						pCaveWall->SetTexture(Engine->GetTexture(Texture));
						pCaveWall->Block();
						pChunk->AddObject(pCaveWall);	
	
						delete pSeg;
					}
					//construct our list...
				}
			}
		}

		pChunk->Save(fp);

		while(pChunk->GetObjects())
		{
			Object *pOb;
			pOb = pChunk->GetObjects();
			pChunk->RemoveObject(pOb);
			delete pOb;
		}

	}

	fseek(fp,0, SEEK_SET);

	delete[] Header.ChunkOffsets;

	Header.ChunkOffsets = NewOffsets;

	SaveHeader(fp);

	fclose(fp);

	fclose(StaticFile);
	
	remove(AreaFileName);

	rename("temp.bin",AreaFileName);

	StaticFile = SafeFileOpen(AreaFileName,"rb");

	delete[] WallAreas;
	delete[] Visited;

	int offset;
	for(yn = 0; yn < ChunkHeight; yn++)
	for(xn = 0; xn < ChunkWidth; xn++)
	{
		offset = xn + yn * ChunkWidth;
		if(BigMap[offset])
		{
			delete BigMap[offset];
			BigMap[offset] = NULL;
		}
	}

	XStart = PreludeWorld->ScreenX - 1;
	YStart = PreludeWorld->ScreenY - 1;
	XEnd = PreludeWorld->ScreenX + 1;
	YEnd = PreludeWorld->ScreenY + 1;

	if(XStart < 0) 
		XStart = 0;
	
	if(YStart < 0) 
		YStart = 0;
	
	if(XEnd >= this->ChunkWidth)
		XEnd = this->ChunkWidth - 1;

	if(YEnd >= this->ChunkHeight)
		YEnd = this->ChunkHeight - 1;


	Engine->Graphics()->GetD3D()->BeginScene();
	for(yn = YStart; yn <= YEnd; yn++)
	for(xn = XStart; xn <= XEnd; xn++)
	{
		LoadChunk(xn,yn);
		GetChunk(xn,yn)->CreateTexture(GetBaseTexture());

	}

	Engine->Graphics()->GetD3D()->EndScene();

	SetCurrentDirectory(Engine->GetRootDirectory());

	return;
}
	
void Area::FixOutDoorTerrain()
{
	SetCurrentDirectory(".\\Areas");
	
	int xn;
	int yn;
	Chunk *pChunk;

	int *NewOffsets;
	
	NewOffsets = new int[ChunkWidth * ChunkHeight];
	
	FILE *fp;

	fp = SafeFileOpen("temp.bin","wb");

	SaveHeader(fp);

	for(yn = 0; yn < ChunkHeight; yn++)
	for(xn = 0; xn < ChunkWidth; xn++)
	{
		LoadChunk(xn,yn);
		pChunk = GetChunk(xn,yn);
		if(pChunk)
		{
			NewOffsets[xn + yn * ChunkWidth] = ftell(fp);
			pChunk->FixTerrainHeights();
			pChunk->Save(fp);
			delete pChunk;
			BigMap[xn + yn * ChunkWidth] = 0;
		}
		else
		{
			NewOffsets[xn + yn * ChunkWidth] = 0;
		}
	}

	delete[] Header.ChunkOffsets;

	Header.ChunkOffsets = NewOffsets;

	fseek(fp,0,SEEK_SET);

	SaveHeader(fp);

	fclose(fp);
	fclose(StaticFile);

	char filename[64];
	sprintf(filename,"%s.bin",Header.Name);

	remove(filename);

	rename("temp.bin",filename);

	StaticFile = SafeFileOpen(filename,"rb");
	
	SetCurrentDirectory(Engine->GetRootDirectory());
}

void World::LoadRoads()
{
	FILE *fp;
	int xn;
	int yn;
	fp = fopen ("roads.bin","rb");
	if(fp)
	{
		for(yn = 0; yn < 200; yn++)
		for(xn = 0; xn < 200; xn++)
		{
			fread(&Roads[xn][yn],1,sizeof(BOOL),fp);
		}
	
		fclose(fp);
	
	/*	fp = fopen("testroads.txt","wt");

		for(yn = 0; yn < 200; yn++)
		{
			for(xn = 0; xn < 200; xn++)
			{
				if(Roads[xn][yn])
				{
					fprintf(fp,"*");
				}
				else
				{
					fprintf(fp,".");
				}
			}
			fprintf(fp,"\n");
		}
		fclose(fp);
	*/	return;
	}

	for(yn = 0; yn < 200; yn++)
	for(xn = 0; xn < 200; xn++)
	{
		Chunk *pChunk;
		Valley->LoadChunk(xn,yn);
		pChunk = Valley->GetChunk(xn,yn);
		Roads[xn][yn] = pChunk->IsRoadChunk();
		Valley->RemoveChunk(xn,yn);	
	
	}

	fp = fopen("roads.bin","wb");
	for(yn = 0; yn < 200; yn++)
	for(xn = 0; xn < 200; xn++)
	{
		fwrite(&Roads[xn][yn],1,sizeof(BOOL),fp);
	
	}
	fclose(fp);


/*	fp = fopen("testroads.txt","wt");

		for(yn = 0; yn < 200; yn++)
		{
			for(xn = 0; xn < 200; xn++)
			{
				if(Roads[xn][yn])
				{
					fprintf(fp,"*");
				}
				else
				{
					fprintf(fp,".");
				}
			}
			fprintf(fp,"\n");
		}
	fclose(fp);
*/
	return;

}

