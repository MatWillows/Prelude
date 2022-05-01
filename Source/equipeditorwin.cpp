#include "equipeditorwin.h"
#include "zsengine.h"
#include "ZSbutton.h"
#include "translucentwindow.h"
#include "mainwindow.h"
#include "ZStext.h"
#include "world.h"
#include "zsutilities.h"
#include "ZSOldListbox.h"
#include "ZSconfirm.h"

#define ACOLOR			D3DRGBA(1.0f,0.0f,0.0f, 1.0f)
#define BCOLOR			D3DRGBA(0.0f,0.0f,1.0f, 1.0f)
#define LINKCOLOR		D3DRGBA(0.0f,1.0f,0.0f, 1.0f)
#define BOUNDCOLOR	D3DRGBA(0.75f,0.25f,0.75f,1.0f)

int E_LEFT	=	0;
int E_RIGHT	= 640;
int E_TOP =		0;
int E_BOTTOM =  480;

typedef enum
{
	IDC_BLOCKING,
	IDC_TEXTURE,
	IDC_TEXTURE_LIST,
	IDC_ROTATE_LEFT = 30, 
	IDC_ROTATE_RIGHT,
	IDC_ROTATE_TOP,			
	IDC_ROTATE_BOTTOM,	
	IDC_DELETE,
	IDC_SHARPEN,

	IDC_NEXT_A,
	IDC_PREV_A,			

	IDC_NEXT_B,
	IDC_PREV_B,			

	IDC_MESH_LIST,

	IDC_NEXT_SLOT,
	IDC_PREV_SLOT,		

	IDC_SET_SLOT,			

	IDC_SET_LINK,				

	IDC_RESET_ROTATION,		

	IDC_EXIT_EDIT,			

	IDC_CUR_SLOT,				

	IDC_CUR_POINTA,			

	IDC_CUR_POINTB,			

	IDC_CUR_POINTLINK,	

	IDC_MESH_NAME,			

	IDC_NEXT_MESH,			
	IDC_PREV_MESH,			

	IDC_CUR_FRAME,			

	IDC_NEXT_FRAME,			
	IDC_PREV_FRAME,			

	IDC_NEXT_LINK,			
	IDC_PREV_LINK,

	IDC_SMOOTH,

	IDC_SAVE,

	IDC_EQUIPINFO1,
	IDC_EQUIPINFO2,
	IDC_EQUIPINFO3,
	IDC_EQUIPINFO4,
	IDC_EQUIPINFO5,

	IDC_SCALE_INFO,
	IDC_SCALE_PLUS,
	IDC_SCALE_MINUS,
	IDC_SCALE,

	IDC_RELOAD,
	IDC_IMPORT,
	IDC_ADD_NEW,

	IDC_ROTATE_X,
	IDC_ROTATE_Y,
	IDC_ROTATE_Z,

	IDC_FLIPNORMALS,

	IDC_NORMALIZE,

	IDC_MOVE_NORTH,
	IDC_MOVE_EAST,
	IDC_MOVE_WEST,
	IDC_MOVE_SOUTH,
	IDC_MOVE_UP,
	IDC_MOVE_DOWN,
	IDC_SCALE_X_UP,
	IDC_SCALE_Y_UP,
	IDC_SCALE_Z_UP,
	IDC_SCALE_X_DOWN,
	IDC_SCALE_Y_DOWN,
	IDC_SCALE_Z_DOWN,

	IDC_EQUIPSLOT1 = 500,
	IDC_EQUIPSLOTPLUS1 = 600,
	IDC_EQUIPSLOTMINUS1 = 700,

} IDC_LIST_EEWIN;

void EEWin::UpdateSlotText()
{
	char blarg[32];
	ZSWindow *pWin;
	for(int n = 0; n < 10; n++)
	{
		sprintf(blarg,"Link: %i  From:  %i",Equipslots[n].LinkIndex,Equipslots[n].RayFromIndex);
		pWin = GetChild(IDC_EQUIPSLOT1 + n);
		if(pWin)
		{
			pWin->SetText(blarg);
		}
	}
}

void EEWin::SyncToMesh()
{
	if(CurFrame >= Engine->GetMesh(CurMesh)->GetNumFrames())
	{
		CurFrame = Engine->GetMesh(CurMesh)->GetNumFrames() - 1;
	}

	for(int n = 0; n < 10; n ++)
	{

		if(Equipslots[n].LinkIndex >= Engine->GetMesh(CurMesh)->numvertex)
		{
			Equipslots[n].LinkIndex = Engine->GetMesh(CurMesh)->numvertex - 1;
		}

		if(Equipslots[n].RayFromIndex >= Engine->GetMesh(CurMesh)->numvertex)
		{
			Equipslots[n].RayFromIndex = Engine->GetMesh(CurMesh)->numvertex - 1;
		}
	}

	if(PointLink >= Engine->GetMesh(CurMesh)->numvertex)
	{
		PointLink = Engine->GetMesh(CurMesh)->numvertex - 1;
	}

	float left, right, top, bottom, front, back;

	left = Engine->GetMesh(CurMesh)->GetLeftBound();
	right = Engine->GetMesh(CurMesh)->GetRightBound();
	top = Engine->GetMesh(CurMesh)->GetTopBound();
	bottom = Engine->GetMesh(CurMesh)->GetBottomBound();
	front = Engine->GetMesh(CurMesh)->GetFrontBound();
	back = Engine->GetMesh(CurMesh)->GetBackBound();
	
	float Width;
	
	Width = Engine->GetMesh(CurMesh)->GetWidth();

	if(Width < Engine->GetMesh(CurMesh)->GetDepth())
	{
		Width = Engine->GetMesh(CurMesh)->GetDepth();
	}

	float Height;
	Height = Engine->GetMesh(CurMesh)->GetHeight();
	
	Height *= 1.33f;

	Width *= 1.33f;

	if(Height > (Width / 1.3333333f))
	{
		Width = Height * 1.3333333f;
	}
	else
	{
		Height = Width / 1.3333333f;
	}
	
	D3DXMatrixOrtho(&matProjection,
		Width,
		Height,
		50.0f,
		-50.0f);


   D3DVECTOR view, cam;

	view.x = (right + left)/2.0f;
	view.y = (back + front)/2.0f;
	view.z = (top + bottom)/2.0f;

	cam.x = view.x + 5;
	cam.y = view.y + 5;
	cam.z = view.z + 5;
	
	D3DVECTOR vEye((left + right), (top + bottom), (front + back));
	D3DVECTOR vLookAt((left + right)/2.0f,(top + bottom)/2.0f, (front + back) / 2.0f);
	D3DVECTOR vUp(0.0f,0.0f,1.0f);

	D3DXMatrixLookAt( &matCamera,
		(D3DXVECTOR3*) &cam,
		(D3DXVECTOR3*) &view,
		(D3DXVECTOR3*) &vUp);

	ZSModelEx *pMesh;
	ZSWindow *pWin; 
	pMesh = Engine->GetMesh(CurMesh);
	char blarg[16];
	if(pMesh->equipmentlist)
	{
		for(n = 0; n < 5; n ++)
		{
			pWin = GetChild(IDC_EQUIPINFO1 + n);
			sprintf(blarg,"%i: %i %i",n,pMesh->equipmentlist[n].LinkIndex,pMesh->equipmentlist[n].RayFromIndex);
			pWin->SetText(blarg);
		}
	}
	else
	{
		for(n = 0; n < 5; n ++)
		{
			pWin = GetChild(IDC_EQUIPINFO1 + n);
			pWin->SetText("null");
		}
	

	}

	pWin = GetChild(IDC_TEXTURE);
	
	if(pMesh->pTexture)
	{
		pWin->SetText(pMesh->pTexture->GetName());
	}
	else
	{
		pMesh->SetTexture(Engine->GetTexture("alltrees"));
		pWin->SetText(pMesh->pTexture->GetName());
	}

	pWin = GetChild(IDC_BLOCKING);
	BYTE Blocking;
	Blocking = pMesh->GetBlocking();
	if(Blocking & ~BLOCK_ANY)
	{
		pMesh->SetBlocking(BLOCK_NONE);
		pWin->SetText("BlockNone");
	}
	else
	{
		if(Blocking & BLOCK_ALL)
		{
			pWin->SetText("BlockAll");
		}
		else
		if(Blocking & BLOCK_PARTIAL)
		{
			pWin->SetText("BlockPart");
		}
		else
		{
			pWin->SetText("BlockNone");
		}
	}


}


void EEWin::DrawPoints()
{

	D3DVERTEX Vert;

	D3DLVERTEX Verts[22];

	for(int n = 0; n < 20; n += 2)
	{
		Vert = Engine->GetMesh(CurMesh)->GetPoint(Equipslots[n/2].LinkIndex,CurFrame);
		Verts[n].x = Vert.x;
		Verts[n].y = Vert.y;
		Verts[n].z = Vert.z;
		Verts[n].color = ACOLOR;
		Verts[n].specular = ACOLOR;
		
		Vert = Engine->GetMesh(CurMesh)->GetPoint(Equipslots[n/2].RayFromIndex,CurFrame);

		Verts[n+1].x = Vert.x;	
		Verts[n+1].y = Vert.y;
		Verts[n+1].z = Vert.z;
		Verts[n+1].color = BCOLOR;
		Verts[n+1].specular = BCOLOR;
	}


	Vert = Engine->GetMesh(CurMesh)->GetPoint(PointLink,CurFrame);

	Verts[20].x = Vert.x;
	Verts[20].y = Vert.y;
	Verts[20].z = Vert.z;
	Verts[20].color = LINKCOLOR;
	Verts[20].specular = LINKCOLOR;
	
	Verts[21].x = (Vert.nx * 0.5f) + Vert.x;	
	Verts[21].y = (Vert.ny * 0.5f) + Vert.y;
	Verts[21].z = (Vert.nz * 0.5f) + Vert.z;
	Verts[21].color = LINKCOLOR;
	Verts[21].specular = LINKCOLOR;

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_RED));
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);    
	
	HRESULT hr;

	hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINELIST, D3DFVF_LVERTEX, Verts, 22, NULL);

	if(hr != D3D_OK)
	{
		Engine->ReportError(hr);
	}

	
	Verts[0].x = -0.5f;
	Verts[0].y = -0.5f;
	Verts[0].z = 0.0f;
	Verts[0].color = BOUNDCOLOR;
	Verts[0].specular = BOUNDCOLOR;
	

	Verts[1].x = 0.5f;
	Verts[1].y = -0.5f;
	Verts[1].z = 0.0f;
	Verts[1].color = BOUNDCOLOR;
	Verts[1].specular = BOUNDCOLOR;

	Verts[2].x = 0.5f;
	Verts[2].y = 0.5f;
	Verts[2].z = 0.0f;
	Verts[2].color = BOUNDCOLOR;
	Verts[2].specular = BOUNDCOLOR;

	Verts[3].x = -0.5f;
	Verts[3].y = 0.5f;
	Verts[3].z = 0.0f;
	Verts[3].color = BOUNDCOLOR;
	Verts[3].specular = BOUNDCOLOR;

	Verts[4].x = -0.5f;
	Verts[4].y = -0.5f;
	Verts[4].z = 0.0f;
	Verts[4].color = BOUNDCOLOR;
	Verts[4].specular = BOUNDCOLOR;

	Verts[5].x = 0.5f;
	Verts[5].y = 0.5f;
	Verts[5].z = 0.0f;
	Verts[5].color = BOUNDCOLOR;
	Verts[5].specular = BOUNDCOLOR;

	Verts[6].x = 0.5f;
	Verts[6].y = -0.5f;
	Verts[6].z = 0.0f;
	Verts[6].color = BOUNDCOLOR;
	Verts[6].specular = BOUNDCOLOR;

	Verts[7].x = -0.5f;
	Verts[7].y = 0.5f;
	Verts[7].z = 0.0f;
	Verts[7].color = BOUNDCOLOR;
	Verts[7].specular = BOUNDCOLOR;

	hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_LVERTEX, Verts, 8, 0);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);    

}

void EEWin::DrawMesh()
{
	D3DXMatrixRotationZ(&matRotation, Angle);

	D3DMATRIX matOldProj;
	D3DMATRIX matOldView;
	D3DMATRIX matOldWorld;

	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_WORLD, &matOldWorld);
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matOldProj);
	Engine->Graphics()->GetD3D()->GetTransform(D3DTRANSFORMSTATE_VIEW, &matOldView);

//	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, &Engine->Identity);

	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)&matProjection);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)&matCamera);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, (D3DMATRIX *)&matRotation);
	

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);

	Engine->Graphics()->GetD3D()->BeginScene();

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	Engine->GetMesh(CurMesh)->Draw(Engine->Graphics()->GetD3D(), CurFrame);

	DrawPoints();
	
	Engine->Graphics()->GetD3D()->EndScene();

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, &matOldWorld);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matOldProj);
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_VIEW, &matOldView);

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	
}

void EEWin::Rotate(DIRECTION_T DirectionType)
{

}

void EEWin::UpdateCurSlotText()
{
	ZSWindow *pWin;

	pWin = GetChild(IDC_CUR_SLOT);

	switch(CurSlot)
	{
		case EQUIP_POS_RHAND: pWin->SetText("RightHand"); break;
		case EQUIP_POS_LHAND: pWin->SetText("LeftHand"); break;
		case EQUIP_POS_HEAD: pWin->SetText("Head"); break;
		case EQUIP_POS_LEFTARM: pWin->SetText("LeftArm"); break;
		case EQUIP_POS_SCABBARD: pWin->SetText("Scabbard"); break;
		case EQUIP_POS_BACK: pWin->SetText("Back"); break;
		case EQUIP_POS_CUSTOM1: pWin->SetText("Custom1"); break;
		case EQUIP_POS_CUSTOM2: pWin->SetText("Custom2"); break;
		case EQUIP_POS_CUSTOM3: pWin->SetText("Custom3"); break;
		case EQUIP_POS_CUSTOM4: pWin->SetText("Custom4"); break;
		default:
			pWin->SetText("Invalid"); break;
	}
}

int EEWin::Command(int IDFrom, int Command, int Param)
{
	D3DXMATRIX matNewRotate;
	D3DXMATRIX matTemp;
	ZSWindow *pWin;
	ZSModelEx *pMesh;
	int n;
	char *Blarg;
			
	switch(Command)
	{
	case COMMAND_BUTTON_CLICKED:
		if(IDFrom >= IDC_EQUIPSLOTMINUS1)
		{
			n = IDFrom - IDC_EQUIPSLOTMINUS1;
			if(n / 10)
			{
				if(Equipslots[n%10].RayFromIndex)
					Equipslots[n%10].RayFromIndex--;
			}
			else
			{
				if(Equipslots[n%10].LinkIndex)
					Equipslots[n%10].LinkIndex--;
			}
			UpdateSlotText();
			return TRUE;
		}
		else
		if(IDFrom >= IDC_EQUIPSLOTPLUS1)
		{
			n = IDFrom - IDC_EQUIPSLOTPLUS1;
			if(n / 10)
			{
				Equipslots[n%10].RayFromIndex++;
			}
			else
			{
				Equipslots[n%10].LinkIndex++;
			}
			UpdateSlotText();
			return TRUE;
		}
		

		switch(IDFrom)
		{
			case IDC_RESET_ROTATION:
				Angle = 0.0f;
				break;

			case  IDC_ROTATE_LEFT:
				Angle += ROTATION_ANGLE;
				break;

			case  IDC_ROTATE_RIGHT:
				Angle -= ROTATION_ANGLE;
				break;
			case  IDC_ROTATE_TOP:

				break;
			case  IDC_ROTATE_BOTTOM:

				break;
			case IDC_TEXTURE:
				pWin = GetChild(IDC_TEXTURE);
				Blarg = pWin->GetText();
				if(!strcmp(Blarg,"alltrees.bmp"))
				{
					pMesh = Engine->GetMesh(CurMesh);
					pMesh->SetTexture(Engine->GetTexture("environ"));
				}
				else
				if(!strcmp(Blarg,"environ.bmp"))
				{
					pMesh = Engine->GetMesh(CurMesh);
					pMesh->SetTexture(Engine->GetTexture("items"));
				}
				else
				if(!strcmp(Blarg,"items.bmp"))
				{
					pMesh = Engine->GetMesh(CurMesh);
					pMesh->SetTexture(Engine->GetTexture("architecture"));
				}
				else
				if(!strcmp(Blarg,"architecture.bmp"))
				{
					pMesh = Engine->GetMesh(CurMesh);
					pMesh->SetTexture(Engine->GetTexture("alltrees"));
				}
				SyncToMesh();
				break;
			case IDC_BLOCKING:
				pWin = GetChild(IDC_BLOCKING);
				Blarg = pWin->GetText();
				if(!strcmp(Blarg,"BlockAll"))
				{
					pMesh = Engine->GetMesh(CurMesh);
					pMesh->SetBlocking(BLOCK_PARTIAL);
				}
				else
				if(!strcmp(Blarg,"BlockPart"))
				{
					pMesh = Engine->GetMesh(CurMesh);
					pMesh->SetBlocking(BLOCK_NONE);
				}
				else
				if(!strcmp(Blarg,"BlockNone"))
				{
					pMesh = Engine->GetMesh(CurMesh);
					pMesh->SetBlocking(BLOCK_ALL);
				}
				SyncToMesh();
				break;
			case IDC_DELETE:
				if(Confirm(this,"Delete this mesh?","yes","no"))
				{
					Engine->DeleteMesh(CurMesh);
					SyncToMesh();
				}
				break;
/*			
			case  IDC_NEXT_A:
				PointA++;
				SyncToMesh();
				pWin = GetChild(IDC_CUR_POINTA);
				pWin->SetText(PointA);
				break;
			case  IDC_PREV_A:
				if(PointA > 0)
					PointA--;
				pWin = GetChild(IDC_CUR_POINTA);
				pWin->SetText(PointA);
				break;
*/
			case  IDC_SCALE_PLUS:
				ScalePercent++;
				pWin = GetChild(IDC_SCALE_INFO);
				pWin->SetText(ScalePercent);
				break;
			case  IDC_SCALE_MINUS:
				ScalePercent--;
				pWin = GetChild(IDC_SCALE_INFO);
				pWin->SetText(ScalePercent);
				break;
			case IDC_SCALE:
				float f;
				f = 1.0f / Engine->GetMesh(CurMesh)->xscale;

				//convertback to full size
				Engine->GetMesh(CurMesh)->Scale(f,f,f);

				Engine->GetMesh(CurMesh)->Scale(ConvertFromPercent(ScalePercent),
															ConvertFromPercent(ScalePercent),
															ConvertFromPercent(ScalePercent));
				
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->xscale = ConvertFromPercent(ScalePercent);
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->Move(0.0f,0.0f,-Engine->GetMesh(CurMesh)->GetBottomBound());
				Engine->GetMesh(CurMesh)->Center();
				Engine->GetMesh(CurMesh)->GetBounds();
				SyncToMesh();
				break;

			case  IDC_NEXT_LINK:
				PointLink++;
				SyncToMesh();
				pWin = GetChild(IDC_CUR_POINTLINK);
				pWin->SetText(PointLink);
				break;

			case  IDC_PREV_LINK:
				if(PointLink > 0)
					PointLink--;
				pWin = GetChild(IDC_CUR_POINTLINK);
				pWin->SetText(PointLink);
				break;

			case  IDC_NEXT_SLOT:
				CurSlot++;
				UpdateCurSlotText();
				break;
			case  IDC_PREV_SLOT:
				if(CurSlot > 0)
					CurSlot--;
				UpdateCurSlotText();
				break;

			case  IDC_NEXT_MESH:
				CurMesh++;
				if(CurMesh == Engine->GetNumMesh())
				{
					CurMesh = 0;
				}
				SyncToMesh();
				pWin = GetChild(IDC_MESH_NAME);
				pWin->SetText(Engine->GetMesh(CurMesh)->GetName());
				if(Engine->GetMesh(CurMesh)->equipmentlist)
				{
					memcpy(Equipslots,Engine->GetMesh(CurMesh)->equipmentlist,sizeof(EquipmentLocator) * 10);
				}
				PointLink = Engine->GetMesh(CurMesh)->equipmentRegistrationMark.LinkIndex;
				pWin = GetChild(IDC_CUR_POINTLINK);
				pWin->SetText(PointLink);
				pWin = GetChild(IDC_SCALE_INFO);
				ScalePercent = ConvertToPercent(Engine->GetMesh(CurMesh)->xscale);
				pWin->SetText(ScalePercent);
				UpdateSlotText();
				break;

			case  IDC_PREV_MESH:
				if(CurMesh > 0)
				{
					CurMesh--;
				}
				else
				{
					CurMesh = Engine->GetNumMesh() - 1;
				}
				SyncToMesh();
				pWin = GetChild(IDC_MESH_NAME);
				pWin->SetText(Engine->GetMesh(CurMesh)->GetName());
				if(Engine->GetMesh(CurMesh)->equipmentlist)
				{
					memcpy(Equipslots,Engine->GetMesh(CurMesh)->equipmentlist,sizeof(EquipmentLocator) * 10);
				}
				PointLink = Engine->GetMesh(CurMesh)->equipmentRegistrationMark.LinkIndex;
				pWin = GetChild(IDC_CUR_POINTLINK);
				pWin->SetText(PointLink);
				ScalePercent = ConvertToPercent(Engine->GetMesh(CurMesh)->xscale);
				pWin->SetText(ScalePercent);
				UpdateSlotText();
				break;

			case IDC_NEXT_FRAME:
				CurFrame++;
				SyncToMesh();
				pWin = GetChild(IDC_CUR_FRAME);
				pWin->SetText(CurMesh);
				break;

			case IDC_PREV_FRAME:
				if(CurFrame > 0)
					CurFrame--;
				SyncToMesh();
				pWin = GetChild(IDC_CUR_FRAME);
				pWin->SetText(CurFrame);
				break;

			case  IDC_SET_SLOT:
				pMesh = Engine->GetMesh(CurMesh);

				if(!pMesh->equipmentlist)
				{
					pMesh->equipmentlist = new EquipmentLocator[10];
					memset(pMesh->equipmentlist,0,sizeof(EquipmentLocator) * 10);
					pMesh->numWeaponSlotsDefined = 5;
				}

				memcpy(pMesh->equipmentlist, Equipslots, sizeof(EquipmentLocator) * 10);

				SyncToMesh();
				break;

			case  IDC_SET_LINK:
				pMesh = Engine->GetMesh(CurMesh);

				if(CurSlot <= EQUIP_POS_CUSTOM4)
				{
					pMesh->equipmentRegistrationMark.LinkIndex = PointLink;
				}
				SyncToMesh();
				break;

			case  IDC_EXIT_EDIT:

				Hide();
				State = WINDOW_STATE_DONE;
				ReleaseFocus();
				break;

			case IDC_SAVE:
				Engine->SaveMeshes("mesh.bin");
				break;

			case IDC_SMOOTH:
				Engine->GetMesh(CurMesh)->SmoothNormals();
				break;

			case IDC_SHARPEN:
				Engine->GetMesh(CurMesh)->Sharpen();
				break;

			case IDC_RELOAD:

				pMesh = Engine->GetMesh(CurMesh);

				BOOL Equipped;

				if(pMesh->equipmentlist)
				{
					Equipped = TRUE;
				}
				
				pMesh->Import(pMesh->filename);
				pMesh->Scale(pMesh->xscale,
								 pMesh->xscale,
								 pMesh->xscale);
				pMesh->GetBounds();
				pMesh->Move(0.0f,0.0f,-pMesh->GetBottomBound());
				pMesh->GetBounds();
				pMesh->Center();
				pMesh->GetBounds();

				if(Equipped)
				{
					pMesh->equipmentlist = new EquipmentLocator[10];
					memset(pMesh->equipmentlist,0,sizeof(EquipmentLocator) * 10);
					pMesh->numWeaponSlotsDefined = 5;
					memcpy(pMesh->equipmentlist, Equipslots, sizeof(EquipmentLocator) * 10);
				}
				break;

			case IDC_ROTATE_X:
				Engine->GetMesh(CurMesh)->Rotate(PI_DIV_4 / 16.0f, 0.0f, 0.0f);
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->Center();
				Engine->GetMesh(CurMesh)->GetBounds();
				
				SyncToMesh();
				break;

			case IDC_ROTATE_Y:
				Engine->GetMesh(CurMesh)->Rotate(0.0f, PI_DIV_4 / 16.0f, 0.0f);
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->Center();
				Engine->GetMesh(CurMesh)->GetBounds();
				
				SyncToMesh();
				break;

			case IDC_ROTATE_Z:
				Engine->GetMesh(CurMesh)->Rotate(0.0f,0.0f,PI_DIV_4 / 16.0f);
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->Center();
				Engine->GetMesh(CurMesh)->GetBounds();
				
				SyncToMesh();
				break;
	
			case IDC_FLIPNORMALS:
				Engine->GetMesh(CurMesh)->FixNormals();
				break;

			case IDC_IMPORT:
				Engine->Import(CurMesh);
				break;

			case IDC_ADD_NEW:
				Engine->ImportMeshes();
				break;

			case IDC_MOVE_NORTH:
				Engine->GetMesh(CurMesh)->Move(0.0f, -0.025f, 0.0f);
				break;
			case IDC_MOVE_EAST:
				Engine->GetMesh(CurMesh)->Move(0.025f, 0.00f, 0.0f);
				break;
	
			case IDC_MOVE_WEST:
				Engine->GetMesh(CurMesh)->Move(-0.025f, 0.00f, 0.0f);
				break;
	
			case IDC_MOVE_SOUTH:
				Engine->GetMesh(CurMesh)->Move(0.0f, 0.025f, 0.0f);
				break;
			
			case IDC_MOVE_UP:
				Engine->GetMesh(CurMesh)->Move(0.0f, 0.0f, 0.025f);
				break;

			case IDC_MOVE_DOWN:
				Engine->GetMesh(CurMesh)->Move(0.0f, 0.0f, -0.025f);
				break;

			case IDC_SCALE_X_UP:
				Engine->GetMesh(CurMesh)->Scale(1.025f,1.0f,1.0f);
				break;

			case IDC_SCALE_X_DOWN:
				Engine->GetMesh(CurMesh)->Scale(1.0f/1.025f,1.0f,1.0f);
				break;

			case IDC_SCALE_Y_UP:
				Engine->GetMesh(CurMesh)->Scale(1.0f,1.025f,1.0f);
				break;

			case IDC_SCALE_Y_DOWN:
				Engine->GetMesh(CurMesh)->Scale(1.0f,1.0f/1.025f,1.0f);
				break;

			case IDC_SCALE_Z_UP:
				Engine->GetMesh(CurMesh)->Scale(1.0f,1.0f,1.025f);
				break;

			case IDC_SCALE_Z_DOWN:
				Engine->GetMesh(CurMesh)->Scale(1.0f,1.0f,1.0f/1.025f);
				break;

			case IDC_NORMALIZE:
				float fleft, fright, ftop, fbottom, ffront, fback;
				float width;
				float height;
				float depth;
				Engine->GetMesh(CurMesh)->GetBounds(&fleft, &fright, &ftop, &fbottom, &ffront, &fback, 0);
				width = fright - fleft;
				height = fback - ffront;
				depth = ftop - fbottom;

				if(width < 0.0f) width *= -1.0f;
				if(height < 0.0f) height *= -1.0f;
				if(depth < 0.0f) depth *= -1.0f;


				Engine->GetMesh(CurMesh)->Scale(1.0f/width, 1.0f / height, 1.0f / depth);
				
				Engine->GetMesh(CurMesh)->GetBounds(&fleft, &fright, &ftop, &fbottom, &ffront, &fback, 0);
				
				Engine->GetMesh(CurMesh)->Move( -1.0f * fleft, -1.0f * ffront, -1.0f * fbottom);

				Engine->GetMesh(CurMesh)->GetBounds(&fleft, &fright, &ftop, &fbottom, &ffront, &fback, 0);
			

				break;
			default:
				break;

		}
		break;
		case COMMAND_BUTTON_RIGHTCLICKED:
		switch(IDFrom)
		{
			case IDC_ROTATE_X:
				Engine->GetMesh(CurMesh)->Rotate(-PI_DIV_4 / 16.0f, 0.0f, 0.0f);
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->Center();
				Engine->GetMesh(CurMesh)->GetBounds();
				
				SyncToMesh();
				break;

			case IDC_ROTATE_Y:
				Engine->GetMesh(CurMesh)->Rotate(0.0f, -PI_DIV_4 / 16.0f, 0.0f);
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->Center();
				Engine->GetMesh(CurMesh)->GetBounds();
				
				SyncToMesh();
				break;

			case IDC_ROTATE_Z:
				Engine->GetMesh(CurMesh)->Rotate(0.0f,0.0f,-PI_DIV_4 / 16.0f);
				Engine->GetMesh(CurMesh)->GetBounds();
				Engine->GetMesh(CurMesh)->Center();
				Engine->GetMesh(CurMesh)->GetBounds();
				
				SyncToMesh();
				break;
		}
		break;
	case COMMAND_LIST_SELECTED:
	case COMMAND_LIST_SELECTIONCHANGED:
		int NewCurMesh;
		if(IDFrom == IDC_MESH_LIST)
		{
			ZSOldList *pWin;
			pWin = (ZSOldList *)GetChild(IDFrom);
			NewCurMesh = pWin->GetSelection();
		}
		if(NewCurMesh != CurMesh)
		{
			CurMesh = NewCurMesh;
			SyncToMesh();
			pWin = GetChild(IDC_MESH_NAME);
			pWin->SetText(Engine->GetMesh(CurMesh)->GetName());
			if(Engine->GetMesh(CurMesh)->equipmentlist)
			{
				memcpy(Equipslots,Engine->GetMesh(CurMesh)->equipmentlist,sizeof(EquipmentLocator) * 10);
			}
			PointLink = Engine->GetMesh(CurMesh)->equipmentRegistrationMark.LinkIndex;
			pWin = GetChild(IDC_CUR_POINTLINK);
			pWin->SetText(PointLink);
			pWin = GetChild(IDC_SCALE_INFO);
			ScalePercent = ConvertToPercent(Engine->GetMesh(CurMesh)->xscale);
			pWin->SetText(ScalePercent);
			UpdateSlotText();
		}
		break;

	default:
		break;
	}

	return TRUE;
}

int EEWin::Draw()
{
	if(Visible)
	{
		if(State == WINDOW_STATE_NORMAL)
		{
			DrawMesh();

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
	else
	{
		if(pSibling)
		{
			pSibling->Draw();
		}
	}
	return TRUE;
}


EEWin::EEWin(ZSWindow *newParent)
{

	E_RIGHT = Engine->Graphics()->GetWidth();
	E_BOTTOM = Engine->Graphics()->GetHeight();

	pChild = NULL;
	State = WINDOW_STATE_NORMAL;
	Show();

	Bounds.left =	 E_LEFT;
	Bounds.right =	 E_RIGHT;
	Bounds.bottom = E_BOTTOM;
	Bounds.top =	 E_TOP;

	memset(Equipslots, 0, sizeof(EquipmentLocator) * 10);

	ZSButton *pButton;
	ZSText *pText;

	BackGroundSurface = NULL;
	CurMesh = 0;
	
	int n;

	pButton = new ZSButton(BUTTON_NORTH, IDC_ROTATE_TOP,		E_LEFT + 192,	E_TOP + 0,		16,	16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_EAST,  IDC_ROTATE_RIGHT,	E_RIGHT -16,	E_TOP + 192,	16,	16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_SOUTH, IDC_ROTATE_BOTTOM,	E_LEFT + 192,	E_BOTTOM - 16,	16,	16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_WEST,  IDC_ROTATE_LEFT,		E_LEFT + 0,		E_TOP + 192,	16,	16);
	AddChild(pButton);
	pButton->Show();
	
	for(n = 0; n < 10; n++)
	{
		pButton = new ZSButton(BUTTON_PLUS, IDC_EQUIPSLOTPLUS1 + n, E_LEFT + 33,    E_TOP + 100 + 33 * n,		16,	16);
		AddChild(pButton);
		pButton->Show();
	
		pButton = new ZSButton(BUTTON_MINUS, IDC_EQUIPSLOTMINUS1 + n, E_LEFT + 16,  E_TOP + 100 + 33 * n,		16,	16);
		AddChild(pButton);
		pButton->Show();
		
		pButton = new ZSButton(BUTTON_PLUS, IDC_EQUIPSLOTPLUS1 + 10 + n, E_LEFT + 33,    E_TOP + 100 + 16 + 33 * n,		16,	16);
		AddChild(pButton);	
		pButton->Show();
		
		pButton = new ZSButton(BUTTON_MINUS, IDC_EQUIPSLOTMINUS1 + 10 + n, E_LEFT + 16,  E_TOP + 100 + 16 + 33 * n,		16,	16);
		AddChild(pButton);
		pButton->Show();
		
		pText = new ZSText(IDC_EQUIPSLOT1 + n, E_LEFT+50, E_TOP+100 + 33 * n, "WWWWWWWWWWW");
		AddChild(pText);
		pText->Show();
	}

	pButton = new ZSButton(BUTTON_PLUS,  IDC_NEXT_LINK,	E_LEFT + 16,	E_BOTTOM - 32,		16,	16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_MINUS, IDC_PREV_LINK,	E_LEFT,			E_BOTTOM - 32,		16,	16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_PLUS,  IDC_NEXT_SLOT,	E_LEFT + 200 + 16,	E_BOTTOM - 32,		16,	16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_MINUS, IDC_PREV_SLOT,	E_LEFT + 200 ,			E_BOTTOM - 32,		16,	16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_PLUS,  IDC_NEXT_MESH,	E_LEFT + 32,	E_TOP + 16,		16,	16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_MINUS, IDC_PREV_MESH,	E_LEFT + 16,	E_TOP + 16,		16,	16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_PLUS,  IDC_NEXT_FRAME,	E_LEFT + 32,	E_TOP + 36,		16,	16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_MINUS, IDC_PREV_FRAME,	E_LEFT + 16,	E_TOP + 36,		16,	16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SET_SLOT,	E_LEFT + 200 + 32 + Engine->Graphics()->GetFontEngine()->GetTextWidth("RIGHTHAND"),	E_BOTTOM - 32,		Engine->Graphics()->GetFontEngine()->GetTextWidth("Set Slot"),	16);
	AddChild(pButton);
	pButton->SetText("Set Slot");
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_NORMAL, IDC_SET_LINK,	E_LEFT + 64,	E_BOTTOM - 32,		Engine->Graphics()->GetFontEngine()->GetTextWidth("Set Link"),	16);
	AddChild(pButton);
	pButton->SetText("Set Link");
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_NORMAL, IDC_SAVE, E_RIGHT - 256, E_TOP + 16, Engine->Graphics()->GetFontEngine()->GetTextWidth("Save"), 16);
	AddChild(pButton);
	pButton->SetText("Save");
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_NORMAL, IDC_IMPORT, E_RIGHT - 160, E_TOP + 16, Engine->Graphics()->GetFontEngine()->GetTextWidth("Import"), 16);
	AddChild(pButton);
	pButton->SetText("Import");
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_NORMAL, IDC_RELOAD, E_RIGHT - 256, E_TOP + 36, Engine->Graphics()->GetFontEngine()->GetTextWidth("Load"), 16);
	AddChild(pButton);
	pButton->SetText("Load");
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_ADD_NEW, E_RIGHT - 160, E_TOP + 36, Engine->Graphics()->GetFontEngine()->GetTextWidth("Add New"), 16);
	AddChild(pButton);
	pButton->SetText("Add New");
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_TEXTURE, E_RIGHT - 256, E_TOP + 54, 128, 16);
	AddChild(pButton);
	pButton->SetText("alltrees");
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_NORMAL, IDC_BLOCKING, E_RIGHT - 256, E_TOP + 72, 128, 16);
	AddChild(pButton);
	pButton->SetText("BlockAll");
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_DELETE, E_RIGHT - 256, E_TOP + 96, 64,16);
	AddChild(pButton);
	pButton->SetText("Delete");
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SMOOTH, E_RIGHT - 128, E_TOP + 72, 64, 16);
	AddChild(pButton);
	pButton->SetText("Smooth");
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SHARPEN, E_RIGHT - 128, E_TOP + 88, 64, 16);
	AddChild(pButton);
	pButton->SetText("Sharp");
	pButton->Show();


	pButton = new ZSButton(BUTTON_PLUS, IDC_SCALE_PLUS, E_RIGHT - 100 + 16, E_TOP +128, 16, 16);
	AddChild(pButton);
	pButton->Show();
	
	pButton = new ZSButton(BUTTON_MINUS, IDC_SCALE_MINUS, E_RIGHT - 100, E_TOP + 128, 16, 16);
	AddChild(pButton);
	pButton->Show();
	
	pText = new ZSText(IDC_SCALE_INFO, E_RIGHT - 100 + 32, E_TOP + 128,"0000000");
	AddChild(pText);
	ScalePercent = ConvertToPercent(Engine->GetMesh(CurMesh)->xscale);
	pText->SetText(ScalePercent);
	pText->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_SCALE, E_RIGHT - 100, E_TOP + 128 + 16, Engine->Graphics()->GetFontEngine()->GetTextWidth("Scale"),16);
	pButton->SetText("Scale");
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_ROTATE_X, E_RIGHT - 100, E_TOP + 128 + 36, Engine->Graphics()->GetFontEngine()->GetTextWidth("RotateX"),16);
	pButton->SetText("RotateX");
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_ROTATE_Y, E_RIGHT - 100, E_TOP + 128 + 72, Engine->Graphics()->GetFontEngine()->GetTextWidth("RotateY"),16);
	pButton->SetText("RotateY");
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_ROTATE_Z, E_RIGHT - 100, E_TOP + 128 + 108, Engine->Graphics()->GetFontEngine()->GetTextWidth("RotateZ"),16);
	pButton->SetText("RotateZ");
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_FLIPNORMALS, E_RIGHT - 100, E_TOP + 128 + 144, Engine->Graphics()->GetFontEngine()->GetTextWidth("Flip Normals"),16);
	pButton->SetText("Flip Normals");
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_NORMALIZE, E_RIGHT - 100, E_TOP + 128 + 126, Engine->Graphics()->GetFontEngine()->GetTextWidth("Normalize"),16);
	pButton->SetText("Normaliz");
	AddChild(pButton);
	pButton->Show();
	
	
//the six movement buttons
	pButton = new ZSButton(BUTTON_NORTH, IDC_MOVE_NORTH, E_RIGHT - 100, E_TOP + 128 + 160, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_WEST, IDC_MOVE_WEST, E_RIGHT - 116, E_TOP + 128 + 176, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_EAST, IDC_MOVE_EAST, E_RIGHT - 84, E_TOP + 128 + 176, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_SOUTH, IDC_MOVE_SOUTH, E_RIGHT - 100, E_TOP + 128 + 192, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORTH, IDC_MOVE_UP, E_RIGHT - 48, E_TOP + 128 + 176, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_SOUTH, IDC_MOVE_DOWN, E_RIGHT - 48, E_TOP + 128 + 192, 16, 16);
	AddChild(pButton);
	pButton->Show();

//the six individual scale factor buttons

	pButton = new ZSButton(BUTTON_NORTH, IDC_SCALE_X_UP, E_RIGHT - 116, E_TOP + 128 + 210, 16, 16);
	AddChild(pButton);
	pButton->SetText("x");
	pButton->Show();

	pButton = new ZSButton(BUTTON_SOUTH, IDC_SCALE_X_DOWN, E_RIGHT - 116, E_TOP + 128 + 226, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORTH, IDC_SCALE_Y_UP, E_RIGHT - 96, E_TOP + 128 + 210, 16, 16);
	AddChild(pButton);
	pButton->SetText("y");
	pButton->Show();

	pButton = new ZSButton(BUTTON_SOUTH, IDC_SCALE_Y_DOWN, E_RIGHT - 96, E_TOP + 128 + 226, 16, 16);
	AddChild(pButton);
	pButton->Show();

	pButton = new ZSButton(BUTTON_NORTH, IDC_SCALE_Z_UP, E_RIGHT - 76, E_TOP + 128 + 210, 16, 16);
	AddChild(pButton);
	pButton->SetText("z");
	pButton->Show();

	pButton = new ZSButton(BUTTON_SOUTH, IDC_SCALE_Z_DOWN, E_RIGHT - 76, E_TOP + 128 + 226, 16, 16);
	AddChild(pButton);
	pButton->Show();




	pText = new ZSText(IDC_CUR_SLOT, E_LEFT + 200+32, E_BOTTOM - 32, "blargnlarg");		
	AddChild(pText);
	pText->Show();

	pText = new ZSText(IDC_CUR_POINTLINK, E_LEFT + 32, E_BOTTOM - 32, "0000");
	AddChild(pText);
	pText->Show();

	pText = new ZSText(IDC_MESH_NAME, E_LEFT + 48, E_TOP + 16, "nullnullnullnullnullnull");
	AddChild(pText);
	pText->Show();

	pText = new ZSText(IDC_CUR_FRAME, E_LEFT + 48, E_TOP + 36, "00000000");
	AddChild(pText);
	pText->Show();

	pButton = new ZSButton(BUTTON_NORMAL, IDC_EXIT_EDIT, E_LEFT + 0, E_TOP + 0, 16, 16);
	pButton->SetText("X");
	AddChild(pButton);
	pButton->Show();
	
	for(n = IDC_EQUIPINFO1; n <= IDC_EQUIPINFO5; n++)
	{
		pText = new ZSText(n, E_RIGHT - 64, E_TOP + 20 * (n - IDC_EQUIPINFO1),"nullnullnullnullnullnullnull");
		AddChild(pText);
		pText->Show();
	}

	newParent->AddChild(this);

	TransWin	 *pTrans;

	pTrans = new TransWin(this, D3DRGBA(0.0f, 0.0f, 0.0f, 0.5f), 0);

	PreludeWorld->AddMainObject((Object *)pTrans);

	PointLink = 0;
	CurSlot = 0;
	CurFrame = 0;

	SetCursor(CURSOR_POINT);

	SyncToMesh();

	Angle = 0.0f;
	UpdateSlotText();

	ZSOldList *pList;
	pList = new ZSOldList(IDC_MESH_LIST, Engine->Graphics()->GetWidth() - 192, Engine->Graphics()->GetHeight() - 192, 190, 190);
	pList->Show();
	AddChild(pList);

	ZSModelEx *pMesh;
	int NumMesh;
	NumMesh = Engine->GetNumMesh();
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



	
}

int EEWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	ZSWindow *pWin;
	int n;
	ZSModelEx *pMesh;
	if(CurrentKeys[DIK_L] & 0x80)
	{
		Angle += ROTATION_ANGLE;
	}
	if(CurrentKeys[DIK_R] & 0x80)
	{
		Angle -= ROTATION_ANGLE;
	}
	
	if(CurrentKeys[DIK_D] & 0x80)
	{
		//doublemeshes
		for(n = 0; n < Engine->GetNumMesh(); n ++)
		{
			pMesh = Engine->GetMesh(n);

			pMesh->Scale(2.0f,2.0f,2.0f);
	
			pMesh->GetBounds();
			pMesh->Move(0.0f,0.0f,-pMesh->GetBottomBound());
			pMesh->Center();
			pMesh->GetBounds();
		
		}
	}


/*
	if(CurrentKeys[DIK_LSHIFT] & 0x80 ||
		CurrentKeys[DIK_RSHIFT])
	{
		if(CurrentKeys[DIK_B] & 0x80
			&& !(LastKeys[DIK_B] & 0x80))
		{
			if(PointB > 0)
				PointB--;
			pWin = GetChild(IDC_CUR_POINTB);
			pWin->SetText(PointB);
		}
		if(CurrentKeys[DIK_A] & 0x80
		&& !(LastKeys[DIK_A] & 0x80))
		{
			if(PointA > 0)
				PointA--;
			pWin = GetChild(IDC_CUR_POINTA);
			pWin->SetText(PointA);
		}

	}
	else
	{
		if(CurrentKeys[DIK_A] & 0x80
			&& !(LastKeys[DIK_A] & 0x80))
		{
			PointA++;
			SyncToMesh();
			pWin = GetChild(IDC_CUR_POINTA);
			pWin->SetText(PointA);
		}
		
		if(CurrentKeys[DIK_B] & 0x80
			&& !(LastKeys[DIK_B] & 0x80))
		{
			PointB++;
			SyncToMesh();
			pWin = GetChild(IDC_CUR_POINTB);
			pWin->SetText(PointB);
		}
	}
	*/
	if(CurrentKeys[DIK_EQUALS] & 0x80
		&& !(LastKeys[DIK_EQUALS] & 0x80))
	{
		PointLink++;
		SyncToMesh();
		pWin = GetChild(IDC_CUR_POINTLINK);
		pWin->SetText(PointLink);
	}

	if(CurrentKeys[DIK_MINUS] & 0x80
		&& !(LastKeys[DIK_MINUS] & 0x80))
	{
		if(PointLink > 0)
			PointLink--;
		pWin = GetChild(IDC_CUR_POINTLINK);
		pWin->SetText(PointLink);
	}

	return TRUE;
}
	
void EEWin::Show()
{
	Visible = TRUE;
	State = WINDOW_STATE_NORMAL;
	SetFocus(this);
}
	
