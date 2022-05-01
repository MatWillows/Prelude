#include "cavewall.h"
#include "zsengine.h"
#include "world.h"

const D3DVECTOR vSegOffsets[] 
= {		
		D3DVECTOR(0.0f,0.0f,0.0f),//none
		D3DVECTOR(-1.0f,0.0f,0.0f),//n
		D3DVECTOR(-1.0f,-1.0f,0.0f),//ne
		D3DVECTOR(0.0f,-1.0f,0.0f),//e
		D3DVECTOR(1.0f,-1.0f,0.0f),//se
		D3DVECTOR(1.0f,0.0f,0.0f),//s
		D3DVECTOR(1.0f,1.0f,0.0f),//sw
		D3DVECTOR(0.0f,1.0f,0.0f),//w
		D3DVECTOR(-1.0f,1.0f,0.0f),//nw
 }; 


#define ANGLE_FACTOR (PI_DIV_2)

void CaveWall::BuildVerts()
{


}

void CaveWall::DrawTemp()
{

}

void CaveWall::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

	if(pTexture)
		Engine->Graphics()->SetTexture(pTexture);

	if(pVertsTop)
		Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_VERTEX, pVertsTop, NumVerts, NULL);
	if(pVertsMiddle)
		Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_VERTEX, pVertsMiddle, NumVerts, NULL);
	if(pVertsBottom)
		Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_VERTEX, pVertsBottom, NumVerts, NULL);
	

}

void CaveWall::FlipSides()
{

}

BOOL CaveWall::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{

	return FALSE;
}

BOOL CaveWall::RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{

	return FALSE;
}

BOOL CaveWall::TileIntersect(int TileX, int TileY)
{

	return FALSE;
}

BOOL CaveWall::TileBlock(int TileX, int TileY)
{

	return FALSE;
}

void CaveWall::Link(CaveWall *pLinkTo)
{

}

void CaveWall::Load(FILE *fp)
{
	fread(&Type,sizeof(Type),1,fp);
	fread(&Height,sizeof(Height),1,fp);
	fread(&NumVerts,sizeof(NumVerts),1,fp);

	if(Type == 1)
	{
		pVertsTop = new D3DVERTEX[NumVerts];	
		pVertsMiddle = new D3DVERTEX[NumVerts];
		pVertsBottom = new D3DVERTEX[NumVerts];

		fread(pVertsTop,sizeof(D3DVERTEX),NumVerts,fp);
		fread(pVertsMiddle,sizeof(D3DVERTEX),NumVerts,fp);
		fread(pVertsBottom,sizeof(D3DVERTEX),NumVerts,fp);
	}
	else
	{
		pVertsMiddle = new D3DVERTEX[NumVerts];
	
		fread(pVertsMiddle,sizeof(D3DVERTEX),NumVerts,fp);
	}
	fread(&TextureNum, sizeof(TextureNum),1,fp);
	pTexture = Engine->GetTexture(TextureNum);

}

void CaveWall::Save(FILE *fp)
{
	OBJECT_T ot;
	ot = GetObjectType();
	fwrite(&ot,sizeof(ot),1,fp);
	fwrite(&Type,sizeof(Type),1,fp);
	fwrite(&Height,sizeof(Height),1,fp);
	fwrite(&NumVerts,sizeof(NumVerts),1,fp);
	if(Type == 1)
	{
		fwrite(pVertsTop,sizeof(D3DVERTEX),NumVerts,fp);
		fwrite(pVertsMiddle,sizeof(D3DVERTEX),NumVerts,fp);
		fwrite(pVertsBottom,sizeof(D3DVERTEX),NumVerts,fp);
	}
	else
	{
		fwrite(pVertsMiddle,sizeof(D3DVERTEX),NumVerts,fp);
	}
	TextureNum = Engine->GetTextureNum(pTexture);
	fwrite(&TextureNum, sizeof(TextureNum),1,fp);
		
}
	
CaveWall::CaveWall()
{
	Type = 0;
	NumVerts = 0;
	pVertsMiddle = NULL;
	pVertsTop = NULL;
	pVertsBottom = NULL;
	Height = 5.0f;
	pNext = NULL;
	pNextUpdate = NULL;
	pPrevUpdate = NULL;
	pTexture = 0;
}

CaveWall::~CaveWall()
{
	if(pVertsMiddle)
	{
		delete[] pVertsMiddle;
		pVertsMiddle = NULL;
	}

	if(pVertsTop)
	{
		delete[] pVertsTop;
		pVertsTop = NULL;
	}

	if(pVertsBottom)
	{
		delete[] pVertsMiddle;
		pVertsMiddle = NULL;
	}

}

void CaveWall::CreateSegments(int x1, int y1, int x2, int y2)
{
	int NumSegments;
	int MaxSegments;

	int CurX;
	int CurY;
	int NextX;
	int NextY;

	int XDif;
	int YDif;

	int Count = 0;
	
	int XInc = -1;
	int YInc = -1;

	XDif = x1 - x2;
	YDif = y1 - y2;

	int XLength;
	int YLength;

	int xn;
	int yn;

	xn = x1;
	yn = y1;

	WALL_SEGMENT_T *SegList;
	
	int CurSeg;

	if(abs(XDif) >= abs(YDif))
	{
		MaxSegments = abs(XDif);	
		NumSegments = MaxSegments;

		SegList = new WALL_SEGMENT_T[MaxSegments];
		
		CurSeg = 0;
		
		if(XDif < 0)
		{
			XInc = 1;
		}

		if(YDif < 0)
		{
			YInc = 1;
		}

		XLength = abs(XDif) / (abs(YDif) + 1);
		Count = 0;

		CurX = x1;
		CurY = y1;

		while(CurX != x2 && CurSeg < MaxSegments)
		{
			NextX = CurX + XInc;
			Count++;
			if(Count >= XLength)
			{
				Count = 0;
				NextY = CurY + YInc;
			}
			else
			{
				NextY = CurY;
			}

			SegList[CurSeg].vStart.x = (float)CurX + 0.5f;
			SegList[CurSeg].vStart.y = (float)CurY + 0.5f;
			SegList[CurSeg].vStart.z = Valley->GetZ(SegList[CurSeg].vStart.x,SegList[CurSeg].vStart.y);

			SegList[CurSeg].vEnd.x = (float)NextX + 0.5f;
			SegList[CurSeg].vEnd.y = (float)NextY + 0.5f;
			SegList[CurSeg].vEnd.z = Valley->GetZ(SegList[CurSeg].vEnd.x,SegList[CurSeg].vEnd.y);
			
			CurX = NextX;
			CurY = NextY;
			CurSeg++;
		}
	}
	else
	{
		MaxSegments = abs(YDif);	
		NumSegments = MaxSegments;

		SegList = new WALL_SEGMENT_T[MaxSegments];
		
		CurSeg = 0;
		
		if(XDif < 0)
		{
			XInc = 1;
		}

		if(YDif < 0)
		{
			YInc = 1;
		}

		YLength = abs(YDif) / (abs(XDif) + 1);
		Count = 0;

		CurX = x1;
		CurY = y1;

		while(CurY != y2 && CurSeg < MaxSegments)
		{
			NextY = CurY + YInc;
			Count++;
			if(Count >= YLength)
			{
				Count = 0;
				NextX = CurX + XInc;
			}
			else
			{
				NextX = CurX;
			}

			SegList[CurSeg].vStart.x = (float)CurX + 0.5f;
			SegList[CurSeg].vStart.y = (float)CurY + 0.5f;
			SegList[CurSeg].vStart.z = Valley->GetZ(SegList[CurSeg].vStart.x,SegList[CurSeg].vStart.y);
			
			SegList[CurSeg].vEnd.x = (float)NextX + 0.5f;
			SegList[CurSeg].vEnd.y = (float)NextY + 0.5f;
			SegList[CurSeg].vEnd.z = Valley->GetZ(SegList[CurSeg].vEnd.x,SegList[CurSeg].vEnd.y);

			CurX = NextX;
			CurY = NextY;
			CurSeg++;
		}
	}

	//now we have a segment list
	//optimize it
	//compress segments w/ same slope into one
	
	//build the wall vertex list;
	D3DVECTOR vNormTop(0.0f, 0.0f, 1.0f);
	D3DVECTOR vNormBottom(0.0f, 0.0f, 1.0f);
	D3DVECTOR vNormMiddle(0.0f, 0.0f, 1.0f);

	float fTop;
	fTop = Height;
	
	float fMiddle;
	fMiddle = Height * 0.75f;
	
	float fBottom;
	fBottom = Height * 0.25f;

	pVertsTop = new D3DVERTEX[(NumSegments + 1) * 2];
	pVertsBottom = new D3DVERTEX[(NumSegments + 1) * 2];
	pVertsMiddle = new D3DVERTEX[(NumSegments + 1) * 2];

	D3DVECTOR vOffset;
	vOffset.y = (x2 - x1);
	vOffset.x = -(y2 - y1);
	vOffset.z = 0.0f;

	vOffset = Normalize(vOffset);

	vNormMiddle = vOffset * -1.0f;

	vOffset *= 0.5f;

	//calculate the top and bottom norms

	D3DVECTOR vA, vB, vC, vSideOne, vSideTwo;

	vA.x = SegList[0].vStart.x;
	vA.y = SegList[0].vStart.y;
	vA.z = SegList[0].vStart.z + fTop;

	vB.x = SegList[0].vStart.x;
	vB.y = SegList[0].vStart.y;
	vB.z = SegList[0].vStart.z + fMiddle;

	vC.x = SegList[0].vEnd.x;
	vC.y = SegList[0].vEnd.y;
	vC.z = SegList[0].vEnd.z + fTop;

	vSideOne = Normalize(vB - vA);
	vSideTwo = Normalize(vC - vA);

	vNormTop = CrossProduct(vSideOne,vSideTwo);

	vNormBottom = vNormTop;
	vNormBottom.z *= -1.0f;

	for(int n = 0; n < NumSegments; n++)
	{
		//top row
		pVertsTop[n*2].x = SegList[n].vStart.x;
		pVertsTop[n*2].y = SegList[n].vStart.y;
		pVertsTop[n*2].z = SegList[n].vStart.z + fTop;

		pVertsTop[n*2].nx = vNormTop.x;
		pVertsTop[n*2].ny = vNormTop.y;
		pVertsTop[n*2].nz = vNormTop.z;

		pVertsTop[n*2].tu = (float)n / Height;
		pVertsTop[n*2].tv = 1.0f;

		pVertsTop[n*2 + 1].x = SegList[n].vStart.x + vOffset.x;
		pVertsTop[n*2 + 1].y = SegList[n].vStart.y + vOffset.y;
		pVertsTop[n*2 + 1].z = SegList[n].vStart.z + fMiddle;

		pVertsTop[n*2 + 1].nx = vNormTop.x;
		pVertsTop[n*2 + 1].ny = vNormTop.y;
		pVertsTop[n*2 + 1].nz = vNormTop.z;

		pVertsTop[n*2 + 1].tu = (float)n / Height;
		pVertsTop[n*2 + 1].tv = .75f;

		//middle
		pVertsMiddle[n*2].x = SegList[n].vStart.x + vOffset.x;
		pVertsMiddle[n*2].y = SegList[n].vStart.y + vOffset.y;
		pVertsMiddle[n*2].z = SegList[n].vStart.z + fMiddle;

		pVertsMiddle[n*2].nx = vNormMiddle.x;
		pVertsMiddle[n*2].ny = vNormMiddle.y;
		pVertsMiddle[n*2].nz = vNormMiddle.z;

		pVertsMiddle[n*2].tu = (float)n / Height;
		pVertsMiddle[n*2].tv = 0.75f;

		pVertsMiddle[n*2 + 1].x = SegList[n].vStart.x + vOffset.x;
		pVertsMiddle[n*2 + 1].y = SegList[n].vStart.y + vOffset.y;
		pVertsMiddle[n*2 + 1].z = SegList[n].vStart.z + fBottom;

		pVertsMiddle[n*2 + 1].nx = vNormMiddle.x;
		pVertsMiddle[n*2 + 1].ny = vNormMiddle.y;
		pVertsMiddle[n*2 + 1].nz = vNormMiddle.z;

		pVertsMiddle[n*2 + 1].tu = (float)n / Height;
		pVertsMiddle[n*2 + 1].tv = 0.25f;

			
		//bottom
		pVertsBottom[n*2].x = SegList[n].vStart.x + vOffset.x;
		pVertsBottom[n*2].y = SegList[n].vStart.y + vOffset.y;
		pVertsBottom[n*2].z = SegList[n].vStart.z + fBottom;

		pVertsBottom[n*2].nx = vNormBottom.x;
		pVertsBottom[n*2].ny = vNormBottom.y;
		pVertsBottom[n*2].nz = vNormBottom.z;

		pVertsBottom[n*2].tu = (float)n / Height;
		pVertsBottom[n*2].tv = 0.25f;

		pVertsBottom[n*2 + 1].x = SegList[n].vStart.x;
		pVertsBottom[n*2 + 1].y = SegList[n].vStart.y;
		pVertsBottom[n*2 + 1].z = SegList[n].vStart.z;

		pVertsBottom[n*2 + 1].nx = vNormBottom.x;
		pVertsBottom[n*2 + 1].ny = vNormBottom.y;
		pVertsBottom[n*2 + 1].nz = vNormBottom.z;

		pVertsBottom[n*2 + 1].tu = (float)n / Height;
		pVertsBottom[n*2 + 1].tv = 0.0f;
	}

	int sn;
	sn = NumSegments - 1;
	n = NumSegments;

	//top row
	pVertsTop[n*2].x = SegList[sn].vEnd.x;
	pVertsTop[n*2].y = SegList[sn].vEnd.y;
	pVertsTop[n*2].z = SegList[sn].vEnd.z + fTop;

	pVertsTop[n*2].nx = vNormTop.x;
	pVertsTop[n*2].ny = vNormTop.y;
	pVertsTop[n*2].nz = vNormTop.z;

	pVertsTop[n*2].tu = (float)n / Height;
	pVertsTop[n*2].tv = 1.0f;

	pVertsTop[n*2 + 1].x = SegList[sn].vEnd.x + vOffset.x;
	pVertsTop[n*2 + 1].y = SegList[sn].vEnd.y + vOffset.y;
	pVertsTop[n*2 + 1].z = SegList[sn].vEnd.z + fMiddle;

	pVertsTop[n*2 + 1].nx = vNormTop.x;
	pVertsTop[n*2 + 1].ny = vNormTop.y;
	pVertsTop[n*2 + 1].nz = vNormTop.z;

	pVertsTop[n*2 + 1].tu = (float)n / Height;
	pVertsTop[n*2 + 1].tv = .75f;

	//middle
	pVertsMiddle[n*2].x = SegList[sn].vEnd.x + vOffset.x;
	pVertsMiddle[n*2].y = SegList[sn].vEnd.y + vOffset.y;
	pVertsMiddle[n*2].z = SegList[sn].vEnd.z + fMiddle;

	pVertsMiddle[n*2].nx = vNormMiddle.x;
	pVertsMiddle[n*2].ny = vNormMiddle.y;
	pVertsMiddle[n*2].nz = vNormMiddle.z;

	pVertsMiddle[n*2].tu = (float)n / Height;
	pVertsMiddle[n*2].tv = 0.75f;

	pVertsMiddle[n*2 + 1].x = SegList[sn].vEnd.x + vOffset.x;
	pVertsMiddle[n*2 + 1].y = SegList[sn].vEnd.y + vOffset.y;
	pVertsMiddle[n*2 + 1].z = SegList[sn].vEnd.z + fBottom;

	pVertsMiddle[n*2 + 1].nx = vNormMiddle.x;
	pVertsMiddle[n*2 + 1].ny = vNormMiddle.y;
	pVertsMiddle[n*2 + 1].nz = vNormMiddle.z;

	pVertsMiddle[n*2 + 1].tu = (float)n / Height;
	pVertsMiddle[n*2 + 1].tv = 0.25f;

	//bottom
	pVertsBottom[n*2].x = SegList[sn].vEnd.x + vOffset.x;
	pVertsBottom[n*2].y = SegList[sn].vEnd.y + vOffset.y;
	pVertsBottom[n*2].z = SegList[sn].vEnd.z + fBottom;

	pVertsBottom[n*2].nx = vNormBottom.x;
	pVertsBottom[n*2].ny = vNormBottom.y;
	pVertsBottom[n*2].nz = vNormBottom.z;

	pVertsBottom[n*2].tu = (float)n / Height;
	pVertsBottom[n*2].tv = 0.25f;

	pVertsBottom[n*2 + 1].x = SegList[sn].vEnd.x;
	pVertsBottom[n*2 + 1].y = SegList[sn].vEnd.y;
	pVertsBottom[n*2 + 1].z = SegList[sn].vEnd.z;

	pVertsBottom[n*2 + 1].nx = vNormBottom.x;
	pVertsBottom[n*2 + 1].ny = vNormBottom.y;
	pVertsBottom[n*2 + 1].nz = vNormBottom.z;

	pVertsBottom[n*2 + 1].tu = (float)n / Height;
	pVertsBottom[n*2 + 1].tv = 0.0f;

	//Done!
	delete[] SegList;

	NumVerts = (NumSegments + 1) * 2;

	Position.x = (x1 + x2) / 2;
	Position.y = (y1 + y2) / 2;
	
}

WallSegmentList *WallSegmentList::pStart = NULL;

WallSegmentList::WallSegmentList()
{
	pNext = NULL;
}

WallSegmentList::~WallSegmentList()
{
	if(pNext && pNext != pStart)
	{
		delete pNext;
		pNext = NULL;
	}
}

BOOL ValidateList(WallSegmentList *pSegList)
{
	WallSegmentList *pCur;

	pCur = pSegList->pStart;
	BOOL Valid = TRUE;

	while(pCur->pNext)
	{
		if(pCur->pNext->Seg.FromAngle != pCur->Seg.ThisAngle ||
			pCur->Seg.ToAngle != pCur->pNext->Seg.ThisAngle)
		{
			DEBUG_INFO("Bad seg list\n");
			Valid = FALSE;
			pCur->Seg.ToAngle = pCur->pNext->Seg.ThisAngle;
			pCur->pNext->Seg.FromAngle = pCur->Seg.ThisAngle;
		}
		pCur = pCur->pNext;
	}
	return Valid;
}

void CaveWall::Build(WallSegmentList *pSegList)
{
	switch(Type)
	{
	case 1:
		BuildCurved(pSegList);
		break;
	default:
		BuildFlat(pSegList);
		break;
	}
	return;
}
void CaveWall::BuildFlat(WallSegmentList *pSegList)
{
	ValidateList(pSegList);
	
	WallSegmentList *pCur;

	pCur = pSegList->pStart;

	NumSegments = 0;
	NumVerts = 0;

	while(pCur)
	{
		NumSegments++;
		pCur = pCur->pNext;
	}
	
	this->NumVerts = NumSegments * 2 + 2;
	pVertsTop = NULL;
	pVertsMiddle = new D3DVERTEX[NumVerts];
	pVertsBottom = NULL;

	pCur = pSegList->pStart;

	int n = 0;

	D3DVECTOR vStartOffset;
	D3DVECTOR vBaseOffset;
	D3DVECTOR vEndOffset;

	D3DVECTOR vNormMiddle(0.0f,0.0f,1.0f);

	float fTop;
	fTop = Height;
	
	n = NumSegments;

	while(pCur)
	{
			
		//calculate the start and end offset;
		vBaseOffset = Normalize(vSegOffsets[pCur->Seg.ThisAngle]);
		vStartOffset = Normalize(vSegOffsets[pCur->Seg.FromAngle]);
		vEndOffset = Normalize(vSegOffsets[pCur->Seg.ToAngle]);
		vStartOffset += vBaseOffset;
		vStartOffset =	Normalize(vStartOffset);
		vEndOffset += vBaseOffset;
		vEndOffset = Normalize(vEndOffset);

		vNormMiddle = vBaseOffset * - 1.0f;
		
		//middle
		pVertsMiddle[n*2].x = pCur->Seg.vStart.x;
		pVertsMiddle[n*2].y = pCur->Seg.vStart.y;
		pVertsMiddle[n*2].z = pCur->Seg.vStart.z + Height;

		pVertsMiddle[n*2].nx = vNormMiddle.x;
		pVertsMiddle[n*2].ny = vNormMiddle.y;
		pVertsMiddle[n*2].nz = vNormMiddle.z;

		pVertsMiddle[n*2].tu = (float)n / Height;
		pVertsMiddle[n*2].tv = 1.0f;

		pVertsMiddle[n*2 + 1].x = pCur->Seg.vStart.x;
		pVertsMiddle[n*2 + 1].y = pCur->Seg.vStart.y;
		pVertsMiddle[n*2 + 1].z = pCur->Seg.vStart.z;

		pVertsMiddle[n*2 + 1].nx = vNormMiddle.x;
		pVertsMiddle[n*2 + 1].ny = vNormMiddle.y;
		pVertsMiddle[n*2 + 1].nz = vNormMiddle.z;

		pVertsMiddle[n*2 + 1].tu = (float)n / Height;
		pVertsMiddle[n*2 + 1].tv = 0.0f;
			
		pCur = pCur->pNext;

		n--;
	}

	pCur = pSegList->pStart;
	while(pCur->pNext)
	{
		pCur = pCur->pNext;
	}


	n = 0;
	vBaseOffset = Normalize(vSegOffsets[pCur->Seg.ThisAngle]);
	vStartOffset = Normalize(vSegOffsets[pCur->Seg.FromAngle]);
	vEndOffset = Normalize(vSegOffsets[pCur->Seg.ToAngle]);
	vStartOffset += vBaseOffset;
	vStartOffset =	Normalize(vStartOffset);
	vEndOffset += vBaseOffset; 
	vEndOffset = Normalize(vEndOffset);

	vNormMiddle = vBaseOffset * - 1.0f;

	//middle
	pVertsMiddle[0].x = pCur->Seg.vEnd.x; 
	pVertsMiddle[0].y = pCur->Seg.vEnd.y;
	pVertsMiddle[0].z = pCur->Seg.vEnd.z + Height;

	pVertsMiddle[0].nx = vNormMiddle.x;
	pVertsMiddle[0].ny = vNormMiddle.y;
	pVertsMiddle[0].nz = vNormMiddle.z;

	pVertsMiddle[0].tu = (float)n / Height;
	pVertsMiddle[0].tv = 1.0f;

	pVertsMiddle[1].x = pCur->Seg.vEnd.x;
	pVertsMiddle[1].y = pCur->Seg.vEnd.y;
	pVertsMiddle[1].z = pCur->Seg.vEnd.z;

	pVertsMiddle[1].nx = vNormMiddle.x;
	pVertsMiddle[1].ny = vNormMiddle.y;
	pVertsMiddle[1].nz = vNormMiddle.z;

	pVertsMiddle[1].tu = (float)n / Height;
	pVertsMiddle[1].tv = 0.0f;

	Position.x = pSegList->pStart->Seg.vEnd.x;
	Position.y = pSegList->pStart->Seg.vEnd.y;
	Position.z = pSegList->pStart->Seg.vEnd.z;

}

void CaveWall::BuildCurved(WallSegmentList *pSegList)
{
	ValidateList(pSegList);
	
	WallSegmentList *pCur;

	pCur = pSegList->pStart;

	NumSegments = 0;
	NumVerts = 0;

	while(pCur)
	{
		NumSegments++;
		pCur = pCur->pNext;
	}
	
	this->NumVerts = NumSegments * 2 + 2;
	pVertsTop = new D3DVERTEX[NumVerts];
	pVertsMiddle = new D3DVERTEX[NumVerts];
	pVertsBottom = new D3DVERTEX[NumVerts];


	pCur = pSegList->pStart;

	int n = 0;

	D3DVECTOR vStartOffset;
	D3DVECTOR vBaseOffset;
	D3DVECTOR vEndOffset;

	D3DVECTOR vNormTop(0.0f,0.0f,1.0f);
	D3DVECTOR vNormBottom(0.0f,0.0f,1.0f);
	D3DVECTOR vNormMiddle(0.0f,0.0f,1.0f);

	float fTop;
	fTop = Height;
	
	float fMiddle;
	fMiddle = Height * 0.75f;
	
	float fBottom;
	fBottom = Height * 0.25f;
		
	n = NumSegments;

	float fx;
	float fy;
	float fz;


		
	while(pCur)
	{
			
		//calculate the start and end offset;
		vBaseOffset = Normalize(vSegOffsets[pCur->Seg.ThisAngle]);
		vStartOffset = Normalize(vSegOffsets[pCur->Seg.FromAngle]);
		vEndOffset = Normalize(vSegOffsets[pCur->Seg.ToAngle]);
		vStartOffset += vBaseOffset;
		vStartOffset =	Normalize(vStartOffset);
		vEndOffset += vBaseOffset;
		vEndOffset = Normalize(vEndOffset);

		vNormBottom = vNormMiddle = vNormTop = vBaseOffset * - 1.0f;
	
		if(n < NumSegments  && n > 1)
		{
			fx = pCur->Seg.vStart.x;
			fy = pCur->Seg.vStart.y;
			fz = pCur->Seg.vStart.z;
		
			float frand;
			frand = 0.25 - 0.5f * ((float)rand() / (float)RAND_MAX);
			vBaseOffset *= frand;

			fx += vBaseOffset.x;
			fy += vBaseOffset.y;
			fz = Valley->GetZ(fx,fy) - 0.3f;

			pCur->Seg.vStart.x = fx;
			pCur->Seg.vStart.y = fy;
			pCur->Seg.vStart.z = fz;
		}

		vNormTop = vNormBottom = vStartOffset * -1.0f;
		
		vStartOffset *= 0.4f;
		vEndOffset *= 0.4f;
		
		vNormTop.z -= 0.3f;
		vNormBottom.z += 0.3f;

		vNormTop = Normalize(vNormTop);
		vNormBottom = Normalize(vNormBottom);

		//top row
		pVertsTop[n*2].x = pCur->Seg.vStart.x;
		pVertsTop[n*2].y = pCur->Seg.vStart.y;
		pVertsTop[n*2].z = pCur->Seg.vStart.z + fTop;

		pVertsTop[n*2].nx = vNormTop.x;
		pVertsTop[n*2].ny = vNormTop.y;
		pVertsTop[n*2].nz = vNormTop.z;

		pVertsTop[n*2].tu = (float)n / Height;
		pVertsTop[n*2].tv = 1.0f;

		pVertsTop[n*2 + 1].x = pCur->Seg.vStart.x + vStartOffset.x;
		pVertsTop[n*2 + 1].y = pCur->Seg.vStart.y + vStartOffset.y;
		pVertsTop[n*2 + 1].z = pCur->Seg.vStart.z + fMiddle;

		pVertsTop[n*2 + 1].nx = vNormTop.x;
		pVertsTop[n*2 + 1].ny = vNormTop.y;
		pVertsTop[n*2 + 1].nz = vNormTop.z;

		pVertsTop[n*2 + 1].tu = (float)n / Height;
		pVertsTop[n*2 + 1].tv = .75f;

		//middle
		pVertsMiddle[n*2].x = pCur->Seg.vStart.x + vStartOffset.x;
		pVertsMiddle[n*2].y = pCur->Seg.vStart.y + vStartOffset.y;
		pVertsMiddle[n*2].z = pCur->Seg.vStart.z + fMiddle;

		pVertsMiddle[n*2].nx = vNormMiddle.x;
		pVertsMiddle[n*2].ny = vNormMiddle.y;
		pVertsMiddle[n*2].nz = vNormMiddle.z;

		pVertsMiddle[n*2].tu = (float)n / Height;
		pVertsMiddle[n*2].tv = 0.75f;

		pVertsMiddle[n*2 + 1].x = pCur->Seg.vStart.x + vStartOffset.x;
		pVertsMiddle[n*2 + 1].y = pCur->Seg.vStart.y + vStartOffset.y;
		pVertsMiddle[n*2 + 1].z = pCur->Seg.vStart.z + fBottom;

		pVertsMiddle[n*2 + 1].nx = vNormMiddle.x;
		pVertsMiddle[n*2 + 1].ny = vNormMiddle.y;
		pVertsMiddle[n*2 + 1].nz = vNormMiddle.z;

		pVertsMiddle[n*2 + 1].tu = (float)n / Height;
		pVertsMiddle[n*2 + 1].tv = 0.25f;

			
		//bottom
		pVertsBottom[n*2].x = pCur->Seg.vStart.x + vStartOffset.x;
		pVertsBottom[n*2].y = pCur->Seg.vStart.y + vStartOffset.y;
		pVertsBottom[n*2].z = pCur->Seg.vStart.z + fBottom;

		pVertsBottom[n*2].nx = vNormBottom.x;
		pVertsBottom[n*2].ny = vNormBottom.y;
		pVertsBottom[n*2].nz = vNormBottom.z;

		pVertsBottom[n*2].tu = (float)n / Height;
		pVertsBottom[n*2].tv = 0.25f;

		pVertsBottom[n*2 + 1].x = pCur->Seg.vStart.x;
		pVertsBottom[n*2 + 1].y = pCur->Seg.vStart.y;
		pVertsBottom[n*2 + 1].z = pCur->Seg.vStart.z;

		pVertsBottom[n*2 + 1].nx = vNormBottom.x;
		pVertsBottom[n*2 + 1].ny = vNormBottom.y;
		pVertsBottom[n*2 + 1].nz = vNormBottom.z;

		pVertsBottom[n*2 + 1].tu = (float)n / Height;
		pVertsBottom[n*2 + 1].tv = 0.0f;

		pCur = pCur->pNext;

		n--;
	}

	pCur = pSegList->pStart;
	while(pCur->pNext)
	{
		pCur = pCur->pNext;
	}


	n = 0;
	vBaseOffset = Normalize(vSegOffsets[pCur->Seg.ThisAngle]);
	vStartOffset = Normalize(vSegOffsets[pCur->Seg.FromAngle]);
	vEndOffset = Normalize(vSegOffsets[pCur->Seg.ToAngle]);
	vStartOffset += vBaseOffset;
	vStartOffset =	Normalize(vStartOffset);
	vEndOffset += vBaseOffset; 
	vEndOffset = Normalize(vEndOffset);

	vNormMiddle = vBaseOffset * - 1.0f;

	vNormTop = vNormBottom = vEndOffset * -1.0f;

	vNormTop.z -= 0.3f;
	vNormBottom.z += 0.3f;

	vNormTop = Normalize(vNormTop);
	vNormBottom = Normalize(vNormBottom);
	vStartOffset *= 0.4f;
	vEndOffset *= 0.4f;
	vBaseOffset *= 0.4f;

	
	

	//top row
	pVertsTop[0].x = pCur->Seg.vEnd.x;
	pVertsTop[0].y = pCur->Seg.vEnd.y;
	pVertsTop[0].z = pCur->Seg.vEnd.z + fTop;

	pVertsTop[0].nx = vNormTop.x;
	pVertsTop[0].ny = vNormTop.y;
	pVertsTop[0].nz = vNormTop.z;

	pVertsTop[0].tu = (float)n / Height;
	pVertsTop[0].tv = 1.0f;

	pVertsTop[1].x = pCur->Seg.vEnd.x + vEndOffset.x;
	pVertsTop[1].y = pCur->Seg.vEnd.y + vEndOffset.y;
	pVertsTop[1].z = pCur->Seg.vEnd.z + fMiddle;

	pVertsTop[1].nx = vNormTop.x;
	pVertsTop[1].ny = vNormTop.y;
	pVertsTop[1].nz = vNormTop.z;

	pVertsTop[1].tu = (float)n / Height;
	pVertsTop[1].tv = .75f;

	//middle
	pVertsMiddle[0].x = pCur->Seg.vEnd.x + vEndOffset.x;
	pVertsMiddle[0].y = pCur->Seg.vEnd.y + vEndOffset.y;
	pVertsMiddle[0].z = pCur->Seg.vEnd.z + fMiddle;

	pVertsMiddle[0].nx = vNormMiddle.x;
	pVertsMiddle[0].ny = vNormMiddle.y;
	pVertsMiddle[0].nz = vNormMiddle.z;

	pVertsMiddle[0].tu = (float)n / Height;
	pVertsMiddle[0].tv = 0.75f;

	pVertsMiddle[1].x = pCur->Seg.vEnd.x + vEndOffset.x;
	pVertsMiddle[1].y = pCur->Seg.vEnd.y + vEndOffset.y;
	pVertsMiddle[1].z = pCur->Seg.vEnd.z + fBottom;

	pVertsMiddle[1].nx = vNormMiddle.x;
	pVertsMiddle[1].ny = vNormMiddle.y;
	pVertsMiddle[1].nz = vNormMiddle.z;

	pVertsMiddle[1].tu = (float)n / Height;
	pVertsMiddle[1].tv = 0.25f;

	//bottom
	pVertsBottom[0].x = pCur->Seg.vEnd.x + vEndOffset.x;
	pVertsBottom[0].y = pCur->Seg.vEnd.y + vEndOffset.y;
	pVertsBottom[0].z = pCur->Seg.vEnd.z + fBottom;

	pVertsBottom[0].nx = vNormBottom.x;
	pVertsBottom[0].ny = vNormBottom.y;
	pVertsBottom[0].nz = vNormBottom.z;

	pVertsBottom[0].tu = (float)n / Height;
	pVertsBottom[0].tv = 0.25f;

	pVertsBottom[1].x = pCur->Seg.vEnd.x;
	pVertsBottom[1].y = pCur->Seg.vEnd.y;
	pVertsBottom[1].z = pCur->Seg.vEnd.z;

	pVertsBottom[1].nx = vNormBottom.x;
	pVertsBottom[1].ny = vNormBottom.y;
	pVertsBottom[1].nz = vNormBottom.z;

	pVertsBottom[1].tu = (float)n / Height;
	pVertsBottom[1].tv = 0.0f;



	Position.x = pSegList->pStart->Seg.vEnd.x;
	Position.y = pSegList->pStart->Seg.vEnd.y;
	Position.z = pSegList->pStart->Seg.vEnd.z;

}

void CaveWall::Block()
{
	int xn;
	int yn;
	int n;
	for(n = 0; n < NumVerts; n+= 2)
	{
		xn = (int)pVertsMiddle[n].x;
		yn = (int)pVertsMiddle[n].y;
		

		Valley->SetBlocking(xn,yn);
	}
}

void CaveWall::Flatten()
{
	if(Type)
	{
		Type = 0;
		
		for(int n = 0; n < NumVerts; n+= 2)
		{
		
			//middle
			pVertsMiddle[n].x = (int)pVertsMiddle[n].x + 0.5f;
			pVertsMiddle[n].y = (int)pVertsMiddle[n].y + 0.5f;
			pVertsMiddle[n].z = pVertsBottom[n + 1].z + Height;

//			pVertsMiddle[n].nx = vNormMiddle.x;
//			pVertsMiddle[n].ny = vNormMiddle.y;
//			pVertsMiddle[n].nz = vNormMiddle.z;

			pVertsMiddle[n].tu = (float)n / Height;
			pVertsMiddle[n].tv = 1.0f;

			pVertsMiddle[n + 1].x = (int)pVertsMiddle[n].x + 0.5f;
			pVertsMiddle[n + 1].y = (int)pVertsMiddle[n].y + 0.5f;
			pVertsMiddle[n + 1].z = pVertsBottom[n + 1].z;

//			pVertsMiddle[n + 1].nx = vNormMiddle.x;
//			pVertsMiddle[n + 1].ny = vNormMiddle.y;
//			pVertsMiddle[n + 1].nz = vNormMiddle.z;

			pVertsMiddle[n + 1].tu = (float)n / Height;
			pVertsMiddle[n + 1].tv = 0.0f;
		
		}
		delete[] pVertsTop;
		delete[] pVertsBottom;
	}
	else
	{
		DEBUG_INFO("flattening already flat wall");
	}

}

void CaveWall::Curve()
{

}

