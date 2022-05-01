#include "fountain.h"
#include "world.h"

int Fountain::HighLight(COLOR_T Color)
{
	ZSModelEx *pMesh;
	pMesh = Engine->GetMesh("sphere");

	D3DVECTOR *pPosition;
	pPosition = this->GetPosition();


	pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x,pPosition->y,pPosition->z,1.0f,0);

	return TRUE;


}


void Fountain::SetNumStreams(int NewNum)
{
	if(Droplets)
	{
		delete[] Droplets;
	}

	NumStreams = NewNum;
	Droplets = new ParticleSystem[NumStreams];

	float AngleOffset;
	AngleOffset = PI_MUL_2 / (float)NumStreams;

	D3DVECTOR vNorth(0.0f,Horizontal,Vertical);
	D3DXVECTOR4 vAngle;
	D3DXMATRIX matRotate;
	D3DXMatrixRotationZ(&matRotate,AngleOffset);
	
	for(int n = 0; n < NumStreams; n++)
	{
		D3DXVec3Transform(&vAngle, (D3DXVECTOR3 *)&vNorth, &matRotate);
		vNorth.x = vAngle.x;
		vNorth.y = vAngle.y;
		vNorth.z = vAngle.z;
		
		Droplets[n].SetOrigin(*GetPosition());
		Droplets[n].SetInitialVector(vNorth);
		Droplets[n].SetInitialVariance(D3DVECTOR(Variance,Variance,Variance));
		Droplets[n].SetOp(D3DTOP_ADD);
		Droplets[n].SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
		Droplets[n].SetParticleType(PARTICLE_SPHERE);
		Droplets[n].SetRadius(0.15f);
		Droplets[n].SetEmissionQuantity(1);
		Droplets[n].SetGravity(D3DVECTOR(0.0f,0.0f,-DropletGravity));
		Droplets[n].AdjustCamera();
		Droplets[n].AdvanceFrame();
		Droplets[n].SetLife(90);
		Droplets[n].SetLifeVariance(10);
	}
}

BOOL Fountain::AdvanceFrame()
{
	for(int n = 0; n < NumStreams; n ++)
	{
		Droplets[n].AdvanceFrame();
	}
	return TRUE;
}

void Fountain::AdjustCamera()
{
	for(int n = 0; n < NumStreams; n ++)
	{
		Droplets[n].AdjustCamera();
	}
}


int Fountain::GetDefaultAction(Object *pactor)
{
	return FALSE;
}

void Fountain::Load(FILE *fp)
{
	fread(&Position,	sizeof(Position),1,fp);
	fread(&Frame, sizeof(Frame),1,fp);
	fread(&Data, sizeof(Data),1,fp);
	fread(&Angle, sizeof(Angle),1,fp);
	fread(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fread(&Scale, sizeof(Scale),1,fp);
	fread(&TextureNum, sizeof(TextureNum),1,fp);

	pMesh = NULL;
	pTexture = Engine->GetTexture(TextureNum);

	fread(&Vertical,sizeof(float),1,fp);
	fread(&Horizontal,sizeof(float),1,fp);
	fread(&DropletGravity,sizeof(float),1,fp);
	fread(&Variance,sizeof(float),1,fp);
	fread(&NumStreams,sizeof(int),1,fp);
	SetNumStreams(NumStreams);
}

void Fountain::Save(FILE *fp)
{
	OBJECT_T Type;
	Type = GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);
	TextureNum = Engine->GetTextureNum(pTexture);
	fwrite(&Position,	sizeof(Position),1,fp);
	fwrite(&Frame, sizeof(Frame),1,fp);
	fwrite(&Data, sizeof(Data),1,fp);
	fwrite(&Angle, sizeof(Angle),1,fp);
	fwrite(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fwrite(&Scale, sizeof(Scale),1,fp);
	fwrite(&TextureNum, sizeof(TextureNum),1,fp);

	fwrite(&Vertical,sizeof(float),1,fp);
	fwrite(&Horizontal,sizeof(float),1,fp);
	fwrite(&DropletGravity,sizeof(float),1,fp);
	fwrite(&Variance,sizeof(float),1,fp);
	fwrite(&NumStreams,sizeof(int),1,fp);
}
	
void Fountain::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

	Engine->Graphics()->SetTexture(Droplets[0].GetTexture());

	for(int n = 0; n < NumStreams; n++)
	{
		Droplets[n].Draw();
	}

}

Fountain::~Fountain()
{
	if(Droplets)
	{
		delete[] Droplets;
	}
}

Fountain::Fountain()
{
	Vertical = 0.1f;
	Horizontal = 0.05f;
	DropletGravity = 0.0025f;
	Droplets = NULL;
	NumStreams = 0;
	Variance = 0.005f;

	pTexture = Engine->GetTexture("particle");

}

BOOL Fountain::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	if(PreludeWorld->GetGameState() == GAME_STATE_EDIT)
	{
		//select by base
		float left;
		float right;
		float top;
		float bottom;
		left = Position.x - 0.5f;
		right = Position.x + 0.5f;
		top = Position.y - 0.5f;
		bottom = Position.y + 0.5f;

		if(vRayEnd->x >= left && vRayEnd->y >= top && vRayEnd->x <= right && vRayEnd->y <= bottom)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL Fountain::RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	//select by base
	float left;
	float right;
	float top;
	float bottom;
	left = Position.x - 0.5f;
	right = Position.x + 0.5f;
	top = Position.y - 0.5f;
	bottom = Position.y + 0.5f;

	if(vRayEnd->x >= left && vRayEnd->y >= top && vRayEnd->x <= right && vRayEnd->y <= bottom)
	{
		return TRUE;
	}

	return FALSE;
}

