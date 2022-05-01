#include "worldedit.h"
#include "ZSbrushwindow.h"
#include "world.h"
#include "ZSobjectwindow.h"
#include "ZStoolwindow.h"
#include "objects.h"
#include "zsengine.h"
#include "ZSOldListbox.h"
#include "mainwindow.h"
#include "creatures.h"
#include "gameitem.h"
#include "events.h"
#include "zsbutton.h"
#include "zsintspin.h"
#include "zstext.h"
#include "water.h"
#include "fountain.h"
#include "zsfire.h"
#include "regions.h"
#include "portals.h"
#include "zsgetnumber.h"
#include "entrance.h"
#include "zsgettext.h"
#include "zsconfirm.h"

#define OBJECT_MOVE_AMOUNT      0.05f
#define OBJECT_ROTATION_ANGLE  (PI_MUL_2)/64
#define OBJECT_SCALE_AMOUNT		0.025f

typedef enum
{
	 IDC_BRUSHWIN,	
	 IDC_OBJECTWIN,
	 IDC_CREATUREWIN,
	 IDC_ITEMWIN,
	 IDC_TOOLWIN,
	 IDC_OBJECT_X_PLUS,
	 IDC_OBJECT_Y_PLUS,
	 IDC_OBJECT_Z_PLUS,
	 IDC_OBJECT_X_MINUS,
	 IDC_OBJECT_Y_MINUS,
	 IDC_OBJECT_Z_MINUS,
	 IDC_OBJECT_DELETE,
	 IDC_OBJECT_ROTATE_LEFT,
	 IDC_OBJECT_ROTATE_RIGHT,
	 IDC_OBJECT_SCALE_UP,
	 IDC_OBJECT_SCALE_DOWN,
	 IDC_OBJECT_SELECT,
	 IDC_OBJECT_SWITCH_MESH,
	 IDC_ITEM_QUANTITY,
	 IDC_ITEM_SELECT,
	 IDC_ITEM_DELETE,
	 IDC_EVENT_NUM,
	 IDC_EVENT_NUM_RAPID_UP,
	 IDC_EVENT_NUM_RAPID_DOWN,
	 IDC_EVENT_TYPE,
	 IDC_EVENT_RADIUS,
	 IDC_EVENT_FREQUENCY,
	 IDC_EVENT_HOUR_START,
	 IDC_EVENT_HOUR_END,
	 IDC_EVENT_DELETE,
	 IDC_EVENT_SELECT,
	 IDC_CUR_TOOL_TEXT,
	 IDC_SELECT_TOOL_WIN,

} WORLD_EDIT_CONTROLS;

void WorldEditWin::DrawBox()
{
//used to display the location of a recta-linear box such as when placing water or adjust heights
	D3DVERTEX Verts[5];
	float z1,z2;
	z1 = Valley->GetTileHeight(TileX,TileY);
	z2 = MouseZ;

	Verts[4].x = Verts[0].x = TileX;
	Verts[4].y = Verts[0].y = TileY;
	Verts[4].z = Verts[0].z = z1;

	Verts[1].x = MouseX;
	Verts[1].y = TileY;
	Verts[1].z = z1;
	
	Verts[2].x = MouseX;
	Verts[2].y = MouseY;
	Verts[2].z = z2;

	Verts[3].x = TileX;
	Verts[3].y = MouseY;
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
}




void WorldEditWin::SetEditMode(EDIT_MODE_T NewMode)
{ 
	ZSWindow *pWin;
	if(NewMode != EDIT_MODE_PLACE_CREATURES)
	{
		pWin = GetChild(IDC_CREATUREWIN);
		pWin->Hide();
		HidePersonControls();
	}
	else
	{
		//placing people no longer allowed in world editor, only through people editor (ctrl-p)
		return;
		pWin = GetChild(IDC_CREATUREWIN);
		pWin->Show();
		ShowPersonControls();
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Place Creatures");
	}

	if(NewMode != EDIT_MODE_PLACE_OBJECT)
	{
		pWin = GetChild(IDC_OBJECTWIN);
		pWin->Hide();
		ShowObjectControls();
	}
	else
	{
		pWin = GetChild(IDC_OBJECTWIN);
		pWin->Show();
		HideObjectControls();
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Place Objects");
	}

	if(NewMode != EDIT_MODE_PLACE_ITEMS)
	{
		pWin = GetChild(IDC_ITEMWIN);
		pWin->Hide();
		ShowItemControls();
	}
	else
	{
		pWin = GetChild(IDC_ITEMWIN);
		pWin->Show();
		HideItemControls();
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Place Items");

	}

	if(NewMode != EDIT_MODE_PLACE_EVENTS)
	{
		HideEventControls();
	}
	else
	{
		ShowEventControls();
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Events");
	}

	if(NewMode == EDIT_MODE_WATER)
	{
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Water");
	}
	
	if(NewMode == EDIT_MODE_TILE_HEIGHTS)
	{
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Tile Heights");
		ShowTiles =  TRUE;
	}

	if(NewMode == EDIT_MODE_SELECT_AREA)
	{
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Select Area");
		if(Selected)
		{
			ZSWindow *pSTWin;
			pSTWin = GetChild(IDC_SELECT_TOOL_WIN);
			pSTWin->Show();
		}
	}
	else
	{
		ZSWindow *pSTWin;
		pSTWin = GetChild(IDC_SELECT_TOOL_WIN);
		pSTWin->Hide();
	}

	if(NewMode == EDIT_MODE_WATER_FILL)
	{
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Fill Water");
	}

	if(NewMode == EDIT_MODE_WATER_FILL_SOUTH)
	{
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Fill East Water");
	}

	if(NewMode == EDIT_MODE_WATER_FILL_EAST)
	{
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Fill South Water");
	}


	EditMode = NewMode; 
}

void WorldEditWin::DrawFire()
{
	D3DVECTOR Blarg;

}

void WorldEditWin::DrawFountain()
{
	Fountain *pFountain;
	pFountain = (Fountain *)pCurObject;
	
	D3DVECTOR *pPosition = pCurObject->GetPosition();

	float Vertical;
	Vertical = pFountain->GetVertical();

	float Horizontal;
	Horizontal = pFountain->GetHorizontal();
	
	float DropletGravity;
	DropletGravity = pFountain->GetDropletGravity();

	float Variance;
	Variance = pFountain->GetVariance();

	int NumStreams;
	NumStreams = pFountain->GetNumStreams();

	D3DVECTOR vRay;

	D3DVERTEX Verts[32]; //enough to handle 16 streams

	float StreamHeight;
	StreamHeight = Vertical * 20.0f;

	float StreamLength;
	StreamLength = Horizontal * 20.0f;
	
	float AngleOffset;
	AngleOffset = PI_MUL_2 / (float)NumStreams;

	D3DXVECTOR3 vNorth;
	vNorth.x = 0.0f;
	vNorth.y = StreamLength;
	vNorth.z = StreamHeight;
	D3DXVECTOR4 vAngle;
	D3DXMATRIX matRotate;
	D3DXMatrixRotationZ(&matRotate,AngleOffset);

	for(int n = 0; n < NumStreams; n++)
	{
		D3DXVec3Transform(&vAngle, (D3DXVECTOR3 *)&vNorth, &matRotate);
		vNorth.x = vAngle.x;
		vNorth.y = vAngle.y;
		vNorth.z = vAngle.z;
		Verts[n * 2].x = pPosition->x;
		Verts[n * 2].y = pPosition->y;
		Verts[n * 2].z = pPosition->z;
		Verts[n * 2].nx = 0.0f;
		Verts[n * 2].ny = 0.0f;
		Verts[n * 2].nz = 1.0f;

		Verts[n * 2 + 1].x = vNorth.x + pPosition->x;
		Verts[n * 2 + 1].y = vNorth.y + pPosition->y;
		Verts[n * 2 + 1].z = vNorth.z + pPosition->z;
		Verts[n * 2 + 1].nx = 0.0f;
		Verts[n * 2 + 1].ny = 0.0f;
		Verts[n * 2 + 1].nz = 1.0f;
	}

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
	Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINELIST, D3DFVF_VERTEX, Verts, NumStreams * 2, NULL);

}

int WorldEditWin::Draw()
{
	if(Visible)
	{
		Engine->Graphics()->GetD3D()->BeginScene();
		
		if(WireFrame)
		{
			Valley->DrawTerrain();
		}
		else
		{
			Valley->Draw();
		}
		
		if(ShowTiles) Valley->DrawTiles();

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);    
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		Engine->Graphics()->ClearTexture();
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
		
		Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
		int XStart;
		int XEnd;
		int YStart;
		int YEnd;
		int xn;
		int yn;
		
		if(EditMode != EDIT_MODE_TILE_HEIGHTS)
		{
			XStart = MouseX;
			XStart -= (XStart % 2);
			YStart = MouseY;
			YStart -= (YStart % 2);
			XEnd = XStart + (BrushWidth*2);
			YEnd = YStart + (BrushWidth*2);
		}
		else
		{
			XStart = MouseX;
			YStart = MouseY;
			XEnd = XStart + BrushWidth;
			YEnd = YStart + BrushWidth;
		}

		if(XEnd >= Valley->GetWidth()) XEnd = Valley->GetWidth();
		if(YEnd >= Valley->GetWidth()) YEnd = Valley->GetWidth();
		if(XStart < 0) XStart = 0;
		if(YStart < 0) YStart = 0;
			
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
		
		if(Selected)
		{
			XStart = rSelectBox.left;
			XEnd = rSelectBox.right + 1;
			YStart = rSelectBox.top;
			YEnd = rSelectBox.bottom + 1;
		}

		for(yn = YStart; yn < YEnd; yn++)
		for(xn = XStart; xn < XEnd; xn++)
		{	
			Valley->HighlightTile(xn,yn);
		}


		if(ShowBlocking)
		{
			Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_BLACK));
			for(yn = YStart; yn < YEnd; yn++)
			for(xn = XStart; xn < XEnd; xn++)
			{	
				if(Valley->GetBlocking(xn,yn) & BLOCK_ANY)
					Valley->HighlightTile(xn,yn);
			}


			Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
		}				

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);
		
		if(EditMode == EDIT_MODE_PLACE_OBJECT)
		{
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
			Engine->GetMesh(CurMesh)->Draw(Engine->Graphics()->GetD3D(),(float)(int)MouseX + 0.5f, (float)(int)MouseY + 0.5f,MouseZ,0.0f,0);
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		}
		else
		if((EditMode == EDIT_MODE_PLACE_CREATURES  || EditMode == EDIT_MODE_PLACE_ITEMS) && pCurThing)
		{
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
			pCurThing->GetMesh()->Draw(Engine->Graphics()->GetD3D(),(float)(int)MouseX + 0.5f, (float)(int)MouseY + 0.5f,MouseZ,0.0f,0);
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		}
		else
		if(EditMode == EDIT_MODE_BLOCKING)
		{
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
			Engine->GetMesh("selectbox")->Draw(Engine->Graphics()->GetD3D(),(float)(int)MouseX + 0.5f, (float)(int)MouseY + 0.5f,MouseZ,0.0f,0);
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		}
		else
		if(EditMode == EDIT_MODE_WATER || EditMode == EDIT_MODE_TILE_HEIGHTS || EditMode == EDIT_MODE_SELECT_AREA)
		{
			if(LBDown)
				DrawBox();
		}
		else
		if(EditMode == EDIT_MODE_FOUNTAIN)
		{
			if(pCurObject)
			{
				DrawFountain();
			}
		}

		if(pCurObject)
		{
			pCurObject->HighLight(COLOR_GREEN);
		}

		if(pCurEvent)
		{
			pCurEvent->DrawVisible();
		}

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

		Engine->Graphics()->GetD3D()->EndScene();

		if(pChild)
		{
			pChild->Draw();
		}
		Engine->Graphics()->DrawCursor(Engine->Input()->GetMouseRect());
	}

	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE;
}

int WorldEditWin::LeftButtonDown(int x, int y)
{
	ZSWindow *pWin;

	pWin = GetChild(x,y);

	if(pWin)
	{
		return pWin->LeftButtonDown(x,y);
	}
	else
	{
		if(Cursor == CURSOR_POINT)
		{
			Engine->Graphics()->SetCursorFrame(1);
		}
	}
	XMark = x;
	YMark = y;
	TileX = MouseX;
	TileY = MouseY;
	WaterStart.x = (int)MouseX + 0.5f;
	WaterStart.y = (int)MouseY + 0.5f;
	WaterStart.z = Valley->GetZ(WaterStart.x,WaterStart.y);
	LBDown = TRUE;

	if(Selected)
	{
		Selected = FALSE;
		ZSWindow *pSTWin;
		pSTWin = GetChild(IDC_SELECT_TOOL_WIN);
		pSTWin->Show();
	}

	return TRUE;
}

void WorldEditWin::GetObject()
{
	D3DVECTOR vRayEnd;
	D3DVECTOR vRayStart;
	
	//create the test ray
	vRayEnd = _D3DVECTOR(this->MouseX,this->MouseY,this->MouseZ);

	D3DVECTOR vMouseNear;
	D3DVECTOR vCameraOffset;
	D3DVECTOR vCameraSpace;

	//convert mouse to camera space
	vMouseNear.x = -(1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->left/(float)Engine->Graphics()->GetWidth());
	vMouseNear.y = (1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->top/(float)(Engine->Graphics()->GetHeight() - 100));
	vMouseNear.z = 0.0f;

	vCameraSpace.x = 0;
	vCameraSpace.y = 0;
	vCameraSpace.z = 0;

	D3DMATRIX matView;
	D3DMATRIX matProj;
	D3DMATRIX matWorld;

	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_VIEW, &matView);
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);
	D3DXVECTOR4 vNear;
	D3DXVECTOR4 vCameraLocation;
	
	D3DXMATRIX matInverseView;
	D3DXMATRIX matInverseProj;
	D3DXMATRIX matInverseWorld;
	D3DXMATRIX matTemp;
	D3DXMATRIX matFinal;
	float blarg;

	D3DXMatrixInverse(&matInverseView,&blarg,(D3DXMATRIX *)&matView);
	D3DXMatrixInverse(&matInverseProj,&blarg,(D3DXMATRIX *)&matProj);
	
	D3DXMatrixMultiply(&matFinal,&matInverseProj,&matInverseView);
	
	D3DXVec3Transform(&vNear,(D3DXVECTOR3 *)&vMouseNear,&matFinal);
	
	//now we have the start and end points of our line
	vRayStart = (D3DVECTOR)vNear;
	
	if(EditMode == EDIT_MODE_FOUNTAIN)
	{
		pCurObject = Valley->GetTarget(&vRayStart,&vRayEnd);
		while(pCurObject && (pCurObject->GetObjectType() != OBJECT_FOUNTAIN || !pCurObject->RayIntersect(&vRayStart,&vRayEnd)))
		{
			pCurObject = pCurObject->GetNextUpdate();
		}
	}
	else
	{
		pCurObject = Valley->GetStaticTarget(&vRayStart,&vRayEnd);
		if(!pCurObject)
		{
			pCurObject = Valley->GetTarget(&vRayStart,&vRayEnd);
		}

		if(pCurObject && pCurObject->GetObjectType() == OBJECT_WATER)
		{
			SetEditMode(EDIT_MODE_WATER);
		}

		if(pCurObject && pCurObject->GetObjectType() == OBJECT_FOUNTAIN)
		{
			SetEditMode(EDIT_MODE_FOUNTAIN);
		}
	}


}

void WorldEditWin::GetEvent()
{
	pCurEvent = (Event *)Valley->GetEvent(MouseX,MouseY);
}

void WorldEditWin::GetItem()
{

}

void WorldEditWin::GetPerson()
{

}

int WorldEditWin::LeftButtonUp(int x, int y)
{
	LBDown = FALSE;
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}
	
	BOOL Cleaned[WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT];

	ZeroMemory(Cleaned, sizeof(BOOL) * WORLD_CHUNK_WIDTH * WORLD_CHUNK_HEIGHT);

	int XStart;
	int XEnd;
	int YStart;
	int YEnd;
	int xn;
	int yn;

	float Height;
	
	float YOffset;
	YOffset = YMark - y;
	
	float XOffset;
	XOffset = XMark - x;

	PreludeWorld->ConvertMouse(XMark,YMark, &MouseX, &MouseY, &MouseZ);
	
	XMark = -1;
	YMark = -1;

	D3DVECTOR Position;

	D3DVECTOR *pVector;
			
	ZSIntSpin *pIS;

	switch(EditMode)
	{
		case EDIT_MODE_EDIT_OBJECT:
			GetObject();
			break;
		case EDIT_MODE_SELECT_EVENTS:
			GetEvent();
			break;
		case EDIT_MODE_PLACE_EVENTS:
			pCurEvent = new Event();
			pVector = pCurEvent->GetPosition();
			pVector->x = (float)((int)(MouseX)) + 0.5f;
			pVector->y = (float)((int)(MouseY)) + 0.5f;
			pVector->z = Valley->GetZ(pVector->x,pVector->y);
			pIS = (ZSIntSpin *)GetChild(IDC_EVENT_NUM);
			pCurEvent->SetNum(pIS->GetValue());

			pCurEvent->SetEventType(EVENT_RADIUS);
			
			pIS = (ZSIntSpin *)GetChild(IDC_EVENT_RADIUS);
			pCurEvent->SetRadius((float)pIS->GetValue() - 0.5f);			
			
			pIS = (ZSIntSpin *)GetChild(IDC_EVENT_FREQUENCY);
			pCurEvent->SetFrequency((float)pIS->GetValue());			
			
			Valley->AddToUpdate(pCurEvent);
			break;
		case EDIT_MODE_PLACE_OBJECT:
			Object *pOb;
			pOb = new Object;
			pVector = pOb->GetPosition();
			pVector->x = (float)((int)(MouseX)) + 0.5f;
			pVector->y = (float)((int)(MouseY)) + 0.5f;
			pVector->z = Valley->GetZ(pVector->x,pVector->y);
			pOb->SetMesh(Engine->GetMesh(CurMesh));
			pOb->SetTexture(Engine->GetMesh(CurMesh)->GetTexture());
			if(FillType == EDIT_FILL_RANDOM)
			{
				float Rand;
				Rand = ((float)rand() / (float)RAND_MAX) * PI_MUL_2;
				pOb->SetAngle(Rand);

				Rand = 0.75f + (((float)rand() / (float)RAND_MAX) *0.5f);
				pOb->SetScale(Rand);
			}
			if(pCurObject && pCurObject->GetMesh() == pOb->GetMesh())
			{
				pOb->SetAngle(pCurObject->GetMyAngle());
				pOb->SetScale(pCurObject->GetScale());
			}
			
			Valley->AddObject(pOb);
			return TRUE;

		case EDIT_MODE_PLACE_ITEMS:
			if(pCurThing)
			{
				GameItem *pGI;
				pGI = new GameItem;
				pGI->SetItem((Item *)pCurThing,1);
				D3DVECTOR vPosition;
				vPosition.x = (float)((int)(MouseX)) + 0.5f;
				vPosition.y = (float)((int)(MouseY)) + 0.5f;
				vPosition.z = Valley->GetZ(vPosition.x,vPosition.y);
				pGI->SetPosition(&vPosition);
				pGI->SetLocation(LOCATION_WORLD,NULL);
				Valley->AddToUpdate((Object *)pGI);
			}
			break;
		case EDIT_MODE_TILE:
			XStart = MouseX;
			XStart -= (XStart % 2);
			YStart = MouseY;
			YStart -= (YStart % 2);
			XEnd = XStart + (BrushWidth*2);
			YEnd = YStart + (BrushWidth*2);
			int Brush;
			BOOL TerrainChange;
			TerrainChange = FALSE;
			for(yn = YStart; yn < YEnd; yn++)
			for(xn = XStart; xn < XEnd; xn++)
			{	
				Brush = GetBrush();
				if(Brush >= 0)
				{
					if((Brush / 64) < (NUM_TERRAIN + 2))
					{
						Valley->SetTerrain(xn, yn, Brush);
						TerrainChange = TRUE;
					}
					else
					{
						Valley->AddOverlay(xn,yn, -1);
					}
				}
			}

			//Engine->Graphics()->GetD3D()->BeginScene();
			for(yn = (YStart)/CHUNK_TILE_HEIGHT; yn <= (YEnd)/CHUNK_TILE_HEIGHT; yn++)
			for(xn = (XStart)/CHUNK_TILE_WIDTH; xn <= (XEnd)/CHUNK_TILE_WIDTH; xn++)
			{	
				if(xn >= 0 && yn >= 0)
				{
					Chunk *CP;
					CP = Valley->GetChunk(xn,yn);
					if(CP)
					{
						if(TerrainChange)
							CP->Smooth();
						//CP->CreateTexture(Valley->GetBaseTexture());
						CP->ConvertTerrain();
					}
				}
			}
			//Engine->Graphics()->GetD3D()->EndScene();
			
			return TRUE;
		case EDIT_MODE_TILE_HEIGHTS:
			PreludeWorld->ConvertMouse(x, y, &MouseX, &MouseY, &MouseZ);
			if(Confirm(this,"Level Heights?","yes","no"))
			{
				float z1, z2, z3, z4, zAverage;

				z1 = Valley->GetTileHeight(TileX,TileY);
				z2 = Valley->GetTileHeight(TileX,MouseY);
				z3 = Valley->GetTileHeight(MouseX,TileY);
				z4 = MouseZ;

				zAverage = (z1 + z2 + z3 + z4) / 4.0f;
				
				
				XStart = TileX;
				YStart = TileY;
				XEnd = MouseX;
				YEnd = MouseY;

				int ChunkTileX;
				int ChunkTileY;
				
				for(yn = YStart; yn <= YEnd; yn++)
				for(xn = XStart; xn <= XEnd; xn++)
				{	
					Chunk *pChunk;
					pChunk = Valley->GetChunk(xn/CHUNK_TILE_WIDTH, yn/ CHUNK_TILE_HEIGHT);
					if(pChunk)
					{
						ChunkTileX = xn % CHUNK_TILE_WIDTH;
						ChunkTileY = yn % CHUNK_TILE_HEIGHT;
						pChunk->SetTileHeight(ChunkTileX, ChunkTileY, zAverage);//BaseHeights[xn - XStart][yn - YStart]);
					}
				}
			}
			return TRUE;
		case EDIT_MODE_FOREST:
			Forest *pForest;
			ToolWin *pToolWin;
			pToolWin = (ToolWin *)GetChild(IDC_TOOLWIN);
			
			pForest = pToolWin->GetForest();
			ZSModelEx *pMesh;
			float FillPercent;
			FillPercent = (float)FillRate / 100.0f;
			XStart = MouseX ;
			YStart = MouseY ;
			XEnd = MouseX + (BrushWidth*2);
			YEnd = MouseY + (BrushWidth*2);
			float ScaleOffset;
			Object *Op;
			float Angle;

			for(yn = YStart; yn < YEnd; yn++)
			for(xn = XStart; xn < XEnd; xn++)
			{	
				Valley->ClearTile(xn,yn);
				pMesh = pForest->GetTree(FillPercent,xn, yn);
				if(!pMesh)
				{
					pMesh = pForest->GetShrub(FillPercent,xn, yn);
				}
				if(pMesh)
				{
					ScaleOffset = (float)rand() / (float)RAND_MAX;
					ScaleOffset *= 0.5f;
					Op = new Object;
					Position.x = (float)xn + 0.5f;
					Position.y = (float)yn + 0.5f;
					Position.z = Valley->GetZ((float)xn + 0.5f, (float)yn + 0.5f);
					Op->SetPosition(&Position);
					Op->SetMesh(pMesh);
					Op->SetMeshNum(Engine->GetMeshNum(pMesh));
					Op->SetScale(0.75f + ScaleOffset);
					Angle = ((float)rand() /(float)RAND_MAX) * PI_MUL_2;
					Op->SetAngle(Angle);
					Op->SetTexture(pMesh->GetTexture());
					Op->SetTextureNum(Engine->GetTextureNum(pMesh->GetTexture()));
					Valley->AddObject(Op);
				}
			}
			return TRUE;
		
		
		case EDIT_MODE_SMOOTH:
			XStart = MouseX ;
			YStart = MouseY ;
			XEnd = MouseX + (BrushWidth*2);
			YEnd = MouseY + (BrushWidth*2);
			

			for(yn = YStart; yn < YEnd; yn+=2)
			for(xn = XStart; xn < XEnd; xn+=2)
			{	
				Height = 0.0f;
				Height += Valley->GetTileHeight(xn,yn);
				Height += Valley->GetTileHeight(xn+2, yn-2);
				Height += Valley->GetTileHeight(xn+2, yn);
				Height += Valley->GetTileHeight(xn+2, yn+2);
				Height += Valley->GetTileHeight(xn, yn-2);
				Height += Valley->GetTileHeight(xn, yn+2);
				Height += Valley->GetTileHeight(xn-2, yn-2);
				Height += Valley->GetTileHeight(xn-2, yn);
				Height += Valley->GetTileHeight(xn-2, yn+2);
				Height /= 9.0f;
				
				
				/*Height = (Valley->GetZ((float)(xn)+0.5f,  (float)(yn)+0.5f) +
							Valley->GetZ((float)(xn+1)+0.5f, (float)(yn)+0.5f) +
							Valley->GetZ((float)(xn)+0.5f,   (float)(yn+1)+0.5f) +
							Valley->GetZ((float)(xn-1)+0.5f, (float)(yn)+0.5f) +
							Valley->GetZ((float)(xn)+0.5f,   (float)(yn-1)+0.5f) +
							Valley->GetZ((float)(xn+1)+0.5f, (float)(yn+1)+0.5f) +
							Valley->GetZ((float)(xn-1)+0.5f, (float)(yn+1)+0.5f) +
							Valley->GetZ((float)(xn-1)+0.5f, (float)(yn-1)+0.5f) +
							Valley->GetZ((float)(xn+1)+0.5f, (float)(yn-1)+0.5f)) / 9.0f;
				*/
				Valley->SetTileHeight(xn,yn,Height);
			}

			YStart = YStart / CHUNK_TILE_HEIGHT;
			YEnd = YEnd / CHUNK_TILE_HEIGHT;
			XStart = XStart / CHUNK_TILE_WIDTH;
			XEnd = XEnd / CHUNK_TILE_WIDTH;

			for(yn = YStart; yn <= YEnd; yn++)
			for(xn = XStart; xn <= XEnd; xn++)
			{
				Valley->GetChunk(xn,yn)->CreateHeightMap();
				Valley->GetChunk(xn,yn)->SetObjectHeights();
			}


			return TRUE;
		case EDIT_MODE_WATER:
			PreludeWorld->ConvertMouse(x, y, &MouseX, &MouseY, &MouseZ);
			WaterEnd.x = (int)MouseX + 0.5f;
			WaterEnd.y = (int)MouseY + 0.5f;
			WaterEnd.z = Valley->GetZ(WaterEnd.x,WaterEnd.y);
			Water *pWater;
			pWater = new Water;

			pWater->MakeVerts(WaterStart.x, WaterEnd.x, WaterStart.y, WaterEnd.y, WaterEnd.z);
			Valley->AddObject(pWater);
			
			pCurObject = pWater;
			break;

	
		case EDIT_MODE_SELECT_AREA:
			PreludeWorld->ConvertMouse(x, y, &MouseX, &MouseY, &MouseZ);
			WaterEnd.x = MouseX;
			WaterEnd.y = MouseY;
			WaterEnd.z = MouseZ;
	
			rSelectBox.left = WaterStart.x;
			rSelectBox.top = WaterStart.y;
			rSelectBox.right = WaterEnd.x;
			rSelectBox.bottom = WaterEnd.y;

			Selected = TRUE;
			ZSWindow *pSTWin;
			pSTWin = GetChild(IDC_SELECT_TOOL_WIN);
			pSTWin->Show();

			break;
	

		case EDIT_MODE_FOUNTAIN:
			PreludeWorld->ConvertMouse(x, y, &MouseX, &MouseY, &MouseZ);
			this->GetObject();

			if(!pCurObject)
			{
				Fountain *pFountain;
				pFountain = new Fountain;
				pFountain->SetPosition(MouseX, MouseY, MouseZ);
				pFountain->SetNumStreams(1);
				pCurObject = pFountain;
				Valley->AddToUpdate(pFountain);
			}
			break;

		case EDIT_MODE_BLOCKING:
			BYTE OldBlocking;
			OldBlocking = Valley->GetBlocking(MouseX, MouseY);
			
			if(!OldBlocking)
			{
				Valley->SetBlocking(MouseX, MouseY);
			}
			else
			{
				Valley->ClearBlocking(MouseX, MouseY);
			}
			break;

		default:
			break;
	}

	return TRUE;
}

int WorldEditWin::RightButtonDown(int x, int y)
{
	SetFocus(this);
	return TRUE;
}
int WorldEditWin::RightButtonUp(int x, int y)
{
	ReleaseFocus();
	int XStart;
	int XEnd;
	int YStart;
	int YEnd;
	int xn;
	int yn;	
	
	XStart = MouseX ;
	YStart = MouseY ;
	XEnd = MouseX + (BrushWidth*2);
	YEnd = MouseY + (BrushWidth*2);
	
	float Height;
	int HeightCount;
	float MaxHeight;
	switch(EditMode)
	{
		case EDIT_MODE_HEIGHT:
		case EDIT_MODE_SMOOTH:
			Height = 0.0f;
			HeightCount = 0;
			MaxHeight = 0.0f;
			
			for(yn = YStart; yn < YEnd; yn++)
			for(xn = XStart; xn < XEnd; xn++)
			{
				Height = Valley->GetTileHeight(xn,yn);
				if(Height > MaxHeight)
				{
					MaxHeight = Height;
				}
			}
			
			for(yn = YStart; yn < YEnd; yn+= 2)
			for(xn = XStart; xn < XEnd; xn+= 2)
			{
				Valley->SetTileHeight(xn,yn,MaxHeight);
			}

			YStart = YStart / CHUNK_TILE_HEIGHT;
			YEnd = YEnd / CHUNK_TILE_HEIGHT;
			XStart = XStart / CHUNK_TILE_WIDTH;
			XEnd = XEnd / CHUNK_TILE_WIDTH;

			for(yn = YStart; yn <= YEnd; yn++)
			for(xn = XStart; xn <= XEnd; xn++)
			{
				Valley->GetChunk(xn,yn)->CreateHeightMap();
			}
			break;		
		case EDIT_MODE_PLACE_EVENTS:
			break;

		case EDIT_MODE_TILE_HEIGHTS:
			Height = 0.0f;
			HeightCount = 0;
			MaxHeight = 0.0f;
			
			for(yn = YStart; yn < YEnd; yn++)
			for(xn = XStart; xn < XEnd; xn++)
			{
				Height = Valley->GetTileHeight(xn,yn);
				if(Height > MaxHeight)
				{
					MaxHeight = Height;
				}
			}

			int ChunkTileX;
			int ChunkTileY;

			for(yn = YStart; yn <= YEnd; yn++)
			for(xn = XStart; xn <= XEnd; xn++)
			{	
				Chunk *pChunk;
				pChunk = Valley->GetChunk(xn/CHUNK_TILE_WIDTH, yn/ CHUNK_TILE_HEIGHT);
				if(pChunk)
				{
					ChunkTileX = xn % CHUNK_TILE_WIDTH;
					ChunkTileY = yn % CHUNK_TILE_HEIGHT;
					pChunk->SetTileHeight(ChunkTileX, ChunkTileY, MaxHeight);//BaseHeights[xn - XStart][yn - YStart]);
				}
			}
			break;

		default:
			for(yn = YStart; yn < YEnd; yn++)
			for(xn = XStart; xn < XEnd; xn++)
			{	
				Valley->ClearTile(xn,yn);
			}
			pCurObject = NULL;
			pCurThing = NULL;
			break;

	}
	return TRUE;
}

int WorldEditWin::MoveMouse(long *x, long *y, long *z)
{	

	int XStart;
	int YStart;
	int XEnd;
	int YEnd;
	int xn, yn;

	if(*x < Bounds.left)
	{
		*x = Bounds.left;
	}
	else
	if(*x > Bounds.right)
	{
		*x = Bounds.right;
	}
	
	if(*y < Bounds.top)
	{
		*y = Bounds.top;
	}
	else
	if(*y > Bounds.bottom)
	{
		*y = Bounds.bottom;
	}

	PreludeWorld->ConvertMouse(Engine->Input()->GetMouseRect()->left,Engine->Input()->GetMouseRect()->top,&MouseX,&MouseY,&MouseZ);

	if(YMark > 0 && EditMode == EDIT_MODE_HEIGHT)
	{
		float BaseHeights[128][128];
		XStart = TileX;
		YStart = TileY;
		XEnd = TileX + (BrushWidth * 2);
		YEnd = TileY + (BrushWidth * 2);
	
		float ZOffset;
		ZOffset = (float)(YMark - (float)*y) / 30.0f;
		YMark = *y;

		for(yn = YStart; yn < YEnd; yn += 2)
		for(xn = XStart; xn < XEnd; xn += 2)
		{	
			BaseHeights[xn - XStart][yn - YStart] = Valley->GetTileHeight(xn,yn) + ZOffset;
		}

		for(yn = YStart; yn < YEnd; yn += 2)
		for(xn = XStart; xn < XEnd; xn += 2)
		{	
			Valley->SetTileHeight(xn,yn,BaseHeights[xn - XStart][yn - YStart]);
		}

		YStart = YStart / CHUNK_TILE_HEIGHT;
		YEnd = YEnd / CHUNK_TILE_HEIGHT;
		XStart = XStart / CHUNK_TILE_WIDTH;
		XEnd = XEnd / CHUNK_TILE_WIDTH;

		for(yn = YStart; yn <= YEnd; yn++)
		for(xn = XStart; xn <= XEnd; xn++)
		{
			Valley->GetChunk(xn,yn)->CreateHeightMap();
			Valley->GetChunk(xn,yn)->SetObjectHeights();
		}

		*x = XMark;
	}
	else
	if(YMark > 0 && EditMode == EDIT_MODE_TILE_HEIGHTS)
	{
		/*
		float BaseHeights[128][128];
		XStart = TileX;
		YStart = TileY;
		XEnd = TileX + BrushWidth;
		YEnd = TileY + BrushWidth;
	
		float ZOffset;
		ZOffset = (float)(YMark - (float)*y) / 30.0f;
		YMark = *y;
		int ChunkTileX, ChunkTileY;

		for(yn = YStart; yn < YEnd; yn ++)
		for(xn = XStart; xn < XEnd; xn ++)
		{	
			BaseHeights[xn - XStart][yn - YStart] = Valley->GetTileHeight(xn,yn) + ZOffset;
			Chunk *pChunk;
			pChunk = Valley->GetChunk(xn/CHUNK_TILE_WIDTH, yn/ CHUNK_TILE_HEIGHT);
			if(pChunk)
			{
				ChunkTileX = xn % CHUNK_TILE_WIDTH;
				ChunkTileY = yn % CHUNK_TILE_HEIGHT;
				pChunk->SetTileHeight(ChunkTileX, ChunkTileY, BaseHeights[xn - XStart][yn - YStart]);
			}
		}
		*x = XMark;
		*/
	}

	return TRUE;
}

void WorldEditWin::SelectHeightChange(float Amount)
{
	int xn, yn;
	float fTHeight;
	int XStart = rSelectBox.left;
	int YStart = rSelectBox.top;
	int XEnd = rSelectBox.right;
	int YEnd = rSelectBox.bottom;

	int ChunkTileX;
	int ChunkTileY;
	
	for(yn = YStart; yn <= YEnd; yn++)
	for(xn = XStart; xn <= XEnd; xn++)
	{	
		Chunk *pChunk;
		pChunk = Valley->GetChunk(xn/CHUNK_TILE_WIDTH, yn/ CHUNK_TILE_HEIGHT);
		if(pChunk)
		{
			ChunkTileX = xn % CHUNK_TILE_WIDTH;
			ChunkTileY = yn % CHUNK_TILE_HEIGHT;
			fTHeight = Valley->GetTileHeight(xn,yn);
			fTHeight += Amount;
			pChunk->SetTileHeight(ChunkTileX, ChunkTileY, fTHeight);
		}
	}
}


void WorldEditWin::HandleSpecialCommand(int SCom)
{
	switch(SCom)
	{
	case SELECT_COMMAND_HEIGHT_UP:
		SelectHeightChange(.05f);
		break;
	case SELECT_COMMAND_HEIGHT_UP_LARGE:
		SelectHeightChange(1.0f);
		break;
	case SELECT_COMMAND_HEIGHT_DOWN:
		SelectHeightChange(-0.05f);
		break;
	case SELECT_COMMAND_HEIGHT_DOWN_LARGE:
		SelectHeightChange(-1.0f);
		break;
	case SELECT_COMMAND_BLOCK:
		{
			int xn, yn;
			for(yn = rSelectBox.top; yn <= rSelectBox.bottom; yn ++)
			for(xn = rSelectBox.left; xn <= rSelectBox.right; xn ++)
			{
				Valley->SetBlocking(xn,yn);
			}
		}
		break;
	case SELECT_COMMAND_UNBLOCK:
		{
			int xn, yn;
			for(yn = rSelectBox.top; yn <= rSelectBox.bottom; yn ++)
			for(xn = rSelectBox.left; xn <= rSelectBox.right; xn ++)
			{
				Valley->ClearBlocking(xn,yn);
			}
		}
		break;
	case SELECT_COMMAND_FLATTEN:
		if(Confirm(this,"Level Heights?","yes","no"))
		{
			float z1, z2, z3, z4, zAverage;

			z1 = Valley->GetTileHeight(rSelectBox.left,rSelectBox.top);
			z2 = Valley->GetTileHeight(rSelectBox.right,rSelectBox.bottom);
			z3 = Valley->GetTileHeight(rSelectBox.left,rSelectBox.bottom);
			z4 = Valley->GetTileHeight(rSelectBox.right,rSelectBox.top);
			
			zAverage = (z1 + z2 + z3 + z4) / 4.0f;

			zAverage = z1;
			
			
			int XStart = rSelectBox.left;
			int YStart = rSelectBox.top;
			int XEnd = rSelectBox.right;
			int YEnd = rSelectBox.bottom;

			int ChunkTileX;
			int ChunkTileY;
			int xn, yn;
			
			for(yn = YStart; yn <= YEnd; yn++)
			for(xn = XStart; xn <= XEnd; xn++)
			{	
				Chunk *pChunk;
				pChunk = Valley->GetChunk(xn/CHUNK_TILE_WIDTH, yn/ CHUNK_TILE_HEIGHT);
				if(pChunk)
				{
					ChunkTileX = xn % CHUNK_TILE_WIDTH;
					ChunkTileY = yn % CHUNK_TILE_HEIGHT;
					pChunk->SetTileHeight(ChunkTileX, ChunkTileY, zAverage);//BaseHeights[xn - XStart][yn - YStart]);
				}
			}
		}
		break;

	case SELECT_COMMAND_DONE:
		break;

	case SELECT_COMMAND_FLATTEN_WALLS:
		Valley->FlattenWalls(&rSelectBox);
		break;

	case SELECT_COMMAND_CLEAR_WALLS:
		Valley->ClearWalls(&rSelectBox);
		break;

	case SELECT_COMMAND_PAINT_WALLS:
		Valley->PaintWalls(&rSelectBox, NULL);
		break;


	default:
		break;
	}

	return;
}

int WorldEditWin::Command(int IDFrom, int Command, int Param)
{
	ZSOldList *pWin;
	char ThingName[64];
			
	switch(Command)
	{
	case COMMAND_SPECIAL:
		this->HandleSpecialCommand(Param);
		return TRUE;
	case COMMAND_LIST_SELECTED:
	case COMMAND_LIST_SELECTIONCHANGED:
		if(IDFrom == IDC_OBJECTWIN)
		{
			pWin = (ZSOldList *)GetChild(IDFrom);
			CurMesh = pWin->GetSelection();
		}
		else
		if(IDFrom == IDC_CREATUREWIN)
		{
			pWin = (ZSOldList *)GetChild(IDFrom);
			pWin->GetText(pWin->GetSelection(),ThingName);
			pCurThing = Thing::Find(Creature::GetFirst(),ThingName);
		}	
		else
		if(IDFrom == IDC_ITEMWIN)
		{
			pWin = (ZSOldList *)GetChild(IDFrom);
			pWin->GetText(pWin->GetSelection(),ThingName);
			pCurThing = Thing::Find(Item::GetFirst(),ThingName);
		}
		break;
	case COMMAND_BUTTON_PRESSED:
		switch(IDFrom)
		{
			case IDC_OBJECT_X_PLUS:
				break;
			 case IDC_OBJECT_Y_PLUS:
				break;
			 case IDC_OBJECT_Z_PLUS:
				break;
			 case IDC_OBJECT_X_MINUS:
				break;
			 case IDC_OBJECT_Y_MINUS:
				break;
			 case IDC_OBJECT_Z_MINUS:
				break;
			 case IDC_OBJECT_DELETE:
				break;
			 case IDC_OBJECT_ROTATE_LEFT:
				break;
			 case IDC_OBJECT_ROTATE_RIGHT:
				break;
			 case IDC_OBJECT_SCALE_UP:
				break;
			 case IDC_OBJECT_SCALE_DOWN:
				break;
			 case IDC_OBJECT_SELECT:
				break;
			 case IDC_OBJECT_SWITCH_MESH:
				break;
			 case IDC_ITEM_QUANTITY:
				break;
			 case IDC_ITEM_SELECT:
				break;
			 case IDC_ITEM_DELETE:
				break;
			 case IDC_EVENT_SELECT:
				EditMode = EDIT_MODE_SELECT_EVENTS;
				GetChild(IDC_CUR_TOOL_TEXT)->SetText("select event");
				break;
			 case IDC_EVENT_TYPE:
				//set the current event type
				 break;
			 case IDC_EVENT_DELETE:
				//delete the currently selected event
				if(pCurEvent)
				{
					Valley->RemoveFromUpdate(pCurEvent);
					delete pCurEvent;
					pCurEvent = NULL;
				}
				break;
			 case IDC_EVENT_NUM_RAPID_UP:
				{
					ZSIntSpin *pISpin;
					pISpin = (ZSIntSpin *)GetChild(IDC_EVENT_NUM);
					pISpin->SetValue(pISpin->GetValue() + 50);
					if(pCurEvent)
					{
						pCurEvent->SetNum(pISpin->GetValue());
					}
				}
				break;
			 case IDC_EVENT_NUM_RAPID_DOWN:
				{
					 ZSIntSpin *pISpin;
					pISpin = (ZSIntSpin *)GetChild(IDC_EVENT_NUM);
					pISpin->SetValue(pISpin->GetValue() - 50);
					if(pCurEvent)
					{
						pCurEvent->SetNum(pISpin->GetValue());
					}
				}
				break;
			}
		break;
	case COMMAND_EDIT_CHANGED:
		switch(IDFrom)
		{
		 case IDC_EVENT_NUM:
			if(pCurEvent)
			{
				ZSIntSpin *pISpin;
				pISpin = (ZSIntSpin *)GetChild(IDFrom);
				pCurEvent->SetNum(pISpin->GetValue());
			}
			break;
		 case IDC_EVENT_RADIUS:
			if(pCurEvent)
			{
				ZSIntSpin *pISpin;
				pISpin = (ZSIntSpin *)GetChild(IDFrom);
				pCurEvent->SetRadius((float)pISpin->GetValue() - 0.5f);
			}
			break;
		 case IDC_EVENT_FREQUENCY:
			if(pCurEvent)
			{
				ZSIntSpin *pISpin;
				pISpin = (ZSIntSpin *)GetChild(IDFrom);
				pCurEvent->SetFrequency(pISpin->GetValue());
			}
			break;
		 case IDC_EVENT_HOUR_START:
			if(pCurEvent)
			{
				ZSIntSpin *pISpin;
				pISpin = (ZSIntSpin *)GetChild(IDFrom);
				pCurEvent->SetStart(pISpin->GetValue());
			}
			break;
		 case IDC_EVENT_HOUR_END:
			if(pCurEvent)
			{
				ZSIntSpin *pISpin;
				pISpin = (ZSIntSpin *)GetChild(IDFrom);
				pCurEvent->SetEnd(pISpin->GetValue());
			}
			break;
		 default:
			break;
		}
	default:
		break;
	}
	return TRUE;
}

int WorldEditWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	
	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_P] & 0x80 && !(LastKeys[DIK_P] & 0x80))
	{
		float FireX, FireY, FireZ;
		RECT rDrawAt = *Engine->Input()->GetMouseRect();
		PreludeWorld->ConvertMouse(rDrawAt.left, rDrawAt.top, &FireX, &FireY, &FireZ);
		Fire *pFire;
		pFire = new Fire(_D3DVECTOR(FireX,FireY,FireZ), 0.1f, NULL, 0);
		Valley->AddToUpdate(pFire);
		pCurObject = pFire;
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
	{
		this->SetEditMode(EDIT_MODE_SELECT_AREA);

	
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_G] & 0x80 && !(LastKeys[DIK_G] & 0x80))
	{
		this->SetEditMode(EDIT_MODE_WATER_FILL_EAST);

		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_Z] & 0x80 && !(LastKeys[DIK_Z] & 0x80))
	{
		if(WireFrame)
			WireFrame = FALSE;
		else
			WireFrame = TRUE;
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_S] & 0x80 && !(LastKeys[DIK_S] & 0x80))
	{
		Valley->Save();
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_B] & 0x80 && !(LastKeys[DIK_B] & 0x80))
	{
		EditMode = EDIT_MODE_BLOCKING;
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Blocking");
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_X] & 0x80)
	{
		State = WINDOW_STATE_DONE;
		Visible = FALSE;
		ReleaseFocus();
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_F] & 0x80 && !(LastKeys[DIK_F] & 0x80))
	{
		EditMode = EDIT_MODE_FOUNTAIN;
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Fountain");
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && 
		CurrentKeys[DIK_N] & 0x80 && !(LastKeys[DIK_N] & 0x80))
	{
		SetEditMode (EDIT_MODE_TILE_HEIGHTS);
		return TRUE;
	}


	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_E] & 0x80 && !(LastKeys[DIK_E] & 0x80))
	{
		char *AName;
		int AX;
		int AY;
		int AE;
		
		if(!pCurObject)
		{
			return TRUE;
		}
	
		Entrance *pEntrance;

		if(pCurObject->GetObjectType() != OBJECT_ENTRANCE)
		{
			pEntrance = new Entrance;
			pEntrance->SetMesh(pCurObject->GetMesh());
			pEntrance->SetAngle(pCurObject->GetMyAngle());
			pEntrance->SetScale(pCurObject->GetScale());
			pEntrance->SetTexture(pCurObject->GetTexture());
			pEntrance->SetPosition(pCurObject->GetPosition());
		
			Valley->RemoveFromUpdate(pCurObject);

			D3DVECTOR *pPosition;
			pPosition = pCurObject->GetPosition();
			Region *pRegion;
			pRegion = Valley->GetRegion(pPosition);
			
			if(pRegion)
			{
				pRegion->RemoveObject(pCurObject);
			}

			Valley->GetChunk(pPosition->x / CHUNK_TILE_WIDTH, pPosition->y/ CHUNK_TILE_WIDTH)->RemoveObject(pCurObject);

			delete pCurObject;
			pCurObject = NULL;

			pCurObject = pEntrance;
			Valley->AddToUpdate(pCurObject);
		}
		else
		{
			pEntrance = (Entrance *)pCurObject;
		}

		AName = GetModalText("Destination?", pEntrance->GetDestinationName(), 32);
		AX = GetNumber(0, 6400, pEntrance->GetDestX(),"X",this);
		AY = GetNumber(0, 6400, pEntrance->GetDestY(),"Y",this);
		AE = GetNumber(0, 1000, pEntrance->GetEvent(),"Event",this);

		strcpy(pEntrance->GetDestinationName() ,AName);
		pEntrance->SetDestX(AX);
		pEntrance->SetDestY(AY);
		pEntrance->SetEvent(AE);
		
		delete[] AName;
		
		return TRUE;
	}


	D3DXMATRIX mRotation;
	D3DXVECTOR4 vResult;
	D3DVECTOR vOffset(0.0f,0.0f,0.0f);

//camera controls
	if(CurrentKeys[DIK_NUMPAD8] & 0x80 || CurrentKeys[DIK_UP] & 0x80)
	{
		D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
		vOffset.y = -((ZSMainWindow *)ZSWindow::GetMain())->GetScrollFactor();
		D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
		//move to tope of screen
		PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
	}
	if(CurrentKeys[DIK_NUMPAD2] & 0x80 || CurrentKeys[DIK_DOWN] & 0x80)
	{
		//move to bottom of screen
		D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
		vOffset.y = ((ZSMainWindow *)ZSWindow::GetMain())->GetScrollFactor();;
		D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
		//move to tope of screen
		PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
	}
	if(CurrentKeys[DIK_NUMPAD6] & 0x80 || CurrentKeys[DIK_RIGHT] & 0x80)
	{
		//move to right of screen
		D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
		vOffset.x = ((ZSMainWindow *)ZSWindow::GetMain())->GetScrollFactor();
		D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
		//move to tope of screen
		PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
	}
	if(CurrentKeys[DIK_NUMPAD4] & 0x80 || CurrentKeys[DIK_LEFT] & 0x80)
	{
		//move to left of screen
		D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
		vOffset.x = -((ZSMainWindow *)ZSWindow::GetMain())->GetScrollFactor();;
		D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
		//move to tope of screen
		PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
	}
	
	if(CurrentKeys[DIK_NUMPAD9] & 0x80 || CurrentKeys[DIK_PRIOR] & 0x80)
	{
		//rotate right
		PreludeWorld->RotateCamera(-ROTATION_ANGLE);
//		Valley->ChangeCamera();
		return TRUE;
	}
	if(CurrentKeys[DIK_NUMPAD7] & 0x80 || CurrentKeys[DIK_HOME] & 0x80)
	{
		//rotate left
		PreludeWorld->RotateCamera(ROTATION_ANGLE);
//		Valley->ChangeCamera();
		return TRUE;
	}

	if(CurrentKeys[DIK_NUMPAD1] & 0x80 || CurrentKeys[DIK_MINUS] & 0x80)
	{
		Engine->Graphics()->Zoom(ZOOM_FACTOR);
//		Valley->ChangeCamera();
		return TRUE;
	}

	if(CurrentKeys[DIK_NUMPAD3] & 0x80 || CurrentKeys[DIK_EQUALS] & 0x80)
	{
		Engine->Graphics()->Zoom(-ZOOM_FACTOR);
//		Valley->ChangeCamera();
		return TRUE;
	}

	if(CurrentKeys[DIK_NUMPAD5] & 0x80 || CurrentKeys[DIK_RETURN] & 0x80)
	{
		PreludeWorld->SetCameraOffset(_D3DVECTOR(0.0f,0.0f,0.0f));
		return TRUE;
	}


	if((CurrentKeys[DIK_LSHIFT] & 0x80) || (CurrentKeys[DIK_RSHIFT] & 0x80))
	{
		if((CurrentKeys[DIK_B] & 0x80) && !(LastKeys[DIK_B] & 0x80))
		{
			BrushWidth += 1.0f;
		}

	}
	else
	{
		if((CurrentKeys[DIK_B] & 0x80) && !(LastKeys[DIK_B] & 0x80))
		{
			BrushWidth -= 1.0f;
		}

	
	}

	if(CurrentKeys[DIK_H] & 0x80 && !(LastKeys[DIK_H] & 0x80))
	{
		EditMode = EDIT_MODE_HEIGHT;
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Tileheight");
	}

	if(CurrentKeys[DIK_T] & 0x80 && !(LastKeys[DIK_T] & 0x80))
	{
		if(ShowTiles)
		{
			ShowTiles = FALSE;
		}
		else
		{
			ShowTiles = TRUE;
		}
	}

	if(CurrentKeys[DIK_M] & 0x80 && !(LastKeys[DIK_M] & 0x80))
	{
		if(ShowBlocking)
		{
			ShowBlocking = FALSE;
		}
		else
		{
			ShowBlocking = TRUE;
		}
	}

	float CurAngle;
	float CurScale;
	D3DVECTOR *pPosition;
	
	if((EditMode == EDIT_MODE_SELECT_EVENTS || EditMode == EDIT_MODE_PLACE_EVENTS ||
		EditMode == EDIT_MODE_EDIT_EVENTS) &&
		CurrentKeys[DIK_DELETE] & 0x80 && !(LastKeys[DIK_DELETE] & 0x80) 
		&& pCurEvent )
	{
		//delete the currently selected event
		Valley->RemoveFromUpdate(pCurEvent);
		delete pCurEvent;
		pCurEvent = NULL;
	}

	
	float MoveAmount;
	float RotateAmount;
	float ScaleAmount;
	
	if(EditMode == EDIT_MODE_EDIT_OBJECT && pCurObject)
	{
		RotateAmount = OBJECT_ROTATION_ANGLE;
		MoveAmount = OBJECT_MOVE_AMOUNT;
		ScaleAmount = OBJECT_SCALE_AMOUNT;
		if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
		{
			ScaleAmount *= 10.0f;
			RotateAmount *= 10.0f;
			MoveAmount *= 10.0f;
		}
		CurAngle = pCurObject->GetMyAngle();
		pPosition = pCurObject->GetPosition();
		CurScale = pCurObject->GetScale();

		if(CurrentKeys[DIK_Q] & 0x80 && !(LastKeys[DIK_Q] & 0x80))
		{
			pCurObject->SetAngle(CurAngle - RotateAmount);
		}
		if(CurrentKeys[DIK_E] & 0x80 && !(LastKeys[DIK_E] & 0x80))
		{
			pCurObject->SetAngle(CurAngle + RotateAmount);
		}
		if(CurrentKeys[DIK_W] & 0x80 && !(LastKeys[DIK_W] & 0x80))
		{
			pPosition->y -= MoveAmount;
		}
		if(CurrentKeys[DIK_S] & 0x80 && !(LastKeys[DIK_S] & 0x80))
		{
			pPosition->y += MoveAmount;
		}
		if(CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
		{
			pPosition->x -= MoveAmount;
		}
		if(CurrentKeys[DIK_DELETE] & 0x80 && !(LastKeys[DIK_DELETE] & 0x80))
		{
			Valley->RemoveFromUpdate(pCurObject);
			Region *pRegion;
			pRegion = Valley->GetRegion(pPosition);
			if(pRegion)
			{
				pRegion->RemoveObject(pCurObject);
			}

			Valley->GetChunk(pPosition->x / CHUNK_TILE_WIDTH,pPosition->y/ CHUNK_TILE_WIDTH)->RemoveObject(pCurObject);

			delete pCurObject;
			pCurObject = NULL;
			return TRUE;
		}

		if(CurrentKeys[DIK_D] & 0x80 && !(LastKeys[DIK_D] & 0x80))
		{
			pPosition->x += MoveAmount;
		}
		if(CurrentKeys[DIK_Z] & 0x80 && !(LastKeys[DIK_Z] & 0x80))
		{
			pPosition->z -= MoveAmount;
		}
		if(CurrentKeys[DIK_C] & 0x80 && !(LastKeys[DIK_C] & 0x80))
		{
			pPosition->z += MoveAmount;
		}

		D3DVECTOR vBlank = _D3DVECTOR(0.0f,0.0f,0.0f);

		pCurObject->Move(&vBlank);

		if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_R] & 0x80 && !(LastKeys[DIK_R] & 0x80))
		{
			Valley->GetChunk(pPosition->x/ CHUNK_TILE_WIDTH, pPosition->y/ CHUNK_TILE_WIDTH)->RemoveObject(pCurObject);

			Region *pRegion;
			pRegion = Valley->GetRegion(pPosition);
			if(pRegion)
			{
				pRegion->AddObject(pCurObject);
			}
			
		}
		else
		if(CurrentKeys[DIK_R] & 0x80 && !(LastKeys[DIK_R] & 0x80))
		{
			pCurObject->SetScale(CurScale + ScaleAmount);
		}

		if(CurrentKeys[DIK_F] & 0x80 && !(LastKeys[DIK_F] & 0x80))
		{
			pCurObject->SetScale(CurScale - ScaleAmount);
		}

		if(CurrentKeys[DIK_M] & 0x80 && !(LastKeys[DIK_M] & 0x80))
		{
			pCurObject->SetMesh(Engine->GetMesh(CurMesh));
			pCurObject->SetTexture(Engine->GetMesh(CurMesh)->GetTexture());
		}

		if(CurrentKeys[DIK_K] & 0x80 && !(LastKeys[DIK_K] & 0x80))
		{
			pCurObject->GetObjectType();
			if(pCurObject->GetObjectType() == OBJECT_PORTAL)
			{
				int nkey;
				nkey =  GetNumber(0, 1000, 0, "Key ID",this);
				
				int nlock;
				nlock = GetNumber(0, 1000, 0, "Lock Value",this);

				int nevent;
				nevent = GetNumber(0, 1000, 0, "Event Number",this);

				((Portal *)pCurObject)->SetKey(nkey);
				((Portal *)pCurObject)->SetEvent(nevent);
				((Portal *)pCurObject)->SetLock(nlock);
			}
			else
			{
				int ndata;
				ndata = GetNumber(0, 10000, 0, "Set Data",this);
				pCurObject->SetData(ndata);
			}

		}

		if(CurrentKeys[DIK_P] & 0x80 && !(LastKeys[DIK_P] & 0x80))
		{
			ZSModelEx *pMesh;
			pMesh = pCurObject->GetMesh();
			D3DVECTOR vPosition;
			vPosition = *pCurObject->GetPosition();
			
			Valley->RemoveFromUpdate(pCurObject);
			Region *pRegion;
			pRegion = Valley->GetRegion(pPosition);
			if(pRegion)
			{
				pRegion->RemoveObject(pCurObject);
			}

			Valley->GetChunk(pPosition->x / CHUNK_TILE_WIDTH,pPosition->y/ CHUNK_TILE_WIDTH)->RemoveObject(pCurObject);


			Portal *pPortal;
			pPortal = new Portal;
			pPortal->SetPosition(vPosition.x, vPosition.y, vPosition.z);
			pPortal->SetMesh(pMesh);
			pPortal->SetFrame(0);
			pPortal->SetTexture(pCurObject->GetTexture());
			pPortal->SetAngle(pCurObject->GetMyAngle());
			pPortal->SetOpenAngle(pPortal->GetMyAngle() + PI_DIV_2);
			pPortal->SetClosedAngle(pPortal->GetMyAngle());
			pPortal->SetScale(pCurObject->GetScale());
			pPortal->SetState(PORTAL_CLOSED);
			Valley->AddToUpdate((Object *)pPortal);

			delete pCurObject;
			pCurObject = NULL;
			
			pCurObject = (Object *)pPortal;
		}

		if(pCurObject->GetObjectType() == OBJECT_FIRE)
		{
			D3DVECTOR vFAdd;
			vFAdd = ((Fire *)pCurObject)->GetAdd();
			if(CurrentKeys[DIK_LBRACKET] & 0x80 && !(LastKeys[DIK_LBRACKET] & 0x80))
			{
				if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
				{
					vFAdd.y += 0.05f;
					((Fire *)pCurObject)->SetAdd(vFAdd);
					return TRUE;
				}	
				else
				{
					vFAdd.y -= 0.05f;
					((Fire *)pCurObject)->SetAdd(vFAdd);
					return TRUE;
				}
			}	
			if(CurrentKeys[DIK_SEMICOLON] & 0x80 && !(LastKeys[DIK_SEMICOLON] & 0x80))
			{
				if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
				{
					vFAdd.x += 0.05f;
					((Fire *)pCurObject)->SetAdd(vFAdd);
					return TRUE;
				}	
				else
				{
					vFAdd.x -= 0.05f;
					((Fire *)pCurObject)->SetAdd(vFAdd);
					return TRUE;
				}
			}			
		}

	}
	else
	{
		if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && EditMode != EDIT_MODE_WATER && (CurrentKeys[DIK_W] & 0x80 && !(LastKeys[DIK_W] & 0x80)))
		{
			SetEditMode(EDIT_MODE_WATER);
		}
		else
		{
			if(pCurObject && pCurObject->GetObjectType() == OBJECT_WATER)
			{
				Water *pWater;
				pWater = (Water *)pCurObject;

				D3DVECTOR vMove(0.0f, 0.0f, 0.0f);
				
				if(CurrentKeys[DIK_W] & 0x80 && !(LastKeys[DIK_W] & 0x80))
				{
					vMove.y -= OBJECT_MOVE_AMOUNT;
					pWater->MoveVerts(&vMove, &vMove, &vMove, &vMove);
				}
				if(CurrentKeys[DIK_S] & 0x80 && !(LastKeys[DIK_S] & 0x80))
				{
					vMove.y += OBJECT_MOVE_AMOUNT;
					pWater->MoveVerts(&vMove, &vMove, &vMove, &vMove);
				}
				if(CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
				{
					vMove.x -= OBJECT_MOVE_AMOUNT;
					pWater->MoveVerts(&vMove, &vMove, &vMove, &vMove);
				}
				
				if(CurrentKeys[DIK_D] & 0x80 && !(LastKeys[DIK_D] & 0x80))
				{
					vMove.x += OBJECT_MOVE_AMOUNT;
					pWater->MoveVerts(&vMove, &vMove, &vMove, &vMove);
				}
				if(CurrentKeys[DIK_Z] & 0x80 && !(LastKeys[DIK_Z] & 0x80))
				{
					vMove.z -= OBJECT_MOVE_AMOUNT;
					pWater->MoveVerts(&vMove, &vMove, &vMove, &vMove);
				}
				
				if(CurrentKeys[DIK_C] & 0x80 && !(LastKeys[DIK_C] & 0x80))
				{
					vMove.z += OBJECT_MOVE_AMOUNT;
					pWater->MoveVerts(&vMove, &vMove, &vMove, &vMove);
				}
				
				if(CurrentKeys[DIK_DELETE] & 0x80 && !(LastKeys[DIK_DELETE] & 0x80))
				{
					
				}

				if(CurrentKeys[DIK_LBRACKET] & 0x80 && !(LastKeys[DIK_LBRACKET] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->Expand(0.1f,0,0,0);
					else
						pWater->Expand(-0.1f,0,0,0);
					return TRUE;
				}
				
				if(CurrentKeys[DIK_SEMICOLON] & 0x80 && !(LastKeys[DIK_SEMICOLON] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->Expand(0,0,0,0.1f);
					else
						pWater->Expand(0,0,0,-0.1f);
					return TRUE;
				}
				
				if(CurrentKeys[DIK_APOSTROPHE] & 0x80 && !(LastKeys[DIK_APOSTROPHE] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->Expand(0,0,-0.1f,0);
					else
						pWater->Expand(0,0,0.1f,0);
					return TRUE;
				}
				
				if(CurrentKeys[DIK_SLASH] & 0x80 && !(LastKeys[DIK_SLASH] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->Expand(0, -0.1f, 0, 0);
					else
						pWater->Expand(0, 0.1f, 0, 0);
					return TRUE;
				}

				if(CurrentKeys[DIK_I] & 0x80 && !(LastKeys[DIK_I] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->RaiseEdge(-0.1f,0,0,0);
					else
						pWater->RaiseEdge(0.1f,0,0,0);
					return TRUE;
				}
				
				if(CurrentKeys[DIK_J] & 0x80 && !(LastKeys[DIK_J] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->RaiseEdge(0,0,0,-0.1f);
					else
						pWater->RaiseEdge(0,0,0,0.1f);
					return TRUE;
				}
				
				if(CurrentKeys[DIK_K] & 0x80 && !(LastKeys[DIK_K] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->RaiseEdge(0,0,-0.1f,0);
					else
						pWater->RaiseEdge(0,0,0.1f,0);
					return TRUE;
				}
				
				if(CurrentKeys[DIK_M] & 0x80 && !(LastKeys[DIK_M] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
						pWater->RaiseEdge(0, -0.1f, 0, 0);
					else
						pWater->RaiseEdge(0, 0.1f, 0, 0);
					return TRUE;
				}

				
				//get surrounding water
				Water *pw[3][3];
				Chunk *pChunk;
				Object *pOb;

				int ChunkX;
				ChunkX = pWater->GetPosition()->x / CHUNK_TILE_WIDTH;
				int ChunkY;
				ChunkY = pWater->GetPosition()->y / CHUNK_TILE_HEIGHT;
				int xn,yn;

				for(yn = -1; yn <= 1; yn ++)
				for(xn = -1; xn <= 1; xn ++)
				{
					pChunk = Valley->GetChunk(xn + ChunkX, yn + ChunkY);
					if(pChunk)
					{
						pOb = pChunk->GetObjects();
						while(pOb)
						{
							if(pOb->GetObjectType() == OBJECT_WATER)
							{ 
								pw[xn+1][yn+1] = (Water *)pOb;
								pOb = NULL;
							}
							else
							{
								pOb = pOb->GetNext();
							}
						}
					}
				}
			

				if(CurrentKeys[DIK_R] & 0x80 && !(LastKeys[DIK_R] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
					{
						vMove.z += OBJECT_MOVE_AMOUNT;
					}
					else
					{
						vMove.z -= OBJECT_MOVE_AMOUNT;
					}
					pWater->MoveVerts(&vMove, NULL, NULL, NULL);
					if(pw[0][0]) pw[0][0]->MoveVerts( NULL, NULL, NULL, &vMove);
					if(pw[1][0]) pw[1][0]->MoveVerts( NULL, NULL, &vMove, NULL);
					if(pw[0][1]) pw[0][1]->MoveVerts( NULL, &vMove, NULL, NULL);
				}

				if(CurrentKeys[DIK_T] & 0x80 && !(LastKeys[DIK_T] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
					{
						vMove.z += OBJECT_MOVE_AMOUNT;
					}
					else
					{
						vMove.z -= OBJECT_MOVE_AMOUNT;
					}
					pWater->MoveVerts(NULL, &vMove, NULL, NULL);
					if(pw[1][0]) pw[1][0]->MoveVerts( NULL, NULL, NULL, &vMove);
					if(pw[2][0]) pw[2][0]->MoveVerts( NULL, NULL, &vMove, NULL);
					if(pw[2][1]) pw[2][1]->MoveVerts( &vMove, NULL, NULL, NULL);
				}
				
				if(CurrentKeys[DIK_F] & 0x80 && !(LastKeys[DIK_F] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
					{
						vMove.z += OBJECT_MOVE_AMOUNT;
					}
					else
					{
						vMove.z -= OBJECT_MOVE_AMOUNT;
					}
					pWater->MoveVerts(NULL, NULL, &vMove, NULL);
					if(pw[0][1]) pw[0][1]->MoveVerts( NULL, NULL, NULL, &vMove);
					if(pw[0][2]) pw[0][2]->MoveVerts( NULL, &vMove, NULL, NULL);
					if(pw[1][2]) pw[1][2]->MoveVerts( &vMove, NULL, NULL, NULL);
				}

				if(CurrentKeys[DIK_G] & 0x80 && !(LastKeys[DIK_G] & 0x80))
				{
					if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
					{
						vMove.z += OBJECT_MOVE_AMOUNT;
					}
					else
					{
						vMove.z -= OBJECT_MOVE_AMOUNT;
					}
					pWater->MoveVerts(NULL, NULL, NULL, &vMove);
					if(pw[2][1]) pw[2][1]->MoveVerts( NULL, NULL, &vMove, NULL);
					if(pw[2][2]) pw[2][2]->MoveVerts( &vMove, NULL, NULL, NULL);
					if(pw[1][2]) pw[1][2]->MoveVerts( NULL, &vMove, NULL, NULL);
				}
			}
			else
			if(pCurObject && pCurObject->GetObjectType() == OBJECT_FOUNTAIN)
			{
				Fountain *pFountain;
				pFountain = (Fountain *)pCurObject;
				pPosition = pCurObject->GetPosition();
				RotateAmount = OBJECT_ROTATION_ANGLE;
				MoveAmount = OBJECT_MOVE_AMOUNT;
				ScaleAmount = OBJECT_SCALE_AMOUNT;
				if(CurrentKeys[DIK_LSHIFT] & 0x80 || CurrentKeys[DIK_RSHIFT] & 0x80)
				{
					ScaleAmount *= 10.0f;
					RotateAmount *= 10.0f;
					MoveAmount *= 10.0f;
				}
				if(CurrentKeys[DIK_Q] & 0x80 && !(LastKeys[DIK_Q] & 0x80))
				{
					pFountain->SetHorizontal(pFountain->GetHorizontal() - 0.005f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_E] & 0x80 && !(LastKeys[DIK_E] & 0x80))
				{
					pFountain->SetHorizontal(pFountain->GetHorizontal() + 0.005f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_W] & 0x80 && !(LastKeys[DIK_W] & 0x80))
				{
					pPosition->y -= MoveAmount;
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_S] & 0x80 && !(LastKeys[DIK_S] & 0x80))
				{
					pPosition->y += MoveAmount;
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
				{
					pPosition->x -= MoveAmount;
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_D] & 0x80 && !(LastKeys[DIK_D] & 0x80))
				{
					pPosition->x += MoveAmount;
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_Z] & 0x80 && !(LastKeys[DIK_Z] & 0x80))
				{
					pPosition->z -= MoveAmount;
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_C] & 0x80 && !(LastKeys[DIK_C] & 0x80))
				{
					pPosition->z += MoveAmount;
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_R] & 0x80 && !(LastKeys[DIK_R] & 0x80))
				{
					pFountain->SetNumStreams(pFountain->GetNumStreams() + 1);
				}
				if(CurrentKeys[DIK_F] & 0x80 && !(LastKeys[DIK_F] & 0x80))
				{
					if(pFountain->GetNumStreams() > 1)
						pFountain->SetNumStreams(pFountain->GetNumStreams() - 1);
				}
				if(CurrentKeys[DIK_T] & 0x80 && !(LastKeys[DIK_T] & 0x80))
				{
					pFountain->SetVertical(pFountain->GetVertical() + 0.005f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_G] & 0x80 && !(LastKeys[DIK_G] & 0x80))
				{
					pFountain->SetVertical(pFountain->GetVertical() - 0.005f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_I] & 0x80 && !(LastKeys[DIK_I] & 0x80))
				{
					pFountain->SetDropletGravity(pFountain->GetDropletGravity() + 0.001f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_K] & 0x80 && !(LastKeys[DIK_K] & 0x80))
				{
					pFountain->SetDropletGravity(pFountain->GetDropletGravity() - 0.001f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}
				if(CurrentKeys[DIK_U] & 0x80 && !(LastKeys[DIK_U] & 0x80))
				{
					pFountain->SetVariance(pFountain->GetVariance() + 0.001f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}

				if(CurrentKeys[DIK_J] & 0x80 && !(LastKeys[DIK_J] & 0x80))
				{
					pFountain->SetVariance(pFountain->GetVariance() - 0.001f);
					pFountain->SetNumStreams(pFountain->GetNumStreams());
				}


			}
		}
	}

	if(CurrentKeys[DIK_O] & 0x80 && !(LastKeys[DIK_O] & 0x80))
	{
		EditMode = EDIT_MODE_EDIT_OBJECT;
		GetChild(IDC_CUR_TOOL_TEXT)->SetText("Modify Object");
	}

	
	
	return TRUE;
}

void WorldEditWin::Show()
{
	Visible = TRUE;
	pCurEvent = NULL;
	pCurObject = NULL;

	State = WINDOW_STATE_NORMAL;
	SetCursor(CURSOR_POINT);
	SetFocus(this);
}

WorldEditWin::WorldEditWin()
{
	rSelectBox.left = -1;
	Selected = FALSE;
	LBDown = FALSE;
	BrushWidth = 2.0f;
	EditMode = EDIT_MODE_TILE;

	Visible = TRUE;
	State = WINDOW_STATE_NORMAL;
	
	ZSWindow::GetMain()->AddChild(this);

	Bounds.left = 0;
	Bounds.top = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.bottom = Engine->Graphics()->GetHeight();

	BrushWin *pBrushWin;
	pBrushWin = new BrushWin(IDC_BRUSHWIN, this);
	pBrushWin->Show();

	FillType = EDIT_FILL_NORMAL;
	FillRate = 10;

	ToolWin *pToolWin;
	pToolWin = new ToolWin(IDC_TOOLWIN, this);
	pToolWin->Show();

	SelectToolWin *pSelectToolWin;
	pSelectToolWin = new SelectToolWin(IDC_SELECT_TOOL_WIN);
	AddChild(pSelectToolWin);
	pSelectToolWin->Hide();

	SetFocus(this);

	CurMesh = 0;

	ZSOldList *pList;
	pList = new ZSOldList(IDC_OBJECTWIN, Engine->Graphics()->GetWidth() - 192, 0, 192, 224);
	pList->SetText("Objects");
	AddChild(pList);

	
	ZSModelEx *pMesh;
	int NumMesh;
	NumMesh = Engine->GetNumMesh();
	int n = 0;
	char TempName[64];
	for(n = 0; n < NumMesh; n++)
	{
		pMesh = Engine->GetMesh(n);
		if(pMesh)
		{
			strcpy(TempName,pMesh->GetName());
			TempName[15] = '\0';
			pList->AddItem(TempName);
		}
	}

	pList = new ZSOldList(IDC_CREATUREWIN, Engine->Graphics()->GetWidth() - 192, 0, 192, 224);
	pList->SetText("Creatures");
	AddChild(pList);

	Thing *pThing;
	pThing = Creature::GetFirst();

	while(pThing)
	{
		pList->AddItem(pThing->GetData(INDEX_NAME).String);
		pThing = (Thing *)pThing->GetNext();
	}

	pList = new ZSOldList(IDC_ITEMWIN, Engine->Graphics()->GetWidth() - 192, 0, 192, 224);
	pList->SetText("Items");
	AddChild(pList);

	pThing = Item::GetFirst();

	while(pThing)
	{
		pList->AddItem(pThing->GetData(INDEX_NAME).String);
		pThing = (Thing *)pThing->GetNext();
	}

	int ItemNum;
	ItemNum = pList->FindItem("sphere");
	if(ItemNum >= 0)
		pList->SetSelection(ItemNum);

	ShowTiles = FALSE;
	ShowBlocking = FALSE;
	pCurObject = NULL;
	pCurThing = NULL;
	pCurEvent = NULL;

	ShownEventControls = FALSE;
	ShownObjectControls = FALSE;
	ShownPersonControls = FALSE;
	ShownItemControls = FALSE;

//for lack of a better place create these in the upper left corner

	ZSButton *pButton;
	ZSIntSpin *pSpin;
	ZSText *pText;

	pButton = new ZSButton(BUTTON_NORMAL,IDC_EVENT_SELECT,0,0,64,20);
	pButton->SetText("Select");
	AddChild(pButton);

	pSpin = new ZSIntSpin(IDC_EVENT_NUM,0,20,64,20);
	AddChild(pSpin);

	pButton = new ZSButton(BUTTON_MINUS, IDC_EVENT_NUM_RAPID_DOWN, 64, 20, 20, 20);
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_PLUS, IDC_EVENT_NUM_RAPID_UP, 84, 20, 20, 20);
	AddChild(pButton);
	
	
	pButton = new ZSButton(BUTTON_NORMAL,IDC_EVENT_TYPE,0,40,64,20);
	pButton->SetText("Radius");
	AddChild(pButton);

	pSpin = new ZSIntSpin(IDC_EVENT_RADIUS,0,80,64,20);
	pSpin->SetValue(1);
	AddChild(pSpin);
	
	pText = new ZSText(IDC_EVENT_RADIUS + 100, 64,80, "Rad");
	AddChild(pText);

	pSpin = new ZSIntSpin(IDC_EVENT_FREQUENCY,0,100,64,20);
	pSpin->SetValue(1);
	AddChild(pSpin);
	
	pText = new ZSText(IDC_EVENT_FREQUENCY + 100, 64,100, "Freq");
	AddChild(pText);
	
	pSpin = new ZSIntSpin(IDC_EVENT_HOUR_START,0,120,64,20);
	AddChild(pSpin);

	pText = new ZSText(IDC_EVENT_HOUR_START + 100, 64,120, "Start");
	AddChild(pText);
	
	pSpin = new ZSIntSpin(IDC_EVENT_HOUR_END,0,140,64,20);
	AddChild(pSpin);
	pText = new ZSText(IDC_EVENT_HOUR_END + 100, 64,140, "End");
	AddChild(pText);
	
	pButton = new ZSButton(BUTTON_NORMAL, IDC_EVENT_DELETE,0,180,64,20);
	pButton->SetText("Del");
	AddChild(pButton);

	HideEventControls();

	pText = new ZSText(IDC_CUR_TOOL_TEXT, 300, 0, "paint tiles");
	pText->Show();
	AddChild(pText);

	WireFrame = FALSE;

}

int WorldEditWin::GetBrush()
{
	int Brush;
	int BaseBrush;
	ZSWindow *pWin;
	pWin = GetChild(IDC_BRUSHWIN);
	Brush = ((BrushWin *)pWin)->GetBrush();

	switch(FillType)
	{
		case EDIT_FILL_NORMAL:
			return Brush;
		case EDIT_FILL_RANDOM:
			BaseBrush = Brush - Brush%64;
			return BaseBrush + ((rand() % 4) * 4);
		case EDIT_FILL_SPRAY:
			if(rand() % 100 < FillRate)
			{
				BaseBrush = Brush - Brush%64;
				return BaseBrush + ((rand() % 4) * 4);
			}
			else
			{
				return -1;
			}
			break;
	}
	return Brush;
}

void WorldEditWin::ShowEventControls()
{
	ZSWindow *pWin;

	pWin = GetChild(IDC_EVENT_SELECT);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_NUM);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_NUM_RAPID_UP);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_NUM_RAPID_DOWN);
	pWin->Show();


	pWin = GetChild(IDC_EVENT_TYPE);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_RADIUS);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_FREQUENCY);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_HOUR_START);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_HOUR_END);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_RADIUS + 100);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_FREQUENCY + 100);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_HOUR_START + 100);
	pWin->Show();

	pWin = GetChild(IDC_EVENT_HOUR_END + 100);
	pWin->Show();


	pWin = GetChild(IDC_EVENT_DELETE);
	pWin->Show();

}

void WorldEditWin::HideEventControls()
{
	ZSWindow *pWin;
	
	pWin = GetChild(IDC_EVENT_SELECT);
	pWin->Hide();
	
	pWin = GetChild(IDC_EVENT_NUM);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_NUM_RAPID_UP);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_NUM_RAPID_DOWN);
	pWin->Hide();

	
	pWin = GetChild(IDC_EVENT_TYPE);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_RADIUS);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_FREQUENCY);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_HOUR_START);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_HOUR_END);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_RADIUS + 100);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_FREQUENCY + 100);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_HOUR_START + 100);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_HOUR_END + 100);
	pWin->Hide();

	pWin = GetChild(IDC_EVENT_DELETE);
	pWin->Hide();

}

void WorldEditWin::ShowObjectControls()
{

}

void WorldEditWin::HideObjectControls()
{

}

void WorldEditWin::ShowPersonControls()
{

}

void WorldEditWin::HidePersonControls()
{

}

void WorldEditWin::ShowItemControls()
{

}

void WorldEditWin::HideItemControls()
{

}

int WorldEditWin::GoModal()
{
	//State = WINDOW_STATE_NORMAL;
	//loop while we're not in a done state
	GAME_STATE_T OldState;
	OldState = PreludeWorld->GetGameState();
	PreludeWorld->SetGameState(GAME_STATE_EDIT);
	
	while(State != WINDOW_STATE_DONE)
	{
		//ensure that all windows are drawn by call the main window's draw routine
		pMainWindow->Draw();
		Engine->Graphics()->Flip();
		
		//get any user input
		Engine->Input()->Update(pInputFocus);
	}

	PreludeWorld->SetGameState(OldState);
	
	return ReturnCode;



}

