#include "entrance.h"
#include "script.h"
#include "world.h"
#include "actions.h"
#include "events.h"
#include "scriptfuncs.h"
#include "zsengine.h"

ZSModelEx *pIntersectBox = NULL;

int Entrance::GetDefaultAction(Object *pactor) 
{ 
	return ACTION_OPEN; 
}

void Entrance::Draw()
{
	Engine->Graphics()->SetTexture(pTexture);
	
	if(pMesh)
		pMesh->Draw(Engine->Graphics()->GetD3D(),Position.x, Position.y, Position.z, Angle, Scale,Scale,Scale, 0);

	return;
}

BOOL Entrance::Go()
{
	if(PreludeWorld->GetGameState() != GAME_STATE_COMBAT)
	{
		if(EventNum)
		{
			PreludeEvents.RunEvent(EventNum);
			ScriptArg *pSA;
			pSA = Pop();
			if(!pSA->GetValue())
			{
				return FALSE;
			}
		}
		
		PreludeWorld->GotoArea(PreludeWorld->GetAreaNum(DestinationName),DestinationX,DestinationY);
	}
	else
	{
		Describe("You can not do that in combat.");
	}
	return TRUE;
}

void Entrance::Save(FILE *fp)
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

	fwrite(DestinationName, sizeof(char), 32, fp);
	fwrite(&EventNum, sizeof(int), 1, fp);
	fwrite(&DestinationX, sizeof(int), 1, fp);
	fwrite(&DestinationY, sizeof(int), 1, fp);
}

void Entrance::Load(FILE *fp)
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

	fread(DestinationName, sizeof(char), 32, fp);
	fread(&EventNum, sizeof(int), 1, fp);
	fread(&DestinationX, sizeof(int), 1, fp);
	fread(&DestinationY, sizeof(int), 1, fp);

}

Entrance::Entrance()
{
	sprintf(DestinationName,"valley");
	EventNum = 0;
	DestinationX = 24;
	DestinationY = 24;
}

BOOL Entrance::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	if(!pIntersectBox)
	{
		pIntersectBox = Engine->GetMesh("selectbox");
	}

	//transform the ray into object coordinates;
	//first check if ray is close enough to matter
	//point to line distance

	float InverseHeight = 1.0f/( (pMesh->GetHeight() + 0.1f ) * GetScale());
	float InverseWidth = 1.0f/( (pMesh->GetWidth() + 0.1f ) * GetScale());
	float InverseDepth = 1.0f/( (pMesh->GetDepth() + 0.1f ) * GetScale());

	//convert the ray to object coordinates
	//first translate then rotate
	D3DXMATRIX matRotate, matScale, matTransform;
	D3DXMatrixRotationZ( &matRotate, -(GetMyAngle() + PI));
	D3DXMatrixScaling(&matScale, InverseWidth, InverseDepth, InverseHeight);
	D3DXMatrixMultiply(&matTransform, &matRotate, &matScale);
	D3DXVECTOR4 vTransStart, vTransEnd;
	D3DVECTOR vMoveStart, vMoveEnd;
	vMoveStart = *vRayStart - *GetPosition();
	vMoveEnd = *vRayEnd - *GetPosition();
	D3DXVec3Transform(&vTransStart,(D3DXVECTOR3 *)&vMoveStart, &matTransform);
	D3DXVec3Transform(&vTransEnd,(D3DXVECTOR3 *)&vMoveEnd, &matTransform);
	return pIntersectBox->Intersect(0, (D3DVECTOR *)&vTransStart,(D3DVECTOR *)&vTransEnd);
}
