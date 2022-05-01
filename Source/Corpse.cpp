#include "corpse.h"
#include "creatures.h"
#include "zsengine.h"
#include "zstexture.h"

void Corpse::Draw()
{
	if(pMesh)
		pMesh->Draw(Engine->Graphics()->GetD3D(),Position.x, Position.y, Position.z, Angle, Scale,Scale,Scale, Frame);
}


Corpse::~Corpse()
{
	if(Engine->GetTextureNum(this->GetTexture()))
	{

	}
	else
	{
		if(this->pTexture)
		{
				delete this->pTexture;
				this->pTexture = NULL;
		}

	}
}
		
Corpse::Corpse(Creature *pFrom)
{
	SetTexture(pFrom->GetTexture());
	SetMesh(pFrom->GetMesh());


	if(Engine->GetTextureNum(this->GetTexture()))
	{

	}
	else
	{
		ZSTexture *pTexture;
		pTexture = new ZSTexture(this->GetTexture()->GetSurface(), Engine->Graphics()->GetD3D(), this->GetTexture()->GetWidth(),this->GetTexture()->GetHeight());
		pFrom->SetTexture(pTexture);
	}
	Frame = Creature::Animations[pFrom->GetData(INDEX_TYPE).Value % 10].GetAnim(DIE)->EndFrame;
	SetAngle(pFrom->GetMyAngle());

	SetScale(pFrom->GetScale());
	SetPosition(pFrom->GetPosition());
}

