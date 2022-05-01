#include "pattern.h"
#include "zsengine.h"

void Pattern::AddNode(D3DVECTOR vToAdd)
{
	if(NumNodes < MAX_PATTERN_NODES)
	{
		vNodes[NumNodes] = vToAdd;
		NumNodes++;
	}
}

Pattern::Pattern()
{
	NumNodes = 0;
	ParSys.SetInitialVector(D3DVECTOR(0.00f,0.00f,0.033f));
	ParSys.SetInitialVariance(D3DVECTOR(0.0f,0.0f,0.03f));
	ParSys.SetWind(D3DVECTOR(0.0025f,0.0025f,0.0f));
	ParSys.SetGravity(D3DVECTOR(0.0f, 0.0f, 0.00f));
	ParSys.SetAddVector(D3DVECTOR(0.0f, 0.0f, 0.0f));
	ParSys.SetOp(D3DTOP_ADD);
	ParSys.SetMaterial(Engine->Graphics()->GetMaterial(COLOR_ORANGE));
	ParSys.SetLife(30);
	ParSys.SetLifeVariance(20);
	ParSys.SetParticleType(PARTICLE_FLAME);
	ParSys.SetRadius(.25f);
	ParSys.SetEmissionQuantity(3);
	ParSys.SetEnd(0);
	ParSys.AdjustCamera();
	ParSys.AdvanceFrame();
}

BOOL Pattern::AdvanceFrame()
{
	if(NumNodes <= 1) return FALSE;
	//AddNewNodes
	D3DVECTOR *pVStart;
	D3DVECTOR *pVEnd;
	D3DVECTOR vRay;
	D3DVECTOR vRayPos;
	float fMag;
	int OldQuantity;
	OldQuantity = ParSys.GetEmissionQuantity();
	ParSys.SetEmissionQuantity(0);

	for(int nn = 0; nn < (NumNodes-1); nn++)
	{
		pVStart = GetNode(nn);
		pVEnd = GetNode(nn + 1);

		vRay = Normalize(*pVEnd - *pVStart);
		for(int pn = 0; pn < OldQuantity; pn++)
		{
			fMag = (float)(rand() % 100) / 100.0f;
			vRayPos = vRay * fMag;
			vRayPos += *pVStart;
			ParSys.AddParticle(&vRayPos,NULL);
		}
	}

	BOOL Done;

	Done = ParSys.AdvanceFrame();
	ParSys.SetEmissionQuantity(OldQuantity);

	return Done;
}

void Pattern::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->SetTexture(ParSys.GetTexture());
	ParSys.Draw();
}

void Pattern::AdjustCamera()
{
	ParSys.AdjustCamera();
}

void Pattern::Move(D3DVECTOR *vDirection)
{
	for(int n = 0; n < NumNodes; n++)
	{
		vNodes[n] += *vDirection;
	}
}

//intersections
BOOL Pattern::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	return FALSE;
}

BOOL Pattern::RayIntersectAlwaysCheck(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{

	return FALSE;
}

void Bolt::AddNode(D3DVECTOR vToAdd)
{
	if(NumNodes < MAX_PATTERN_NODES)
	{
		vBaseNodes[NumNodes] = vToAdd;

		float frand;
		frand = (float)(rand() % 100) / 100.0f;
		frand = Variation - ((Variation * 2.0f) * frand);
	
		vBaseNodes[NumNodes].x = vBaseNodes[NumNodes].x + frand;
		frand = (float)(rand() % 100) / 100.0f;
		frand = Variation - ((Variation * 2.0f) * frand);
	
		vBaseNodes[NumNodes].y = vBaseNodes[NumNodes].y + frand;
	
		frand = (float)(rand() % 100) / 100.0f;
		frand = Variation - ((Variation * 2.0f) * frand);
	
		vBaseNodes[NumNodes].z = vBaseNodes[NumNodes].z + frand;


		vNodes[NumNodes].x = vToAdd.x;
		vNodes[NumNodes].y = vToAdd.y;
		vNodes[NumNodes].z = vToAdd.z;
		vNodes[NumNodes].nx = 0.0f;
		vNodes[NumNodes].ny = 0.0f;
		vNodes[NumNodes].nz = 1.0f;
		vNodes[NumNodes].tu = 0.0f;
		vNodes[NumNodes].tv = 0.0f;
		NumNodes++;
	}
}

Bolt::Bolt()
{
	Frame = 0;
	NumNodes = 0;
	Color = COLOR_WHITE;
	Variation = 0.2f;
}

BOOL Bolt::AdvanceFrame()
{
	int n;
	Frame ++;
	if(Duration && Frame > Duration)
	{
		return FALSE;
	}
	
	for(n = 0; n < NumNodes; n++)
	{
		float frand;
		frand = (float)(rand() % 100) / 100.0f;
		frand = (Variation - ((Variation * 2.0f) * frand)) / 2.0f;
	
		vNodes[n].x = vBaseNodes[n].x + frand;
		frand = (float)(rand() % 100) / 100.0f;
		frand = (Variation - ((Variation * 2.0f) * frand)) / 2.0f;
	
		vNodes[n].y = vBaseNodes[n].y + frand;
	
		frand = (float)(rand() % 100) / 100.0f;
		frand = (Variation - ((Variation * 2.0f) * frand)) / 2.0f;
	
		vNodes[n].z = vBaseNodes[n].z + frand;
	}

	return TRUE;
}

void Bolt::Draw()
{
	if(NumNodes > 1)
	{
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

		Engine->Graphics()->SetTexture(NULL);
		
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(Color));

		Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

		Engine->Graphics()->GetD3D()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

		Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_VERTEX, vNodes, NumNodes, 0);

		Engine->Graphics()->GetD3D()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	}
}

void Bolt::Create(D3DVECTOR *vStart, D3DVECTOR *vEnd, COLOR_T cColor, float fVariation)
{
	SetColor(cColor);
	SetVariation(fVariation);
	//get the distance;
	float fDistance;
	fDistance = abs(Magnitude(*vEnd - *vStart));

	int NumDivisions = (int)fDistance * 2 + 1;

	float RayDistance;
	RayDistance = fDistance / (float)NumDivisions;

	D3DVECTOR vRay, vCur;
	vRay = Normalize(*vEnd - *vStart);
	
	int n;
	
	for(n = 0; n <= NumDivisions; n++)
	{
		vCur = *vStart + (vRay * (RayDistance*n));	
		AddNode(vCur);
	}

}



