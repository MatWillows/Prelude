#include "translucentwindow.h"
#include "zsutilities.h"
#include "zsengine.h"

void TransWin::Draw()
{
	if(!pWinLink->IsVisible())
	{
		return;
	}
	Engine->Graphics()->ClearTexture();
	
	RECT rBounds;

	pWinLink->GetBounds(&rBounds);

	D3DTLVERTEX	Verts[4];

	Verts[0].sx = rBounds.left + Border;
	Verts[0].sy = rBounds.top + Border;
	Verts[0].sz = 0;
	Verts[0].color = Color;
	Verts[0].rhw = 1.0f;
	Verts[0].specular = Color;
	Verts[0].tu = 0.0f;
	Verts[0].tv = 0.0f;

	Verts[1].sx = rBounds.right - Border;
	Verts[1].sy = rBounds.top + Border;
	Verts[1].sz = 0;
	Verts[1].rhw = 1.0f;
	Verts[1].color = Color;
	Verts[1].specular = Color;
	Verts[1].tu = 0.0f;
	Verts[1].tv = 0.0f;

	Verts[2].sx = rBounds.left + Border;
	Verts[2].sy = rBounds.bottom - Border;
	Verts[2].sz = 0;
	Verts[2].color = Color;
	Verts[2].rhw = 1.0f;
	Verts[2].specular = Color;
	Verts[2].tu = 0.0f;
	Verts[2].tv = 0.0f;

	Verts[3].sx = rBounds.right - Border;
	Verts[3].sy = rBounds.bottom - Border;
	Verts[3].sz = 0;
	Verts[3].color = Color;
	Verts[3].rhw = 1.0f;
	Verts[3].specular = Color;
	Verts[3].tu = 0.0f;
	Verts[3].tv = 0.0f;

	HRESULT hr;
	
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	
	hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, Verts, 4, 0);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);

	if(hr != DD_OK)
	{
		Engine->ReportError(hr);
		SafeExit("failed to draw Translucent window\n");
	}



}

int TransWin::AdvanceFrame()
{


	return TRUE;
}

TransWin::TransWin(ZSWindow *pLink, D3DCOLOR TransColor, int BorderWidth)
{
	Frame = 0;
	Border = BorderWidth;
	pTexture = Engine->GetTexture("items");
	pWinLink = pLink;
	Color = TransColor;
}
