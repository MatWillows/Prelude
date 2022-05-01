#include "zsparticle.h"
#include "world.h"
#include "zsengine.h"

D3DVECTOR BaseFactorA(0.05f, 0.0f, 0.0f);
D3DVECTOR BaseFactorB(0.0f, 0.05f, 0.0f);

BOOL ParticleSystem::AdvanceFrame()
{
  D3DXMATRIX mxRotate;
  D3DXVECTOR4	vxV;
  D3DVECTOR vTemp;
  
  DrawLength = 0;
  
  D3DXMatrixRotationYawPitchRoll(&mxRotate, vRotation.x, vRotation.y, vRotation.z);

	//position the camera factors properly
	int n;

	int IndexOffset = 0;
	for(n = 0; n < DrawParticles; n++)
	{
		//Move position by the wind
		if(LifeFrame[n] <= LifeLength[n])
		{

			vTemp = Position[n] - ParticleOrigin[n];

			D3DXVec3Transform(&vxV,(D3DXVECTOR3* )&vTemp,&mxRotate);
			
			Position[n] = ((D3DVECTOR)vxV) + ParticleOrigin[n];
		
			Position[n] += vWind;
		
			//Move position by velocity
			Position[n] += Velocity[n];
			
			//remove gravity from velocity
			Velocity[n] += vGravity;

		   D3DXVec3Transform(&vxV,(D3DXVECTOR3* )&Velocity[n],&mxRotate);

		   Velocity[n] = (D3DVECTOR)vxV;

			Particles[IndexOffset].x = Position[n].x;	
			Particles[IndexOffset].y = Position[n].y;
			Particles[IndexOffset].z = Position[n].z;

			Particles[IndexOffset+1].x = Position[n].x + CameraFactorA.x;	
			Particles[IndexOffset+1].y = Position[n].y + CameraFactorA.y;
			Particles[IndexOffset+1].z = Position[n].z + CameraFactorA.z;
		
			Particles[IndexOffset+2].x = Position[n].x + CameraFactorB.x;	
			Particles[IndexOffset+2].y = Position[n].y + CameraFactorB.y;
			Particles[IndexOffset+2].z = Position[n].z + CameraFactorB.z;
			LifeFrame[n]++;

			if(Pulling)
			{
				vTemp = vDestination - Position[n];

				vTemp.x *= Pull;
				vTemp.y *= Pull;
				vTemp.z *= Pull;
				
				Velocity[n] += vTemp;
			}

			ParticleDrawList[DrawLength++] = IndexOffset;
			ParticleDrawList[DrawLength++] = IndexOffset+1;
			ParticleDrawList[DrawLength++] = IndexOffset+2;

		}
		else
		{
		}

		IndexOffset += 3;
	}
	
	EmissionFrame++;
	
	if(!(EmissionFrame % EmissionRate))
	{
		for(n = 0; n < EmissionQuantity; n++)
		{
			AddParticle();
		}
	}
	if(End && EmissionFrame > End)
	{
		return FALSE;
	}

	if(Orbitting)
	{
		 D3DXMatrixRotationYawPitchRoll(&mxRotate, vOrbit.x, vOrbit.y, vOrbit.z);
		 vTemp = vOrigin - vDestination;
		 D3DXVec3Transform(&vxV,(D3DXVECTOR3* )&vTemp,&mxRotate);
			
		 vOrigin = ((D3DVECTOR)vxV) + vDestination;
	}

	vOrigin += vMotion;

	return TRUE;
}

void ParticleSystem::AdjustCamera()
{
	D3DVECTOR vBase(0.0f,0.0f,0.0f);
	CameraFactorA.x = PRadius;
	CameraFactorB.y = PRadius;
	
	CameraFactorA.y = CameraFactorB.x = CameraFactorA.z = CameraFactorB.z = 0;


	PreludeWorld->ConvertToWorld(&vBase);
	PreludeWorld->ConvertToWorld(&CameraFactorA);
	PreludeWorld->ConvertToWorld(&CameraFactorB);

	CameraFactorA -= vBase;
	CameraFactorB -= vBase;
}
	
void ParticleSystem::Draw()
{

	if(!DrawLength)
	{
		return;
	}

	HRESULT hr;


	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, AlphaOp);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, SourceBlend);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, DestBlend);
	Engine->Graphics()->GetD3D()->SetMaterial(&Material);

	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, Particles, MAX_PARTICLES * 3, ParticleDrawList, DrawLength, 0);

	if(hr != D3D_OK)
	{
		Engine->ReportError("problem drawing particle system");
	}

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

	return;
}

int ParticleSystem::AddParticle(D3DVECTOR *pvPosition, D3DVECTOR *pvVelocity)
{
	int Add;

	Add = NumParticles;

	D3DVECTOR pV;

	if(pLink)
	{
		pV = pLink->GetCenter();
	}
	else
	{
		pV = D3DVECTOR(0.0f,0.0f,0.0f);
	}

	ParticleOrigin[Add] = pV + vOrigin;

	Position[Add] = pV + vOrigin + *pvPosition;
	
	if(pvVelocity)
		Velocity[Add] = *pvVelocity;
	else
	{
		Velocity[Add].x = vInitial.x - vInitialVariance.x + (((float)rand()/(float)RAND_MAX) * vInitialVariance.x * 2.0f);
		Velocity[Add].y = vInitial.y - vInitialVariance.y + (((float)rand()/(float)RAND_MAX) * vInitialVariance.y * 2.0f);
		Velocity[Add].z = vInitial.z - vInitialVariance.z + (((float)rand()/(float)RAND_MAX) * vInitialVariance.z * 2.0f);
	}


	LifeFrame[Add] = 0;
	
	if(LifeVariance)
		LifeLength[Add] = Life + (rand() % (LifeVariance*2) - LifeVariance); 
	else
		LifeLength[Add] = Life;
	NumParticles++;
	if(NumParticles >= MAX_PARTICLES)
	{
		NumParticles = 0;
	}

	if(DrawParticles < (MAX_PARTICLES - 1))
	{
		DrawParticles++;
	}

	return TRUE;
}

int ParticleSystem::AddParticle(D3DVECTOR *pvVelocity)
{
	int Add;

	Add = NumParticles;

	D3DVECTOR pV;

	if(pLink)
	{
		pV = *pLink->GetPosition();
	}
	else
	{
		pV = D3DVECTOR(0.0f,0.0f,0.0f);
	}

	ParticleOrigin[Add] = pV + vOrigin;

	Position[Add].x = pV.x + vOrigin.x + ((((float)rand()/(float)RAND_MAX) * (vAdd.x*2)) - vAdd.x);
	
	Position[Add].y = pV.y + vOrigin.y + ((((float)rand()/(float)RAND_MAX) * (vAdd.y*2)) - vAdd.y);
	
	Position[Add].z = pV.z + vOrigin.z + ((((float)rand()/(float)RAND_MAX) * (vAdd.z*2)) - vAdd.z);
	
	Velocity[Add] = *pvVelocity;

	LifeFrame[Add] = 0;
	
	LifeLength[Add] = Life + (rand() % (LifeVariance*2) - LifeVariance); 

	NumParticles++;
	if(NumParticles >= MAX_PARTICLES)
	{
		NumParticles = 0;
	}

	if(DrawParticles < (MAX_PARTICLES - 1))
	{
		DrawParticles++;
	}
	return TRUE;

}

int ParticleSystem::AddParticle()
{
	int Add;

	Add = NumParticles;

	D3DVECTOR pV;

	if(pLink)
	{
		pV = *pLink->GetPosition();
		
		if(LinkPoint)
		{
			pV += pLink->GetMesh()->GetPointVector(LinkPoint,pLink->GetFrame(),pLink->GetMyAngle());		
		}
	}
	else
	{
		pV = D3DVECTOR(0.0f,0.0f,0.0f);
	}

	ParticleOrigin[Add] = pV + vOrigin;

	Position[Add].x = pV.x + vOrigin.x + ((((float)rand()/(float)RAND_MAX) * (vAdd.x*2)) - vAdd.x);
	
	Position[Add].y = pV.y + vOrigin.y + ((((float)rand()/(float)RAND_MAX) * (vAdd.y*2)) - vAdd.y);
	
	Position[Add].z = pV.z + vOrigin.z + ((((float)rand()/(float)RAND_MAX) * (vAdd.z*2)) - vAdd.z);
		
	Velocity[Add].x = vInitial.x - vInitialVariance.x + (((float)rand()/(float)RAND_MAX) * vInitialVariance.x * 2.0f);

	Velocity[Add].y = vInitial.y - vInitialVariance.y + (((float)rand()/(float)RAND_MAX) * vInitialVariance.y * 2.0f);

	Velocity[Add].z = vInitial.z - vInitialVariance.z + (((float)rand()/(float)RAND_MAX) * vInitialVariance.z * 2.0f);

	LifeFrame[Add] = 0;

	if(LifeVariance)
	{
		LifeLength[Add] = Life + (rand() % (LifeVariance*2) - LifeVariance); 
	}
	else
	{
		LifeLength[Add] = Life;
	}
	NumParticles++;
	if(NumParticles >= MAX_PARTICLES)
	{
		NumParticles = 0;
	}

	if(DrawParticles < (MAX_PARTICLES - 1))
	{
		DrawParticles++;
	}
	return TRUE;
}


//constructor
ParticleSystem::ParticleSystem()
{
	int n;
	DrawParticles = NumParticles = 0;
	for(n = 0; n < MAX_PARTICLES; n++)
	{
		Particles[n*3].tu = 0.0f;
		Particles[n*3].tv = 1.0f;
		
		Particles[n*3 + 1].tu = 1.0f;
		Particles[n*3 + 1].tv = 1.0f;

		Particles[n*3 + 2].tu = 0.0f;
		Particles[n*3 + 2].tv = 0.0f;

		Particles[n*3].nx = 0.0f;
		Particles[n*3].ny = 0.0f;
		Particles[n*3].nz = 1.0f;

		Particles[n*3+1].nx = 0.0f;
		Particles[n*3+1].ny = 0.0f;
		Particles[n*3+1].nz = 1.0f;

		Particles[n*3+2].nx = 0.0f;
		Particles[n*3+2].ny = 0.0f;
		Particles[n*3+2].nz = 1.0f;

		LifeFrame[n] = 0;
		LifeLength[n] = 0;
/*
		Particles[n].color = D3DRGBA(1.0f,1.0f,1.0f,0.2f);
		Particles[n].specular = D3DRGBA(1.0f,1.0f,1.0f,0.2f);
		//Particles[n].rhw = 1.0f;
	
		Particles[n + 1].color = D3DRGBA(1.0f,1.0f,0.0f,0.2f);
		Particles[n + 1].specular = D3DRGBA(1.0f,1.0f,1.0f,0.2f);
		//Particles[n + 1].rhw = 1.0f;
	
		Particles[n + 2].color = D3DRGBA(1.0f,1.0f,1.0f,0.2f);
		Particles[n + 2].specular = D3DRGBA(1.0f,1.0f,1.0f,0.2f);
		//Particles[n + 2].rhw = 1.0f;
*/
		
	}

	AlphaOp = D3DTOP_ADD;
	SourceBlend = D3DBLEND_ONE;
	DestBlend = D3DBLEND_ONE;
	Material = *Engine->Graphics()->GetMaterial(COLOR_WHITE);
	
	pTexture = Engine->GetTexture("particle");

	D3DVECTOR vNull = D3DVECTOR(0.0f,0.0f,0.0f);
	
	vGravity = vNull;
	vRotation = vNull;
	vWind = vNull;
	vOrigin = vNull;
	vAdd = vNull;
	vInitial = vNull;
	vInitialVariance = vNull;

	vDestination = vNull;
	vMotion = vNull;

	vOrbit = vNull;

	Pull = 0.0f;

	Orbitting = FALSE;
	Pulling = FALSE;
  
	CameraFactorA = BaseFactorA;
	CameraFactorB = BaseFactorB;

	ParticleType = PARTICLE_SPHERE;

	EmissionFrame = 0;
	EmissionRate = 1;
	EmissionQuantity = 1;

	LinkPoint = 0;
	pLink = NULL;

	Life = 0;
	LifeVariance = 0;

	End = 0;
	DrawLength = 0;
	ZeroMemory(ParticleDrawList,sizeof(unsigned short) * MAX_PARTICLES * 3);
}

ParticleSystem::ParticleSystem(ParticleSystem *pFrom)
{

}

//destructor
ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::SetParticleType(PARTICLE_T NewType)
{
	ParticleType = NewType;

}

void ParticleSystem::Move(D3DVECTOR *vDirection)
{
	vOrigin += *vDirection;
}

//i/o
void ParticleSystem::Load(FILE *fp)
{
	fread(this,sizeof(ParticleSystem),1,fp);
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

void ParticleSystem::Save(FILE *fp)
{
	fwrite(this,sizeof(ParticleSystem),1,fp);
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

//intersections
BOOL ParticleSystem::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{

	return FALSE;
}

BOOL ParticleSystem::RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{

	return FALSE;
}

BOOL ParticleSystem::TestDraw()
{
	BOOL Supported = TRUE;

	HRESULT hr;

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, AlphaOp);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, SourceBlend);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, DestBlend);
	Engine->Graphics()->GetD3D()->SetMaterial(&Material);

	DWORD dwblarg = 1;

	hr = Engine->Graphics()->GetD3D()->ValidateDevice(&dwblarg);

	if(hr != D3D_OK)
	{
		Supported = FALSE;
		Engine->ReportError(hr);
	}

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);


	return Supported;
}

