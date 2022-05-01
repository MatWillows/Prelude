#include "zsRegionEdit.h"
#include "world.h"
#include "zsengine.h"
#include "ZSListbox.h"
#include "zsbutton.h"
#include "zsfloatspin.h"
#include "zstext.h"
#include "zsconfirm.h"
#include "portals.h"

#define UV_ADJUSTMENT 0.05f

typedef enum
{
	IDC_XSCALE,
	IDC_YSCALE,
	IDC_ROTATE_FLOOR_LEFT,
	IDC_ROTATE_FLOOR_RIGHT,
	IDC_TEXTURE,
	IDC_UP_REGION,
	IDC_DOWN_REGION,
	IDC_RESET_REGION,
	IDC_PREVIEW,
	IDC_DELETE,
	IDC_NEW,
	IDC_DIVIDE_WALL,
	IDC_DIVIDE_REGION,
	IDC_ADD_WALL,
	IDC_MOVE,
	IDC_CREATE_REGION,
	IDC_SHOWALL,
	IDC_ROOFHEIGHT,
	IDC_WALLHEIGHT,
	IDC_OVERHANG,
	IDC_ADD_DOOR,
	IDC_ADD_WINDOW,
	IDC_PAINT_WALL,
	IDC_PAINT_WALLS,
	IDC_PAINT_FLOOR,
	IDC_ROTATE_FLOOR,
	IDC_FLOOR_ROTATE_AMOUNT,
	IDC_PAINT_ROOF,
	IDC_SELECT_WALL,
	IDC_WALL_U_PLUS,
	IDC_WALL_U_MINUS,
	IDC_WALL_V_PLUS,
	IDC_WALL_V_MINUS,
	IDC_SET_INTERIOR,
	IDC_SET_EXTERIOR,
	IDC_SET_FLOOR,
	IDC_SET_ROOF,
	IDC_FLOOR_TOOLS,
	IDC_ROOF_TOOLS,
	IDC_WALL_TOOLS,
	IDC_MAIN_TOOLS,
	IDC_ROOF_TYPE,
	IDC_ROOF_SCALE,
	IDC_ROOF_SCALE_SPIN,
	IDC_ROOF_TEXTURE,
	IDC_SCALE_FLOOR,
	IDC_FLOOR_SCALE_AMOUNT,
	IDC_SELECT_FLOOR,
	IDC_PAINT_ALL_FLOORS,
	IDC_PAINT_INTERIOR,
	IDC_PAINT_EXTERIOR,
	IDC_SAVE_REGIONS,
	IDC_LOAD_REGIONS,
	IDC_WALL_TILE,
	IDC_WALL_TILE_AMOUNT,
} REGION_BUTTONS;


BOOL Previewing = FALSE;
BOOL Creating = TRUE;
BOOL Dividing = FALSE;
Wall *pWall = NULL;

extern EditWall *ToDrawWall;
extern EditWall *ToDrawNext;
extern EditRoom *ToDrawRoom;
extern EditPoint *ToDrawPoint;

D3DVECTOR vA;
BOOL APoint = FALSE;

D3DVECTOR vB;
BOOL BPoint = FALSE;

EditPoint *PointA, *PointB;

BOOL ShowTiles = FALSE;

int RegionEditWin::Draw()
{
	if(Visible)
	{
		Engine->Graphics()->GetD3D()->BeginScene();

		Valley->Draw();
	
		Engine->Graphics()->ClearTexture();
		if(ShowTiles) Valley->DrawTiles();

		if(pRegion)
		{
			Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
			if(Previewing)
			{
				if(pRegion->GetParent())
				{
					pRegion->GetParent()->Draw();
					//pRegion->GetParent()->DrawObjects();
				}
				else
				{
					pRegion->Draw();
					//Region->DrawObjects();
				}

			}
			

			if(pWallSide)
			{
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_PURPLE));
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
				Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_VERTEX,pWallSide->Verts,pWallSide->NumVerts,pWallSide->DrawList,pWallSide->NumTriangles,0);
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
			}

			if(pFloorRegion)
			{
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_PURPLE));
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
				pFloorRegion->UnDraw();
				pFloorRegion->Draw();
				pFloorRegion->UnDraw();
//				Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,D3DFVF_VERTEX,pWallSide->Verts,pWallSide->NumVerts,pWallSide->DrawList,pWallSide->NumTriangles,0);
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
				Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
			}

			/*Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_PURPLE));
			pRegion->Highlight();

			if(pWall)
			{
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
				pWall->Draw2D();
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
			}
			if(Dividing)
			{
				if(APoint)
				{
					Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_GREEN));
				
					Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vA.x, vA.y, vA.z, 1.0f, 0);
					if(BPoint)
					{
						Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_YELLOW));
						Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), vB.x, vB.y, vB.z, 1.0f, 0);
					}
				}
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
			}
			*/
		}
		if(!pRegion || !Previewing)
		{
			ERegion.DrawPoints();
			
			Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
			
			ERegion.DrawWalls();
			
			if(ToDrawWall && (Tool == TOOL_ADD_DOOR || Tool == TOOL_ADD_WINDOW || Tool == TOOL_DIVIDE_WALL))
			{
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_BLUE));
				ToDrawWall->Draw();
				if(Tool == TOOL_ADD_DOOR && ToDrawWall->CanDoor())
				{
					D3DVECTOR Blarg;
					Blarg = D3DVECTOR(MouseX,MouseY,MouseZ);
					Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
					DrawDoor(&Blarg,ToDrawWall);
				}
			}
			if(ToDrawRoom)
			{
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
				ToDrawRoom->Draw();
			}		
			if(ToDrawPoint)
			{
				Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_YELLOW));
				Engine->GetMesh(0)->Draw(Engine->Graphics()->GetD3D(), ToDrawPoint->Location.x, ToDrawPoint->Location.y, ToDrawPoint->Location.z, 1.0f, 0);
			}				
		}	
		
		Engine->GetMesh("sphere")->Draw(Engine->Graphics()->GetD3D(),MouseX,MouseY,MouseZ,0.5f,0.5f,0.5f,1.0f,0);

		Engine->Graphics()->GetD3D()->EndScene();

		if(pChild)
		{
			pChild->Draw();
		}
		Engine->Graphics()->DrawCursor(Engine->Input()->GetMouseRect());

	}
	else
	if(pSibling)
	{
		pSibling->Draw();
	}
	return TRUE;
}

int RegionEditWin::LeftButtonDown(int x, int y)
{
	ZSWindow *pWin;
	EditPoint *pPoint;
		

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

		PreludeWorld->ConvertMouse(x, y, &MouseX, &MouseY, &MouseZ);
		if(Tool == TOOL_DIVIDE_WALL ||
			Tool == TOOL_ADD_DOOR)
		{
			D3DVECTOR pIVect;
			D3DVECTOR Blarg;
			EditWall *Closest;
			Blarg = D3DVECTOR(MouseX,MouseY,MouseZ);

			ToDrawWall = Closest = ERegion.GetNearestWall(&Blarg);
			if(Closest)
			{
				PointToLineIntersect(&pIVect, &Blarg, &Closest->Start->Location, &Closest->End->Location);
				MouseX = pIVect.x;
				MouseY = pIVect.y;
				MouseZ = pIVect.z;
				SetFocus(this);
			}
			return TRUE;			
		}
		else
		if(Tool == TOOL_MOVE_POINT)
		{
			D3DVECTOR vB;
			vB = D3DVECTOR(MouseX,MouseY,MouseZ);
			pPoint = ERegion.GetNearestPoint(&vB);

			if(GetDistance(&pPoint->Location, &vB) < 0.1f)
			{
				PointA = pPoint;
				MouseX = pPoint->Location.x;
				MouseY = pPoint->Location.y;
				MouseZ = pPoint->Location.z;
			}
			SetFocus(this);
			return TRUE;
		}


		MouseX = (float)(int)(MouseX) + 0.5f;
		MouseY = (float)(int)(MouseY) + 0.5f;
		MouseZ = Valley->GetTileHeight(MouseX,MouseY);

		D3DVECTOR Blarg;
		Blarg = D3DVECTOR(MouseX, MouseY, MouseZ);
		pPoint = ERegion.FindPoint(&Blarg);

		switch(Tool)
		{
		case TOOL_MOVE_POINT:
			if(pPoint)
			{
				//store the current point.
				PointA = pPoint;
			}
			else
			{

			}
			break;

		case TOOL_ADD_WALL:
			if(!pPoint)
			{
				pPoint = new EditPoint(&Blarg);
				ERegion.AddPoint(pPoint);
			}
			PointA = pPoint;
			break;
		case TOOL_PAINT_WALL:
			break;		
		case TOOL_PAINT_FLOOR:
			break;
		}

		SetFocus(this);
	}
	return TRUE;
}

int RegionEditWin::LeftButtonUp(int x, int y)
{
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}
	

	D3DVECTOR IntersectPoint;
	D3DVECTOR pIVect;
	D3DVECTOR Blarg;
	EditWall *Closest;
		
	PreludeWorld->ConvertMouse(x, y, &MouseX, &MouseY, &MouseZ);
	if(Tool == TOOL_DIVIDE_WALL ||
		Tool == TOOL_ADD_DOOR)
	{
		Blarg = D3DVECTOR(MouseX,MouseY,MouseZ);

		Closest = ERegion.GetNearestWall(&Blarg);
		if(Closest)
		{
			PointToLineIntersect(&pIVect, &Blarg, &Closest->Start->Location, &Closest->End->Location);
			MouseX = pIVect.x;
			MouseY = pIVect.y;
			MouseZ = pIVect.z;
		
			if(Tool == TOOL_ADD_DOOR)
			{
				switch(Closest->GetOrientation())
				{
					case NORTH:
						MouseX = (float)(int)MouseX + 0.5f;
						MouseY = (float)(int)MouseY;
						break;
					case SOUTH:
						MouseX = (float)(int)MouseX + 0.5f;
						MouseY = (float)(int)MouseY;
						break;
					case EAST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY + 0.5f;
						break;
					case WEST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY + 0.5;
						break;
					case NORTHEAST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					case SOUTHEAST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					case NORTHWEST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					case SOUTHWEST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					default:
						break;
				}
			}
		}
	}
	else
	{	
		if(Tool != TOOL_PAINT_WALL &&
			Tool != TOOL_PAINT_FLOOR &&
			Tool != TOOL_PAINT_ROOF && 
			Tool != TOOL_SELECT_WALL &&
			Tool != TOOL_SELECT_FLOOR)
		{
			MouseX = (float)(int)(MouseX) + 0.5f;
			MouseY = (float)(int)(MouseY) + 0.5f;
		}
		MouseZ = Valley->GetTileHeight(MouseX,MouseY);
	}

	Blarg = D3DVECTOR(MouseX, MouseY, MouseZ);
	EditPoint *pPoint;
	pPoint = ERegion.FindPoint(&Blarg);

	switch(Tool)
	{
	case TOOL_ADD_DOOR:
		if(Closest)
		{
			ERegion.DoorWall(Closest,&Blarg);
			ToDrawWall = NULL;
		}
		break;		
	case TOOL_DIVIDE_WALL:
		EditWall *pWall;
		pWall = ERegion.GetNearestWall(&Blarg);
		if(pWall)
		{
			PointToLineIntersect(&IntersectPoint,&Blarg, &pWall->Start->Location, &pWall->End->Location);
			ERegion.DivideWall(pWall, &IntersectPoint);
			ToDrawWall = NULL;
		}
		break;
	case TOOL_MOVE_POINT:
		if(PointA)
		{
			PointA->Location = Blarg;
			PointA = NULL;
		}
		break;
	case TOOL_ADD_WALL:
		if(!pPoint)
		{
			pPoint = new EditPoint(&Blarg);
			ERegion.AddPoint(pPoint);
		}
		
		PointB = pPoint;
		
		ERegion.DefineWall(PointA, PointB);

		break;
	case TOOL_SELECT_WALL:
		GetWall();
		break;
	case TOOL_SELECT_FLOOR:
		GetFloor();
		break;
	case TOOL_LINK_DOOR:
		LinkDoor(x,y);
		break;
	}

	ReleaseFocus();
	return TRUE;
}

int RegionEditWin::RightButtonDown(int x, int y)
{
	SetFocus(this);
	return TRUE;
}

int RegionEditWin::RightButtonUp(int x, int y)
{
	EditPoint *pKillPoint;
	D3DVECTOR Blarg;
	switch(Tool)
	{
		case TOOL_DIVIDE_WALL:
			if(ToDrawWall)
			{
				ERegion.RemoveWall(ToDrawWall);
				ToDrawWall = NULL;
			}
			break;
		case TOOL_ADD_WALL:
			Blarg = D3DVECTOR(MouseX,MouseY,MouseZ);
			pKillPoint = ERegion.FindPoint(&Blarg);
			if(pKillPoint)
			{
				ERegion.RemovePoint(pKillPoint);
			}
			break;
		default:
			
			break;
	}

	D3DVECTOR MouseVect;
	MouseVect.x = MouseX;
	MouseVect.y = MouseY;
	MouseVect.z = MouseZ;
	
	Region *pFoundRegion;
	
	pFoundRegion = Valley->GetRegion(&MouseVect);

	if(pFoundRegion)
	{
		pRegion = pFoundRegion;
	}

	ReleaseFocus();
	return TRUE;
}

int RegionEditWin::MoveMouse(long *x, long *y, long *z)
{	

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
	
	if(Tool == TOOL_DIVIDE_WALL ||
		Tool == TOOL_ADD_DOOR)
	{
		D3DVECTOR pIVect;
		D3DVECTOR Blarg;
		EditWall *Closest;
		Blarg = D3DVECTOR(MouseX,MouseY,MouseZ);

		ToDrawWall = Closest = ERegion.GetNearestWall(&Blarg);

		if(Closest)
		{
			PointToLineIntersect(&pIVect, &Blarg, &Closest->Start->Location, &Closest->End->Location);
			MouseX = pIVect.x;
			MouseY = pIVect.y;
			MouseZ = pIVect.z;
			if(Tool == TOOL_ADD_DOOR)
			{
				switch(Closest->GetOrientation())
				{
					case NORTH:
						MouseX = (float)(int)MouseX + 0.5f;
						MouseY = (float)(int)MouseY;
						break;
					case SOUTH:
						MouseX = (float)(int)MouseX + 0.5f;
						MouseY = (float)(int)MouseY;
						break;
					case EAST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY + 0.5f;
						break;
					case WEST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY + 0.5;
						break;
					case NORTHEAST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					case SOUTHEAST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					case NORTHWEST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					case SOUTHWEST:
						MouseX = (float)(int)MouseX;
						MouseY = (float)(int)MouseY;
						break;
					default:
						break;
				}
			}
		}		
	}
	else
	{
		if(Tool == TOOL_SELECT_WALL || Tool == TOOL_SELECT_FLOOR)
		{
			return TRUE;
		}
		MouseX = (float)(int)MouseX + 0.5f;
		MouseY = (float)(int)MouseY + 0.5f;
		MouseZ = Valley->GetTileHeight(MouseX, MouseY);
	
		if(Tool == TOOL_MOVE_POINT && PointA)
		{
			PointA->Location.x = MouseX;
			PointA->Location.y = MouseY;
			PointA->Location.z = MouseZ;
		}
	}
	return TRUE;
}

int RegionEditWin::Command(int IDFrom, int Command, int Param)
{
	ZSWindow *pWin;
	ZSFloatSpin *pFSpin;
	ZSButton *pButton;		
	switch(Command)
	{
		case COMMAND_EDIT_CHANGED:
			switch(IDFrom)
			{

				case IDC_WALLHEIGHT:
					pWin = GetChild(IDC_WALL_TOOLS);
					pFSpin = (ZSFloatSpin *)pWin->GetChild(IDFrom);
					ERegion.SetWallHeight(pFSpin->GetValue());
					break;
				case IDC_OVERHANG:
					pWin = GetChild(IDC_ROOF_TOOLS);
					pFSpin = (ZSFloatSpin *)pWin->GetChild(IDFrom);
					ERegion.SetOverHang(pFSpin->GetValue());
					break;
				case IDC_ROOFHEIGHT:
					pWin = GetChild(IDC_ROOF_TOOLS);
					pFSpin = (ZSFloatSpin *)pWin->GetChild(IDFrom);
					ERegion.SetRoofHeight(pFSpin->GetValue());
					break;
				break;
			}
		case COMMAND_BUTTON_CLICKED:
			switch(IDFrom)
			{
				case IDC_MOVE:
					Tool = TOOL_MOVE_POINT;
					break;
				case IDC_ADD_DOOR:
					Tool = TOOL_ADD_DOOR;
					break;
				case IDC_ADD_WINDOW:
					Tool = TOOL_ADD_WINDOW;
					break;
				case IDC_SELECT_WALL:
					Tool = TOOL_SELECT_WALL;
					break;
				case IDC_PAINT_WALL:
					PaintWall();
					break;
				case IDC_WALL_TILE:
					if(pWallSide)
					{
						pWin = GetChild(IDC_WALL_TOOLS);
						pFSpin = (ZSFloatSpin *)pWin->GetChild(IDC_WALL_TILE_AMOUNT);
						float TileAmount;
						TileAmount = pFSpin->GetValue();
						pWallSide->Tile(TileAmount);
					}
					break;
				case IDC_WALL_U_PLUS:
					if(pWallSide)
						pWallSide->AdjustUV(UV_ADJUSTMENT,0);
					break;
				case IDC_WALL_U_MINUS:
					if(pWallSide)
						pWallSide->AdjustUV(-UV_ADJUSTMENT,0);
					break;
				case IDC_WALL_V_PLUS:
					if(pWallSide)
						pWallSide->AdjustUV(0,UV_ADJUSTMENT);
					break;
				case IDC_WALL_V_MINUS:
					if(pWallSide)
						pWallSide->AdjustUV(0,-UV_ADJUSTMENT);
					break;

				case IDC_ROOF_TYPE:
					int NewRoof;
					NewRoof = ERegion.GetRoofType() + 1;
					if(NewRoof > ROOF_DOME)
					{
						NewRoof = ROOF_FLAT;
					}
					ERegion.SetRoofType((FLOORROOF_T)NewRoof);
					pWin = GetChild(IDC_ROOF_TOOLS);
					pButton = (ZSButton *)pWin->GetChild(IDC_ROOF_TYPE);
					switch(NewRoof)
					{
						case ROOF_FLAT:
							pButton->SetText("Flat");
							break;
						case ROOF_POINT:
							pButton->SetText("Point");
							break;
						case ROOF_GABLE:
							pButton->SetText("Gable");
							break;
						case ROOF_SLOPE:
							pButton->SetText("Slope");
							break;
						case ROOF_DOME:
							pButton->SetText("Dome");
							break;
						default:
							break;
					}
					break;
				case IDC_SELECT_FLOOR:
					Tool = TOOL_SELECT_FLOOR;
					break;
				case IDC_PAINT_FLOOR:
					PaintFloor();
					break;
				case IDC_PAINT_ROOF:
					PaintRoof();
					break;
				
				case IDC_CREATE_REGION:
					ERegion.ClearRooms();
					int OldID;
					ERegion.CreateInterior();
					if(pRegion)
					{
						while(pRegion->GetParent())
						{
							pRegion = pRegion->GetParent();
						}
						OldID = pRegion->GetID();
						delete pRegion;
						pRegion = NULL;
						Creating = TRUE;
						Previewing = FALSE;
						pWall = NULL;
					}
					else
					{
						OldID = 0;
					}

					int FloorTexture;
					ZSWindow *pWin;
					pWin = GetChild(IDC_TEXTURE);
					FloorTexture = ((ZSList *)pWin)->GetSelection();

					pRegion = ERegion.CreateGameRegion(NULL,NULL,Engine->GetTexture(FloorTexture),NULL);
					pRegion->SetID(OldID);
					//FILE *FPRO;
					//FPRO = fopen("createregion.txt","wt");
					//pRegion->OutputDebugInfo(FPRO);
					//fclose(FPRO);
					PreludeWorld->AddRegion(pRegion);
					break;
				case IDC_SHOWALL:
					if(pRegion)
					{
						while(pRegion->GetParent())
						{
							pRegion = pRegion->GetParent();
						}
						pRegion->SetOccupancy(REGION_OCCUPIED);

						Region *pRegionSub;
						pRegionSub = pRegion->GetSubRegion();
						while(pRegionSub)
						{
							pRegionSub->SetOccupancy(REGION_OCCUPIED);
							pRegionSub = pRegionSub->GetNext();
						}
						
						pRegion->CreateDrawList();
					}
					break;
				case IDC_UP_REGION:
					if(pRegion && pRegion->GetParent())
					{
						pRegion->SetOccupancy(REGION_UNSEEN);
						pRegion = pRegion->GetParent();
						pRegion->SetOccupancy(REGION_OCCUPIED);
						pRegion->CreateDrawList();
					}
					break;
				case IDC_DOWN_REGION:
					if(pRegion)
					{
						if(pRegion->GetSubRegion())
						{
							pRegion->SetOccupancy(REGION_UNSEEN);
							pRegion = pRegion->GetSubRegion();
							pRegion->SetOccupancy(REGION_OCCUPIED);
							pRegion->GetParent()->CreateDrawList();
						}
						else
						{
							if(pRegion->GetNext())
							{
								pRegion->SetOccupancy(REGION_UNSEEN);
								pRegion = pRegion->GetNext();
								pRegion->SetOccupancy(REGION_OCCUPIED);
								pRegion->GetParent()->CreateDrawList();
							}
						}
					}
					break;
				case IDC_PREVIEW:
					if(Previewing)
						Previewing = FALSE;
					else
						Previewing = TRUE;
					break;
				case IDC_DELETE:
					if(pRegion)
					{
						if(Confirm(this,"Delete Region","yes","no"))
						{
							while(pRegion->GetParent())
							{
								pRegion = pRegion->GetParent();
							}
							PreludeWorld->RemoveRegion(pRegion->GetID());
							pRegion = NULL;
							Creating = TRUE;
							Previewing = FALSE;
							pWall = NULL;
						}
					}
					ERegion.ClearAll();
					break;
				case IDC_DIVIDE_WALL:
					Tool = TOOL_DIVIDE_WALL;
					break;
				case IDC_ADD_WALL:
					Tool = TOOL_ADD_WALL;
					break;
				case IDC_NEW:
					pRegion = NULL;
					Creating = TRUE;
					Previewing = FALSE;
					pWall = NULL;
					ERegion.ClearAll();
					break;
				case IDC_DIVIDE_REGION:
					if(Dividing)
					{
						Dividing = FALSE;
					}
					else
					{ 
						Dividing = TRUE;
						APoint = FALSE;
						BPoint = FALSE;
					}
					break;
				case IDC_PAINT_EXTERIOR:
					if(pRegion)
					{
						pWin = GetChild(IDC_WALL_TOOLS);
						pFSpin = (ZSFloatSpin *)pWin->GetChild(IDC_WALL_TILE_AMOUNT);
						float TileAmount;
						TileAmount = pFSpin->GetValue();
						
						ZSList *pLWin;
						pLWin = (ZSList *)GetChild(IDC_TEXTURE);
						int Texture;
						Texture = pLWin->GetSelection();
						pRegion->PaintExterior(Engine->GetTexture(Texture), TileAmount);
					}
					break;
				case IDC_PAINT_INTERIOR:
					if(pRegion)
					{
						pWin = GetChild(IDC_WALL_TOOLS);
						pFSpin = (ZSFloatSpin *)pWin->GetChild(IDC_WALL_TILE_AMOUNT);
						float TileAmount;
						TileAmount = pFSpin->GetValue();
						
						ZSList *pLWin;
						pLWin = (ZSList *)GetChild(IDC_TEXTURE);
						int Texture;
						Texture = pLWin->GetSelection();
						pRegion->PaintInterior(Engine->GetTexture(Texture), TileAmount);
					
					}
					break;
				case IDC_PAINT_ALL_FLOORS:
					if(pRegion)
					{
						ZSList *pWin;
						pWin = (ZSList *)GetChild(IDC_TEXTURE);
						int Texture;
						Texture = pWin->GetSelection();
						pRegion->PaintFloors(Engine->GetTexture(Texture));
					}
					break;
				case IDC_SAVE_REGIONS:
					if(Confirm(this,"Save Regions?","yes","no"))
						PreludeWorld->SaveRegions("blarg.tmp");

					break;
				case IDC_LOAD_REGIONS:
					if(Confirm(this,"Load Regions?","yes","no"))
					{}	//Valley->LoadRegions("world.rgn");
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return TRUE;
}

int RegionEditWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80)
	{
		D3DVECTOR vMove(0.0f,0.0f,0.0f);
		if(CurrentKeys[DIK_Q] & 0x80 && !(LastKeys[DIK_Q] & 0x80))
		{
			if(pRegion)
			{
				
				vMove.z = 0.025f;
				if(pRegion->GetParent())
					pRegion->GetParent()->Move(&vMove);
				else
					pRegion->Move(&vMove);
					
			}
		}

		if(CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
		{
			if(pRegion)
			{
				vMove.z = -0.025f;
				if(pRegion->GetParent())
					pRegion->GetParent()->Move(&vMove);
				else
					pRegion->Move(&vMove);
			}
		}

		if(CurrentKeys[DIK_X] & 0x80 && !(LastKeys[DIK_X] & 0x80))
		{
			State = WINDOW_STATE_DONE;
			Visible = FALSE;
			ReleaseFocus();
		}

		return TRUE;
	}

	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
		Visible = FALSE;
		ReleaseFocus();
	}




		if(CurrentKeys[DIK_R] & 0x80)
	{
		PreludeWorld->RotateCamera(ROTATION_ANGLE);
	}

	if(CurrentKeys[DIK_L] & 0x80)
	{
		PreludeWorld->RotateCamera(-ROTATION_ANGLE);
	}

	if(CurrentKeys[DIK_Y] & 0x80)
	{
		Engine->Graphics()->Zoom(-ZOOM_FACTOR);
	}

	if(CurrentKeys[DIK_U] & 0x80)
	{
		Engine->Graphics()->Zoom(ZOOM_FACTOR);
	}

	if((CurrentKeys[DIK_LSHIFT] & 0x80) || (CurrentKeys[DIK_RSHIFT] & 0x80))
	{

	}
	else
	{
		
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

	if(CurrentKeys[DIK_W] & 0x80 && !(LastKeys[DIK_W] &0x80))
	{
		Tool = TOOL_PAINT_WALL;
	}
	
	if(CurrentKeys[DIK_D] & 0x80 && !(LastKeys[DIK_D] & 0x80))
	{
		if(pRegion)
		{
			ERegion.ClearAll();
			pRegion->Devolve(&ERegion);
		}
	}

	if(CurrentKeys[DIK_P] & 0x80 && !(LastKeys[DIK_P] & 0x80))
	{
		Tool = TOOL_LINK_DOOR;
		Describe("Linking doors");
	}
	
	



	return TRUE;
}

void RegionEditWin::Show()
{
	Visible = TRUE;
	State = WINDOW_STATE_NORMAL;
	SetCursor(CURSOR_POINT);
	SetFocus(this);
}

RegionEditWin::RegionEditWin()
{
	Tool = TOOL_ADD_WALL;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	
	ZSWindow::GetMain()->AddChild(this);

	Bounds.left = 0;
	Bounds.top = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.bottom = Engine->Graphics()->GetHeight();

	Recording = FALSE;

	pRegion = NULL;
	pWallSide = NULL;
	pFloorRegion = NULL;
	NumBounds = 0;
	ZeroMemory(VertList, sizeof(D3DVECTOR)*16);

	//create all our sub windows
	//texturing windows first

	//smalllistboxs
	int n;
	ZSList *pList;
	int NumTextures;
	NumTextures = Engine->GetNumTextures();
	ZSTexture *pTexture;
	int ItemNum;
		
	pList = new ZSList(IDC_TEXTURE,Engine->Graphics()->GetWidth() - 192, 0, 192, 256);
	pList->Show();
	AddChild(pList);
	n = 0;
	for(n = 0; n < NumTextures; n++)
	{
		pTexture = Engine->GetTexture(n);
		pList->AddItem(pTexture->GetName());
	}
	ItemNum = pList->FindItem("exwall.bmp");
	if(ItemNum >= 0)
		pList->SetSelection(ItemNum);

	ZSButton *pButton;
	int Width;
	
	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Parent");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_UP_REGION, Engine->Graphics()->GetWidth() - Width, 128+128,Width, 16);
	pButton->SetText("Parent");
	pButton->Show();
	AddChild(pButton);
	
	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("SubRegion");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_DOWN_REGION, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 16, Width, 16);
	pButton->SetText("SubRegion");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Preview");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_PREVIEW, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 32, Width, 16);
	pButton->SetText("Preview");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Delete");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_DELETE, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 48, Width, 16);
	pButton->SetText("Delete");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Divide Wall");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_DIVIDE_WALL, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 64, Width, 16);
	pButton->SetText("Divide Wall");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Add Walls");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_ADD_WALL, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 80, Width, 16);
	pButton->SetText("Add Walls");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Move Points");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_MOVE, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 96, Width, 16);
	pButton->SetText("Move Points");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("New Region");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_NEW, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 112, Width, 16);
	pButton->SetText("New Region");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Create Region");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_CREATE_REGION, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 128, Width, 16);
	pButton->SetText("Create Region");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Show All");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_SHOWALL, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 144, Width, 16);
	pButton->SetText("Show All");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Door");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_ADD_DOOR, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 160, Width, 16);
	pButton->SetText("Door");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Window");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_ADD_WINDOW, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 176, Width, 16);
	pButton->SetText("Window");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Save");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_SAVE_REGIONS, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 192, Width, 16);
	pButton->SetText("Save");
	pButton->Show();
	AddChild(pButton);

	Width = Engine->Graphics()->GetFontEngine()->GetTextWidth("Load");
	pButton = new ZSButton(BUTTON_NORMAL, IDC_LOAD_REGIONS, Engine->Graphics()->GetWidth() - Width, 128 + 128 + 208, Width, 16);
	pButton->SetText("Load");
	pButton->Show();
	AddChild(pButton);


	ZSWindow *pWin;

	pWin = (ZSWindow *)new FloorToolWin(IDC_FLOOR_TOOLS, 0, 0, 100, 134);
	pWin->Show();
	AddChild(pWin);

	pWin = (ZSWindow *)new WallToolWin(IDC_WALL_TOOLS, 100, 0, 100, 184);
	pWin->Show();
	AddChild(pWin);

	pWin = (ZSWindow *)new RoofToolWin(IDC_ROOF_TOOLS, 200, 0, 100, 164);
	pWin->Show();
	AddChild(pWin);

	pWin = (ZSWindow *)new RegionToolWin(IDC_MAIN_TOOLS, 300, 0, 100, 100);
	pWin->Show();
	AddChild(pWin);


}

RegionEditWin::~RegionEditWin()
{
	if(pRegion)
	{
		delete pRegion;
	}
}

int RegionEditWin::GoModal()
{
	State = WINDOW_STATE_NORMAL;
	//loop while we're not in a done state
	while(State != WINDOW_STATE_DONE)
	{
		//ensure that all windows are drawn by call the main window's draw routine
		this->Draw();
		Engine->Graphics()->Flip();
		
		//get any user input
		Engine->Input()->Update(pInputFocus);
	}

	return ReturnCode;
}

void RegionEditWin::ScaleFloor(float Amount)
{
	if(pFloorRegion)
	{
		pFloorRegion->ScaleFloor(Amount);
	}
}

void RegionEditWin::RotateFloor(float Angle)
{
	if(pFloorRegion)
	{
		pFloorRegion->RotateFloor(Angle);
	}
}


void RegionEditWin::GetFloor()
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
	
	Region *prParent;

	if(pRegion->GetParent())
	{
		prParent = pRegion->GetParent();
	}
	else
	{
		prParent = pRegion;
	}

	pFloorRegion = prParent->GetFloor(&vRayStart,&vRayEnd);

	return;


}

void RegionEditWin::PaintFloor()
{
	if(pFloorRegion)
	{
		
		ZSList *pWin;
		pWin = (ZSList *)GetChild(IDC_TEXTURE);
		int Texture;
		Texture = pWin->GetSelection();
		pFloorRegion->SetFRTexture(Engine->GetTexture(Texture));
	}

}

void RegionEditWin::PaintRoof()
{
	if(pRegion)
	{
		ZSList *pWin;
		pWin = (ZSList *)GetChild(IDC_TEXTURE);
		int Texture;
		Texture = pWin->GetSelection();
		if(pRegion->GetParent())
		{
			pRegion->GetParent()->SetFRTexture(Engine->GetTexture(Texture));
		}
		else
		{
			pRegion->SetFRTexture(Engine->GetTexture(Texture));		
		}
	}
}

void RegionEditWin::ScaleRoof(float Amount)
{
	if(pRegion)
	{
		ZSList *pWin;
		pWin = (ZSList *)GetChild(IDC_TEXTURE);
		int Texture;
		Texture = pWin->GetSelection();
		if(pRegion->GetParent())
		{
			pRegion->GetParent()->ScaleRoof(Amount);
		}
		else
		{
			pRegion->ScaleRoof(Amount);
		}
	}
}

void RegionEditWin::PaintWall()
{
	if(!pWallSide)
	{
		return;
	}

	ZSList *pWin;
	pWin = (ZSList *)GetChild(IDC_TEXTURE);
	int Texture;
	Texture = pWin->GetSelection();
	pWallSide->pTexture = Engine->GetTexture(Texture);
	
	Region *pParent;

	if(pRegion->GetParent())
	{
		pParent = pRegion->GetParent();
	}
	else
	{
		pParent = pRegion;
	}

	pParent->SortWalls();
}


void RegionEditWin::GetWall()
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
	vMouseNear.y = (1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->top/(float)Engine->Graphics()->GetHeight());
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
	
	Region *pParent;

	if(pRegion->GetParent())
	{
		pParent = pRegion->GetParent();
	}
	else
	{
		pParent = pRegion;
	}

	pWallSide = pParent->GetWallSide(&vRayStart,&vRayEnd);

	return;
}

void RegionEditWin::DivideWall()
{

}

void RegionEditWin::WallTextureChange(float uOff, float vOff)
{

}

int FloorToolWin::Command(int IDFrom, int Command, int Param)
{
	float ScaleValue;
	float RotateValue;
	if(Command == COMMAND_BUTTON_CLICKED && (IDFrom == IDC_SCALE_FLOOR || IDFrom == IDC_ROTATE_FLOOR))
	{
		ZSFloatSpin *pFSpin;
		pFSpin = (ZSFloatSpin *)GetChild(IDC_FLOOR_SCALE_AMOUNT);
		ScaleValue = pFSpin->GetValue();
		RegionEditWin *pEWin;
		pEWin = (RegionEditWin *)pParent;
		pEWin->ScaleFloor(ScaleValue);

		pFSpin = (ZSFloatSpin *)GetChild(IDC_FLOOR_ROTATE_AMOUNT);
		RotateValue = pFSpin->GetValue();
		pEWin->RotateFloor(RotateValue);
	}
	else
	{
		return pParent->Command(IDFrom, Command, Param);
	}
	return TRUE;
}

FloorToolWin::FloorToolWin(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	Type = WINDOW_NONE;
	Moveable = TRUE;
	Visible = FALSE;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = Bounds.top + height;

	CreateWoodBorderedBackground(4,1);

	SetText("Floor");

	//create buttons here
	//create buttons here
	//select
	ZSButton *pButton;

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SELECT_FLOOR, x, y+16, width, 16);
	pButton->SetText("Select");
	pButton->Show();
	AddChild(pButton);

	//paint
	pButton = new ZSButton(BUTTON_NORMAL, IDC_PAINT_FLOOR, x, y+34, width, 16);
	pButton->SetText("Paint");
	pButton->Show();
	AddChild(pButton);
	
	//textureup
	ZSFloatSpin *pFSpin;
//	ZSText *pText;

//	pText = new ZSText(-1,x,y+52,"UV Scale");
//	AddChild(pText);
//	pText->Show();

	pFSpin = new ZSFloatSpin(IDC_FLOOR_SCALE_AMOUNT, x, y+52, width, 18);
	pFSpin->Show();
	pFSpin->SetValue(2.0f);
	AddChild(pFSpin);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SCALE_FLOOR, x, y+72, width, 18);
	pButton->SetText("Scale");
	pButton->Show();
	AddChild(pButton);

	pFSpin = new ZSFloatSpin(IDC_FLOOR_ROTATE_AMOUNT, x, y+90, width, 18);
	pFSpin->Show();
	pFSpin->SetValue(0.0f);
	AddChild(pFSpin);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_ROTATE_FLOOR, x, y+110, width, 18);
	pButton->SetText("Rotate");
	pButton->Show();
	AddChild(pButton);
	
	pButton = new ZSButton(BUTTON_NORMAL, IDC_PAINT_ALL_FLOORS, x, y+128, width, 18);
	pButton->SetText("Paint All");
	pButton->Show();
	AddChild(pButton);

	Shrink();

}

int WallToolWin::Command(int IDFrom, int Command, int Param)
{
	return pParent->Command(IDFrom,Command,Param);
	return TRUE;
}

WallToolWin::WallToolWin(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	Type = WINDOW_NONE;
	Moveable = TRUE;
	Visible = FALSE;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = Bounds.top + height;

	CreateWoodBorderedBackground(4,1);

	SetText("Walls");

	//create buttons here
	//select
	ZSButton *pButton;

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SELECT_WALL, x, y+16, width, 16);
	pButton->SetText("Select");
	pButton->Show();
	AddChild(pButton);

	//paint
	pButton = new ZSButton(BUTTON_NORMAL, IDC_PAINT_WALL, x, y+36, width, 16);
	pButton->SetText("Paint");
	pButton->Show();
	AddChild(pButton);
	
	//textureright
	pButton = new ZSButton(BUTTON_NORMAL, IDC_WALL_U_PLUS, x, y+56, width/2, 16);
	pButton->SetText("+ U");
	pButton->Show();
	AddChild(pButton);
	
	//textureleft
	pButton = new ZSButton(BUTTON_NORMAL, IDC_WALL_U_MINUS, x+width/2, y+56, width/2, 16);
	pButton->SetText("- U");
	pButton->Show();
	AddChild(pButton);

	//paintInterrior
	pButton = new ZSButton(BUTTON_NORMAL, IDC_PAINT_INTERIOR, x, y+72, width, 16);
	pButton->SetText("Interior");
	pButton->Show();
	AddChild(pButton);

	//paintExterior
	pButton = new ZSButton(BUTTON_NORMAL, IDC_PAINT_EXTERIOR, x, y+88, width, 16);
	pButton->SetText("Exterior");
	pButton->Show();
	AddChild(pButton);

	ZSFloatSpin *pFSpin;
	ZSText *pText;

	pText = new ZSText(-1,	x,	y+104,	"WallHeight");
	AddChild(pText);
	pText->Show();

	pFSpin = new ZSFloatSpin(IDC_WALLHEIGHT, x, y+120, width, 16);
	pFSpin->Show();
	pFSpin->SetValue(4.0f);
	AddChild(pFSpin);

	//tiling
	pText = new ZSText(-1,	x,	y+126,	"Tiling");
	AddChild(pText);
	pText->Show();

	pFSpin = new ZSFloatSpin(IDC_WALL_TILE_AMOUNT, x, y+142, width, 16);
	pFSpin->Show();
	pFSpin->SetValue(1.0f);
	AddChild(pFSpin);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_WALL_TILE, x, y+158, width, 16);
	pButton->SetText("Tile");
	pButton->Show();
	AddChild(pButton);
	Shrink();

	//textureup
	pButton = new ZSButton(BUTTON_NORMAL, IDC_WALL_V_PLUS, x, y+174, width/2, 16);
	pButton->SetText("+ V");
	pButton->Show();
	AddChild(pButton);
	
	//texturedown
	pButton = new ZSButton(BUTTON_NORMAL, IDC_WALL_V_MINUS, x+width/2, y+174, width/2, 16);
	pButton->SetText("- V");
	pButton->Show();
	AddChild(pButton);
}

int RegionToolWin::Command(int IDFrom, int Command, int Param)
{

	return TRUE;
}

RegionToolWin::RegionToolWin(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	Type = WINDOW_NONE;
	Moveable = TRUE;
	Visible = FALSE;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = Bounds.top + height;

	CreateWoodBorderedBackground(4,1);

	SetText("Main");

	//create buttons here

	Shrink();

}

int RoofToolWin::Command(int IDFrom, int Command, int Param)
{
	if(Command == COMMAND_BUTTON_CLICKED && IDFrom == IDC_ROOF_SCALE)
	{
		ZSFloatSpin *pFSpin;
		float ScaleValue;
		pFSpin = (ZSFloatSpin *)GetChild(IDC_ROOF_SCALE_SPIN);
		ScaleValue = pFSpin->GetValue();
		RegionEditWin *pEWin;
		pEWin = (RegionEditWin *)pParent;
		pEWin->ScaleRoof(ScaleValue);
	}
	else
	{
		pParent->Command(IDFrom,Command,Param);
	}
	return TRUE;
}

RoofToolWin::RoofToolWin(int NewID, int x, int y, int width, int height)
{
	ID = NewID;
	Type = WINDOW_NONE;
	Moveable = TRUE;
	Visible = FALSE;
	Bounds.left = x;
	Bounds.top = y;
	Bounds.right = x + width;
	Bounds.bottom = Bounds.top + height;

	CreateWoodBorderedBackground(4,1);

	SetText("Roof");

	//create buttons here
	ZSFloatSpin *pFSpin;
	ZSText *pText;

	pText = new ZSText(-1,x,y+16,"Overhang");
	AddChild(pText);
	pText->Show();

	pFSpin = new ZSFloatSpin(IDC_OVERHANG, x, y+32, width, 16);
	pFSpin->Show();
	pFSpin->SetValue(0.0f);
	AddChild(pFSpin);

	pText = new ZSText(-1,x,y+48,"Height");
	AddChild(pText);
	pText->Show();

	pFSpin = new ZSFloatSpin(IDC_ROOFHEIGHT, x, y+64, width, 16);
	pFSpin->Show();
	pFSpin->SetValue(2.0f);
	AddChild(pFSpin);


	ZSButton *pButton;

	pButton = new ZSButton(BUTTON_NORMAL, IDC_ROOF_SCALE,x,y+80, 64, 16);
	pButton->SetText("Scale");
	AddChild(pButton);
	pButton->Show();

	pFSpin = new ZSFloatSpin(IDC_ROOF_SCALE_SPIN, x, y+96, width, 16);
	pFSpin->Show();
	pFSpin->SetValue(1.0f);
	AddChild(pFSpin);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_ROOF_TYPE, x, y+116, width, 16);
	pButton->SetText("Dome");
	pButton->Show();
	AddChild(pButton);

	pButton = new ZSButton(BUTTON_NORMAL, IDC_PAINT_ROOF, x, y+134, width, 16);
	pButton->SetText("Paint");
	pButton->Show();
	AddChild(pButton);

	Shrink();

}

void RegionEditWin::LinkDoor(int x, int y)
{
	if(!pRegion)
		return; //nothing to link a door to

	D3DVECTOR vRayEnd;
	D3DVECTOR vRayStart;
	
	float MouseX;
	float MouseY;
	float MouseZ;

	PreludeWorld->ConvertMouse(x,y,&MouseX,&MouseY,&MouseZ);
	
	//create the test ray
	vRayEnd = _D3DVECTOR(MouseX, MouseY, MouseZ);

	D3DVECTOR vMouseNear;
	D3DVECTOR vCameraOffset;
	D3DVECTOR vCameraSpace;

	//convert mouse to camera space
	vMouseNear.x = -(1.0f - 2.0f * (float)x/(float)Engine->Graphics()->GetWidth());
	vMouseNear.y = (1.0f - 2.0f * (float)y/(float)(Engine->Graphics()->GetHeight() - 100));
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

	Object *pObject;

	pObject = Valley->GetTarget(&vRayStart, &vRayEnd);

	if(!pObject || pObject->GetObjectType() != OBJECT_PORTAL) 
		return;

	Portal *pPortal;
	
	pPortal = (Portal *)pObject;

	if(pPortal->GetRegionOne() == pRegion || pPortal->GetRegionTwo() == pRegion)
	{
		return;
	}

	if(pPortal->GetRegionOne())
	{
		pPortal->SetRegionTwo(pRegion);
	}
	else
	{
		pPortal->SetRegionOne(pRegion);
	}
	
	



}



























