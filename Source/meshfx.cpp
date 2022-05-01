#include "meshfx.h"


BOOL ZSMeshFX::AdvanceFrame()
{
	if(End)
	{
		if(this->GetFrame() > End)
		{
			return FALSE;
		}
	}

	this->SetScale(this->GetScale() + this->GetExpansionRate());
	this->SetAlpha(this->GetAlpha() + this->GetAlphaRate());
	this->SetGreen(this->GetGreen() + this->GetGreenRate());
	this->SetRed(this->GetRed() + this->GetRedRate());
	this->SetBlue(this->GetBlue() + this->GetBlueRate());
	this->SetFrame(this->GetFrame() + 1);

	Material.diffuse.r = Material.ambient.r = this->GetRed();
	Material.diffuse.g = Material.ambient.g = this->GetGreen();
	Material.diffuse.b = Material.ambient.b = this->GetBlue();
	Material.diffuse.a = Material.ambient.a = this->GetAlpha();
	
	return TRUE;
}

//display
void ZSMeshFX::Draw()
{

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_ADD);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	Engine->Graphics()->GetD3D()->SetMaterial(&Material);
	
	if(!Lighting)
	{
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);    
	
		if(this->GetMesh())
		{
			this->GetMesh()->DrawLit(Engine->Graphics()->GetD3D(),
									this->GetPosition()->x,
									this->GetPosition()->y,
									this->GetPosition()->z - (0.5f * this->GetScale()),
									this->GetMyAngle(),
									this->GetScale(),
									this->GetScale(),
									this->GetScale(),
									0); //frame

		}

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);    
	}
	else
	{
		if(this->GetMesh())
		{
			this->GetMesh()->Draw(Engine->Graphics()->GetD3D(),
									this->GetPosition()->x,
									this->GetPosition()->y,
									this->GetPosition()->z - (0.5f * this->GetScale()),
									this->GetMyAngle(),
									this->GetScale(),
									this->GetScale(),
									this->GetScale(),
									0); //frame

		}

	}


	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

	return;
}



ZSMeshFX::ZSMeshFX()
{
	Scale = 1.0f;
	Angle = 1.0f;
	Frame = 0;
	fExpansionRate = 0.0f;
	fAlpha = 0.0f;
	fAlphaRate = 0.0f;
	fRed = 1.0f;
	fRedRate = 0.0f;
	fGreen = 1.0f;
	fGreenRate = 0.0f;
	fBlue = 1.0f;
	fBlueRate = 0.0f;
	Start = 0;
	End = 0;
	pLink = NULL;
	Lighting = FALSE;
	Material = *Engine->Graphics()->GetMaterial(COLOR_WHITE);

	
}
