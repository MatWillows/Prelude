#include "ZSbrushwindow.h"
#include "zsengine.h"
#include "chunks.h"
#include "world.h"

LPDIRECTDRAWSURFACE7 BrushWin::CreateBackground()
{
	//fit all the terrains and overlays onto a single 128x128 bitmap
	BackGroundSurface = Engine->Graphics()->CreateSurface(128,128,0,Engine->Graphics()->GetMask());
	
	RECT rSource;
	DDBLTFX BltFX;
	
	ZeroMemory(&BltFX,sizeof(BltFX));

	BltFX.dwSize = sizeof(BltFX);

	int n, xn;
	int left, right, top, bottom;

	D3DTLVERTEX *vp;
	
	Engine->Graphics()->GetD3D()->BeginScene();
	if(!PreludeWorld->GetCurAreaNum())
		Engine->Graphics()->SetTexture(Engine->GetTexture("terrain"));
	else
		Engine->Graphics()->SetTexture(Engine->GetTexture("caveterrain"));
	  
	Engine->Graphics()->GetD3D()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
		               0x00000000, 1.0f, 0L );
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

	for(n = 0; n < NUM_TERRAIN; n++)
	{
		top = (n % 8) * 16;
		bottom = top + 16;
		
		for(xn = 0; xn < 4; xn ++)
		{
			left = ((n / 8) * 64) + xn * 16;
			right = left + 16;
			vp = &Chunk::vTerrain[n*64 + xn*4];
			vp[0].dvSX = (float)left;
			vp[0].dvSY = (float)top;
			vp[0].dvSZ = 0.0f;
			vp[1].dvSX = (float)right;
			vp[1].dvSY = (float)top;
			vp[1].dvSZ = 0.0f;
			vp[2].dvSX = (float)left;
			vp[2].dvSY = (float)bottom;
			vp[2].dvSZ = 0.0f;
			vp[3].dvSX = (float)right;
			vp[3].dvSY = (float)bottom;
			vp[3].dvSZ = 0.0f;
				
			Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX, vp, 4, NULL);
		}
	}
	Engine->Graphics()->GetD3D()->EndScene();

	rSource.left = 0;
	rSource.right = 128;
	rSource.bottom = 128;
	rSource.top = 0;

	BackGroundSurface->Blt(&rSource, Engine->Graphics()->GetBBuffer(), &rSource, NULL, NULL);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
	
	return BackGroundSurface;
}


int BrushWin::Draw()
{
	if(Visible)
	{
		if(State == WINDOW_STATE_NORMAL)
		{
			Engine->Graphics()->GetBBuffer()->Blt(&Bounds,BackGroundSurface,NULL,DDBLT_KEYSRC,NULL);
			if(pChild)
			{
				pChild->Draw();
			}
		}
		else
		{
			if(ValidateMove(&Bounds))
			{
				Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(),&Bounds,RGB(255,0,0));
			}
			else
			{
				Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(),&Bounds,RGB(0,255,0));			
			}
		}
	}
	if(pSibling)
	{
		pSibling->Draw();
	}

	RECT r;
	r.left = BrushX*16 + Bounds.left;
	r.top = (BrushY - TopBrush)*16 + Bounds.top;
	r.right = r.left + 16;
	r.bottom = r.top + 16;

	Engine->Graphics()->DrawBox(Engine->Graphics()->GetBBuffer(), &r, RGB(0,255,0));

	return TRUE; 
}

int BrushWin::LeftButtonDown(int x, int y) 
{ 
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(1);
	}

	SetFocus(this);
	return TRUE; 
}

	
int BrushWin::LeftButtonUp(int x, int y)
{
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}

	int row;
	int column;
	int newbrush;

	row = (y - Bounds.top) / 16;

	BrushY = row + TopBrush;
	BrushX = column = (x - Bounds.left) / 16;
	newbrush = TopBrush+row;

	if(newbrush < NUM_TERRAIN)
	{
		if(column > 3)
		{
			CurBrush = (newbrush+8)*64+(column-4)*4;
		}
		else
		{
			CurBrush = newbrush*64+column*4;
		}
	}
	else
	{
		CurBrush = (NUM_TERRAIN + ((newbrush-NUM_TERRAIN)/4))*64 + 16*((newbrush-NUM_TERRAIN)%4)+column*4;
	}

	ReleaseFocus();
	return TRUE;
}


BrushWin::BrushWin(int NewID, ZSWindow *NewParent)
{
	CurBrush = 0;
	BrushX = 0;
	BrushY = 0;
	TopBrush = 0;

	ID = NewID;

	State = WINDOW_STATE_NORMAL;
	Visible = TRUE;

	CreateBackground();
	NewParent->AddChild(this);

	RECT rParentBounds;
	NewParent->GetBounds(&rParentBounds);

	Bounds.left = rParentBounds.right - 128;
	Bounds.right = rParentBounds.right;
	Bounds.top = rParentBounds.bottom - 128;
	Bounds.bottom = rParentBounds.bottom;
}

