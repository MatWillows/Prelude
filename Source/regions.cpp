#include "regions.h"
#include "zsengine.h"
#include "world.h"
#include "portals.h"
#include "editregion.h"
#include "party.h"

void Region::ReBlock()
{
	D3DVECTOR vStart;
	D3DVECTOR vEnd;
	Wall *pWall;
	pWall = Walls;

	int XRound;
	int YRound;
	int LastX = -1;
	int LastY = -1;
	float Length;
	float MaxLength;
	
	D3DVECTOR Ray;
	D3DVECTOR Resultant;

	while(pWall)
	{
		pWall->GetStart(&vStart);
		pWall->GetEnd(&vEnd);
		LastX = -1;
		LastY = -1;

		Ray = vEnd - vStart;
		MaxLength = Magnitude(Ray);
		Ray = Normalize(Ray);

		for(Length = 0.0f; Length <= MaxLength; Length += 0.1f)
		{
			Resultant = vStart + (Ray * Length);
			XRound = (int)Resultant.x;
			YRound = (int)Resultant.y;
			if((XRound != LastX) || (YRound != LastY))
			{
				LastX = XRound;
				LastY = YRound;
				Valley->SetBlocking(XRound, YRound);
			}
		}

		pWall = pWall->GetNext();
	}
}

void Region::RePortal()
{

}


void Region::AddWallSide(WallSide *ToAdd)
{
	TextureList *pList;
	//first add the front
	pList = TextureWalls;
	if(!pList)
	{
		TextureWalls = pList = new TextureList(ToAdd->pTexture);
	}

	while(pList->pNext)
	{
		if(pList->pTexture == ToAdd->pTexture)
		{
			break;
		}
		else
		{
			pList = pList->pNext;
		}
	}

	if(pList->pTexture != ToAdd->pTexture)
	{
		pList->pNext = new TextureList(ToAdd->pTexture);
		pList = pList->pNext;
	}

	ToAdd->pNext = pList->pWall;
	pList->pWall = ToAdd;
}

void Region::AddWall(Wall *ToAdd)
{
	ToAdd->SetNext(Walls);

	Walls = ToAdd;

	AddWallSide(ToAdd->GetFront());
	AddWallSide(ToAdd->GetBack());
}


Region *Region::GetSubRegion(float x, float y)
{

	return NULL;
}

Wall *Region::GetNearestWall(float x, float y)
{
	float Distance = 200.0f;
	float NewDistance = 0.0f;
	Wall *pClosestWall = NULL;
/*	pWall = pClosestWall = Walls;
	D3DVECTOR Start;
	D3DVECTOR End;
	D3DVECTOR Point;
	Point.z = 0.0f;
	Point.x = x;
	Point.y = y;

	while(pWall)
	{
		Start = pWall->GetStart();
		Start.z = 0.0f;
		End = pWall->GetEnd();
		End.z = 0.0f;
		
		NewDistance = GetDistance(&Point,&Start) + GetDistance(&Point,&End);
		
		if(NewDistance < Distance)
		{
			Distance = NewDistance;
			pClosestWall = pWall;
		}
		pWall = pWall->GetNext();
	}
*/	return pClosestWall;
}

D3DVECTOR Region::GetNearestPoint(float x, float y)
{
/*	float Distance = 200.0f;
	float NewDistance = 0.0f;
	Wall *pWall;
	Wall *pClosestWall;
	pWall = pClosestWall = Walls;
	D3DVECTOR Point;
	Point.z = 0.0f;
	Point.x = x;
	Point.y = y;

	while(pWall)
	{
		Start = pWall->GetStart();
		Start.z = 0.0f;
		
		NewDistance = GetDistance(&Point,&Start);
		
		if(NewDistance < Distance)
		{
			Distance = NewDistance;
			pClosestWall = pWall;
		}
		pWall = pWall->GetNext();
	}
	return pClosestWall->GetStart();;
*/
	D3DVECTOR Start;
	return Start;
}


void Region::CreateWalls(D3DVECTOR *Bounds, int NumBounds)
{
/*	Wall *pWall;
	int n;
	Wall *pNewWall;
	float UOffset = 0;

	while(Walls)
	{
		pWall = Walls;
		Walls = pWall->GetNext();
		delete pWall;
	}

	if(Type == REGION_INTERIOR)
	{
		for(n = 0; n < NumBounds - 1; n++)
		{
			pNewWall = new Wall(&Bounds[n],&Bounds[n+1],WALL_INTERIOR, Height, UOffset);
			AddWall(pNewWall);
			UOffset += pNewWall->GetLength();
		}
		pNewWall = new Wall(&Bounds[NumBounds-1],&Bounds[0],WALL_INTERIOR, Height, UOffset);
		AddWall(pNewWall);
	}
	else
	{
		for(n = 0; n < NumBounds - 1; n++)
		{
			pNewWall = new Wall(&Bounds[n],&Bounds[n+1],WALL_EXTERIOR, 2.0f, UOffset);
			AddWall(pNewWall);
			UOffset += pNewWall->GetLength();
		}
		pNewWall = new Wall(&Bounds[NumBounds-1],&Bounds[0],WALL_EXTERIOR, 2.0f, UOffset);
		AddWall(pNewWall);
	}
	CreateFRMesh();
*/
}


void Region::CreateFRMesh()
{
/*
	if(Verts)
	{
		delete[] Verts;
		delete[] DrawList;
	}

	Wall *pWall;
	D3DVERTEX Temp[16 * 8];
	unsigned short TempDraw[16 * 8 * 6];

	if(Type == REGION_INTERIOR)
	{
		NumVerts = 0;
		pWall = Walls;
		while(pWall)
		{
			D3DVECTOR vA;
			vA = pWall->GetStart();
			Temp[NumVerts].x = vA.x;
			Temp[NumVerts].y = vA.y;
			Temp[NumVerts].z = vA.z + 0.01f;
			Temp[NumVerts].nx = 0;
			Temp[NumVerts].ny = 0;
			Temp[NumVerts].nz = 1.0f;
			Temp[NumVerts].tu = (Temp[NumVerts].x );
			Temp[NumVerts].tv = (Temp[NumVerts].y );
			NumVerts++;
			pWall = pWall->GetNext();
		}
		NumTriangles = 0;
		int n;
		for(n = 1; n < NumVerts - 1; n++)
		{
			TempDraw[(n-1)*3] = 0;
			TempDraw[(n-1)*3 + 1] = n+1;
			TempDraw[(n-1)*3 + 2] = n;
			NumTriangles+=3;
		}
		Verts = new D3DVERTEX[NumVerts];
		memcpy(Verts,Temp,sizeof(D3DVERTEX) * NumVerts);
		DrawList = new unsigned short[NumTriangles];
		memcpy(DrawList,TempDraw,sizeof(unsigned short) * NumTriangles);
	}
	else
	{
	switch(FRType)
	{
		case ROOF_DOME:
		default:
			D3DVECTOR	VCenter;
			FindCenter(&VCenter);
			float xpercent;
			float zpercent;
			int numdivisions = 4;
			int nd;
			float xn,yn,zn;
			int NumWalls;
			NumWalls = NumVerts = 0;
			pWall = Walls;
			while(pWall)
			{
				D3DVECTOR vA;
				vA = pWall->GetStart();
				Temp[NumVerts].x = vA.x;
				Temp[NumVerts].y = vA.y;
				Temp[NumVerts].z = vA.z + Height;
				Temp[NumVerts].nx = 0;
				Temp[NumVerts].ny = 0;
				Temp[NumVerts].nz = 1.0f;
				Temp[NumVerts].tu = (Temp[NumVerts].x );
				Temp[NumVerts].tv = (Temp[NumVerts].y );
				NumVerts++;
				NumWalls++;
				pWall = pWall->GetNext();
			}
			NumTriangles = 0;
			int OffSetStart = 0;
			int n;
			for(nd = 1; nd < numdivisions; nd++)
			{
				zpercent = (float)nd/(float)numdivisions;
				xpercent = sqrt(1 - (zpercent * zpercent));
				for(n = OffSetStart; n < NumWalls + OffSetStart; n++)
				{
					Temp[n + NumWalls] = Temp[n];
					xn = (xpercent * (Temp[n].x - VCenter.x))+ VCenter.x;
					yn = (xpercent * (Temp[n].y - VCenter.y))+ VCenter.y;
					zn = (zpercent * FRHeight) + Height;
					Temp[n + NumWalls].x = xn;
					Temp[n + NumWalls].y = yn;
					Temp[n + NumWalls].z = zn;
					Temp[n + NumWalls].tu = Temp[n + NumWalls].x;
					Temp[n + NumWalls].tv = Temp[n + NumWalls].y;
					NumVerts++;
				}
				for(n = OffSetStart; n < OffSetStart + NumWalls - 1; n++)
				{
					TempDraw[NumTriangles++] = n;
					TempDraw[NumTriangles++] = n + NumWalls;
					TempDraw[NumTriangles++] = (n + 1) + NumWalls;
					TempDraw[NumTriangles++] = (n + 1) + NumWalls;
					TempDraw[NumTriangles++] = n + 1;
					TempDraw[NumTriangles++] = n;
				}
				n = OffSetStart + NumWalls - 1;
				TempDraw[NumTriangles++] = n;
				TempDraw[NumTriangles++] = n + NumWalls;
				TempDraw[NumTriangles++] = OffSetStart + NumWalls;
				TempDraw[NumTriangles++] = OffSetStart + NumWalls;
				TempDraw[NumTriangles++] = OffSetStart;
				TempDraw[NumTriangles++] = n;
				OffSetStart += NumWalls;
			}
			for(n = OffSetStart + 1; n < OffSetStart + NumWalls - 1; n++)
			{
				TempDraw[NumTriangles++] = OffSetStart;
				TempDraw[NumTriangles++] = n+1;
				TempDraw[NumTriangles++] = n;
			}

			Verts = new D3DVERTEX[NumVerts];
			memcpy(Verts,Temp,sizeof(D3DVERTEX) * NumVerts);
			DrawList = new unsigned short[NumTriangles];
			memcpy(DrawList,TempDraw,sizeof(unsigned short) * NumTriangles);
			break;
		}
	}
*/
}

void Region::RotateFRTexture(float Angle)
{

}

BOOL Region::IsIn(D3DVECTOR *vCheck)
{

	return FALSE;
}

void Region::SetWallHeights(float NewHeight)
{
/*	Wall *pWall;

	pWall = Walls;

	while(pWall)
	{
		pWall->SetHeight(NewHeight);
//		pWall->CreateMesh();
		pWall = pWall->GetNext();
	}
*/
}

Region::Region()
{
	ID = 0;
	Verts = NULL;
	NumVerts = 0;
	DrawList = NULL;
	NumTriangles = 0;
	NumBorderTriangles = 0;
	SubRegions = NULL;
	pFRTexture = NULL;
	pWallTexture = NULL;
	Parent = NULL;
	pNext = NULL;
	TextureWalls = NULL;
	Walls = NULL;
	Occupied = REGION_UNSEEN;
	NumObjects = 0;
	Drawn = FALSE;
	pObjectList = NULL;
	pObjectListEnd = NULL;

}

Region::Region(REGION_T NewType, D3DVECTOR *Bounds, int NumBounds)
{
/*	Walls = NULL;
	Verts = NULL;
	NumVerts = 0;
	DrawList = NULL;
	NumTriangles = 0;
	SubRegions = NULL;
	pFRTexture = NULL;
	pWallTexture = NULL;
	Height = 2.0f;
	FRHeight = 2.0f;
	Parent = NULL;
	pNext = NULL;

	Wall *pNewWall;
	Type = NewType;
	
	int n = 0;
	float UOffset = 0;
	
	if(NewType == REGION_INTERIOR)
	{
		for(n = 0; n < NumBounds - 1; n++)
		{
			pNewWall = new Wall(&Bounds[n],&Bounds[n+1],WALL_INTERIOR, Height, UOffset);
			AddWall(pNewWall);
			UOffset += pNewWall->GetLength();
		}
		pNewWall = new Wall(&Bounds[NumBounds-1],&Bounds[0],WALL_INTERIOR, Height, UOffset);
		AddWall(pNewWall);
	}
	else
	{
		for(n = 0; n < NumBounds - 1; n++)
		{
			pNewWall = new Wall(&Bounds[n],&Bounds[n+1],WALL_EXTERIOR, 2.0f, UOffset);
			AddWall(pNewWall);
			UOffset += pNewWall->GetLength();
		}
		pNewWall = new Wall(&Bounds[NumBounds-1],&Bounds[0],WALL_EXTERIOR, 2.0f, UOffset);
		AddWall(pNewWall);
	}
	CreateFRMesh();
*/
}

Region::~Region()
{
	if(Verts)
	{
		delete[] Verts;
	}
	if(DrawList)
	{
		delete[] DrawList;
	}
	if(SubRegions)
	{
		delete SubRegions;
	}
	TextureList *pTL;

	pTL = TextureWalls;
	while(TextureWalls)
	{
		pTL = TextureWalls->pNext;
		delete TextureWalls;
		TextureWalls = pTL;
	}

	Object *pObject, *pLastObject;
	pLastObject = pObject = pObjectList;
	while(pObject)
	{
		pLastObject = pObject;
		if(pObject == pObjectListEnd)
		{
			pObject = NULL;
		}
		else
		{
			pObject = pObject->GetNext();
		}
		delete pLastObject;
	}
	pObjectList = NULL;
//	Valley->RemoveRegion(ID);
}

void Region::UnDraw()
{
	Drawn = FALSE;
	if(SubRegions)
	{
		SubRegions->UnDraw();
	}
	if(pNext)
	{
		pNext->UnDraw();
	}
}


int Region::Draw()
{
	HRESULT hr;
			
	if(!Drawn)
	{
		Drawn = TRUE;
				
		if(SubRegions)
		{
			if(!SubRegions->Draw())
			{
				if(PreludeParty.Inside())
				{
					return TRUE;
				}

				//draw the roof
				if(Verts && Occupied)
				{
					Engine->Graphics()->SetTexture(pFRTexture);
					HRESULT hr;
					hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, Verts, NumVerts, DrawList, NumTriangles, 0);
					if(FAILED(hr))
					{
						DEBUG_INFO("Failed to draw Region FR\n");
						Engine->ReportError(hr);
					}
					if(NumBorderTriangles)
					{
						Engine->Graphics()->SetTexture(pBorderTexture);
						hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, Verts, NumVerts, &DrawList[NumTriangles], NumBorderTriangles, 0);
						if(FAILED(hr))
						{
							DEBUG_INFO("Failed to draw Region Border\n");
							Engine->ReportError(hr);
						}
					}
				}

			}

			TextureList *pList;

			pList = TextureWalls;

			WallSide *pWallSide;

			while(pList)
			{
				Engine->Graphics()->SetTexture(pList->pTexture);
				
				pWallSide = pList->pWall;
				
				if(pWallSide)
					pWallSide->Draw();
				pList = pList->pNext;
			}

			//this handles transparency

			if(Walls)
			{
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
				Engine->Graphics()->ClearTexture();
				Walls->Draw();
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
			}
		
		}
		else
		{
			if(Verts && Occupied)
			{
				Engine->Graphics()->SetTexture(pFRTexture);
				hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, Verts, NumVerts, DrawList, NumTriangles, 0);
				if(FAILED(hr))
				{
					DEBUG_INFO("Failed to draw Region FR\n");
				}
				DrawObjects();
			}
			else
			if(GetType() != REGION_EXTERIOR)
			{
				Engine->Graphics()->ClearTexture();
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_BLACK));

				hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, Verts, NumVerts, DrawList, NumTriangles, 0);
				if(FAILED(hr))
				{
					DEBUG_INFO("Failed to draw Region FR\n");
				}
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
			}
			
			if(pNext)
			{
				if(pNext->Draw() || Occupied)
				{
					return TRUE;
				}
				else
				{
					return FALSE;
				}
			}
			return Occupied;
		}
	}
	
	return TRUE;
	
}

void Region::OutputDebugInfo(FILE *fp)
{
	fprintf(fp,"REGION ********************************\n");
	fprintf(fp,"Type: %i\n",Type);
	fprintf(fp,"Walls:\n");

/*	Wall *pWall;
	pWall = Walls;
	while(pWall)
	{
		pWall->OutPutDebugInfo(fp);
		pWall = pWall->GetNext();
	}
*/
	if(SubRegions)
	{

		SubRegions->OutputDebugInfo(fp);
	}

	if(pFRTexture)
	{
		fprintf(fp,"FRTexture: %s\n",pFRTexture->GetName());
	}
	
	if(pWallTexture)
	{
		fprintf(fp,"FRTexture: %s\n",pWallTexture->GetName());
	}
	
	fprintf(fp,"***************************************\n");

	if(pNext)
	{
		pNext->OutputDebugInfo(fp);
	}

}

void Region::FindCenter(D3DVECTOR *vCenter)
{
/*	ZeroMemory(vCenter,sizeof(D3DVECTOR));

	float Left = 0.0f;
	float Right = 0.0f;
	float Top = 0.0f;
	float Bottom = 0.0f;
	float ZLevel = 0.0f;
	D3DVECTOR vStart;

	Wall *pWall;

	pWall = Walls;

	if(pWall)
	{
		vStart = pWall->GetStart();
		Left = Right = vStart.x;
		Top = Bottom = vStart.y;
		ZLevel = vStart.z;
	}

	while(pWall)
	{
		vStart = pWall->GetStart();

		if(vStart.x < Left) { Left = vStart.x; }
		if(vStart.x > Right) { Right = vStart.x; }
		if(vStart.y < Top) { Top = vStart.y; }
		if(vStart.y > Bottom) { Bottom = vStart.y; }

		pWall = pWall->GetNext();
	}

	vCenter->x = (Left + Right) / 2.0f;
	vCenter->y = (Top + Bottom) / 2.0f;
	vCenter->z = ZLevel;
*/
	return;
}

//CreateDrawList
//assembles the walls of region into drawable form
void Region::CreateDrawList()
{
	Wall *pWall;

	pWall = Walls;

	while(pWall)
	{
		pWall->SetTransparent(FALSE);
		if(pWall->GetFront()->RegionFacing->IsOccupied() == REGION_OCCUPIED)
		{
			if(pWall->GetBack()->RegionFacing->IsOccupied() == REGION_OCCUPIED)
			{
				pWall->GetFront()->DrawMe = FALSE;
				pWall->GetBack()->DrawMe = FALSE;
				pWall->SetTransparent(TRUE);
			}
			else
			{
				pWall->GetFront()->DrawMe = TRUE;
			
				if(pWall->GetBack()->RegionFacing->IsOccupied())
				{
					pWall->GetBack()->DrawMe = TRUE;	
				}
				else
				{
					pWall->GetBack()->DrawMe = FALSE;
				}
			}
		}
		else
		{
			if(pWall->GetFront()->RegionFacing->IsOccupied())
			{
				pWall->GetFront()->DrawMe = TRUE;
			}
			else
			{
				pWall->GetFront()->DrawMe = FALSE;
			}
		
			if(pWall->GetBack()->RegionFacing->IsOccupied())
			{
				pWall->GetBack()->DrawMe = TRUE;
			}
			else
			{
				pWall->GetBack()->DrawMe = FALSE;
			}
		}
		pWall = pWall->GetNext();
	}

}

int Region::AddObject(Object *pAddObject)
{
	if(!pObjectList)
	{
		pObjectListEnd = pAddObject;
	}
	pAddObject->SetNext(pObjectList);
	pObjectList = pAddObject;
	NumObjects++;
	return TRUE;
}

void Region::DrawObjects()
{
	if(Occupied)
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
	else
	{
		if(SubRegions)
		{
			SubRegions->DrawObjects();
		}
	}
	if(pNext)
	{
		pNext->DrawObjects();
	}
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
}

void Region::SetOccupancy(REGION_VIEW_T NewOc)
{
	if(Occupied == NewOc) return;

	switch(Occupied)
	{
	case REGION_UNSEEN:
		switch(NewOc)
		{
		case REGION_UNSEEN:
			break;
		case REGION_SEEN:
			break;
		case REGION_OCCUPIED:
//check all regions which border this one and mark them as seen if they are not seen nor occupied			
			break;
		}
		break;
	case REGION_SEEN:
		switch(NewOc)
		{
		case REGION_UNSEEN:
			break;
		case REGION_SEEN:
			break;
		case REGION_OCCUPIED:
			//check all regions which border this one and mark them as seen if they are not seen nor occupied			
			break;
		}
		break;
	case REGION_OCCUPIED:
		switch(NewOc)
		{
		case REGION_UNSEEN:
//			Valley->RemoveFromDrawList(pObjectList, pObjectListEnd);
//			break;
		case REGION_SEEN:
			//check all regions which border this one and if they are not occupied make them unseen
//			Valley->RemoveFromDrawList(pObjectList, pObjectListEnd);
			break;
		case REGION_OCCUPIED:
			break;
		}		
		break;
	}

	Occupied = NewOc;

	SortView();

	if(Parent)
	{
		Parent->CreateDrawList();
	}
	else
	{
		CreateDrawList();
	}

}

void Region::Load(FILE *fp)
{
	fread(&ID, sizeof(ID), 1, fp);

	fread(&Type, sizeof(Type), 1, fp);

	//ignore parent 
	//Region *Parent;
	fread(&NumVerts, sizeof(int), 1, fp);
	Verts = new D3DVERTEX[NumVerts];
	fread(Verts, sizeof(D3DVERTEX), NumVerts, fp);

	fread(&NumTriangles, sizeof(int), 1, fp);
	fread(&NumBorderTriangles, sizeof(int), 1, fp);
	DrawList = new unsigned short[NumTriangles + NumBorderTriangles];
	fread(DrawList, sizeof(unsigned short), NumTriangles + NumBorderTriangles, fp);

	//texture numbers
	int TNum;
	fread(&TNum, sizeof(int), 1, fp);
	pFRTexture = Engine->GetTexture(TNum);

	fread(&TNum, sizeof(int), 1, fp);
	pBorderTexture = Engine->GetTexture(TNum);
	
	fread(&TNum, sizeof(int), 1, fp);
	pWallTexture = Engine->GetTexture(TNum);
	
	fread(&FRType, sizeof(FRType), 1, fp);
	
	fread(&Bounds, sizeof(Bounds), 1, fp);

	fread(&Height, sizeof(Height), 1, fp);
	fread(&FRHeight, sizeof(FRHeight), 1, fp);

	fread(&vCenter, sizeof(vCenter), 1, fp);

	Wall *pWall;

	BYTE Tester;

	Walls = NULL;

	do
	{
		fread(&Tester,sizeof(BYTE),1,fp);
		if(Tester)
		{
			pWall = new Wall;
			pWall->Load(fp);
			AddWall(pWall);
		}
	}while(Tester);

	Object *pOb;

	pObjectList = NULL;
	pOb = NULL;


	do
	{
		fread(&Tester,sizeof(BYTE),1,fp);
		if(Tester)
		{
			pOb = LoadObject(fp);
			AddObject(pOb);
		}
	}while(Tester);

	fread(&Tester,sizeof(BYTE),1,fp);
	if(Tester)
	{
		SubRegions = new Region;
		SubRegions->Parent = this;
		SubRegions->Load(fp);
	}
	else
	{
		SubRegions = NULL;
	}
	
	fread(&Tester,sizeof(BYTE),1,fp);
	if(Tester)
	{
		pNext = new Region;
		pNext->Parent = this->Parent;
		pNext->Load(fp);
	}
	else
	{
		pNext = NULL;
	}

	Occupied = REGION_UNSEEN;
	if(SubRegions)
	{
		Occupied = REGION_OCCUPIED;
		AssignWalls();
		//SortWalls();
		
		pOb = pObjectList;
		while(pOb)
		{
			if(pOb->GetObjectType() == OBJECT_PORTAL)
			{
				Portal *pPortal;
				pPortal = (Portal *)pOb;

				pPortal->SetRegionOne(GetSubRegion(pPortal->GetRegionOneNum()));
				pPortal->SetRegionTwo(GetSubRegion(pPortal->GetRegionTwoNum()));
			}
			pOb = pOb->GetNext();
		}

	}
}

void Region::Save(FILE *fp)
{
	fwrite(&ID, sizeof(ID), 1, fp);

	fwrite(&Type, sizeof(Type), 1, fp);

	//ignore parent 
	//Region *Parent;
	fwrite(&NumVerts, sizeof(int), 1, fp);
	fwrite(Verts, sizeof(D3DVERTEX), NumVerts, fp);

	fwrite(&NumTriangles, sizeof(int), 1, fp);
	fwrite(&NumBorderTriangles, sizeof(int), 1, fp);
	fwrite(DrawList, sizeof(unsigned short), NumTriangles + NumBorderTriangles, fp);
	
	//texture numbers
	int TNum;
	TNum = Engine->GetTextureNum(pFRTexture);
	fwrite(&TNum, sizeof(int), 1, fp);
	
	TNum = Engine->GetTextureNum(pBorderTexture);
	fwrite(&TNum, sizeof(int), 1, fp);
	
	TNum = Engine->GetTextureNum(pWallTexture);
	fwrite(&TNum, sizeof(int), 1, fp);
	
	fwrite(&FRType, sizeof(FRType), 1, fp);
	
	fwrite(&Bounds, sizeof(Bounds), 1, fp);

	fwrite(&Height, sizeof(Height), 1, fp);
	fwrite(&FRHeight, sizeof(FRHeight), 1, fp);

	fwrite(&vCenter, sizeof(vCenter), 1, fp);

	Wall *pWall;

	pWall = Walls;
	BYTE True = 1;
	BYTE False = 0;
	while(pWall)
	{
		fwrite(&True,sizeof(BYTE),1,fp);
		pWall->Save(fp);
		pWall = pWall->GetNext();
	}
	fwrite(&False,sizeof(BYTE),1,fp);

	Object *pOb;

	pOb = pObjectList;

	while(pOb)
	{
		fwrite(&True,sizeof(BYTE),1,fp);
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
	fwrite(&False,sizeof(BYTE),1,fp);

	if(SubRegions)
	{
		fwrite(&True,sizeof(BYTE),1,fp);
		SubRegions->Save(fp);
	}
	else
	{
		fwrite(&False,sizeof(BYTE),1,fp);
	}
	
	if(pNext)
	{
		fwrite(&True,sizeof(BYTE),1,fp);
		pNext->Save(fp);
	}
	else
	{
		fwrite(&False,sizeof(BYTE),1,fp);
	}

}

Region *Region::GetSubRegion(int FindID)
{
	if(!FindID) 
		return this;
	Region *pRegion;
	pRegion = SubRegions;
	while(pRegion)
	{
		if(pRegion->ID == FindID)
		{
			return pRegion;
		}
		pRegion = pRegion->GetNext();
	}
	return NULL;
}

void Region::AssignWalls()
{
	Wall *pWall;
	pWall = Walls;

	WallSide *pWS;
	while(pWall)
	{
		pWS = pWall->GetBack();
		if(pWS)
		{
			pWS->RegionFacing = GetSubRegion(pWS->RegionNum);
		}
		pWS = pWall->GetFront();
		if(pWS)
		{
			pWS->RegionFacing = GetSubRegion(pWS->RegionNum);
		}
		pWall = pWall->GetNext();
	}
}

void Region::SortView()
{
	Portal *pPortal;
	D3DVECTOR vCenter;
	//only primary regions sort viewpoints
	if(Parent)
	{
		Parent->SortView();
	}
	else
	{
		Wall *pWall;
		pWall = Walls;
		while(pWall)
		{
			if(pWall->GetType() == WALL_DOOR)
			{
				if(pWall->GetFront()->RegionFacing->IsOccupied() &&
					!pWall->GetBack()->RegionFacing->IsOccupied())
				{
					pWall->GetCenter(&vCenter);
					pPortal = (Portal *)Valley->FindObject((int)vCenter.x,(int)vCenter.y,OBJECT_PORTAL);
					if(!pPortal || pPortal->GetState() == PORTAL_OPEN || pPortal->GetState() == PORTAL_OPENNING)
					{
						pWall->GetBack()->RegionFacing->SetOccupancy(REGION_SEEN);
					}
				}
				else
				if(pWall->GetBack()->RegionFacing->IsOccupied() &&
					!pWall->GetFront()->RegionFacing->IsOccupied())
				{
					pWall->GetCenter(&vCenter);
					pPortal = (Portal *)Valley->FindObject((int)vCenter.x,(int)vCenter.y,OBJECT_PORTAL);
					if(!pPortal || pPortal->GetState() == PORTAL_OPEN || pPortal->GetState() == PORTAL_OPENNING)
					{
						pWall->GetFront()->RegionFacing->SetOccupancy(REGION_SEEN);
					}
				}
			}
			else
			if(pWall->GetType() == WALL_OPEN)
			{
				if(pWall->GetFront()->RegionFacing->IsOccupied() ||
					pWall->GetBack()->RegionFacing->IsOccupied())
				{
					pWall->GetBack()->RegionFacing->SetOccupancy(REGION_OCCUPIED);
					pWall->GetFront()->RegionFacing->SetOccupancy(REGION_OCCUPIED);
				}
			}
			pWall = pWall->GetNext();
		}
	}
}


Region *Region::PointIntersect(D3DVECTOR *vAt)
{
	int AtX;
	int AtY;

	AtX = (int)vAt->x;
	AtY = (int)vAt->y;

	//first check bounding box;
	if(AtX < Bounds.left || AtX > Bounds.right || AtY < Bounds.top || AtY > Bounds.bottom)
	{
		if(pNext)
		{
			return pNext->PointIntersect(vAt);
		}
		else
		{
			return NULL;
		}
	}
	
	//we're inside the bounding box

	if(SubRegions)
	{
		//now we have to check individual intersections by checking the floor triangles of the region.
		return SubRegions->PointIntersect(vAt);
	}
	else
	{
		//check the actual floor triangles
		for(int n = 0; n < NumTriangles; n+= 3)
		{
			if(Triangle2DIntersect(vAt,Verts,&DrawList[n]))
			{
				return this;
			}
		}
		
		if(pNext)
		{
			return pNext->PointIntersect(vAt);
		}
		else
		{
			return NULL;
		}
	}	
}

Wall *Region::GetWall(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{

	return NULL;
}

WallSide *Region::GetWallSide(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	Wall *pWall;
	pWall = Walls;
	WallSide *pWallSide = NULL;
	while(pWall)
	{
		pWallSide = pWall->RayIntersect(vRayStart,vRayEnd);
		if(pWallSide)
		{
			return pWallSide;
		}
		pWall = pWall->GetNext();
	}
	return NULL;
}

Region *Region::GetRoom(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	return NULL;
}

Region *Region::GetFloor(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{	
	if(!Parent)
	{
		return SubRegions->GetFloor(vRayStart,vRayEnd);
	}
	else
	{
		//check our floor
		//success return us
		D3DVERTEX vA,vB,vC;
		D3DVECTOR vRay;
		vRay = *vRayEnd - *vRayStart;

		//check front facing
		//is the front facing the camera
		int n;
		BOOL Found = FALSE;
		for(n = 0; n < NumTriangles; n += 3)
		{	
			vA = Verts[DrawList[n]];
			vB = Verts[DrawList[n+1]];
			vC = Verts[DrawList[n+2]];
			if(Triangle3DIntersect(vRayStart, vRayEnd, &vA, &vB, &vC))
			{
				Found = TRUE;
			}
		}
	
		if(Found)
		{
			return this;
		}
		else
		{
			if(pNext)
			{
				return pNext->GetFloor(vRayStart,vRayEnd);
			}
			else
			{
				//if we have no siblings return false
			return NULL;
			}
		}
	}
	return NULL;
}

Region *Region::GetRoof(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	return NULL;
}

void Region::SortWalls()
{
	TextureList *pTL;

	pTL = TextureWalls;
	while(TextureWalls)
	{
		pTL = TextureWalls->pNext;
		delete TextureWalls;
		TextureWalls = pTL;
	}

	Wall *pWall;
	pWall = Walls;

	while(pWall)
	{
		AddWallSide(pWall->GetFront());
		AddWallSide(pWall->GetBack());
		pWall = pWall->GetNext();
	}
}

void Region::Devolve(EditRegion *pER)
{

	Wall *pWall;
	pWall = Walls;
	
	D3DVECTOR vA;
	EditPoint *pA, *pB;
	EditWall *pEWall;

	while(pWall)
	{
		pWall->GetStart(&vA);
		vA.x = (int)vA.x + 0.5f;
		vA.y = (int)vA.y + 0.5f;
		vA.z = Valley->GetZ(vA.x,vA.y);

		pA = pER->FindPoint(&vA);
		if(!pA)
		{
			pA = new EditPoint(&vA);
			pER->AddPoint(pA);
		}

		pWall->GetEnd(&vA);
		vA.x = (int)vA.x + 0.5f;
		vA.y = (int)vA.y + 0.5f;
		vA.z = Valley->GetZ(vA.x,vA.y);

		pB = pER->FindPoint(&vA);
		if(!pB)
		{
			pB = new EditPoint(&vA);
			pER->AddPoint(pB);
		}
		
		pEWall = new EditWall(pA,pB);

		pER->AddWall(pEWall);

		pWall = pWall->GetNext();
	}

	pER->CreateInterior();

	return;

}

void Region::ScaleFloor(float Amount)
{
	int n;
	for(n = 0; n < NumVerts; n++)
	{
		Verts[n].tu = Verts[n].x * Amount;
		Verts[n].tv = Verts[n].y * Amount;
	}
}

void Region::ScaleRoof(float Amount)
{
	int n;
	for(n = 0; n < NumVerts -  ((NumBorderTriangles / 6) * 4); n++)
	{
		Verts[n].tu = Verts[n].tu * Amount;
		Verts[n].tv = Verts[n].tv * Amount;
	}
}

void Region::RotateFloor(float Angle)
{
	D3DXVECTOR4 vOut;
	D3DXMATRIX mRotate;
	D3DXVECTOR3	vIn;
	
	D3DXMatrixRotationZ(&mRotate,Angle);

	float Scale;
	Scale = Verts[0].tu / Verts[0].x;

	int n;
	for(n = 0; n < NumVerts; n++)
	{
		vIn.x = Verts[n].x - vCenter.x;
		vIn.y = Verts[n].y - vCenter.y;
		vIn.z = Verts[n].z - vCenter.z;
		D3DXVec3Transform(&vOut,(D3DXVECTOR3 *)&vIn,&mRotate);
		Verts[n].tu = vOut.x * Scale;
		Verts[n].tv = vOut.y * Scale;
	}
}

void Region::UnOccupy()
{
	if(Parent)
	{
		DEBUG_INFO("Can only unoccupy primay regions\n");
		exit(1);
	}
	Occupied = REGION_SEEN;

	Region *pRegion;
	pRegion = SubRegions;
	while(pRegion)
	{
		pRegion->Occupied = REGION_UNSEEN;
		pRegion = pRegion->GetNext();
	}

	SortView();
}

void Region::PaintInterior(ZSTexture *pText, float TileAmount)
{
	if(Parent)
	{
		Parent->PaintInterior(pText, TileAmount);
		return;
	}

	Wall *pWall;
	WallSide *pWallSide;
	pWall = Walls;
	while(pWall)
	{
		pWallSide = pWall->GetFront();
		if(pWallSide->RegionFacing->GetType() == REGION_INTERIOR)
		{
			pWallSide->pTexture = pText;
			pWallSide->Tile(TileAmount);
		}

		pWallSide = pWall->GetBack();
		if(pWallSide->RegionFacing->GetType() == REGION_INTERIOR)
		{
			pWallSide->pTexture = pText;
			pWallSide->Tile(TileAmount);
		}

		pWall = pWall->GetNext();
	}

	SortWalls();
}

void Region::PaintExterior(ZSTexture *pText, float TileAmount)
{
	if(Parent)
	{
		Parent->PaintExterior(pText, TileAmount);
		return;
	}

	Wall *pWall;
	WallSide *pWallSide;
	pWall = Walls;
	while(pWall)
	{
		pWallSide = pWall->GetFront();
		if(pWallSide->RegionFacing->GetType() == REGION_EXTERIOR)
		{
			pWallSide->pTexture = pText;
			pWallSide->Tile(TileAmount);
		}

		pWallSide = pWall->GetBack();
		if(pWallSide->RegionFacing->GetType() == REGION_EXTERIOR)
		{
			pWallSide->pTexture = pText;
			pWallSide->Tile(TileAmount);
		}

		pWall = pWall->GetNext();
	}

	SortWalls();
}

void Region::PaintFloors(ZSTexture *pText)
{
	if(Parent)
	{
		Parent->PaintFloors(pText);
		return;
	}

	Region *pRegion;
	pRegion = SubRegions;

	while(pRegion)
	{
		pRegion->SetFRTexture(pText);

		pRegion = pRegion->GetNext();
	}
}

void Region::SetFRTexture(ZSTexture *pNT, ZSTexture *pBT)
{ 
	if(pNT)
		pFRTexture = pNT; 
	if(!pBT)
	{
		pBorderTexture = Engine->GetTexture(Engine->GetTextureNum(pNT) + 1);
	}
	else
	{
		pBorderTexture = pBT;
	}
};

void Region::MakeBounds()
{
	float fLeft;
	float fRight;
	float fTop;
	float fBottom;
	
	fLeft = fRight = Verts[0].x;
	fTop = fBottom = Verts[0].y;

	for(int n = 0; n < NumVerts; n++)
	{

		if(Verts[n].x < fLeft) fLeft = Verts[n].x;
		if(Verts[n].x > fRight) fRight = Verts[n].x;
		if(Verts[n].y < fTop) fTop = Verts[n].y;
		if(Verts[n].y > fBottom) fBottom = Verts[n].y;
	}

	Bounds.left = (int)fLeft;
	Bounds.right = (int)(fRight + 0.5f);
	Bounds.bottom = (int)(fBottom + 0.5f);
	Bounds.top = (int)fTop;
}

void Region::UnGatherObjects()
{
	Region *pRegion;
	pRegion = SubRegions;
	Object *pOb;
	
	while(pRegion)
	{
		while(pRegion->pObjectList)
		{
			pOb = pRegion->pObjectList;
			pRegion->pObjectList = pRegion->pObjectList->GetNext();
			Valley->AddObject(pOb);
			pRegion->NumObjects--;
		}
		pRegion->NumObjects = 0;	
		pRegion->pObjectListEnd = NULL;
		pRegion = pRegion->GetNext();
	}
}

void Region::GatherObjects()
{
	int xn,yn;
	int xStart,yStart,xEnd,yEnd;

	xStart = Bounds.left / CHUNK_TILE_WIDTH;
	xEnd = Bounds.right / CHUNK_TILE_WIDTH;
	yStart = Bounds.top / CHUNK_TILE_HEIGHT;
	yEnd = Bounds.bottom / CHUNK_TILE_HEIGHT;

	for(yn = yStart; yn <= yEnd; yn++)
	for(xn = xStart; xn <= xEnd; xn++)
	{
		Chunk *pChunk;
		pChunk = Valley->GetChunk(xn,yn);
		if(!pChunk)
		{
			Valley->LoadChunk(xn,yn);
			pChunk = Valley->GetChunk(xn,yn);
		}
		Object *pOb;
		pOb = pChunk->GetObjects();

		while(pOb)
		{
			D3DVECTOR *pPosition;
			pPosition = pOb->GetPosition();
			Region *pRegion;
			pRegion = PointIntersect(pPosition);
			if(pRegion)
			{
				pChunk->RemoveObject(pOb);
				pRegion->AddObject(pOb);
				pOb = pChunk->GetObjects();
			}
			else
			{
				pOb = pOb->GetNext();
			}
		}
	}
}

void Region::RemoveObject(Object *pToRemove)
{
	Object *pOb, *pLOb;

	pOb = pObjectList;

	if(pObjectList == pToRemove)
	{
		pObjectList = pOb->GetNext();
		return;
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
			return;
		}
		pLOb = pOb;
		pOb = pOb->GetNext();
	}

	return;
}

void Region::Move(D3DVECTOR *vMoveRay)
{
	Wall *pWall;
	pWall = Walls;
	while(pWall)
	{
		pWall->Move(vMoveRay);
		pWall = pWall->GetNext();
	}

	int n;

	for(n = 0; n < NumVerts; n++)
	{
		Verts[n].x += vMoveRay->x;
		Verts[n].y += vMoveRay->y;
		Verts[n].z += vMoveRay->z;
	}

	Object *pObject;
	pObject = pObjectList;

	while(pObject)
	{
		pObject->Move(vMoveRay);
		pObject = pObject->GetNext();
	}

	Region *pRegion;
	pRegion = SubRegions;
	while(pRegion)
	{
		pRegion->Move(vMoveRay);
		pRegion = pRegion->GetNext();
	}
}

BOOL Region::LineIntersect(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd)
{
	if(Parent)
	{
		return Parent->LineIntersect(vLineStart,vLineEnd);
	}

	if(LOSChecked)
		return FALSE;
	
	LOSChecked = TRUE;

	Wall *pWall;
	pWall = Walls;
	while(pWall)
	{
		if(pWall->LineIntersect(vLineStart, vLineEnd))
		{
			return TRUE;
		}
		pWall = pWall->GetNext();
	}

	return FALSE;
}
	
