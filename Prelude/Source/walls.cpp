#include "walls.h"
#include "regions.h"
#include "zsengine.h"

unsigned short Wall::DrawList[12] = {0,1,2,1,3,2,4,5,6,5,7,6};

WallSide::WallSide()
{
	Verts = NULL;
	DrawList = NULL;
	RegionFacing = NULL;
	NumVerts = 0;
	NumTriangles = 0;
	DrawMe = FALSE;
	pNext = NULL;
	pTexture = NULL;
}

WallSide::~WallSide()
{
	delete Verts;
	delete DrawList;
}

void WallSide::AdjustUV(float uOff, float vOff)
{
	for(int n = 0; n < NumVerts; n++)
	{
		Verts[n].tu += uOff;
		Verts[n].tv += vOff;
	}
}

void WallSide::Tile(float Amount)
{
	float WallHeight;
	
	WallHeight = (Verts[1].z - Verts[0].z) / 4.0f;

	float tvAmount;

	tvAmount = Amount * WallHeight;
	Verts[0].tv = Verts[1].tv + tvAmount;
	Verts[3].tv = Verts[2].tv + tvAmount;

	float Length;
	D3DVECTOR vA, vB;
	vA.x = Verts[1].x;
	vA.y = Verts[1].y;
	vA.z = 0;

	vB.x = Verts[2].x;
	vB.y = Verts[2].y;
	vB.z = 0;

	Length = GetDistance(&vA, &vB);

	Length /= 4.0f;

	Verts[2].tu = Verts[0].tu + (Length * Amount);
	Verts[3].tu = Verts[1].tu + (Length * Amount);


}

void WallSide::FixUV()
{

	float WallHeight;
	
	WallHeight = Verts[0].z - Verts[2].z;

	Verts[2].tv = 0;
	Verts[3].tv = 0;
	Verts[0].tv = Verts[2].tv + WallHeight;
	Verts[1].tv = Verts[3].tv + WallHeight;


}



void WallSide::Save(FILE *fp)
{
	fwrite(&NumVerts,sizeof(int),1,fp);
	fwrite(Verts,sizeof(D3DVERTEX),NumVerts,fp);
	fwrite(&NumTriangles,sizeof(int),1,fp);
	fwrite(DrawList,sizeof(unsigned short), NumTriangles,fp);
	RegionNum = RegionFacing->GetID();
	if(!RegionFacing->GetParent())//SubRegion())
	{
		RegionNum = 0;
	}
	fwrite(&RegionNum,sizeof(int),1,fp);
	int TNum;
	TNum = Engine->GetTextureNum(pTexture);
	fwrite(&TNum,sizeof(int),1,fp);
}

void WallSide::Load(FILE *fp)
{
	fread(&NumVerts,sizeof(int),1,fp);
	
	Verts = new D3DVERTEX[NumVerts];

	fread(Verts,sizeof(D3DVERTEX),NumVerts,fp);

	fread(&NumTriangles,sizeof(int),1,fp);
	
	DrawList = new unsigned short[NumTriangles];
	
	fread(DrawList,sizeof(unsigned short), NumTriangles,fp);

	fread(&RegionNum,sizeof(int),1,fp);
	
	int TNum;

	fread(&TNum,sizeof(int),1,fp);

	pTexture = Engine->GetTexture(TNum);

}


void WallSide::Draw()
{
	if(DrawMe)
	{
		HRESULT hr;
		hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_VERTEX,Verts,NumVerts,DrawList,NumTriangles,0);
	}
	if(pNext)
		pNext->Draw();
}


Wall::Wall()
{
	Type = WALL_INTERIOR;
	Front = NULL;
	Back = NULL;
	pNext = NULL;
	Transparent = FALSE;
}


Wall::~Wall()
{


}

void Wall::OutPutDebugInfo(FILE *fp)
{
//	fprintf(fp,"Wall: %i at (%f,%f,%f) - (%f,%f,%f)  Height: %f\n",Type,Start.x,Start.y,Start.z,End.x,End.y,End.z,Height);
}

void Wall::Draw()
{
	HRESULT hr;
	if(Transparent)
	{
		if(Front)
			hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_VERTEX,Front->Verts,Front->NumVerts,Front->DrawList,Front->NumTriangles,0);

/*		if(Back)
			hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
												D3DFVF_VERTEX,
												&Back->StridedInfo,
												Back->NumVerts,
												Back->DrawList,
												Back->NumTriangles,
												0);
*/
	}
/*	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 
												D3DFVF_LVERTEX,
												TBVerts,
												8,
												DrawList,
												12,
												0);
*/

	if(pNext)
		pNext->Draw();
}

void Wall::Save(FILE *fp)
{
	BYTE True = 1;
	BYTE False = 0;
	fwrite(&Type,sizeof(Type),1,fp);
	fwrite(TBVerts,sizeof(D3DLVERTEX),8,fp);
	//front
	if(Front)
	{
		fwrite(&True,sizeof(BYTE),1,fp);
		Front->Save(fp);
	}
	else
	{
		fwrite(&False,sizeof(BYTE),1,fp);
	}
	//back
	if(Back)
	{
		fwrite(&True,sizeof(BYTE),1,fp);
		Back->Save(fp);
	}
	else
	{
		fwrite(&False,sizeof(BYTE),1,fp);
	}
}

void Wall::Load(FILE *fp)
{
	BYTE Load = 1;

	fread(&Type,sizeof(Type),1,fp);
	fread(TBVerts,sizeof(D3DLVERTEX),8,fp);

	//front
	fread(&Load,sizeof(BYTE),1,fp);
	if(Load)
	{
		Front = new WallSide;
		Front->Load(fp);
	}
	else
	{
		Front = NULL;
	}

	fread(&Load,sizeof(BYTE),1,fp);
	if(Load)
	{
		Back = new WallSide;
		Back->Load(fp);
	}
	else
	{
		Back = NULL;
	}
}

WallSide *Wall::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	float Angle;
	D3DVECTOR vNorm;
	D3DVERTEX vA,vB,vC;
	D3DVECTOR vRay;
	WallSide *pWallSide;
	vRay = *vRayEnd - *vRayStart;

	//check front facing
	//is the front facing the camera
	if(Front)
	{
		vNorm.x = Front->Verts[0].nx;
		vNorm.y = Front->Verts[0].ny;
		vNorm.z = Front->Verts[0].nz;
	
		Angle = GetAngle(&vNorm,&vRay);
		if(Angle > (PI_DIV_2 + PI) || Angle < (PI_DIV_2))
		{
			pWallSide = Front;
		}
		else
		{
			pWallSide = Back;
		}
	}
	else
	{
		pWallSide = Back;			
	}
	
	if(pWallSide)
	{
		int n;
		for(n = 0; n < pWallSide->NumTriangles; n += 3)
		{	
			vA = pWallSide->Verts[DrawList[n]];
			vB = pWallSide->Verts[DrawList[n+1]];
			vC = pWallSide->Verts[DrawList[n+2]];
		
			if(Triangle3DIntersect(vRayStart, vRayEnd, &vA, &vB, &vC))
			{
				return pWallSide;
			}
		}
	}
	return NULL;
}

void Wall::GetStart(D3DVECTOR *pvDest)
{
	pvDest->x = Front->Verts[0].x;
	pvDest->y = Front->Verts[0].y;
	pvDest->z = Front->Verts[0].z;
}

void Wall::GetEnd(D3DVECTOR *pvDest)
{
	pvDest->x = Front->Verts[3].x;
	pvDest->y = Front->Verts[3].y;
	pvDest->z = Front->Verts[3].z;
}

void Wall::GetCenter(D3DVECTOR *pvDest)
{
	pvDest->x = Front->Verts[0].x;
	pvDest->y = Front->Verts[0].y;
	pvDest->z = Front->Verts[0].z;
	
	pvDest->x += Front->Verts[3].x;
	pvDest->y += Front->Verts[3].y;
	pvDest->z += Front->Verts[3].z;

	pvDest->x /= 2.0f;
	pvDest->y /= 2.0f;
	pvDest->z /= 2.0f;
}

void Wall::Move(D3DVECTOR *vMoveRay)
{
	int n;
	if(Front)
	{
		for(n = 0; n < Front->NumVerts; n++)
		{
			Front->Verts[n].x += vMoveRay->x;
			Front->Verts[n].y += vMoveRay->y;
			Front->Verts[n].z += vMoveRay->z;
		}
	}

	if(Back)
	{
		for(n = 0; n < Back->NumVerts; n++)
		{
			Back->Verts[n].x += vMoveRay->x;
			Back->Verts[n].y += vMoveRay->y;
			Back->Verts[n].z += vMoveRay->z;
		}
	}

	for(n = 0; n < 8; n++)
	{
		TBVerts[n].x += vMoveRay->x;
		TBVerts[n].y += vMoveRay->y;
		TBVerts[n].z += vMoveRay->z;
	}
}

BOOL Wall::LineIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	D3DVECTOR vLineStart;
	D3DVECTOR vLineEnd;

	GetStart(&vLineStart);
	GetEnd(&vLineEnd);

	D3DVECTOR vSideA;
	D3DVECTOR vSideB;
	D3DVECTOR vL1;
	D3DVECTOR vL2;
	D3DVECTOR vL3;
	D3DVECTOR vL4;
	D3DVECTOR vNormA;

	D3DVECTOR vA;
	vA.x = vLineStart.x;
	vA.y = vLineStart.y;
	vA.z = vLineStart.z + 5.0f;

	D3DVECTOR vB;
	vB.x = vLineEnd.x;
	vB.y = vLineEnd.y;
	vB.z = vLineEnd.z + 5.0f;
	
	D3DVECTOR vC;
	vC.x = vLineStart.x;
	vC.y = vLineStart.y;
	vC.z = vLineStart.z;

	D3DVECTOR vD;
	vD.x = vLineEnd.x;
	vD.y = vLineEnd.y;
	vD.z = vLineEnd.z;

	D3DVECTOR vIntersect;

	D3DVECTOR vRay;

	vRay = *vRayEnd - *vRayStart;
	
	float DistToPlane;
	float ProjectedLength;
	float Ratio;

	float AngleTotal;

	vSideA = vA - vB;

	vSideB = vC - vB;

	vNormA = Normalize(CrossProduct(vSideA,vSideB));
	
	//vNorm now contains the normal to the upper left triangle of this tile
	vL1 = vB - *vRayStart;

	DistToPlane = DotProduct(vL1,vNormA);
	
	ProjectedLength = DotProduct((*vRayEnd - *vRayStart),vNormA);
	Ratio = DistToPlane / ProjectedLength;

	vIntersect = *vRayStart + (vRay * Ratio);

	//vIntersect now contains the intersection point of with the plane.
		
	AngleTotal = 0.0f;
	//now check to see if the interect is in the polygon;
	vL1 = vA - vIntersect;
	vL1 = Normalize(vL1);

	vL2 = vB - vIntersect;
	vL2 = Normalize(vL2);

	vL3 = vC - vIntersect;
	vL3 = Normalize(vL3);
	
	vL4 = vD - vIntersect;
	vL4 = Normalize(vL4);

	AngleTotal += (float)acos(DotProduct(vL1,vL2));
	AngleTotal += (float)acos(DotProduct(vL2,vL4));
	AngleTotal += (float)acos(DotProduct(vL3,vL1));
	AngleTotal += (float)acos(DotProduct(vL4,vL3));

	if((AngleTotal > 6.27f && AngleTotal < 6.29f) || (AngleTotal > -6.29f && AngleTotal < -6.27f))
	{
		//now check vs. segment bounds
		vL1 = *vRayStart - vIntersect;
		vL2 = *vRayEnd - vIntersect;

		if(Magnitude(vL1) + Magnitude (vL2) == Magnitude(vRay))
		{
			return TRUE;
		}

		return FALSE;
	}
	
	return FALSE;
}