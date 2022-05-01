//********************************************************************* 
//********************************************************************* 
//**************                Objects.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:                                                                                        * 
//*Revisor:                                               
//*Purpose:                        
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*        			               
//*			
//********************************************************************* 
//*********************************************************************
#include "objects.h"
#include "zsengine.h"
#include "zsutilities.h"
#include "mainwindow.h"
#include "items.h"
#include "gameitem.h"
#include "area.h"
#include "party.h"
#include "flags.h"
#include "script.h"

//************** static Members *********************************




//************** Constructors  ****************************************

//simple constructor
Object::Object()
{
	pNext = NULL;
	Data = NULL;
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
	pContents = NULL;
}

//copy constructor


//end:  Constructors ***************************************************



//*************** Destructor *******************************************
Object::~Object()
{
	if(ZSWindow::GetMain())
	{
		if(((ZSMainWindow *)ZSWindow::GetMain())->Target() == this)
		{
			((ZSMainWindow *)ZSWindow::GetMain())->SetTarget(NULL);
		}
	}

}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************
/*
Object *Object::GetNext();

{
	return pNext;
}
*/

D3DVECTOR *Object::GetPosition()
{
	return &Position;
}

int Object::GetFrame();

float Object::GetMyAngle()
{
	return Angle;
}

float Object::GetRadius()
{
	return BlockingRadius;
}

float Object::GetScale()
{
	return Scale;
}
/*
ZSModelEx *Object::GetMesh()
{
	return pMesh;
}
*/
int Object::GetTextureNum()
{
	return TextureNum;
}

ZSTexture *Object::GetTexture()
{
	return pTexture;
}

BOOL Object::RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	if(!pMesh) return FALSE;
	//transform the ray into object coordinates;
	//first check if ray is close enough to matter
	//point to line distance
	float Scale;
	Scale = GetScale();
	float InverseScale;

	InverseScale = 1.0f/Scale;

	//convert the ray to object coordinates
	//first translate then rotate
	D3DXMATRIX matRotate, matScale, matTransform;
	D3DXMatrixRotationZ( &matRotate, -(GetMyAngle() + PI));
	D3DXMatrixScaling(&matScale, InverseScale, InverseScale, InverseScale);
	D3DXMatrixMultiply(&matTransform, &matRotate, &matScale);
	D3DXVECTOR4 vTransStart, vTransEnd;
	D3DVECTOR vMoveStart, vMoveEnd;
	vMoveStart = *vRayStart - *GetPosition();
	vMoveEnd = *vRayEnd - *GetPosition();
	D3DXVec3Transform(&vTransStart,(D3DXVECTOR3 *)&vMoveStart, &matTransform);
	D3DXVec3Transform(&vTransEnd,(D3DXVECTOR3 *)&vMoveEnd, &matTransform);
	return pMesh->Intersect(GetFrame(), (D3DVECTOR *)&vTransStart,(D3DVECTOR *)&vTransEnd);
}


BOOL Object::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	if(!pMesh) return FALSE;
	//transform the ray into object coordinates;
	//first check if ray is close enough to matter
	//point to line distance
	float Distance;



	Distance = GetDistance(GetPosition(),vRayEnd);
	
	float Scale;
	Scale = GetScale();
	float InverseScale;

	InverseScale = 1.0f/Scale;

	if(Distance < pMesh->GetWidth() || Distance < pMesh->GetHeight())
	{
		//convert the ray to object coordinates
		//first translate then rotate
		D3DXMATRIX matRotate, matScale, matTransform;
		D3DXMatrixRotationZ( &matRotate, -(GetMyAngle() + PI));
		D3DXMatrixScaling(&matScale, InverseScale, InverseScale, InverseScale);
		D3DXMatrixMultiply(&matTransform, &matRotate, &matScale);
		D3DXVECTOR4 vTransStart, vTransEnd;
		D3DVECTOR vMoveStart, vMoveEnd;
		vMoveStart = *vRayStart - *GetPosition();
		vMoveEnd = *vRayEnd - *GetPosition();
		D3DXVec3Transform(&vTransStart,(D3DXVECTOR3 *)&vMoveStart, &matTransform);
		D3DXVec3Transform(&vTransEnd,(D3DXVECTOR3 *)&vMoveEnd, &matTransform);
		return pMesh->Intersect(GetFrame(), (D3DVECTOR *)&vTransStart,(D3DVECTOR *)&vTransEnd);
	}
	else
	{
		return FALSE;
	}
}

//get center.
//returns the coordinates of the objects center
D3DVECTOR Object::GetCenter()
{
	int Frame;
	Frame = this->GetFrame();

	//get the bounds
	if(!pMesh)
	{
		return *GetPosition();
	}

	D3DXVECTOR3 vA, vB;
	D3DXVECTOR4 vxA, vxB;
	D3DXMATRIX	matRotate;

	D3DXMatrixRotationZ(&matRotate,GetMyAngle());

	pMesh->GetBounds(&vA.x, &vB.x, &vA.z, &vB.z, &vA.y, &vB.y, Frame);

	D3DXVec3Transform(&vxA,&vA,&matRotate);
	D3DXVec3Transform(&vxB,&vB,&matRotate);

	D3DVECTOR vCenter;
	vCenter = *GetPosition();
	
	vCenter.x += (vxA.x + vxB.x)/2.0f;
	vCenter.y += (vxA.y + vxB.y)/2.0f;
	vCenter.z += (vxA.z + vxB.z)/2.0f;

	return vCenter;
}


float Object::GetCurrentRadius()
{
	float Left, Right, Top, Bottom, Front, Back;

	pMesh->GetBounds(&Left, &Right, &Top, &Bottom, &Front, &Back, Frame);

	if(Left < 0.0f) Left = Left * -1.0f;
	if(Front < 0.0f) Front = Front * -1.0f;
	if(Back < 0.0f) Back = Back * -1.0f;
	if(Right < 0.0f) Right = Right * -1.0f;
	if(Left > Right)
	{
		if(Front > Back)
		{
			if(Front > Left)
			{
				return Front;
			}
			else
			{
				return Left;
			}
		}
		else
		if(Back > Left)
		{
			return Back;
		}
		else
		{
			return Left;
		}
	}
	else
	{
		if(Front > Back)
		{
			if(Front > Right)
			{
				return Front;
			}
			else
			{
				return Right;
			}
		}
		else
		if(Back > Right)
		{
			return Back;
		}
		else
		{
			return Right;
		}
	}
	
	return 1.0f;

}


BOOL Object::TileIntersect(int TileX, int TileY)
{
	if(TileX == (int)Position.x && TileY == (int)Position.y)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL Object::TileBlock(int TileX, int TileY)
{
	return FALSE;
}


//end: Accessors *******************************************************



//************  Display Functions  *************************************
void Object::Draw(HDC hdc)
{
	int DrawX;
	int DrawY;

	//convert the x/y vector position coordinates into screen space

	DrawX = (int)(10.0f * Position.x);
	DrawY = (int)(10.0f * Position.y);

	TextOut(hdc,DrawX,DrawY,"o",1);

}

void Object::AdjustCamera()
{

}


void Object::Draw()
{
	//get a pointer to the mesh indicated by the things mesh field
	//set the current texture to the texture specified in the texture field
	//call the mesh's  draw function with the proper position, rotation,
	//and scaling factors
	//done

	if(pMesh)
		pMesh->Draw(Engine->Graphics()->GetD3D(),Position.x, Position.y, Position.z, Angle, Scale,Scale,Scale, 0);

	return;
}

int Object::HighLight(COLOR_T Color)
{
	Engine->Graphics()->ClearTexture();
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(Color));
	D3DVECTOR *pPosition;

	pPosition = GetPosition();

	float Rotation;

	Rotation = GetMyAngle();

	if(GetObjectType() != OBJECT_CREATURE || PreludeWorld->FullSelect())
	{
		if(GetObjectType() == OBJECT_CREATURE)
		{
			Creature *pCreature;
			pCreature = (Creature *)this;
			if(pCreature->IsLarge())
			{
				if(pMesh)
					pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x + 0.5f,pPosition->y + 0.5f, pPosition->z,Rotation,GetScale(),GetScale(),GetScale(),GetFrame());
			}
			else
			if(pMesh)
				pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x,pPosition->y,pPosition->z,Rotation,GetScale(),GetScale(),GetScale(),GetFrame());
		}
		else
		if(pMesh)
			pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x,pPosition->y,pPosition->z,Rotation,GetScale(),GetScale(),GetScale(),GetFrame());
	}
	else
	{
		float MeshHeight;
		float MeshWidth = 1.0f;
		float MeshDepth = 1.0f;

		MeshHeight = 0.1f;//pMesh->GetHeight() * this->GetScale();
	
		if(pMesh->GetWidth() > 1.6f  || pMesh->GetDepth() > 1.6f)
		{
			MeshWidth = 2.0f;
			MeshDepth = 2.0f;
		}
	
		if(pMesh)
		{
			if(MeshDepth > 1.6f || MeshWidth > 1.6f)
				Engine->GetMesh("selectbox")->Draw(Engine->Graphics()->GetD3D(), pPosition->x + 0.5f, pPosition->y + 0.5f,pPosition->z, 0.0f, MeshWidth, MeshDepth,MeshHeight,0);
			else
				Engine->GetMesh("selectbox")->Draw(Engine->Graphics()->GetD3D(), pPosition->x,pPosition->y,pPosition->z, 0.0f, MeshWidth, MeshDepth,MeshHeight,0);
		}
	}


	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));

	return TRUE;
}

int Object::AltHighLight()
{
	return TRUE;
}


//end: Display functions ***********************************************



//************ Mutators ************************************************
int Object::SetPosition(D3DVECTOR *pNewPosition)
{
	D3DVECTOR *pPosition;
	pPosition = GetPosition();
	
	*pPosition = *pNewPosition;
	return TRUE;
}

int Object::SetPosition(float x, float y, float z)
{
	D3DVECTOR *pPosition;

	pPosition = GetPosition();
	pPosition->x = x;
	pPosition->y = y;
	pPosition->z = z;
	return TRUE;
}

int Object::SetFrame(int NewFrame)
{
	Frame = NewFrame;
	return TRUE;
}

int Object::SetAngle(float NewAngle)
{
	Angle = NewAngle;
	return TRUE;
}

int Object::SetRadius(float NewRadius)
{
	return TRUE;
}

int Object::SetScale(float NewScale)
{
	Scale = NewScale;
	return TRUE;
}

int Object::SetMesh(ZSModelEx *pNewMesh)
{
	pMesh = pNewMesh;
	return TRUE;
}

int Object::SetNext(Object *pNewNext)
{
	pNext = pNewNext;
	return TRUE;
}

int Object::SetTextureNum(int NewNum)
{
	TextureNum = NewNum;
	return TRUE;
}

int Object::SetTexture(ZSTexture *pNewTexture)
{
	pTexture = pNewTexture;
	return TRUE;
}

BOOL Object::AdvanceFrame()
{
	if(pMesh && ++Frame < pMesh->GetNumFrames())
	{
		return TRUE;
	}
	else
	{
		Frame = 0;
		return TRUE;
	}
}

void Object::Load(FILE *fp)
{
	fread(&Position,	sizeof(Position),1,fp);
	fread(&Frame, sizeof(Frame),1,fp);
	fread(&Data, sizeof(Data),1,fp);
	fread(&Angle, sizeof(Angle),1,fp);
	fread(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fread(&Scale, sizeof(Scale),1,fp);
	fread(&MeshNum, sizeof(MeshNum),1,fp);
	fread(&TextureNum, sizeof(TextureNum),1,fp);

	pMesh = Engine->GetMesh(MeshNum);
	pTexture = Engine->GetTexture(TextureNum);
}

void Object::Move(D3DVECTOR *vDirection)
{
	D3DVECTOR *pPosition;
	pPosition = GetPosition();

	pPosition->x += vDirection->x;
	pPosition->y += vDirection->y;	
	pPosition->z += vDirection->z;
}

BOOL Object::AddItem(GameItem *pGameItem)
{
	//is it stackable
	Object *pOb;
	GameItem *pGI;
	pGI = pGameItem;

	if(this->GetObjectType() == OBJECT_CREATURE && 
		PreludeParty.IsMember((Creature *)this) &&
		pGI->GetData(INDEX_ID).Value == 100)
	{
		Flag *pFlag;
		pFlag = PreludeFlags.Get("PARTYDRACHS");
		pFlag->Value = (void *)((int)pFlag->Value + pGI->GetQuantity());
		char blarg[64];
		sprintf(blarg,"The party receives %i drachs.",pGI->GetQuantity());
		Describe(blarg);
		Engine->Sound()->PlayEffect(27);
		return TRUE;
	}

	if(!pGI->GetModStat())
	{
		pOb = GetContents();
		while(pOb)
		{
			if(((GameItem *)pOb)->GetItem() == pGI->GetItem() && !((GameItem *)pOb)->GetModStat())
			{
				((GameItem *)pOb)->SetQuantity(pGI->GetQuantity() + ((GameItem *)pOb)->GetQuantity());
				return TRUE;
			}
			pOb = pOb->GetNext();
		}
	}

	pGI = new GameItem(pGI);
	if(this->GetObjectType() != OBJECT_CREATURE)
		pGI->SetLocation(LOCATION_CONTAINER);
	else
		pGI->SetLocation(LOCATION_PERSON);
	pGI->SetOwner(this);
	pGI->SetNext(pContents);
	pContents = pGI;

	return	TRUE;	
}

BOOL Object::AddItem(Item *pItem, int Quantity)
{
	GameItem *pGI;
	pGI = new GameItem();
	pGI->SetItem(pItem,Quantity);
	this->AddItem(pGI);
	delete pGI;
	return TRUE;
}
	
BOOL Object::RemoveItem(GameItem *pGameItem)
{
	return RemoveItem(pGameItem->GetItem(),pGameItem->GetQuantity());
}

BOOL Object::RemoveItem(Item *pItem, int Quantity)
{
	Object *pOb;
	GameItem *pGI;
	Object *pLastOb;

	pOb = GetContents();
	//check the first item;
	pGI = (GameItem *)pOb;
	if(!pGI) return FALSE;
	if(pGI->GetItem() == pItem)
	{
		if(pGI->GetQuantity() > Quantity)
		{
			pGI->SetQuantity(pGI->GetQuantity() - Quantity);
		}
		else
		{
			pGI = (GameItem *)pContents;
			pContents = pContents->GetNext();
			delete pGI;
		}
		return TRUE;
	}
		
	pLastOb = pOb;
	pOb = pOb->GetNext();

	while(pOb)
	{
		pGI = (GameItem *)pOb;
		if(pGI->GetItem() == pItem)
		{
			if(pGI->GetQuantity() > Quantity)
			{
				pGI->SetQuantity(pGI->GetQuantity() - Quantity);
			}
			else
			{
				pLastOb->SetNext(pOb->GetNext());
				delete pGI;
			}
			return TRUE;
		}	
		pLastOb = pOb;
		pOb = pOb->GetNext();
	}
	DEBUG_INFO("Tried to remove item from object that didn't have it.\n");
	return FALSE;
}

void Object::AddToArea(Area *pArea)
{
	pArea->AddToUpdate(this);
}



//end: Mutators ********************************************************



//************ Outputs *************************************************
void Object::Save(FILE *fp)
{
	OBJECT_T Type;
	Type = GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);
	MeshNum = Engine->GetMeshNum(pMesh);
	TextureNum = Engine->GetTextureNum(pTexture);
	fwrite(&Position,	sizeof(Position),1,fp);
	fwrite(&Frame, sizeof(Frame),1,fp);
	fwrite(&Data, sizeof(Data),1,fp);
	fwrite(&Angle, sizeof(Angle),1,fp);
	fwrite(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fwrite(&Scale, sizeof(Scale),1,fp);
	fwrite(&MeshNum, sizeof(MeshNum),1,fp);
	fwrite(&TextureNum, sizeof(TextureNum),1,fp);

}

BOOL Object::LineIntersect(D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd)
{
	return FALSE;
}
	

//end: Outputs ********************************************************



//************ Debug ***************************************************

//end: Debug ***********************************************************
int LoadObjects(const char *filename)
{


	return TRUE;
}


