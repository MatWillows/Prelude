#include "blood.h"

#define MAX_BLOOD_STREAMS			24
#define MIN_BLOOD_STREAMS			12
#define MAX_STREAM_VELOCITY		0.02f
#define MAX_STREAM_SIZE		8

BOOL Blood::BloodEnabled = TRUE;

void Blood::Draw()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->SetTexture(BloodSys.GetTexture());

	BloodSys.Draw();


}

BOOL Blood::AdvanceFrame()
{
	return BloodSys.AdvanceFrame();
}

void Blood::AdjustCamera()
{
	BloodSys.AdjustCamera();
}


Blood::Blood(float size, Thing *pLink)
{
	BloodSys.SetOrigin(D3DVECTOR(0.0f,0.0f,1.0f));
	BloodSys.SetEmissionQuantity(0);
	BloodSys.SetParticleType(PARTICLE_CIRCLE);
	BloodSys.SetRadius(size);
	BloodSys.SetGravity(D3DVECTOR(0.0f,0.0f,-0.01f));
	BloodSys.SetLink(pLink);
	BloodSys.SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
	BloodSys.SetLife(100);
	BloodSys.SetLifeVariance(20);
	BloodSys.SetEnd(25);
	BloodSys.SetOp(D3DTOP_MODULATE);
	BloodSys.SetSourceBlend(D3DBLEND_SRCALPHA);
	BloodSys.SetDestBlend(D3DBLEND_INVSRCALPHA);
	BloodSys.SetLinkPoint(0);
	
	pTexture = Engine->GetTexture("circle");

	int NumStreams;
	int n;
	float xs;
	float ys;
	float zs;
	D3DVECTOR vNewParticle;

	NumStreams = rand() % (MAX_BLOOD_STREAMS - MIN_BLOOD_STREAMS) + MIN_BLOOD_STREAMS;
	
	for(n = 0; n < NumStreams; n++)
	{
		
		xs = ((float)rand()/(float)RAND_MAX * (MAX_STREAM_VELOCITY*2)) - MAX_STREAM_VELOCITY;
		ys = ((float)rand()/(float)RAND_MAX * (MAX_STREAM_VELOCITY*2)) - MAX_STREAM_VELOCITY;
		zs = ((float)rand()/(float)RAND_MAX * (MAX_STREAM_VELOCITY*2)) - MAX_STREAM_VELOCITY;
	
		vNewParticle.x = xs;
		vNewParticle.y = ys;
		vNewParticle.z = zs;
		
		BloodSys.AddParticle(&vNewParticle);
	}
	
	BloodSys.AdjustCamera();

}





