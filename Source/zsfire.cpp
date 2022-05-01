#include "zsfire.h"
#include "world.h"

Fire::Fire(D3DVECTOR Location, float size, Thing *pLink, int newDuration)
{
	Position = Location;
	Size = size;
	Duration = newDuration;
	Scale = Size * 10.0f;
	Red = new ParticleSystem;
	Yellow = new ParticleSystem;
	vAdd = _D3DVECTOR(size,size,size);
		
	Setup();

	pTexture = Engine->GetTexture("flame");

}

BOOL Fire::AdvanceFrame()
{
	Yellow->AdvanceFrame();
	
	return Red->AdvanceFrame();
}

void Fire::AdjustCamera()
{
	Red->AdjustCamera();
	Yellow->AdjustCamera();
}


void Fire::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->SetTexture(Yellow->GetTexture());
	if(rand() % 2)
	{
		Yellow->Draw();
		Red->Draw();
	}
	else
	{
		Yellow->Draw();
		Red->Draw();
	}
}



Fire::~Fire()
{
	if(Red)
	{
		delete Red;
	}
	if(Yellow)
	{
		delete Yellow;
	}
}

void Fire::SetLinkPoint(int NewPoint)
{

	Red->SetLinkPoint(NewPoint);

	Yellow->SetLinkPoint(NewPoint);

}

void Fire::SetLink(Object *NewLink)
{
	Red->SetLink(NewLink);

	Yellow->SetLink(NewLink);

}

Fire::Fire()
{
	Red = new ParticleSystem();
	Yellow = new ParticleSystem();

	pTexture = Engine->GetTexture("flame");
	vAdd = _D3DVECTOR(0.1f,0.1f,0.1f);

}

void Fire::Move(D3DVECTOR *vDirection)
{
	Red->SetOrigin(Position);
	Yellow->SetOrigin(Position);
}

//i/o
void Fire::Load(FILE *fp)
{
	fread(&Position, sizeof(D3DVECTOR),1,fp);
	fread(&Size, sizeof(float),1,fp);
	Scale = Size * 10;
	fread(&Duration, sizeof(int),1,fp);
	fread(&vAdd, sizeof(D3DVECTOR),1,fp);

	Setup();
}

void Fire::Save(FILE *fp)
{
	OBJECT_T Type;
	Type = GetObjectType();
	fwrite(&Type,sizeof(OBJECT_T),1,fp);
	fwrite(&Position, sizeof(D3DVECTOR),1,fp);
	fwrite(&Size, sizeof(float),1,fp);
	fwrite(&Duration, sizeof(int),1,fp);
	fwrite(&vAdd, sizeof(D3DVECTOR),1,fp);
}

//intersections
BOOL Fire::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	if(PreludeWorld->GetGameState() == GAME_STATE_EDIT)
	{
		//select by base
		float left;
		float right;
		float top;
		float bottom;
		left = Position.x - (Scale / 2.0f);
		right = Position.x + (Scale / 2.0f);
		top = Position.y - (Scale / 2.0f);
		bottom = Position.y + (Scale / 2.0f);

		if(vRayEnd->x >= left && vRayEnd->y >= top && vRayEnd->x <= right && vRayEnd->y <= bottom)
		{
			return TRUE;
		}
	}

	return FALSE;
}

BOOL Fire::RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	//select by base
	float left;
	float right;
	float top;
	float bottom;
	left = Position.x - (Scale / 2.0f);
	right = Position.x + (Scale / 2.0f);
	top = Position.y - (Scale / 2.0f);
	bottom = Position.y + (Scale / 2.0f);

	if(vRayEnd->x >= left && vRayEnd->y >= top && vRayEnd->x <= right && vRayEnd->y <= bottom)
	{
		return TRUE;
	}

	return FALSE;
}

int Fire::HighLight(COLOR_T Color)
{
	ZSModelEx *pMesh;
	pMesh = Engine->GetMesh("sphere");

	D3DVECTOR *pPosition;
	pPosition = this->GetPosition();

//used to display the location of a recta-linear box such as when placing water or adjust heights
	D3DVERTEX Verts[5];
	float z1,z2;
	z1 = pPosition->z;
	z2 = pPosition->z;

	Verts[4].x = Verts[0].x = pPosition->x - vAdd.x;
	Verts[4].y = Verts[0].y = pPosition->y - vAdd.y;
	Verts[4].z = Verts[0].z = z1;

	Verts[1].x = pPosition->x + vAdd.x;
	Verts[1].y = pPosition->y - vAdd.y;
	Verts[1].z = z1;
	
	Verts[2].x = pPosition->x + vAdd.x;
	Verts[2].y = pPosition->y + vAdd.y;
	Verts[2].z = z2;

	Verts[3].x = pPosition->x - vAdd.x;
	Verts[3].y = pPosition->y + vAdd.y;
	Verts[3].z = z2;

	Verts[0].nx = 0;
	Verts[0].ny = 0;
	Verts[0].nz = 1.0f;
	Verts[1].nx = 0;
	Verts[1].ny = 0;
	Verts[1].nz = 1.0f;
	Verts[2].nx = 0;
	Verts[2].ny = 0;
	Verts[2].nz = 1.0f;
	Verts[3].nx = 0;
	Verts[3].ny = 0;
	Verts[3].nz = 1.0f;
	Verts[4].nx = 0;
	Verts[4].ny = 0;
	Verts[4].nz = 1.0f;


	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_VERTEX, Verts, 5,NULL);

//	pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x,pPosition->y,pPosition->z, 1.0f, this->Scale, this->Scale, this->Scale, 0);

	return TRUE;
}

void Fire::Setup()
{
	int n;
	Red->SetOrigin(Position);
	Red->SetInitialVector(D3DVECTOR(0.00f,0.00f,0.33f * Size));
	Red->SetInitialVariance(D3DVECTOR(0.0f,0.0f,0.3f * Size));
	Red->SetWind(D3DVECTOR(0.0f,0.0f,0.0f));
	Red->SetGravity(D3DVECTOR(0.0f, 0.0f, 0.00f));
	Red->SetAddVector(vAdd);
	Red->SetOp(D3DTOP_ADD);
	Red->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_ORANGE));
	Red->SetLife(30);
	Red->SetLifeVariance(20);
	Red->SetParticleType(PARTICLE_FLAME);
	Red->SetRadius(Size * 2.5f);
	Red->SetEmissionQuantity(3);
	Red->SetEnd(Duration);
	for(n = 0; n < 32; n++)
	{
		Red->AddParticle();
		Red->AddParticle();
	}
	Red->AdjustCamera();
	Red->AdvanceFrame();

	Yellow->SetOrigin(Position);
	Yellow->SetInitialVector(D3DVECTOR(0.00f,0.00f,0.2f * Size));
	Yellow->SetInitialVariance(D3DVECTOR(0.0f,0.0f,0.1f * Size));
	Yellow->SetWind(D3DVECTOR(0.0f,0.0f,0.0f));
	Yellow->SetGravity(D3DVECTOR(0.0f, 0.0f, 0.00f));
	Yellow->SetAddVector(vAdd);
	Yellow->SetOp(D3DTOP_ADD);
	Yellow->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_YELLOW));
	Yellow->SetLife(30);
	Yellow->SetLifeVariance(10);
	Yellow->SetEmissionQuantity(1);
	Yellow->SetRadius(Size * 2.5f);

	for( n = 0; n < 8; n++)
	{
		Yellow->AddParticle();
		Yellow->AddParticle();
	}
	Yellow->AdjustCamera();
	Yellow->SetParticleType(PARTICLE_FLAME);
	Yellow->AdvanceFrame();

}

int Fire::SetScale(float NewScale)
{
	Scale = NewScale;
	Size = Scale / 10.0f;
	
	Setup();

	return TRUE;
}

void Fire::SetAdd(D3DVECTOR vNewAdd)
{
	vAdd = vNewAdd;

	Setup();
}



