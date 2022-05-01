#include "editregion.h"
#include "zsengine.h"
#include "zsutilities.h"
#include "zsMessage.h"
#include "world.h"
#include "portals.h"
#include "assert.h"

#define DOOR_HEIGHT 3.60f
#define DOOR_WIDTH 2.0f
#define WALL_WIDTH 0.2f
#define POINT_TOLERANCE	0.3f

EditWall *ToDrawWall = NULL;
EditWall *ToDrawNext = NULL;
EditRoom *ToDrawRoom = NULL;
EditPoint *ToDrawPoint = NULL;

unsigned short BaseDrawList[6] = { 0,1,2,0,2,3 };
unsigned short InvertDrawList[6] = { 3,2,0,2,1,0 };

float EditGetAngle(D3DVECTOR *v1, D3DVECTOR *v2)
{
	float DegAngle, Angle;

	D3DVECTOR vNorth(0.0f,1.0f,0.0f);
	float AngleA;
	float AngleB;

	AngleA = acos(DotProduct(vNorth,*v1)/Magnitude(*v1));
	AngleB = acos(DotProduct(vNorth,*v2)/Magnitude(*v2));

	if(v1->x < 0.0f) AngleA = PI_MUL_2 - AngleA;
	if(v2->x < 0.0f) AngleB = PI_MUL_2 - AngleB;
	 
	Angle = AngleB - AngleA;

	if(Angle < 0)
	{
		Angle = PI_MUL_2 + Angle;
	}
	
	DegAngle = RadToDeg(Angle);

	return Angle;
}

float GetWallAngle(EditWall *A, EditWall *B)
{
	D3DVECTOR v1;
	D3DVECTOR v2;

	if(A->Start == B->Start)
	{
		v1 = A->End->Location - A->Start->Location;	
		v2 = B->End->Location - B->Start->Location;
	}
	else
	if(A->End == B->Start)
	{
		v1 = A->Start->Location - A->End->Location;	
		v2 = B->End->Location - B->Start->Location;
	}
	else
	if(A->Start == B->End)
	{
		v1 = A->End->Location - A->Start->Location;	
		v2 = B->Start->Location - B->End->Location;
	}
	else
	if(A->End == B->End)
	{
		v1 = A->Start->Location - A->End->Location;	
		v2 = B->Start->Location - B->End->Location;
	}

	

	return GetAngle(&v1,&v2);
}

float EditWall::GetWallAngle()
{
	D3DVECTOR vNorth(0.0f,1.0f,0.0f);
	D3DVECTOR vMe;
	vMe = End->Location - Start->Location;

	return GetAngle(&vNorth,&vMe);
}

DIRECTION_T EditWall::GetOrientation()
{
/*	D3DVECTOR vNorth(0.0f,1.0f,0.0f);
	D3DVECTOR vMe;
	vMe = End->Location - Start->Location;

	return FindFacing(&vNorth,&vMe);
*/

	if(End->Location.x - Start->Location.x)
	{
		if(End->Location.y - Start->Location.y)
		{
			if(End->Location.y > Start->Location.y)
			{
				if(End->Location.x > Start->Location.x)
				{
					return SOUTHEAST;
				}
				else
				{
					return SOUTHWEST;
				}
			}
			else
			{
				if(End->Location.x > Start->Location.x)
				{
					return NORTHEAST;
				}
				else
				{
					return NORTHWEST;
				}
			}
		}
		else
		{
			if(End->Location.x > Start->Location.x)
			{
				return EAST;
			}
			else
			{	
				return WEST;
			}
		}
	}
	else
	{
		if(End->Location.y > Start->Location.y)
		{
			return SOUTH;
		}
		else
		{
			return NORTH;
		}
	}
	return DIR_NONE;
}

BOOL EditWall::CanDoor()
{
	float Angle;
	Angle = GetWallAngle();

	if(Angle == 0.0f ||
		Angle == PI_DIV_2 ||
		Angle == PI_DIV_4 ||
		Angle == PI_DIV_2 + PI_DIV_4 ||
		Angle == PI)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}

}

void EditPoint::Draw()
{
	Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), Location.x, Location.y, Location.z, 1.0, 0);
}

EditPoint::EditPoint()
{
	Location.x = 0.0f;
	Location.y = 0.0f;
	Location.z = 0.0f;

	pNext = NULL;
	Touched = FALSE;
	ID = -1;
}

EditPoint::EditPoint(D3DVECTOR *pVector)
{
	Location = *pVector;
	pNext = NULL;
}

void EditRegion::DivideWall(EditWall *ToDivide, D3DVECTOR *vAt)
{
	if(FindPoint(vAt))
	{
		return;
	}

	RemoveWall(ToDivide);

	EditWall *NewWall;

	EditPoint *pPoint;

	pPoint = new EditPoint(vAt);
	
	AddPoint(pPoint);

	NewWall = new EditWall(ToDivide->Start,pPoint,WallHeight);
	
	AddWall(NewWall);

	NewWall = new EditWall(pPoint,ToDivide->End, WallHeight);

	AddWall(NewWall);

	ToDivide->Start = NULL;
	ToDivide->End = NULL;

	delete ToDivide;
	ToDivide = NULL;
	
}

void EditWall::CreateMesh()
{
	if(!Here) return;
	
	if(Here->Front)
	{
		delete Here->Front;
	}
	Here->Front = new WallSide;
	
	if(Here->Back)
	{
		delete Here->Back;
	}
	Here->Back = new WallSide;
	
	Here->Front->RegionFacing = pRoomRight->Here;
	Here->Back->RegionFacing = pRoomLeft->Here;

	//allocate space for the new arrays
	Here->Front->Verts = new D3DVERTEX[4];
	Here->Back->Verts = new D3DVERTEX[4];
	
	Here->Front->DrawList = new unsigned short[6];
	Here->Back->DrawList = new unsigned short[6];
	
	//basic walls all have 6 draw coordinates
	Here->Front->NumTriangles = 6;
	Here->Back->NumTriangles = 6;
	
	//basic walls all have 4 vertices
	Here->Front->NumVerts = 4;
	Here->Back->NumVerts = 4;

	//basic draw list
	D3DVECTOR vA, vB;
	float Angle;
	//align front ang back so that they are drawn properly
	//first check to see if front is an exterior facing
	if(Here->Front->RegionFacing->GetType() == REGION_EXTERIOR)
	{
		vA = Start->Location - Here->Back->RegionFacing->GetCenter();
		vB = End->Location - Here->Back->RegionFacing->GetCenter();
		Angle = RadToDeg(EditGetAngle(&vB,&vA));	
		if(Angle > 180.0f)
		{
			memcpy(Here->Front->DrawList,&BaseDrawList,sizeof(unsigned short) * 6);
		}
		else
		{
			memcpy(Here->Front->DrawList,&InvertDrawList,sizeof(unsigned short) * 6);
		}
	}
	else
	{
		vA = Start->Location - Here->Front->RegionFacing->GetCenter();
		vB = End->Location - Here->Front->RegionFacing->GetCenter();
		Angle = RadToDeg(EditGetAngle(&vB,&vA));
		if(Angle < 180.1f)
		{
			memcpy(Here->Front->DrawList,&BaseDrawList,sizeof(unsigned short) * 6);
		}
		else
		{
			memcpy(Here->Front->DrawList,&InvertDrawList,sizeof(unsigned short) * 6);
		}
	}
	
	
	if(Here->Back->RegionFacing->GetType() == REGION_EXTERIOR)
	{
		vA = Start->Location - Here->Front->RegionFacing->GetCenter();
		vB = End->Location - Here->Front->RegionFacing->GetCenter();
		Angle = RadToDeg(EditGetAngle(&vB,&vA));
		if(Angle > 180.0f)
		{
			memcpy(Here->Back->DrawList,&BaseDrawList,sizeof(unsigned short) * 6);
		}
		else
		{
			memcpy(Here->Back->DrawList,&InvertDrawList,sizeof(unsigned short) * 6);
		}
	}
	else
	{
		vA = Start->Location - Here->Back->RegionFacing->GetCenter();
		vB = End->Location - Here->Back->RegionFacing->GetCenter();
		Angle = RadToDeg(EditGetAngle(&vB,&vA));
		if(Angle < 180.1f)
		{
			memcpy(Here->Back->DrawList,&BaseDrawList,sizeof(unsigned short) * 6);
		}
		else
		{
			memcpy(Here->Back->DrawList,&InvertDrawList,sizeof(unsigned short) * 6);
		}
	
	}

	D3DVECTOR FrontStart, FrontEnd;
	D3DVECTOR BackStart, BackEnd;
	D3DVECTOR vCenter;
	D3DVECTOR vRay;
	float RayLength;
	FrontStart = BackStart = Start->Location;
	FrontEnd = BackEnd = End->Location;

	if(Here->Front->RegionFacing->GetType() == REGION_INTERIOR)
	{
		vCenter = Here->Front->RegionFacing->GetCenter();
		vRay = FrontStart - vCenter;
		RayLength = Magnitude(vRay);
		vRay = Normalize(vRay);
		RayLength -= WALL_WIDTH;
		vRay = vRay * RayLength;
		FrontStart = vRay + vCenter;
		
		vRay = FrontEnd - vCenter;
		RayLength = Magnitude(vRay);
		vRay = Normalize(vRay);
		RayLength -= WALL_WIDTH;
		vRay = vRay * RayLength;
		FrontEnd = vRay + vCenter;
	}

	if(Here->Back->RegionFacing->GetType() == REGION_INTERIOR)
	{
		vCenter = Here->Back->RegionFacing->GetCenter();
		vRay = BackStart - vCenter;
		RayLength = Magnitude(vRay);
		vRay = Normalize(vRay);
		RayLength -= WALL_WIDTH;
		vRay = vRay * RayLength;
		BackStart = vRay + vCenter;
		
		vRay = BackEnd - vCenter;
		RayLength = Magnitude(vRay);
		vRay = Normalize(vRay);
		RayLength -= WALL_WIDTH;
		vRay = vRay * RayLength;
		BackEnd = vRay + vCenter;
	}
	
	//set up coordinate info
	Here->Front->Verts[0].x = FrontStart.x;
	Here->Front->Verts[0].y = FrontStart.y;
	Here->Front->Verts[0].z = FrontStart.z;
	if(Type == EWALL_DOOR)
	{
		Here->Front->Verts[0].z += DOOR_HEIGHT;
	}

	Here->Front->Verts[1].x = FrontStart.x;
	Here->Front->Verts[1].y = FrontStart.y;
	Here->Front->Verts[1].z = FrontStart.z + Height;

	Here->Front->Verts[2].x = FrontEnd.x;
	Here->Front->Verts[2].y = FrontEnd.y;
	Here->Front->Verts[2].z = FrontEnd.z + Height;

	Here->Front->Verts[3].x = FrontEnd.x;
	Here->Front->Verts[3].y = FrontEnd.y;
	Here->Front->Verts[3].z = FrontEnd.z;
	if(Type == EWALL_DOOR)
	{
		Here->Front->Verts[3].z += DOOR_HEIGHT;
	}

	//set up coordinate info
	Here->Back->Verts[0].x = BackStart.x;
	Here->Back->Verts[0].y = BackStart.y;
	Here->Back->Verts[0].z = BackStart.z;
	if(Type == EWALL_DOOR)
	{
		Here->Back->Verts[0].z += DOOR_HEIGHT;
	}

	Here->Back->Verts[1].x = BackStart.x;
	Here->Back->Verts[1].y = BackStart.y;
	Here->Back->Verts[1].z = BackStart.z + Height;

	Here->Back->Verts[2].x = BackEnd.x;
	Here->Back->Verts[2].y = BackEnd.y;
	Here->Back->Verts[2].z = BackEnd.z + Height;

	Here->Back->Verts[3].x = BackEnd.x;
	Here->Back->Verts[3].y = BackEnd.y;
	Here->Back->Verts[3].z = BackEnd.z;
	if(Type == EWALL_DOOR)
	{
		Here->Back->Verts[3].z += DOOR_HEIGHT;
	}

	//get the normals
	//calculate the Surface Normal
	D3DVECTOR VA;
	D3DVECTOR VB;
	D3DVECTOR VFrontNormal;
	D3DVECTOR VBackNormal;
	VA.x = Here->Front->Verts[Here->Front->DrawList[0]].x - Here->Front->Verts[Here->Front->DrawList[1]].x;
	VA.y = Here->Front->Verts[Here->Front->DrawList[0]].y - Here->Front->Verts[Here->Front->DrawList[1]].y;
	VA.z = Here->Front->Verts[Here->Front->DrawList[0]].z- Here->Front->Verts[Here->Front->DrawList[1]].z;

	VB.x = Here->Front->Verts[Here->Front->DrawList[2]].x - Here->Front->Verts[Here->Front->DrawList[1]].x;
	VB.y = Here->Front->Verts[Here->Front->DrawList[2]].y - Here->Front->Verts[Here->Front->DrawList[1]].y;
	VB.z = Here->Front->Verts[Here->Front->DrawList[2]].z - Here->Front->Verts[Here->Front->DrawList[1]].z;

	VFrontNormal = Normalize(CrossProduct(VA,VB));
	
	for(int n = 0; n < 4; n ++)
	{
		if(Here->Front->RegionFacing->GetType() == REGION_EXTERIOR)
		{
			Here->Front->Verts[n].nx = VFrontNormal.x;
			Here->Front->Verts[n].ny = VFrontNormal.y;
			Here->Front->Verts[n].nz = VFrontNormal.z;
		}
		else
		{
			Here->Front->Verts[n].nx = 0.0f;
			Here->Front->Verts[n].ny = 0.0f;
			Here->Front->Verts[n].nz = 1.0f;
		}

		if(Here->Back->RegionFacing->GetType() == REGION_EXTERIOR)
		{
			Here->Back->Verts[n].nx = -VFrontNormal.x;
			Here->Back->Verts[n].ny = -VFrontNormal.y;
			Here->Back->Verts[n].nz = -VFrontNormal.z;
		}
		else
		{
			Here->Back->Verts[n].nx = 0.0f;
			Here->Back->Verts[n].ny = 0.0f;
			Here->Back->Verts[n].nz = 1.0f;
		}

//		Here->Back->Verts[n].nx = -(VFrontNormal.x);
//		Here->Back->Verts[n].ny = -(VFrontNormal.y);
//		Here->Back->Verts[n].nz = -(VFrontNormal.z);
	}
	
//setup the UV coordinates;

	float Length;

	Length = GetDistance(&Start->Location,&End->Location) / Height;

	Here->Front->Verts[0].tu = 0.0f;
	Here->Front->Verts[1].tv = 0.0f;
	if(Type == EWALL_DOOR)
	{
		Here->Front->Verts[1].tv = DOOR_HEIGHT / 4.0f;
	}

	Here->Front->Verts[1].tu = 0.0f;
	Here->Front->Verts[0].tv = 1.0f;//Height / 4.0f;

	Here->Front->Verts[2].tu = Length;
	Here->Front->Verts[3].tv = 1.0f;//Height / 4.0f;

	Here->Front->Verts[3].tu = Length;
	Here->Front->Verts[2].tv = 0.0f;
	if(Type == EWALL_DOOR)
	{
		Here->Front->Verts[2].tv = DOOR_HEIGHT / 4.0f;
	}

	Here->Back->Verts[0].tu = 0.0f;
	Here->Back->Verts[1].tv = 0.0f;
	if(Type == EWALL_DOOR)
	{
		Here->Back->Verts[1].tv = DOOR_HEIGHT / 4.0f;
	}

	Here->Back->Verts[1].tu = 0.0f;
	Here->Back->Verts[0].tv = 1.0f;//Height / 4.0f;

	Here->Back->Verts[2].tu = Length;
	Here->Back->Verts[3].tv = 1.0f;//Height / 4.0f;

	Here->Back->Verts[3].tu = Length;
	Here->Back->Verts[2].tv = 0.0f;
	if(Type == EWALL_DOOR)
	{
		Here->Back->Verts[2].tv = DOOR_HEIGHT / 4.0f;
	}



	//setup the tob and bottom verts
	Here->TBVerts[0].x = FrontStart.x;
	Here->TBVerts[0].y = FrontStart.y;
	Here->TBVerts[0].z = FrontStart.z;
	
	Here->TBVerts[1].x = BackStart.x;
	Here->TBVerts[1].y = BackStart.y;
	Here->TBVerts[1].z = BackStart.z;

	Here->TBVerts[2].x = FrontEnd.x;
	Here->TBVerts[2].y = FrontEnd.y;
	Here->TBVerts[2].z = FrontEnd.z;

	Here->TBVerts[3].x = BackEnd.x;
	Here->TBVerts[3].y = BackEnd.y;
	Here->TBVerts[3].z = BackEnd.z;

	Here->TBVerts[4].x = FrontStart.x;
	Here->TBVerts[4].y = FrontStart.y;
	Here->TBVerts[4].z = FrontStart.z + Height;
	
	Here->TBVerts[5].x = BackStart.x;
	Here->TBVerts[5].y = BackStart.y;
	Here->TBVerts[5].z = BackStart.z + Height;

	Here->TBVerts[6].x = FrontEnd.x;
	Here->TBVerts[6].y = FrontEnd.y;
	Here->TBVerts[6].z = FrontEnd.z + Height;

	Here->TBVerts[7].x = BackEnd.x;
	Here->TBVerts[7].y = BackEnd.y;
	Here->TBVerts[7].z = BackEnd.z + Height;

	for(n = 0; n < 8; n++)
	{
		Here->TBVerts[n].color = D3DRGBA(0.0f,0.0f,0.0f, 1.0f);
		Here->TBVerts[n].specular = D3DRGBA(0.0f,0.0f,0.0f, 1.0f);
		Here->TBVerts[n].tu = 0.0f;
		Here->TBVerts[n].tv = 0.0f;
	}

	//done
}



void EditWall::Draw()
{
	if(!Start || !End) return;
	D3DLVERTEX Verts[5];

	Verts[4].x = Verts[0].x = Start->Location.x;
	Verts[4].y = Verts[0].y = Start->Location.y;
	Verts[4].z = Verts[0].z = Start->Location.z + Height;
	Verts[4].color = Verts[0].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[4].specular = Verts[0].specular = D3DRGB(1.0f,1.0f,1.0f);

	Verts[1].x = End->Location.x;
	Verts[1].y = End->Location.y;
	Verts[1].z = End->Location.z + Height;
	Verts[1].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[1].specular = D3DRGB(1.0f,1.0f,1.0f);

	Verts[3].x = Start->Location.x;
	Verts[3].y = Start->Location.y;
	Verts[3].z = Start->Location.z;
	if(Type == EWALL_DOOR)
	{
		Verts[3].z += DOOR_HEIGHT;
	}
	Verts[3].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[3].specular = D3DRGB(1.0f,1.0f,1.0f);

	Verts[2].x = End->Location.x;
	Verts[2].y = End->Location.y;
	Verts[2].z = End->Location.z;
	if(Type == EWALL_DOOR)
	{
		Verts[2].z += DOOR_HEIGHT;
	}
	Verts[2].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[2].specular = D3DRGB(1.0f,1.0f,1.0f);

	HRESULT hr;
	hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_LVERTEX, Verts, 5, 0 );
	if(hr != D3D_OK)
	{
		SafeExit("problem drawing edit wall\n");
	}
}

float EditRoom::SumAngles()
{
	AngleTotal = 0;
	for(int n = 0; n < NumWalls -1; n++)
	{
		AngleTotal += RadToDeg(GetWallAngle(Walls[n+1],Walls[n]));
	}
	AngleTotal += RadToDeg(GetWallAngle(Walls[0],Walls[n]));
	return AngleTotal;
}

EditRoom *EditWall::GetRight(EditPoint *pFrom)
{
	if(pFrom == Start)
	{
		return pRoomRight;
	}
	else
	{
		return pRoomLeft;
	}
}

EditRoom *EditWall::GetLeft(EditPoint *pFrom)
{
	if(pFrom == Start)
	{
		return pRoomLeft;
	}
	else
	{
		return pRoomRight;
	}
}

void EditWall::SetLeft(EditPoint *pFrom, EditRoom *pRoom)
{	
	if(pFrom == Start)
	{
		pRoomLeft = pRoom;
	}
	else
	{
		pRoomRight = pRoom;
	}

}


void EditWall::SetRight(EditPoint *pFrom, EditRoom *pRoom)
{
	if(pFrom == Start)
	{
		pRoomRight = pRoom;
	}
	else
	{
		pRoomLeft = pRoom;
	}
}

EditPoint *EditWall::GetEnd(EditPoint *pFrom)
{
	if(pFrom == Start)
	{
		return End;
	}
	else
	{
		return Start;
	}
}

EditWall::EditWall()
{
	Start = NULL;
	End = NULL;
	Height = 4.0f;
	pNext = NULL;
	NumRefs = 0;
	Exterior = FALSE;
	ID = -1;
	pRoomRight = NULL;
	pRoomLeft = NULL;
}

EditWall::EditWall(EditPoint *vStart, EditPoint *vEnd, float StartHeight)
{
	Start = vStart;
	End = vEnd;
	Height = StartHeight;
	pNext = NULL;
	NumRefs = 0;
	Exterior = FALSE;
	pRoomRight = NULL;
	pRoomLeft = NULL;
}

void EditRoom::CreateFloor()
{
	D3DVERTEX VTemp[64];
	unsigned short TempDraw[128];
	EditPoint *PointList[64];

	memset(PointList,0,sizeof(EditPoint *) * 64);
	int n;
	int pn = 0;
	int sn;

	Here->SetCenter(FindCenter());
		
	//assemble all the vertices for the room;
	for(n = 0; n < NumWalls; n++)
	{
		BOOL FoundStart;
		BOOL FoundEnd;
		FoundStart = FALSE;
		FoundEnd = FALSE;
		for(sn = 0; sn < 64; sn++)
		{
			if(PointList[sn] == Walls[n]->Start)
			{
				FoundStart = TRUE;
			}
			if(PointList[sn] == Walls[n]->End)
			{
				FoundEnd = TRUE;
			}
		}
		if(!FoundStart)
		{
			PointList[pn++] = Walls[n]->Start;
		}
		if(!FoundEnd)
		{
			PointList[pn++] = Walls[n]->End;
		}
	}

	//sort points
	D3DVECTOR vA;
	D3DVECTOR vB;

	//arbitrary start point 0
	vA = PointList[0]->Location - Here->GetCenter();
	
	float LastAngle = 0.0f;
	float ThisAngle;
	EditPoint *pTemp;
	
	//bubble sort points
	for(n = 1; n < pn; n++)
	{
		vB = PointList[n]->Location - Here->GetCenter();
		if(vB == vA)
		{
			ThisAngle = 0.0f;
		}
		else
		{
			ThisAngle = EditGetAngle(&vA,&vB);
		}

		if(ThisAngle < LastAngle)
		{
			pTemp = PointList[n - 1];
			PointList[n-1] = PointList[n];
			PointList[n] = pTemp;
			n = 0; //after loop N will be incremented to 1
			LastAngle = 0.0f;
		}
		else
		{
			LastAngle = ThisAngle;
		}
	}

	//create the floor texture
	Here->SetNumVerts(pn);
	for(n = 0; n < pn; n++)
	{
		VTemp[n].x = PointList[n]->Location.x;
		VTemp[n].y = PointList[n]->Location.y;
		VTemp[n].z = PointList[n]->Location.z + 0.01f;
		VTemp[n].nx = 0;
		VTemp[n].ny = 0;
		VTemp[n].nz = 1.0f;
		VTemp[n].tu = (VTemp[n].x/2.0f);
		VTemp[n].tv = (VTemp[n].y/2.0f);
	}

	Here->NumTriangles = 0;
	for(n = 1; n < pn - 1; n++)
	{
		TempDraw[(n-1)*3] = 0;
		TempDraw[(n-1)*3 + 1] = n+1;
		TempDraw[(n-1)*3 + 2] = n;
		Here->NumTriangles+=3;
	}
	Here->Verts = new D3DVERTEX[Here->NumVerts];
	memcpy(Here->Verts, VTemp,sizeof(D3DVERTEX) * Here->NumVerts);
	Here->DrawList = new unsigned short[Here->NumTriangles];
	memcpy(Here->DrawList, TempDraw, sizeof(unsigned short) * Here->NumTriangles);
}

D3DVECTOR EditRoom::FindCenter()
{
	int n, sn;
	float Left;
	float Right;
	float Top;
	float Bottom;
	EditPoint *PointList[64];
	int pn = 0;

	memset(PointList,0,sizeof(EditPoint *) * 64);
	
	//assemble all the vertices for the room;
	for(n = 0; n < NumWalls; n++)
	{
		BOOL FoundStart;
		BOOL FoundEnd;
		FoundStart = FALSE;
		FoundEnd = FALSE;
		for(sn = 0; sn < 64; sn++)
		{
			if(PointList[sn] == Walls[n]->Start)
			{
				FoundStart = TRUE;
			}
			if(PointList[sn] == Walls[n]->End)
			{
				FoundEnd = TRUE;
			}
		}
		if(!FoundStart)
		{
			PointList[pn++] = Walls[n]->Start;
		}
		if(!FoundEnd)
		{
			PointList[pn++] = Walls[n]->End;
		}
	}

	Right = Left = PointList[0]->Location.x;
	Top = Bottom = PointList[0]->Location.y;

	for(n = 0; n < pn; n++)
	{
		if(PointList[n]->Location.x < Left)
		{
			Left = PointList[n]->Location.x;
		}
		if(PointList[n]->Location.x > Right)
		{
			Right = PointList[n]->Location.x;
		}
		if(PointList[n]->Location.y < Top)
		{
			Top = PointList[n]->Location.y;
		}
		if(PointList[n]->Location.y > Bottom)
		{
			Bottom = PointList[n]->Location.y;
		}
	}
	D3DVECTOR Center((Right + Left)/2,(Top + Bottom)/2, Walls[0]->End->Location.z);;


	return Center;
}

void EditRoom::CreateRoof(FLOORROOF_T FRType, float RoofHeight, float OverHang)
{
	static D3DVERTEX Temp[512];
	static unsigned short TempDraw[1024];
	static EditPoint *PointList[64];

	float ThisRoomHeight;
	float FRHeight = RoofHeight;

	D3DVECTOR vCenter;

	float Length;
	float LengthTotal;
	float FaceLength;
	float FaceHeight;
	float SideOneLength;
	float PointU;
	float Angle;
	int NumTriangles = 0;
	D3DVECTOR TempList[64];
	D3DVECTOR vA, vB, vC, vD, vSide1, vSide2, vNorm;  //a =left, b == point, vc = right

	Here->SetCenter(FindCenter());

	vCenter = Here->GetCenter();

	memset(PointList,0,sizeof(EditPoint *) * 64);
	int n, sn;
	int pn = 0;

	//assemble all the vertices for the room;
	for(n = 0; n < NumWalls; n++)
	{
		BOOL FoundStart;
		BOOL FoundEnd;
		FoundStart = FALSE;
		FoundEnd = FALSE;
		for( sn = 0; sn < 64; sn++)
		{
			if(PointList[sn] == Walls[n]->Start)
			{
				FoundStart = TRUE;
			}
			if(PointList[sn] == Walls[n]->End)
			{
				FoundEnd = TRUE;
			}
		}
		if(!FoundStart)
		{
			PointList[pn++] = Walls[n]->Start;
		}
		if(!FoundEnd)
		{
			PointList[pn++] = Walls[n]->End;
		}
	}

	if(pn > 64)
	{
		SafeExit("Point list overflow in roof creation\n");
	}

	PointList[pn] = PointList[0];

	ThisRoomHeight = Walls[0]->Height;//+ Walls[0]->Start->Location.z;

	//arbitrary start point 0
	vA = PointList[0]->Location - Here->GetCenter();
	
	float LastAngle = 0.0f;
	float ThisAngle;
	EditPoint *pTemp;
	
	//bubble sort points
	for(n = 1; n < pn; n++)
	{
		vB = PointList[n]->Location - Here->GetCenter();
		if(vB == vA)
		{
			ThisAngle = 0.0f;
		}
		else
		{
			ThisAngle = EditGetAngle(&vA,&vB);
		}

		if(ThisAngle < LastAngle)
		{
			pTemp = PointList[n - 1];
			PointList[n-1] = PointList[n];
			PointList[n] = pTemp;
			n = 0; //after loop n will be integrated
			LastAngle = 0.0f;
		}
		else
		{
			LastAngle = ThisAngle;
		}
	}

	//get rid of unnecessary points

	for(n = 1; n < (pn -1); n++)
	{
		vA = Normalize(PointList[n-1]->Location - PointList[n]->Location);
		vB = Normalize(PointList[n+1]->Location - PointList[n]->Location);
			
		ThisAngle = EditGetAngle(&vA,&vB);
		
		if((ThisAngle < PI + 0.05f && ThisAngle > PI - 0.05f) ||
			(ThisAngle > -0.05f && ThisAngle < 0.05f) ||
			(ThisAngle < PI_MUL_2 + 0.05f && ThisAngle > PI_MUL_2 - 0.05f))
		{
			DEBUG_INFO("removing Point from roof calc\n");
			for(sn = n; sn < pn -1; sn++)
			{
				PointList[sn] = PointList[sn + 1];
			}
			pn--;
			n = 1;
		}
	}


	vCenter = Here->GetCenter();
	//create the outer vertexes
	for(n = 0; n < pn; n++)
	{
		TempList[n] = PointList[n]->Location - vCenter;
		Length = Magnitude(TempList[n]);
		Length += OverHang;
		TempList[n] = Normalize(TempList[n]);
		TempList[n] = TempList[n] * Length;
		TempList[n] += vCenter;
	}
	TempList[n] = PointList[0]->Location - Here->GetCenter();
	Length = Magnitude(TempList[n]);
	Length += OverHang;
	TempList[n] = Normalize(TempList[n]);
	TempList[n] = TempList[n] * Length;
	TempList[n] += Here->GetCenter();

	D3DVECTOR ARay;
	D3DVECTOR CRay;
	D3DVECTOR vRay;
	
	float xpercent;
	float zpercent;
	int numdivisions;
	numdivisions = 4;
	int nd;
	int NumSideVerts;
	int VertOffset;
	char blarg[64];
	sprintf(blarg,"pn = %i\n",pn);
	DEBUG_INFO(blarg);
	sprintf(blarg,"walls = %i\n",NumWalls);
	DEBUG_INFO(blarg);
	D3DVECTOR vAOffset,vBOffset;
					
	switch(FRType)
	{
		case ROOF_SLOPE:
			Here->SetNumVerts(pn);
			for(n = 0; n < pn; n++)
			{
				Temp[n].x = TempList[n].x;
				Temp[n].y = TempList[n].y;
				Temp[n].z = TempList[n].z + ThisRoomHeight + FRHeight;
				Temp[n].nx = 0;
				Temp[n].ny = 0;
				Temp[n].nz = 1.0f;
				Temp[n].tu = (Temp[n].x);
				Temp[n].tv = (Temp[n].y);
			}

			Here->NumTriangles = 0;

			for(n = 1; n < pn - 1; n++)
			{
				TempDraw[(n-1)*3] = 0;
				TempDraw[(n-1)*3 + 1] = n+1;
				TempDraw[(n-1)*3 + 2] = n;
				Here->NumTriangles+=3;
			}

			vCenter.z = 0;
			
			vAOffset.z = 0.0f;
			vBOffset.z = 0.0f;

			//that created the top
			//now create the sides
			//blarg, a lot like wall creation
			//four verts per side
			for(n = 0; n < pn; n ++)
			{
				float Length;
				Length = GetDistance(&TempList[n],&TempList[n+1]);
				Length /= FRHeight;

				//quicknormal calculation
				vRay = TempList[n + 1] - TempList[n];
				vNorm.x = vRay.y;
				vNorm.y = -vRay.x;
				vNorm.z = 0;
				vNorm = Normalize(vNorm);
				
				vAOffset.x = vCenter.x - TempList[n].x;
				vAOffset.y = vCenter.y - TempList[n].y;
				
				vAOffset = Normalize(vAOffset);
				vAOffset *= FRHeight;	

				vBOffset.x = vCenter.x - TempList[n + 1].x;
				vBOffset.y = vCenter.y - TempList[n + 1].y; 

				vBOffset = Normalize(vBOffset);

				vBOffset *= FRHeight;

				//a
				Temp[pn + (n*4)].x = TempList[n].x + vAOffset.x;
				Temp[pn + (n*4)].y = TempList[n].y + vAOffset.y;
				Temp[pn + (n*4)].z = TempList[n].z + ThisRoomHeight + FRHeight;
				//b
				Temp[pn + (n*4)+1].x = TempList[n+1].x + vBOffset.x;
				Temp[pn + (n*4)+1].y = TempList[n+1].y + vBOffset.y;
				Temp[pn + (n*4)+1].z = TempList[n+1].z + ThisRoomHeight + FRHeight;
				//c
				Temp[pn + (n*4)+2].x = TempList[n].x;
				Temp[pn + (n*4)+2].y = TempList[n].y;
				Temp[pn + (n*4)+2].z = TempList[n].z + ThisRoomHeight;
				//d
				Temp[pn + (n*4)+3].x = TempList[n+1].x;
				Temp[pn + (n*4)+3].y = TempList[n+1].y;
				Temp[pn + (n*4)+3].z = TempList[n+1].z + ThisRoomHeight;
				//find the normal

				vA.x = Temp[pn + (n*4)].x;
				vA.y = Temp[pn + (n*4)].y;
				vA.z = Temp[pn + (n*4)].z;

				vB.x = Temp[pn + (n*4)+1].x;
				vB.y = Temp[pn + (n*4)+1].y;
				vB.z = Temp[pn + (n*4)+1].z;

				vC.x = Temp[pn + (n*4)+2].x;
				vC.y = Temp[pn + (n*4)+2].y;
				vC.z = Temp[pn + (n*4)+2].z;

				vD.x = Temp[pn + (n*4)+3].x;
				vD.y = Temp[pn + (n*4)+3].y;
				vD.z = Temp[pn + (n*4)+3].z;
				
				vSide1 = Normalize(vA - vB);
				vSide2 = Normalize(vC - vB);

				vNorm = Normalize(CrossProduct(vSide2,vSide1));

				//a
				Temp[pn + (n*4)].nx = vNorm.x;
				Temp[pn + (n*4)].ny = vNorm.y;
				Temp[pn + (n*4)].nz = vNorm.z;
				//b
				Temp[pn + (n*4)+1].nx = vNorm.x;
				Temp[pn + (n*4)+1].ny = vNorm.y;
				Temp[pn + (n*4)+1].nz = vNorm.z;
				//c
				Temp[pn + (n*4)+2].nx = vNorm.x;
				Temp[pn + (n*4)+2].ny = vNorm.y;
				Temp[pn + (n*4)+2].nz = vNorm.z;

				//d
				Temp[pn + (n*4)+3].nx = vNorm.x;
				Temp[pn + (n*4)+3].ny = vNorm.y;
				Temp[pn + (n*4)+3].nz = vNorm.z;
				
				
				FaceLength = GetDistance(&vC, &vD);

				Temp[pn + (n*4) + 2].tu = 0.0f;
				Temp[pn + (n*4) + 2].tv = 0.0f;

				Temp[pn + (n*4)+3].tu = FaceLength;
				Temp[pn + (n*4)+3].tv = 0.0f;


				FaceHeight = PointToLine(&vB, &vC, &vD);

				Temp[pn + (n*4)].tv = Temp[pn + (n*4) + 1].tv = FaceHeight;
			
				SideOneLength = GetDistance(&vA, &vC);
				
				Angle = DotProduct(Normalize(vA - vC),Normalize(vD - vC));

				PointU = Angle * SideOneLength;
				Temp[pn + (n*4)].tu = PointU;
				
				SideOneLength = GetDistance(&vB, &vD);
				
				Angle = DotProduct(Normalize(vC - vD),Normalize(vB - vD));

				PointU = Angle * SideOneLength;
				Temp[pn + (n*4) + 1].tu = FaceLength - PointU;
			}

			for(n = 0; n < pn; n++)
			{
				vAOffset.x = vCenter.x - TempList[n].x;
				vAOffset.y = vCenter.y - TempList[n].y;
			
				vAOffset = Normalize(vAOffset);
				vAOffset *= FRHeight;

				Temp[n].x = TempList[n].x + vAOffset.x;
				Temp[n].y = TempList[n].y + vAOffset.y;
				Temp[n].z = TempList[n].z + ThisRoomHeight + FRHeight;
				Temp[n].nx = 0;
				Temp[n].ny = 0;
				Temp[n].nz = 1.0f;
				Temp[n].tu = (Temp[n].x);
				Temp[n].tv = (Temp[n].y);
			}
			
			//assign it
			Here->NumVerts += pn * 4;
			//all verts are created
			//n is the number of the wall we're working with
			for(n = 0; n < pn; n++)
			{
				TempDraw[Here->NumTriangles+(n*6)] = pn + n * 4;
				TempDraw[Here->NumTriangles+(n*6)+1] = pn + n * 4 + 1;
				TempDraw[Here->NumTriangles+(n*6)+2] = pn + n * 4 + 2;
				TempDraw[Here->NumTriangles+(n*6)+3] = pn + n * 4 + 1;
				TempDraw[Here->NumTriangles+(n*6)+4] = pn + n * 4 + 3;
				TempDraw[Here->NumTriangles+(n*6)+5] = pn + n * 4 + 2;
			}
							
			Here->NumBorderTriangles = 0; //pn * 6;
			Here->NumTriangles += pn * 6;
			
			Here->Verts = new D3DVERTEX[Here->NumVerts];
			memcpy(Here->Verts, Temp,sizeof(D3DVERTEX) * Here->NumVerts);
			Here->DrawList = new unsigned short[Here->NumTriangles + Here->NumBorderTriangles];
			memcpy(Here->DrawList, TempDraw, sizeof(unsigned short) * (Here->NumTriangles + Here->NumBorderTriangles));
			//same algorithm as floor creation	
			break;
		case ROOF_POINT:
			Here->NumVerts = pn * 3;

			for(n = 0; n < pn; n++)
			{
				D3DVECTOR vA, vB, vC, vSide1, vSide2, vNorm;  //a =left, b == point, vc = right

				vA.x = Temp[n*3].x = TempList[n].x;
				vA.y = Temp[n*3].y = TempList[n].y;
				vA.z = Temp[n*3].z = TempList[n].z + ThisRoomHeight;
				
				vC.x = Temp[n*3+2].x = TempList[n+1].x;
				vC.y = Temp[n*3+2].y = TempList[n+1].y;
				vC.z = Temp[n*3+2].z = TempList[n+1].z + ThisRoomHeight;

				vB.x = Temp[n*3+1].x = vCenter.x;
				vB.y = Temp[n*3+1].y = vCenter.y;
				vB.z = Temp[n*3+1].z = vCenter.z + ThisRoomHeight + FRHeight;
			
				vSide1 = Normalize(vA - vB);
				vSide2 = Normalize(vC - vB);

				vNorm = Normalize(CrossProduct(vSide2,vSide1));

				Temp[n*3].nx = Temp[n*3 + 1].nx = Temp[n*3+2].nx = vNorm.x;
				Temp[n*3].ny = Temp[n*3 + 1].ny = Temp[n*3+2].ny = vNorm.y;
				Temp[n*3].nz = Temp[n*3 + 1].nz = Temp[n*3+2].nz = vNorm.z;

				FaceLength = GetDistance(&vA,&vC);

				Temp[n*3].tv = Temp[n*3+2].tv = 0.0f;
				Temp[n*3].tu = 0.0f;
				Temp[n*3+2].tu = FaceLength;
	
				FaceHeight = PointToLine(&vB,&vA,&vC);

				Temp[n*3+1].tv = FaceHeight;

				SideOneLength = GetDistance(&vA, &vB);
				
				Angle = acos(DotProduct(Normalize(vB - vA),Normalize(vC - vA)));

				PointU = cos(Angle) * SideOneLength;
				Temp[n*3+1].tu = PointU;
			}
			
			Here->NumTriangles = pn * 3;
			
			//create the draw list
			for(n = 0; n < pn; n++)
			{
				TempDraw[n*3] = n*3;
				TempDraw[n*3 + 1] = n*3 + 1;
				TempDraw[n*3 + 2] = n*3 + 2;
			}

			Here->Verts = new D3DVERTEX[Here->NumVerts];
			memcpy(Here->Verts,Temp,sizeof(D3DVERTEX) * Here->NumVerts);
	
			Here->DrawList = new unsigned short[Here->NumTriangles];
			memcpy(Here->DrawList,TempDraw,sizeof(unsigned short) * Here->NumTriangles);
			
			break;

		case ROOF_DOME:
			LengthTotal = 0.0f;
			
			NumSideVerts = (numdivisions*2) + 1;

			if((NumSideVerts * (pn - 1)) > 512)
			{
				DEBUG_INFO("lower dome resolution\n");
				numdivisions--;
				NumSideVerts = numdivisions*2 + 1;
				if((NumSideVerts * (pn - 1)) > 512)
				{
					DEBUG_INFO("lower dome resolution again\n");
					numdivisions--;
					NumSideVerts = numdivisions*2 + 1;
					if((NumSideVerts * (pn - 1)) > 512)
					{
						SafeExit("Overflowing roof creation\n");
					}
				}
			}

			int OffSetStart;
			OffSetStart = 0;
			int n;

			for(n = 0; n < pn; n++)
			{
				VertOffset = n*NumSideVerts;

				ARay = TempList[n] - vCenter;
				CRay = TempList[n+1] - vCenter;

				//calculate all the side vertex positions
				//base
				Temp[VertOffset].x = TempList[n].x;
				Temp[VertOffset].y = TempList[n].y;
				Temp[VertOffset].z = TempList[n].z + ThisRoomHeight;
				
				Temp[VertOffset + 1].x = TempList[n + 1].x;
				Temp[VertOffset + 1].y = TempList[n + 1].y;
				Temp[VertOffset + 1].z = TempList[n + 1].z + ThisRoomHeight;

				for(nd = 1; nd < numdivisions; nd++)
				{
					zpercent = (float)nd/(float)numdivisions;
					xpercent = sqrt(1 - (zpercent * zpercent));
					vA = (ARay * xpercent) + vCenter;
					vC = (CRay * xpercent) + vCenter;
					
					vC.z = vA.z = TempList[0].z + ThisRoomHeight + (zpercent *FRHeight);
					
					Temp[VertOffset + 2*nd].x = vA.x;
					Temp[VertOffset + 2*nd].y = vA.y;
					Temp[VertOffset + 2*nd].z = vA.z;
					
					Temp[VertOffset + 2*nd + 1].x = vC.x;
					Temp[VertOffset + 2*nd + 1].y = vC.y;
					Temp[VertOffset + 2*nd + 1].z = vC.z;
				
					Temp[VertOffset + 2*nd].nz = 0.0f;
					Temp[VertOffset + 2*nd + 1].nz = 0.0f;

					Temp[VertOffset + 2*nd].nx = 0.0f;
					Temp[VertOffset + 2*nd + 1].nx = 0.0f;
				
					Temp[VertOffset + 2*nd].ny = 0.0f;
					Temp[VertOffset + 2*nd + 1].ny = 0.0f;
				}

				//peak
				Temp[VertOffset + numdivisions*2].x = vCenter.x;		
				Temp[VertOffset + numdivisions*2].y = vCenter.y;
				Temp[VertOffset + numdivisions*2].z = vCenter.z+ ThisRoomHeight + FRHeight;
				
				//    b----d
				//   /      \
				//  a--------c 
				//first row
				vA.x = Temp[VertOffset].x;
				vA.y = Temp[VertOffset].y;
				vA.z = Temp[VertOffset].z;

				vC.x = Temp[VertOffset+1].x;
				vC.y = Temp[VertOffset+1].y;
				vC.z = Temp[VertOffset+1].z;

				FaceLength = GetDistance(&vA,&vC);

				Temp[VertOffset].tv = Temp[VertOffset+1].tv = 0.0f;
				Temp[VertOffset].tu = LengthTotal;
				LengthTotal += FaceLength;
				Temp[VertOffset+1].tu = LengthTotal;

			}

			for(n = 0; n < pn; n++)
			{
				VertOffset = n*NumSideVerts;

				for(nd = 0; nd < (numdivisions-1); nd++)
				{
					vA.x = Temp[VertOffset + nd*2].x;
					vA.y = Temp[VertOffset + nd*2].y;
					vA.z = Temp[VertOffset + nd*2].z;

					vC.x = Temp[VertOffset + nd*2+1].x;
					vC.y = Temp[VertOffset + nd*2+1].y;
					vC.z = Temp[VertOffset + nd*2+1].z;

					vB.x = Temp[VertOffset + nd*2+2].x;
					vB.y = Temp[VertOffset + nd*2+2].y;
					vB.z = Temp[VertOffset + nd*2+2].z;

					vD.x = Temp[VertOffset + nd*2+3].x;
					vD.y = Temp[VertOffset + nd*2+3].y;
					vD.z = Temp[VertOffset + nd*2+3].z;

					vSide1 = Normalize(vA - vB);
					vSide2 = Normalize(vC - vB);

					vNorm = Normalize(CrossProduct(vSide2,vSide1));

					Temp[VertOffset + nd*2].nx = Temp[VertOffset + nd*2 + 1].nx = Temp[VertOffset + nd*2 + 2].nx = Temp[VertOffset + nd*2 + 3].nx = vNorm.x;
					Temp[VertOffset + nd*2].ny = Temp[VertOffset + nd*2 + 1].ny = Temp[VertOffset + nd*2 + 2].ny = Temp[VertOffset + nd*2 + 3].ny = vNorm.y;
					Temp[VertOffset + nd*2].nz = Temp[VertOffset + nd*2 + 1].nz = Temp[VertOffset + nd*2 + 2].nz = Temp[VertOffset + nd*2 + 3].nz = vNorm.z;

					FaceLength = GetDistance(&vA,&vC);

					FaceHeight = PointToLine(&vB,&vA,&vC);

					Temp[VertOffset+ nd*2 + 2].tv = Temp[VertOffset + nd*2].tv + FaceHeight;
					Temp[VertOffset+ nd*2 + 3].tv = Temp[VertOffset + nd*2 + 1].tv + FaceHeight;

					SideOneLength = GetDistance(&vA, &vB);
					
					Angle = DotProduct(Normalize(vB - vA),Normalize(vC - vA));

					PointU = Angle * SideOneLength;
					Temp[VertOffset+ nd*2+2].tu = Temp[VertOffset + nd*2].tu + PointU;
					
					SideOneLength = GetDistance(&vC, &vD);
					
					Angle = DotProduct(Normalize(vA - vC),Normalize(vD - vC));

					PointU = Angle * SideOneLength;
					Temp[VertOffset+ nd*2 + 3].tu = Temp[VertOffset+ nd*2 + 1].tu - PointU;
				}

				nd = numdivisions - 1;

				//peak
				vA.x = Temp[VertOffset + nd*2].x;
				vA.y = Temp[VertOffset + nd*2].y;
				vA.z = Temp[VertOffset + nd*2].z;

				vC.x = Temp[VertOffset + nd*2 + 1].x;
				vC.y = Temp[VertOffset + nd*2 + 1].y;
				vC.z = Temp[VertOffset + nd*2 + 1].z;

				vB.x = Temp[VertOffset + nd*2 + 2].x;
				vB.y = Temp[VertOffset + nd*2 + 2].y;
				vB.z = Temp[VertOffset + nd*2 + 2].z;
			
				vSide1 = Normalize(vA - vB);
				vSide2 = Normalize(vC - vB);

				vNorm = Normalize(CrossProduct(vSide2,vSide1));

				Temp[VertOffset + nd*2+2].nx = vNorm.x;
				Temp[VertOffset + nd*2+2].ny = vNorm.y;
				Temp[VertOffset + nd*2+2].nz = vNorm.z;

				if(Temp[VertOffset + nd*2 + 1].nx == 0.0f)
				{
					Temp[VertOffset + nd*2+1].nx = vNorm.x;
					Temp[VertOffset + nd*2+1].ny = vNorm.y;
					Temp[VertOffset + nd*2+1].nz = vNorm.z;
				}
				else
				{

				}

				if(Temp[VertOffset + nd*2].nx == 0.0f)
				{
					Temp[VertOffset + nd*2].nx = vNorm.x;
					Temp[VertOffset + nd*2].ny = vNorm.y;
					Temp[VertOffset + nd*2].nz = vNorm.z;
				}
				else
				{

				}

				FaceLength = GetDistance(&vA,&vC);

				FaceHeight = PointToLine(&vB,&vA,&vC);

				Temp[VertOffset + nd*2 + 2].tv = Temp[VertOffset + nd*2].tv + FaceHeight;

				SideOneLength = GetDistance(&vA, &vB);
				
				Angle = DotProduct(Normalize(vB - vA),Normalize(vC - vA));

				PointU = Angle * SideOneLength;
				Temp[VertOffset + nd*2 + 2].tu = Temp[VertOffset + nd*2].tu + PointU;
			}

			//set up the draw List

			NumTriangles = 0;

			for(n = 0; n < pn; n++)
			{
				VertOffset = n*NumSideVerts;
				for(nd = 0; nd < numdivisions-1; nd++)
				{
					TempDraw[NumTriangles++] = VertOffset + nd*2; 
					TempDraw[NumTriangles++] = VertOffset + nd*2 + 2;
					TempDraw[NumTriangles++] = VertOffset + nd*2 + 1;
		
					TempDraw[NumTriangles++] = VertOffset + nd*2 + 2;
					TempDraw[NumTriangles++] = VertOffset + nd*2 + 3;
					TempDraw[NumTriangles++] = VertOffset + nd*2 + 1;
				}

				nd = numdivisions - 1;
				TempDraw[NumTriangles++] = VertOffset + nd*2;
				TempDraw[NumTriangles++] = VertOffset + nd*2 + 2;
				TempDraw[NumTriangles++] = VertOffset + nd*2 + 1;
				if(NumTriangles > 1024)
				{
					SafeExit("overflowing triangles in roof creation\n");
				}
			}


			Here->NumTriangles = NumTriangles;
			Here->NumVerts = NumSideVerts * pn;

			Here->Verts = new D3DVERTEX[Here->NumVerts];
			memcpy(Here->Verts,Temp,sizeof(D3DVERTEX) * Here->NumVerts);
			Here->DrawList = new unsigned short[Here->NumTriangles];
			memcpy(Here->DrawList,TempDraw,sizeof(unsigned short) * Here->NumTriangles);
			break;
		case ROOF_FLAT:
		default:
			Here->SetNumVerts(pn);
			for(n = 0; n < pn; n++)
			{
				Temp[n].x = TempList[n].x;
				Temp[n].y = TempList[n].y;
				Temp[n].z = TempList[n].z + ThisRoomHeight + FRHeight;
				Temp[n].nx = 0;
				Temp[n].ny = 0;
				Temp[n].nz = 1.0f;
				Temp[n].tu = (Temp[n].x);
				Temp[n].tv = (Temp[n].y);
			}

			Here->NumTriangles = 0;

			for(n = 1; n < pn - 1; n++)
			{
				TempDraw[(n-1)*3] = 0;
				TempDraw[(n-1)*3 + 1] = n+1;
				TempDraw[(n-1)*3 + 2] = n;
				Here->NumTriangles+=3;
			}
			
			//that created the top
			//now create the sides
			//blarg, a lot like wall creation
			//four verts per side
			for(n = 0; n < pn; n ++)
			{
				float Length;
				Length = GetDistance(&TempList[n],&TempList[n+1]);
				Length /= FRHeight;

				//quicknormal calculation
				vRay = TempList[n + 1] - TempList[n];
				vNorm.x = vRay.y;
				vNorm.y = -vRay.x;
				vNorm.z = 0;
				vNorm = Normalize(vNorm);
				
				//a
				Temp[pn + (n*4)].x = TempList[n].x;
				Temp[pn + (n*4)].y = TempList[n].y;
				Temp[pn + (n*4)].z = TempList[n].z + ThisRoomHeight + FRHeight;
				Temp[pn + (n*4)].nx = vNorm.x;
				Temp[pn + (n*4)].ny = vNorm.y;
				Temp[pn + (n*4)].nz = vNorm.z;
				Temp[pn + (n*4)].tu = 0;
				Temp[pn + (n*4)].tv = 1.0;
				//b
				Temp[pn + (n*4)+1].x = TempList[n+1].x;
				Temp[pn + (n*4)+1].y = TempList[n+1].y;
				Temp[pn + (n*4)+1].z = TempList[n+1].z + ThisRoomHeight + FRHeight;
				Temp[pn + (n*4)+1].nx = vNorm.x;
				Temp[pn + (n*4)+1].ny = vNorm.y;
				Temp[pn + (n*4)+1].nz = vNorm.z;
				Temp[pn + (n*4)+1].tu = Length;
				Temp[pn + (n*4)+1].tv = 1.0;
				//c
				Temp[pn + (n*4)+2].x = TempList[n].x;
				Temp[pn + (n*4)+2].y = TempList[n].y;
				Temp[pn + (n*4)+2].z = TempList[n].z + ThisRoomHeight;
				Temp[pn + (n*4)+2].nx = vNorm.x;
				Temp[pn + (n*4)+2].ny = vNorm.y;
				Temp[pn + (n*4)+2].nz = vNorm.z;
				Temp[pn + (n*4)+2].tu = 0;
				Temp[pn + (n*4)+2].tv = 0;
				//d
				Temp[pn + (n*4)+3].x = TempList[n+1].x;
				Temp[pn + (n*4)+3].y = TempList[n+1].y;
				Temp[pn + (n*4)+3].z = TempList[n+1].z + ThisRoomHeight;
				Temp[pn + (n*4)+3].nx = vNorm.x;
				Temp[pn + (n*4)+3].ny = vNorm.y;
				Temp[pn + (n*4)+3].nz = vNorm.z;
				Temp[pn + (n*4)+3].tu = Length;
				Temp[pn + (n*4)+3].tv = 0;
				//find the normal


				//assign it
			}

			Here->NumVerts += pn * 4;
			//all verts are created
			//n is the number of the wall we're working with
			for(n = 0; n < pn; n++)
			{
				TempDraw[Here->NumTriangles+(n*6)] = pn + n * 4;
				TempDraw[Here->NumTriangles+(n*6)+1] = pn + n * 4 + 1;
				TempDraw[Here->NumTriangles+(n*6)+2] = pn + n * 4 + 2;
				TempDraw[Here->NumTriangles+(n*6)+3] = pn + n * 4 + 1;
				TempDraw[Here->NumTriangles+(n*6)+4] = pn + n * 4 + 3;
				TempDraw[Here->NumTriangles+(n*6)+5] = pn + n * 4 + 2;
			}
							
			Here->NumBorderTriangles = pn * 6;
			//Here->NumTriangles += pn * 6;

			Here->Verts = new D3DVERTEX[Here->NumVerts];
			memcpy(Here->Verts, Temp,sizeof(D3DVERTEX) * Here->NumVerts);
			Here->DrawList = new unsigned short[Here->NumTriangles + Here->NumBorderTriangles];
			memcpy(Here->DrawList, TempDraw, sizeof(unsigned short) * (Here->NumTriangles + Here->NumBorderTriangles));
			//same algorithm as floor creation	
						
			break;
	}

}
	
void EditRoom::Draw()
{
	for(int n = 0; n < NumWalls; n++)
	{
		Walls[n]->Draw();
	}
}

void EditRoom::SortWalls()
{

}

EditRoom::EditRoom()
{
	pNext = NULL;
	NumWalls = 0;
	AngleTotal = 0.0f;
	ID = -1;
}

EditRoom::~EditRoom()
{
	pNext = NULL;

	for(int n = 0; n < NumWalls; n++)
	{
		Walls[n]->NumRefs--;
		if(Walls[n]->pRoomLeft == this)
		{
			Walls[n]->pRoomLeft = NULL;
		}
		else
		if(Walls[n]->pRoomRight == this)
		{
			Walls[n]->pRoomRight = NULL;
		}
		Walls[n] = NULL;
	}
	NumWalls = 0;
}

void EditRoom::OutputDebugInfo(FILE *fp)
{
	fprintf(fp,"Room *****************\n");
	fprintf(fp,"# %i\n",ID);
	fprintf(fp,"Angles: %f\n",AngleTotal);
	for(int n = 0; n < NumWalls; n++)
	{
		fprintf(fp, "Wall %i:  (%i - %i)\n",Walls[n]->ID,Walls[n]->Start->ID,Walls[n]->End->ID);
	}
	fprintf(fp,"*******************\n");
	fflush(fp);
}

BOOL EditRoom::Validate()
{
	int n, sn;

	BOOL SF;
	BOOL EF;

	for(n = 0; n < NumWalls; n++)
	{
		SF = FALSE;
		EF = FALSE;
		//startpoint
		for(sn = 0; sn < NumWalls; sn++)
		{
			if(sn != n)
			{
				if(Walls[sn]->Start == Walls[n]->Start ||
					Walls[sn]->End == Walls[n]->Start)
				{
					SF = TRUE;
				}
				if(Walls[sn]->Start == Walls[n]->End ||
					Walls[sn]->End == Walls[n]->End)
				{
					EF = TRUE;
				}
			}
		}
		if(!SF || !EF)
		{
			return FALSE;
		}
	}
	
	return TRUE;

}


BOOL EditRoom::HasWall(EditWall *ToTest)
{
	for(int n = 0; n < NumWalls; n++)
	{
		if(Walls[n] == ToTest)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL EditRoom::TestPoint(D3DVECTOR *vTest)
{
	float AngleTotal = 0.0f;
	D3DVECTOR vA;
	D3DVECTOR vB;
	for(int n = 0; n < NumWalls; n++)
	{
		vA = Walls[n]->Start->Location - *vTest;
		vB = Walls[n]->End->Location - *vTest;
		AngleTotal += EditGetAngle(&vA,&vB);
	}
	if(AngleTotal > 6.27 && AngleTotal < 6.29)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void EditRegion::DrawPoints()
{
	EditPoint *pPoint;
	pPoint = Points;
	while(pPoint)
	{
		pPoint->Draw();
		pPoint = pPoint->pNext;
	}
}

void EditRegion::DrawInterior()
{
	EditRoom *pRoom;
	pRoom = Interior;
	while(pRoom)
	{
		pRoom->Draw();
		pRoom = pRoom->pNext;
	}
}

void EditRegion::DrawWalls()
{
	EditWall *pWall;
	pWall = Walls;
	while(pWall)
	{
		pWall->Draw();
		pWall = pWall->pNext;
	}
}

void EditRegion::Draw()
{
	EditPoint *pPoint;
	pPoint = Points;
	while(pPoint)
	{
		pPoint->Draw();
		pPoint = pPoint->pNext;
	}
	
	DrawWalls();
}

EditPoint *EditRegion::GetNearestPoint(D3DVECTOR *vCloseTo)
{
	EditPoint *pPoint, *pNearest;
	pNearest = pPoint = Points;
	float Distance = 50.0f;
	float CurDistance;

	while(pPoint)
	{
		CurDistance = GetDistance(vCloseTo,&pPoint->Location);
		if(CurDistance < Distance)
		{
			Distance = CurDistance;
			pNearest = pPoint;
		}
		pPoint = pPoint->pNext;
	}
	
	return pNearest;
}

EditWall *EditRegion::GetNearestWall(D3DVECTOR *vCloseTo)
{
	EditWall *pWall;
	EditWall *pClosest;
	
	float Distance = 50.0f;
	float ClosestDistance = 50.0f;
	float WallLength;
	pClosest = pWall = Walls;

	D3DVECTOR vIP;
	while(pWall)
	{

		PointToLineIntersect(&vIP, vCloseTo, &pWall->Start->Location, &pWall->End->Location);

		WallLength = GetDistance(&pWall->Start->Location, &pWall->End->Location);

		if(GetDistance(&vIP, &pWall->End->Location) < WallLength &&
			GetDistance(&pWall->Start->Location, &vIP) < WallLength)
		{
			Distance = PointToLine(vCloseTo, &pWall->Start->Location, &pWall->End->Location);
			if(Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				pClosest = pWall;
			}
		}
		pWall = pWall->pNext;
	}

	if(ClosestDistance < 1.0f)
		return pClosest;
	else
		return NULL;

}

EditRoom *EditRegion::GetNearestRoom(D3DVECTOR *vCloseTo)
{
	float Angle = 0.0f;

	EditRoom *pRoom;

	pRoom = Interior;

	while(pRoom)
	{
		if(pRoom->TestPoint(vCloseTo))
		{
			return pRoom;
		}
		else
		{
			pRoom = pRoom->pNext;
		}
	}
	return NULL;
}

EditWall *EditRegion::FindWall(EditPoint *pPStart, EditPoint *pPEnd)
{
	EditWall *pWall;
	pWall = Walls;
	if(pPStart == pPEnd) return NULL;
	while(pWall)
	{
		if((pWall->Start == pPStart && pWall->End == pPEnd) ||
			(pWall->Start == pPEnd && pWall->End == pPStart))
		{
			return pWall;
		}
		pWall = pWall->pNext;
	}
	return NULL;
}

void EditRegion::RemovePoint(EditPoint *ToRemove)
{
	EditPoint *pPoint;

	pPoint = Points;

	if(Points == ToRemove)
	{
		Points = Points->pNext;
	}
	else
	{
		while(pPoint->pNext)
		{
			if(pPoint->pNext == ToRemove)
			{
				pPoint->pNext = ToRemove->pNext;
				break;
			}
			pPoint = pPoint->pNext;
		}
	}
	EditWall *pWall;

	pWall = Walls;

	while(pWall)
	{
		if(pWall->Start == ToRemove || pWall->End == ToRemove)
		{
			RemoveWall(pWall);
			delete pWall;
			pWall = Walls;
		}
		else
		{
			pWall = pWall->pNext;
		}
	}
	return ;
}

void EditRegion::RemoveWall(EditWall *ToRemove)
{
	EditWall *pWall, *lastwall;

	if(Walls == ToRemove)
	{
		Walls = Walls->pNext;
		NumWalls--;
		return;
	}

	lastwall = pWall = Walls;

	while(pWall)
	{
		if(pWall == ToRemove)
		{
			lastwall->pNext = ToRemove->pNext;
			NumWalls--;
			return;
		}
		lastwall = pWall;
		pWall = pWall->pNext;
	}

	return;
}

void EditRegion::RemoveRoom(EditRoom *ToRemove)
{
	if(!ToRemove)
	{
		return;
	}

	if(!Interior)
	{
		SafeExit("bad interior in remove.\n");
	}

	EditRoom *pRoom;

	pRoom = Interior;

	if(!Interior)
	{
		return;
	}

	if(Interior == ToRemove)
	{
		Interior = Interior->pNext;
		return;
	}

	while(pRoom->pNext)
	{
		if(pRoom->pNext == ToRemove)
		{
			pRoom->pNext = ToRemove->pNext;
			return;
		}
		pRoom = pRoom->pNext;
	}

	return;
}

EditRegion::EditRegion()
{
	Walls = NULL;
	Points = NULL;
	Exterior = Interior = NULL;
	NumWalls = NumPoints = NumRooms = 0;
	NextID = 0;
	WallHeight = 4.0f;
	RoofHeight = 4.0f;
	OverHang = 0.0f;
	RoofType = ROOF_DOME;
	return;
}

EditRegion::~EditRegion()
{
	EditRoom *pRoom;
	pRoom = Interior;
	
	while(pRoom)
	{
		Interior = pRoom;
		pRoom = pRoom->pNext;
		delete Interior;
		Interior = NULL;
	}

	EditWall *pWall;
	pWall = Walls;
	while(pWall)
	{
		Walls = pWall;
		pWall = pWall->pNext;
		delete Walls;
		Walls = NULL;
	}

	EditPoint *pPoint;
	pPoint = Points;
	while(pPoint)
	{
		Points = pPoint;
		pPoint = pPoint->pNext;
		delete Points;
		Points = NULL;
	}
	
	if(Exterior)
	{
		delete Exterior;
		Exterior = NULL;
	}

	return;
}

BOOL EditRegion::CheckWallRelation(EditWall *A, EditWall *B)
{
	EditRoom *pRoom;

	pRoom = Interior;
	while(pRoom)
	{
		if(pRoom->HasWall(A) && pRoom->HasWall(B))
		{
			return TRUE;
		}
		pRoom = pRoom->pNext;
	}
	return FALSE;
}

EditPoint *EditRegion::GetCW(EditPoint *pPoint, EditPoint *pFrom)
{
	int NumAround;
	EditPoint *AroundList[32];

	NumAround = PointsAround(pPoint, AroundList);

	int n = 1;

	while(TRUE)
	{
		if(AroundList[n] == pFrom)
		{
			return AroundList[n-1];
		}
		n++;
	}
}

EditPoint *EditRegion::GetCCW(EditPoint *pPoint, EditPoint *pFrom)
{
	int NumAround;
	EditPoint *AroundList[16];

	NumAround = PointsAround(pPoint, AroundList);

	int n = 0;

	while(TRUE)
	{
		if(AroundList[n] == pFrom)
		{
			return AroundList[n+1];
		}
		n++;
	}

}


int EditRegion::PointsAround(EditPoint *pPoint, EditPoint *List[16])
{
	int NumAround = 0;
	//sort the points which connect to this one
	//search through walls and get all points which have a wall incommon witht the point we're at
	EditPoint *pNewPoint;
	pNewPoint = Points;
	while(pNewPoint)
	{
		if(FindWall(pNewPoint,pPoint))
		{
			List[NumAround] = pNewPoint;
			NumAround++;
		}
		pNewPoint = pNewPoint->pNext;
	}
	if(NumAround < 2)
	{
		SafeExit("Invalid wall.\n");
	}

	//sort points
	D3DVECTOR vA;
	D3DVECTOR vB;

	//arbitrary start point 0
	vA = List[0]->Location - pPoint->Location;
	
	float LastAngle = 0.0f;
	float ThisAngle;
	EditPoint *pTemp;
	int n;

	//bubble sort points
	for(n = 1; n < NumAround; n++)
	{
		vB = List[n]->Location - pPoint->Location;
		if(vB == vA)
		{
			ThisAngle = 0.0f;
		}
		else
		{
			ThisAngle = EditGetAngle(&vA,&vB);
		}

		if(ThisAngle < LastAngle)
		{
			pTemp = List[n - 1];
			List[n-1] = List[n];
			List[n] = pTemp;
			n = 1;
			LastAngle = 0.0f;
		}
		else
		{
			LastAngle = ThisAngle;
		}
	}

	List[NumAround] = List[0];

	return NumAround;

}

void EditRegion::CreateRooms()
{

	FILE *fp;
	fp = SafeFileOpen("createinterior.txt","wt");

	EditRoom *pRoom;
	//clear all rooms
	while(Interior)
	{
		pRoom = Interior;
		RemoveRoom(pRoom);
		delete pRoom;
	}
	EditWall *pWallTemp;
	pWallTemp = Walls;
	while(pWallTemp)
	{
		pWallTemp->NumRefs = 0;
		pWallTemp->pRoomLeft = NULL;
		pWallTemp->pRoomRight = NULL;
		pWallTemp->Exterior = FALSE;
		pWallTemp = pWallTemp->pNext;
	}
	

	NumRooms = 0;
		
	EditPoint *pPoint;
	EditWall *pWallA, *pWallB;
	pPoint = Points;
	D3DVECTOR v1, v2;

	EditPoint *AroundList[32];

	int NumAround = 0;

	float Angle;
	EditPoint *pStart;
	EditPoint *pCur;
	EditWall *ToAdd;
	int n;

	while(pPoint)
	{
		pStart = pPoint;
		//sort the points around this one
		NumAround = PointsAround(pPoint,AroundList);
		assert(NumAround >= 2);
	//	ToDrawPoint = pPoint;
		for(n = 0; n < NumAround; n++)
		{
			pWallA = FindWall(pPoint,AroundList[n]);
			assert(pWallA);
			pWallB = FindWall(pPoint,AroundList[n+1]);
			assert(pWallB);
			if(!pWallA->GetRight(pPoint))
			{
				v1 = AroundList[n]->Location - pPoint->Location;
				v2 = AroundList[n+1]->Location - pPoint->Location;
				Angle = RadToDeg(EditGetAngle(&v1,&v2));
				if(Angle < 180.1f)
				{
					DEBUG_INFO("Creating Room\n");
					pRoom = new EditRoom;
					AddRoom(pRoom);
					ToAdd = pWallA;
					pCur = pStart;
					EditPoint *pLast, *pNext;
					do
					{
					//	ToDrawWall = ToAdd;
			
						pRoom->AddWall(ToAdd);
						ToAdd->SetRight(pCur,pRoom);
						pLast = pCur;
						pCur = ToAdd->GetEnd(pCur);
						ToAdd = FindWall(pCur,GetCW(pCur,ToAdd->GetEnd(pCur)));
						if(!ToAdd)
						{
							SafeExit("bad wall find");
						}
						pNext = ToAdd->GetEnd(pCur);
						
					//	ToDrawNext = ToAdd;
						
					//	pWin = new ZSMessage("next","ok",ZSWindow::GetMain());
					//	pWin->Show();
					//	pWin->GoModal();
					//	ZSWindow::GetMain()->RemoveChild(pWin);
						
						v1 = pLast->Location - pCur->Location;
						v2 = pNext->Location - pCur->Location;
						Angle = RadToDeg(EditGetAngle(&v2,&v1));
						if(Angle > 180.2f)
						{
							DEBUG_INFO("removing room\n");
							RemoveRoom(pRoom);
							delete pRoom;
							pRoom = NULL;
						}
					}while(pRoom && pCur != pStart);

					if(pRoom)
					{
						pRoom->SumAngles();
						pRoom->OutputDebugInfo(fp);
					}

//					ToDrawRoom = pRoom;
//					pWin = new ZSMessage("next","ok",ZSWindow::GetMain());
//					pWin->Show();
//					pWin->GoModal();
//					ZSWindow::GetMain()->RemoveChild(pWin);
//					
//					ToDrawRoom = NULL;
				}
			}
		}
		pPoint = pPoint->pNext;
	}

	ToDrawWall = NULL;
	ToDrawNext = NULL;
	ToDrawPoint = NULL;

	//check for the exterior of the building
	pRoom = Interior;

	DEBUG_INFO("Create Exterior\n");
	
	while(pRoom)
	{
		Angle = pRoom->SumAngles();
		if(Angle > (float)(pRoom->NumWalls - 2)*180.15f || Angle < (float)(pRoom->NumWalls -2)*179.8f)
		{
			Exterior = pRoom;
			RemoveRoom(pRoom);
			break;
		}
		else
		{
			pRoom = pRoom->pNext;
		}
	}
	

	if(!Exterior)
	{
		Exterior = new EditRoom;

		EditWall *pWall;
		
		pWall = Walls;

		while(pWall)
		{
			if(pWall->NumRefs < 2)
			{
				pWall->Exterior = TRUE;
				Exterior->AddWall(pWall);
				if(pWall->pRoomLeft)
				{
					pWall->pRoomRight = Exterior;
				}
				else
				{
					pWall->pRoomLeft = Exterior;
				}
			}
			pWall = pWall->pNext;
		}
	}

	fprintf(fp,"\n\nExterior:\n\n");
	Exterior->OutputDebugInfo(fp);
	fclose(fp);

//	ToDrawRoom = Exterior;/
//	pWin = new ZSMessage("next","ok",ZSWindow::GetMain());/
//	pWin->Show();
//	pWin->GoModal();
//	ZSWindow::GetMain()->RemoveChild(pWin);
//	ToDrawRoom = NULL;				
}

void EditRegion::CreateInterior()
{
	EditPoint *pPoint;
	pPoint = Points;
	while(pPoint)
	{
		pPoint->Touched = FALSE;
		pPoint = pPoint->pNext;
	}

	FILE *fp;
	fp = SafeFileOpen("createinterior.txt","wt");
	fclose(fp);

	CreateRooms();
}

void EditRegion::AddWall(EditWall *ToAdd)
{
	ToAdd->pNext = Walls;
	Walls = ToAdd;
	NumWalls++;
	ToAdd->ID = NextID++;
}

void EditRegion::AddPoint(EditPoint *ToAdd)
{
	ToAdd->pNext = Points;
	Points = ToAdd;
	ToAdd->ID = NextID++;
	NumPoints++;
}

void EditRegion::AddRoom(EditRoom *ToAdd)
{
	ToAdd->pNext = Interior;
	Interior = ToAdd;
	NumRooms++;
	ToAdd->ID = NextID++;
}

void EditRegion::DefineWall(EditPoint *A, EditPoint *B)
{
	if((A == B) || FindWall(A,B))
	{
		return;
	}
	else
	{
		EditWall *NewWall;

		NewWall = new EditWall(A,B);

		AddWall(NewWall);
	}

}

void EditRoom::AddWall(EditWall *ToAdd)
{
	if(!HasWall(ToAdd))
	{
		Walls[NumWalls] = ToAdd;
		Walls[NumWalls]->NumRefs++;
		NumWalls++;
	}
}

EditPoint *EditRegion::FindPoint(D3DVECTOR *vToFind)
{
	EditPoint *pPoint;
	pPoint = Points;

	while(pPoint)
	{
		if(pPoint->Location.x > (vToFind->x - POINT_TOLERANCE) && pPoint->Location.x < (vToFind->x + POINT_TOLERANCE) &&
			pPoint->Location.y > (vToFind->y - POINT_TOLERANCE) && pPoint->Location.y < (vToFind->y + POINT_TOLERANCE) &&
			pPoint->Location.z > (vToFind->z - POINT_TOLERANCE) && pPoint->Location.z < (vToFind->z + POINT_TOLERANCE))
		{
			return pPoint;
		}
		else
		{
			pPoint = pPoint->pNext;
		}
	}
	return pPoint;
}

void EditRegion::OutputDebugInfo(char *FileName)
{
	FILE *fp;

	fp = SafeFileOpen(FileName, "wt");

	EditPoint *pPoint;
	pPoint = Points;
	while(pPoint)
	{
		fprintf(fp, "Point # %i  at (%f, %f, %f)\n",pPoint->ID,pPoint->Location.x,pPoint->Location.y,pPoint->Location.z);
		pPoint = pPoint->pNext;
	}
	
	EditWall *pWall;

	pWall = Walls;

	while(pWall)
	{
		fprintf(fp,"Wall ************\n");
		fprintf(fp,"# %i\n",pWall->ID);
		fprintf(fp,"NumRefs: %i\n",pWall->NumRefs);
		fprintf(fp,"Exterior: %i\n",pWall->Exterior);
		fprintf(fp,"Height: %f\n", pWall->Height);
		fprintf(fp,"%i - %i\n", pWall->Start->ID, pWall->End->ID);
		fprintf(fp,"********\n");
		pWall = pWall->pNext;
	}

	EditRoom *pRoom;
	
	pRoom = Interior;

	while(pRoom)
	{
		pRoom->OutputDebugInfo(fp);
		pRoom = pRoom->pNext;
	}

	if(Exterior)
	{
		fprintf(fp,"exterior:");
		Exterior->OutputDebugInfo(fp);
	}

	fclose(fp);

}

Region *EditRegion::CreateGameRegion(ZSTexture *ExWall, ZSTexture *InWall, ZSTexture *Floor, ZSTexture *Roof)
{
	DEBUG_INFO("creating game region\n");
	GetBounds();

	Region *pRegion;
	
	pRegion = new Region;
	pRegion->SetBounds(&Bounds);

	pRegion->ID = PreludeWorld->GetNextFreeRegion();
	pRegion->Height = WallHeight;
	pRegion->FRHeight = RoofHeight;
	pRegion->Type = REGION_EXTERIOR;
	Region *pSubRegion;

	//create the rooms
	EditRoom *pRoom;

	pRoom = Interior;

	EditPoint *PointList[32];

	memset(PointList,0,sizeof(EditPoint *)*32);

	//a temporary vertex buffer;
	D3DVERTEX VTemp[128];
	//number of verts in the buffer
	int NumVerts = 0;
	int n = 0;
	int pn = 0;
	//number of vertexes in the draw buffer

	D3DVECTOR vCenter;

	int xn;
	int yn;
	DEBUG_INFO("Clearing blocking\n");
	for(yn = Bounds.top / CHUNK_TILE_HEIGHT; yn <= Bounds.bottom / CHUNK_TILE_HEIGHT; yn++)
	for(xn = Bounds.left / CHUNK_TILE_WIDTH; xn <= Bounds.right / CHUNK_TILE_WIDTH; xn++)
	{
		Chunk *pChunk;
		pChunk = Valley->GetChunk(xn,yn);
		if(pChunk)
		{
			pChunk->ResetBlocking();
		}
	}

	int RoomID = 1;

	while(pRoom)
	{
		DEBUG_INFO("Making room region\n");
		memset(PointList,0,sizeof(EditPoint *)*32);
		pSubRegion = new Region;
		pRoom->Here = pSubRegion;
		pSubRegion->Parent = pRegion;
		pSubRegion->pNext = pRegion->SubRegions;
		pRegion->SubRegions = pSubRegion;
		pSubRegion->pFRTexture = Floor;
		pSubRegion->ID = RoomID;
		pSubRegion->Type = REGION_INTERIOR;
		pSubRegion->Height = this->WallHeight;
		pSubRegion->FRHeight = this->RoofHeight;
		pn = 0;

		pRoom->CreateFloor();

		pSubRegion->pFRTexture = Engine->GetTexture("floor");
		pSubRegion->SetWallTexture(Engine->GetTexture("inwall"));
		pSubRegion->vCenter = pRoom->FindCenter();
		pSubRegion->SetOccupancy(REGION_UNSEEN);
		pSubRegion->MakeBounds();
		
		pRoom = pRoom->pNext;
		RoomID++;
	}
	pRegion->SetFRTexture(Engine->GetTexture("roof"));
	pRegion->SetWallTexture(Engine->GetTexture("exwall"));
	Exterior->Here = pRegion;
	Exterior->CreateRoof(RoofType, RoofHeight, OverHang);

	//create the exterior
	//create the walls;
	EditWall *pWall;
	Wall *NewWall;
	pWall = Walls;
	while(pWall)
	{
		DEBUG_INFO("Creating Wall\n");
		NewWall = new Wall;
		pWall->Here = NewWall;
		pWall->Height = WallHeight;

		pWall->CreateMesh();
		
		NewWall->Front->pTexture = NewWall->Front->RegionFacing->GetWallTexture();
		NewWall->Back->pTexture = NewWall->Back->RegionFacing->GetWallTexture();

		pRegion->AddWall(NewWall);
		if(pWall->Type == EWALL_DOOR)
		{
			D3DVECTOR DoorLocation;
			D3DVECTOR FrameLocation;
			float Rotation;

			switch(pWall->GetOrientation())
			{
			case NORTH:
				DoorLocation.x = pWall->End->Location.x;
				DoorLocation.y = pWall->End->Location.y;
				DoorLocation.z = pWall->End->Location.z;

				FrameLocation.x = pWall->End->Location.x;
				FrameLocation.y = pWall->End->Location.y + 0.5f;
				FrameLocation.z = pWall->End->Location.z;

				Rotation = PI_DIV_2;
				break;

			case EAST:
				DoorLocation.x = pWall->Start->Location.x;
				DoorLocation.y = pWall->Start->Location.y;
				DoorLocation.z = pWall->Start->Location.z;

				FrameLocation.x = pWall->Start->Location.x + 0.5f;
				FrameLocation.y = pWall->Start->Location.y;
				FrameLocation.z = pWall->Start->Location.z;

				Rotation = 0.0f;
				break;

			case WEST:
				DoorLocation.x = pWall->End->Location.x;
				DoorLocation.y = pWall->End->Location.y;
				DoorLocation.z = pWall->End->Location.z;

				FrameLocation.x = pWall->End->Location.x + 0.5f;
				FrameLocation.y = pWall->End->Location.y;
				FrameLocation.z = pWall->End->Location.z;

				Rotation = 0.0f;
				break;
			case SOUTH:
				DoorLocation.x = pWall->Start->Location.x;
				DoorLocation.y = pWall->Start->Location.y;
				DoorLocation.z = pWall->Start->Location.z;

				FrameLocation.x = pWall->Start->Location.x;
				FrameLocation.y = pWall->Start->Location.y + 0.5f;
				FrameLocation.z = pWall->Start->Location.z;

				Rotation = PI_DIV_2;
				break;
			case NORTHEAST:
				DoorLocation.x = pWall->End->Location.x;
				DoorLocation.y = pWall->End->Location.y;
				DoorLocation.z = pWall->End->Location.z;

				FrameLocation.x = pWall->End->Location.x;
				FrameLocation.y = pWall->End->Location.y;
				FrameLocation.z = pWall->End->Location.z;

				Rotation = PI_DIV_4;
				break;
			case SOUTHEAST:
				DoorLocation.x = pWall->End->Location.x;
				DoorLocation.y = pWall->End->Location.y;
				DoorLocation.z = pWall->End->Location.z;

				FrameLocation.x = pWall->End->Location.x;
				FrameLocation.y = pWall->End->Location.y;
				FrameLocation.z = pWall->End->Location.z;

				Rotation = PI_DIV_2 + PI_DIV_4;
				break;
			case SOUTHWEST:
				DoorLocation.x = pWall->End->Location.x;
				DoorLocation.y = pWall->End->Location.y;
				DoorLocation.z = pWall->End->Location.z;

				FrameLocation.x = pWall->End->Location.x;
				FrameLocation.y = pWall->End->Location.y;
				FrameLocation.z = pWall->End->Location.z;

				Rotation = PI_DIV_4;
				break;
			case NORTHWEST:
				DoorLocation.x = pWall->End->Location.x;
				DoorLocation.y = pWall->End->Location.y;
				DoorLocation.z = pWall->End->Location.z;

				FrameLocation.x = pWall->End->Location.x;
				FrameLocation.y = pWall->End->Location.y;
				FrameLocation.z = pWall->End->Location.z;

				Rotation = PI_DIV_2 + PI_DIV_4;
				break;
			default:
				SafeExit("INVALID direction in create region\n");
				break;
			}
			//get the wall's angle
			Object *pOb;

			Portal *pPortal;
			pPortal = new Portal;
			pPortal->SetMesh(Engine->GetMesh("door01"));
			pPortal->SetPosition(&DoorLocation);
			pPortal->SetTexture(pPortal->GetMesh()->GetTexture());
			pPortal->SetAngle(Rotation);
			pPortal->SetClosedAngle(Rotation);
			pPortal->SetOpenAngle(Rotation + PI_DIV_2);
			pPortal->SetRegionOne(NewWall->Front->RegionFacing);
			if(pPortal->GetRegionOne() == pRegion)
			{
				pPortal->SetRegionOneSubNum(0);
			}
			else
			{
				pPortal->SetRegionOneSubNum(pPortal->GetRegionOne()->GetID());
			}

			pPortal->SetRegionTwo(NewWall->Back->RegionFacing);
			if(pPortal->GetRegionTwo() == pRegion)
			{
				pPortal->SetRegionTwoSubNum(0);
			}
			else
			{
				pPortal->SetRegionTwoSubNum(pPortal->GetRegionTwo()->GetID());
			}
			Valley->AddToUpdate(pPortal);
			Valley->SetBlocking((int)DoorLocation.x, (int)DoorLocation.y);


			pOb = new Object;
			pOb->SetMesh(Engine->GetMesh("doorframe01"));
			pOb->SetPosition(&FrameLocation);
			pOb->SetTexture(pOb->GetMesh()->GetTexture());
			pOb->SetAngle(Rotation);
			Valley->AddObject(pOb);

/*			pOb = new Object;
			pOb->SetMesh(Engine->GetMesh("doorframe01"));
			pOb->SetPosition(&FrameLocation);
			pOb->SetTexture(pOb->GetMesh()->GetTexture());
			pOb->SetAngle(Rotation);
			NewWall->Back->RegionFacing->AddObject(pOb);
*/
		
		}
		else
		if(pWall->Type == EWALL_WINDOW)
		{

		}
		int XRound;
		int YRound;
		int LastX = -1;
		int LastY = -1;
		float Length;
		float MaxLength;
		
		D3DVECTOR Ray;
		D3DVECTOR Resultant;

		Ray = pWall->End->Location - pWall->Start->Location;
		MaxLength = Magnitude(Ray);
		Ray = Normalize(Ray);

		for(Length = 0.0f; Length <= MaxLength; Length += 0.1f)
		{
			Resultant = pWall->Start->Location + (Ray * Length);
			XRound = (int)Resultant.x;
			YRound = (int)Resultant.y;
			if((XRound != LastX) || (YRound != LastY))
			{
				LastX = XRound;
				LastY = YRound;
				Valley->SetBlocking(XRound, YRound);
			}
		}
	
		pWall = pWall->pNext;
	}
	pRegion->CreateDrawList();
	pRegion->SetOccupancy(REGION_OCCUPIED);
	return pRegion;
}

void EditRegion::ClearAll()
{
	GetBounds();
	EditWall *pWall;
	EditRoom *pRoom;
	EditPoint *pPoint;
	//clear all rooms
	while(Interior)
	{
		pRoom = Interior;
		RemoveRoom(pRoom);
		delete pRoom;
	}
	Interior = NULL;
	NumRooms = 0;
	while(Walls)
	{
		pWall = Walls;
		RemoveWall(pWall);
		delete pWall;
	}
	Walls = NULL;
	NumWalls = 0;

	while(Points)
	{
		pPoint = Points;
		RemovePoint(pPoint);
		delete pPoint;
	}
	Points = NULL;
	NumPoints = 0;

	if(Exterior)
	{
		delete Exterior;
		Exterior = NULL;
	}

	int xn;
	int yn;

	for(yn = Bounds.top / CHUNK_TILE_HEIGHT; yn <= Bounds.bottom / CHUNK_TILE_HEIGHT; yn++)
	for(xn = Bounds.left / CHUNK_TILE_WIDTH; xn <= Bounds.right / CHUNK_TILE_WIDTH; xn++)
	{
		Chunk *pChunk;
		pChunk = Valley->GetChunk(xn,yn);
		if(pChunk)
		{
			pChunk->ResetBlocking();
		}
	}
}

void EditRegion::ClearRooms()
{
	EditRoom *pRoom;
	//clear all rooms
	while(Interior)
	{
		pRoom = Interior;
		RemoveRoom(pRoom);
		delete pRoom;
	}
	NumRooms = 0;

	if(Exterior)
	{
		delete Exterior;
		Exterior = NULL;
	}
}

void EditRegion::SetWallHeight(float NewWallHeight)
{ 
	WallHeight = NewWallHeight; 

	EditWall *pWall;
	pWall = Walls;
	while(pWall)
	{
		pWall->Height = WallHeight;
		pWall = pWall->pNext;
	}
}

void DrawDoor(D3DVECTOR *vA, EditWall *pWallOn)
{
	D3DVECTOR End;
	D3DVECTOR DoorRay;

	DoorRay = *vA - pWallOn->Start->Location;

	float RayLength = Magnitude(DoorRay);

	RayLength += DOOR_WIDTH;

	DoorRay = Normalize(DoorRay);

	DoorRay *= RayLength;
	
	End = DoorRay + pWallOn->Start->Location;

	D3DLVERTEX	Verts[4];
	Verts[0].x = vA->x;
	Verts[0].y = vA->y;
	Verts[0].z = vA->z;
	Verts[0].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[0].specular = D3DRGB(1.0f,1.0f,1.0f);

	Verts[2].x = End.x;
	Verts[2].y = End.y;
	Verts[2].z = End.z + DOOR_HEIGHT;
	Verts[2].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[2].specular = D3DRGB(1.0f,1.0f,1.0f);

	Verts[1].x = vA->x;
	Verts[1].y = vA->y;
	Verts[1].z = vA->z + DOOR_HEIGHT;
	Verts[1].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[1].specular = D3DRGB(1.0f,1.0f,1.0f);

	Verts[3].x = End.x;
	Verts[3].y = End.y;
	Verts[3].z = End.z;
	Verts[3].color = D3DRGB(1.0f,1.0f,1.0f);
	Verts[3].specular = D3DRGB(1.0f,1.0f,1.0f);

	HRESULT hr;
	hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_LVERTEX, Verts, 4, 0 );
	if(hr != D3D_OK)
	{
		SafeExit("problem drawing edit wall\n");
	}
}

void EditRegion::DoorWall(EditWall *ToDoor, D3DVECTOR *vAt)
{
	D3DVECTOR End;
	D3DVECTOR DoorRay;

	DoorRay = *vAt - ToDoor->Start->Location;

	float RayLength = Magnitude(DoorRay);

	RayLength += 2.0f;

	DoorRay = Normalize(DoorRay);

	DoorRay *= RayLength;
	
	End = DoorRay + ToDoor->Start->Location;

	RemoveWall(ToDoor);
	
	EditWall *NewWall;

	EditPoint *pPointA, *pPointB;

	pPointA = new EditPoint(vAt);
	pPointB = new EditPoint(&End);

	AddPoint(pPointA);
	AddPoint(pPointB);

	NewWall = new EditWall(ToDoor->Start,pPointA,WallHeight);
	
	AddWall(NewWall);

	NewWall = new EditWall(pPointB,ToDoor->End, WallHeight);

	AddWall(NewWall);

	//this is the actual door segment
	NewWall = new EditWall(pPointA,pPointB, WallHeight);
	NewWall->Type = EWALL_DOOR;

	AddWall(NewWall);

	ToDoor->Start = NULL;
	ToDoor->End = NULL;

	delete ToDoor;
}

void EditRegion::GetBounds()
{
	Bounds.left = 100000;
	Bounds.right = 0;
	Bounds.top = 100000;
	Bounds.bottom = 0;
	EditPoint *pPoint;

	pPoint = Points;

	while(pPoint)
	{
		if(pPoint->Location.x < Bounds.left)
		{
			Bounds.left = (int)pPoint->Location.x;
		}
		if(pPoint->Location.x > Bounds.right)
		{
			Bounds.right = (int)pPoint->Location.x;
		}
		if(pPoint->Location.y < Bounds.top)
		{
			Bounds.top = (int)pPoint->Location.y;
		}
		if(pPoint->Location.y > Bounds.bottom)
		{
			Bounds.bottom = (int)pPoint->Location.y;
		}
		pPoint = pPoint->pNext;
	}
}


