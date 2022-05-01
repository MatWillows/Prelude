#include "peopleedit.h"
#include "creatures.h"
#include "world.h"
#include "locator.h"
#include "zsbutton.h"
#include "zsintspin.h"
#include "zstext.h"
#include "ZSOldListbox.h"
#include "party.h"

typedef enum
{
	IDC_LOCATOR_START_TIME,
	IDC_LOCATOR_END_TIME,
	IDC_LOCATOR_LEFT,
	IDC_LOCATOR_RIGHT,
	IDC_LOCATOR_TOP,
	IDC_LOCATOR_BOTTOM,
	IDC_LOCATOR_KILL,
	IDC_LOCATOR_DIRECTION,
	IDC_LOCATOR_STATE,
	IDC_PEOPLE_EDIT_CREATUREWIN,
	IDC_ADD_LOCATOR,
	IDC_LOCATOR_AREA
} LOCATOR_CONTROLS;

LocatorEditWin::LocatorEditWin(int NewID, int x, int y, int width, int height)
{
	pLocator = NULL;
	Visible = FALSE;
	State = WINDOW_STATE_NORMAL;
	ID = NewID;

	Bounds.left = x;
	Bounds.right = x + width;
	Bounds.top = y;
	Bounds.bottom = y + height;

	ZSIntSpin *pIS;
	ZSText *pText;
	ZSButton *pButton;

	//start
	pText = new ZSText(-1,x+0,y+0,"start");
	pText->Show();
	AddChild(pText);

	pIS = new ZSIntSpin(IDC_LOCATOR_START_TIME, x+0, y+16, 64, 16);
	pIS->Show();
	pIS->SetValue(0);
	AddChild(pIS);

	//end
	pText = new ZSText(-1,x+64,y+0,"end");
	pText->Show();
	AddChild(pText);

	pIS = new ZSIntSpin(IDC_LOCATOR_END_TIME, x+64, y+16, 64, 16);
	pIS->Show();
	pIS->SetValue(0);
	AddChild(pIS);

	
	//left
	pText = new ZSText(-1,x+0,y+32,"left");
	pText->Show();
	AddChild(pText);

	pIS = new ZSIntSpin(IDC_LOCATOR_LEFT, x+0, y+48, 64, 16);
	pIS->Show();
	pIS->SetValue(0);
	AddChild(pIS);
	
	//top
	pText = new ZSText(-1,x+64,y+32,"top");
	pText->Show();
	AddChild(pText);
	
	pIS = new ZSIntSpin(IDC_LOCATOR_TOP, x+64, y+48, 64, 16);
	pIS->Show();
	pIS->SetValue(0);
	AddChild(pIS);
	
	
	//right
	pText = new ZSText(-1,x+0,y+64,"right");
	pText->Show();
	AddChild(pText);
	
	pIS = new ZSIntSpin(IDC_LOCATOR_RIGHT, x+0, y+80, 64, 16);
	pIS->Show();
	pIS->SetValue(0);
	AddChild(pIS);

	//bottom
	pText = new ZSText(-1,x+64,y+64,"bottom");
	pText->Show();
	AddChild(pText);

	pIS = new ZSIntSpin(IDC_LOCATOR_BOTTOM, x+64, y+80, 64, 16);
	pIS->Show();
	pIS->SetValue(0);
	AddChild(pIS);

	//kill
	pButton = new ZSButton(BUTTON_NORMAL, IDC_LOCATOR_KILL, x+64, y+96, 64, 16);
	pButton->SetText("kill");
	pButton->Show();
	AddChild(pButton);

	//direction
	pButton = new ZSButton(BUTTON_NORMAL, IDC_LOCATOR_DIRECTION,x,y+112, 64, 16);
	pButton->SetText("N");
	pButton->Show();
	AddChild(pButton);

	//state
	pButton = new ZSButton(BUTTON_NORMAL, IDC_LOCATOR_STATE, x + 64, y + 112, 64, 16);
	pButton->SetText("none");
	pButton->Show();
	AddChild(pButton);

	//area
	pButton = new ZSButton(BUTTON_NORMAL, IDC_LOCATOR_AREA, x+ 128, y, 64, 24);
	pButton->SetText("none");
	pButton->Show();
	AddChild(pButton);


}

void LocatorEditWin::SetLocator(Locator *pNewLocator)
{
	pLocator = pNewLocator;

	ZSIntSpin *pIS;
	RECT rLoc;

	pLocator->GetBounds(&rLoc);
	
	pIS = (ZSIntSpin *)GetChild(IDC_LOCATOR_LEFT);
	pIS->SetValue(rLoc.left);

	pIS = (ZSIntSpin *)GetChild(IDC_LOCATOR_RIGHT);
	pIS->SetValue(rLoc.right);

	pIS = (ZSIntSpin *)GetChild(IDC_LOCATOR_TOP);
	pIS->SetValue(rLoc.top);

	pIS = (ZSIntSpin *)GetChild(IDC_LOCATOR_BOTTOM);
	pIS->SetValue(rLoc.bottom);

	pIS = (ZSIntSpin *)GetChild(IDC_LOCATOR_START_TIME);
	pIS->SetValue(pLocator->GetStart());

	pIS = (ZSIntSpin *)GetChild(IDC_LOCATOR_END_TIME);
	pIS->SetValue(pLocator->GetEnd());

	SetLocatorState();
	SetLocatorDirection();
	SetLocatorArea();

}

void LocatorEditWin::SetLocatorArea()
{
	ZSWindow *pWin;
	pWin = GetChild(IDC_LOCATOR_AREA);
	pWin->SetText(PreludeWorld->GetArea(pLocator->GetArea())->GetName());
}

void LocatorEditWin::SetLocatorState()
{
	ZSWindow *pWin;
	pWin = GetChild(IDC_LOCATOR_STATE);
	switch(pLocator->GetState())
	{
	case 0:
	default:
		pLocator->SetState(0);
		pWin->SetText("none");
		break;
	case 1:
		pWin->SetText("face");
		break;
	case 2:
		pWin->SetText("sit");
		break;
	case 3:
		pWin->SetText("lay");
		break;
	}
}


void LocatorEditWin::SetLocatorDirection()
{
	ZSWindow *pWin;
	pWin = GetChild(IDC_LOCATOR_DIRECTION);
	switch(pLocator->GetAngle())
	{
	case NORTH:
	default:
		pLocator->SetAngle(NORTH);
		pWin->SetText("N");
		break;
	case SOUTH:
		pWin->SetText("S");
		break;
	case EAST:
		pWin->SetText("E");
		break;
	case WEST:
		pWin->SetText("W");
		break;
	case NORTHEAST:
		pWin->SetText("NE");
		break;
	case NORTHWEST:
		pWin->SetText("NW");
		break;
	case SOUTHEAST:
		pWin->SetText("SE");
		break;
	case SOUTHWEST:
		pWin->SetText("SW");
		break;
	}

}


int LocatorEditWin::Command(int IDFrom, int Command, int Param)
{
	RECT rBounds;
	ZSIntSpin *pIS;
		switch(IDFrom)
		{
		case IDC_LOCATOR_START_TIME:
			pIS = (ZSIntSpin *)GetChild(IDFrom);
			pLocator->SetStart(pIS->GetValue());
			break;

		case IDC_LOCATOR_END_TIME:
			pIS = (ZSIntSpin *)GetChild(IDFrom);
			pLocator->SetEnd(pIS->GetValue());
			break;

		case IDC_LOCATOR_LEFT:
			pIS = (ZSIntSpin *)GetChild(IDFrom);
			pLocator->GetBounds(&rBounds);
			rBounds.left = pIS->GetValue();
			pLocator->SetBounds(&rBounds);
			break;
			
		case IDC_LOCATOR_RIGHT:
			pIS = (ZSIntSpin *)GetChild(IDFrom);
			pLocator->GetBounds(&rBounds);
			rBounds.right = pIS->GetValue();
			pLocator->SetBounds(&rBounds);
			break;
	
		case IDC_LOCATOR_TOP:
			pIS = (ZSIntSpin *)GetChild(IDFrom);
			pLocator->GetBounds(&rBounds);
			rBounds.top = pIS->GetValue();
			pLocator->SetBounds(&rBounds);
			break;
		
		case IDC_LOCATOR_BOTTOM:
			pIS = (ZSIntSpin *)GetChild(IDFrom);
			pLocator->GetBounds(&rBounds);
			rBounds.bottom = pIS->GetValue();
			pLocator->SetBounds(&rBounds);
			break;

	}
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		switch(IDFrom)
		{
		case IDC_LOCATOR_KILL:
			if(Command == COMMAND_BUTTON_CLICKED)
			{
				return pParent->Command(ID,COMMAND_BUTTON_CLICKED,ID - 1000);
			}
			break;
		
		case IDC_LOCATOR_DIRECTION:
			pLocator->SetAngle(pLocator->GetAngle() + 1);
			SetLocatorDirection();
			break;
		
		case IDC_LOCATOR_STATE:
			pLocator->SetState(pLocator->GetState() + 1);
			SetLocatorState();
			break;
		}
	}


	return TRUE;
}

void PeopleEditWin::DrawLocators()
{
	Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

	D3DVERTEX Verts[5];

	Verts[0].nz = Verts[1].nz = Verts[1].nz = Verts[3].nz = Verts[4].nz = 0.0f;
	Verts[0].tu = Verts[1].tu = Verts[1].tu = Verts[3].tu = Verts[4].tu = 0.0f;
	Verts[0].tv = Verts[1].tv = Verts[1].tv = Verts[3].tv = Verts[4].tv = 0.0f;
			
	Locator *pLoc;
	RECT rLoc;

	for(int n = 0; n < NumLocators; n++)
	{
		pLoc = LocationEditors[n]->GetLocator();
		if(PreludeWorld->GetArea(pLoc->GetArea()) == Valley)
		{
			pLoc->GetBounds(&rLoc);

			Verts[4].x = Verts[0].x = rLoc.left;
			Verts[4].y = Verts[0].y = rLoc.top;
			Verts[4].z = Verts[0].z = Valley->GetTileHeight(rLoc.left,rLoc.top) + 0.2f;

			Verts[1].x = rLoc.right;
			Verts[1].y = rLoc.top;
			Verts[1].z = Valley->GetTileHeight(rLoc.right,rLoc.top) + 0.2f;

			Verts[2].x = rLoc.right;
			Verts[2].y = rLoc.bottom;
			Verts[2].z = Valley->GetTileHeight(rLoc.right,rLoc.bottom) + 0.2f;

			Verts[3].x = rLoc.left;
			Verts[3].y = rLoc.bottom;
			Verts[3].z = Valley->GetTileHeight(rLoc.left,rLoc.bottom) + 0.2f;

			HRESULT hr;
			hr = Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_LINESTRIP, D3DFVF_VERTEX, Verts, 5, 0 );
			if(hr != D3D_OK)
			{
				DEBUG_INFO("problem drawing Locator wall\n");
				exit(1);
			}
		}
	}



}

int PeopleEditWin::Draw()
{
	if(Visible)
	{
		Engine->Graphics()->GetD3D()->BeginScene();
		
		Valley->Draw();
		if(ShowTiles) 
			Valley->DrawTiles();

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);    
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
		Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);

		Engine->Graphics()->ClearTexture();
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
		
		Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());
			
		if(EditMode == PEDIT_MODE_PLACE_PERSON && pCurPerson)
		{
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
			pCurPerson->GetMesh()->Draw(Engine->Graphics()->GetD3D(),(float)(int)MouseX + 0.5f, (float)(int)MouseY + 0.5f,MouseZ,0.0f,0);
			Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		}
	
		if(pCurPerson)
		{
			pCurPerson->HighLight(COLOR_GREEN);
			
			DrawLocators();
		}

		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

		Engine->Graphics()->GetD3D()->EndScene();

		if(pChild)
		{
			pChild->Draw();
		}
		Engine->Graphics()->DrawCursor(Engine->Input()->GetMouseRect());
	}
	return TRUE;
}
	
int PeopleEditWin::LeftButtonDown(int x, int y)
{
	ZSWindow *pWin;

	pWin = GetChild(x,y);

	if(pWin)
	{
		return pWin->LeftButtonDown(x,y);
	}
	else
	{
		SetFocus(this);
		if(Cursor == CURSOR_POINT)
		{
			Engine->Graphics()->SetCursorFrame(1);
		}
	}

	PreludeWorld->ConvertMouse(x,y, &vLocatorStart.x, &vLocatorStart.y, &vLocatorStart.z);

	return TRUE;
}

int PeopleEditWin::LeftButtonUp(int x, int y)
{
	ReleaseFocus();
	if(Cursor == CURSOR_POINT)
	{
		Engine->Graphics()->SetCursorFrame(0);
	}
	PreludeWorld->ConvertMouse(x,y, &vLocatorEnd.x, &vLocatorEnd.y, &vLocatorEnd.z);
	if(EditMode == PEDIT_ADD_LOCATOR)
	{
		AddLocator();
		return TRUE;
	}
	else
	if(EditMode == PEDIT_MODE_PLACE_PERSON)
	{
		if(!pCurPerson)
		{
			return TRUE;
		}

		Creature *pCreature;
		
		if(((Thing *)pCurPerson)->GetData(INDEX_BATTLEID).Value)
		{
			pCreature = new Creature((Creature *)pCurPerson);
			pCurPerson = pCreature;
			if(!Engine->GetTextureNum(pCurPerson->GetTexture()))
			{
				pCurPerson->SetTexture(NULL);
				pCreature->CreateTexture();
			}
		}
		else
		{
			pCreature = (Creature *)pCurPerson;
			if(PreludeWorld->GetArea(pCreature->GetAreaIn()))
				PreludeWorld->GetArea(pCreature->GetAreaIn())->RemoveFromUpdate((Object *)pCreature);
		}
		if(pCreature->GetData(INDEX_BATTLEID).Value)
		{
			while(pCreature->GetNumLocators())
			{
				pCreature->RemoveLocator(0);
			}
			pCreature->AddLocator();
			Locator *pLoc;
			pLoc = pCreature->GetLocator(0);
			
			RECT LocRect;
			
			LocRect.left = (int)MouseX;
			LocRect.right = (int)MouseX + 1;
			LocRect.top = (int)MouseY;
			LocRect.bottom = (int)MouseY + 1;
	
			pLoc->SetBounds(&LocRect);
			pLoc->SetArea(PreludeWorld->GetCurAreaNum());
			SetupLocationEditors();

			ZSOldList *pList;
			pList = (ZSOldList *)GetChild(IDC_PEOPLE_EDIT_CREATUREWIN);
			pList->Clear();

			Thing *pThing;
			pThing = Creature::GetFirst();
			char IDString[64];

			while(pThing)
			{
				if(pThing->GetData(INDEX_BATTLEID).Value)
				{
					sprintf(IDString, "%s # %i", pThing->GetData(INDEX_NAME).String, pThing->GetData(INDEX_UID).String);
					pList->AddItem(IDString);
				}
				else
				{
					pList->AddItem(pThing->GetData(INDEX_NAME).String);
				}
				pThing = (Thing *)pThing->GetNext();
			}
		}

		D3DVECTOR *pVector;
		pVector = pCreature->GetPosition();
		pVector->x = (float)((int)(MouseX)) + 0.5f;
		pVector->y = (float)((int)(MouseY)) + 0.5f;
		pVector->z = Valley->GetZ(pVector->x,pVector->y);
		pCreature->SetAreaIn(PreludeWorld->GetCurAreaNum());
		pCreature->AddToWorld();
	}

	return TRUE;
}

int PeopleEditWin::RightButtonDown(int x, int y)
{
	SetFocus(this);
	return TRUE;
}

int PeopleEditWin::RightButtonUp(int x, int y)
{
	ReleaseFocus();
	return TRUE;
}

int PeopleEditWin::MoveMouse(long *x, long *y, long *z)
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

	return TRUE;
}

int PeopleEditWin::Command(int IDFrom, int Command, int Param)
{
	ZSOldList *pWin;
	if(Command == COMMAND_BUTTON_CLICKED)
	{
		if(!pCurPerson)
		{
			return TRUE;
		}
		Creature *pCreature;
		pCreature = (Creature *)pCurPerson;
		pCreature->RemoveLocator(Param);
		SetupLocationEditors();
		return TRUE;
	}
	else
	if(IDFrom == IDC_PEOPLE_EDIT_CREATUREWIN)
	{
		char ThingName[64];
		char *IDString;
		int UID;
		int ID;
		pWin = (ZSOldList *)GetChild(IDFrom);
		pWin->GetText(pWin->GetSelection(),ThingName);
		IDString = strchr(ThingName, '#');
		if(IDString)
		{
			IDString--;
			IDString[0] = '\0';
			IDString++;
			IDString++;
			UID = atoi(IDString);
			pCurPerson = Thing::Find(Creature::GetFirst(),ThingName);
			ID = ((Creature *)pCurPerson)->GetData(INDEX_ID).Value;
			pCurPerson = Thing::Find(Creature::GetFirst(),ID, UID);
		}
		else
		{
			pCurPerson = Thing::Find(Creature::GetFirst(),ThingName);
		}
		SetupLocationEditors();
		EditMode = PEDIT_MODE_PLACE_PERSON;
	}	


	return TRUE;
}

int PeopleEditWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_X] & 0x80)
	{
		State = WINDOW_STATE_DONE;
		Visible = FALSE;
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_M] & 0x80)
	{
		if(pCurPerson)
		{
			PreludeParty.Teleport((int)pCurPerson->GetPosition()->x, (int)pCurPerson->GetPosition()->y);
			State = WINDOW_STATE_DONE;
			Visible = FALSE;
		}
		return TRUE;
	}

	if((CurrentKeys[DIK_LCONTROL] & 0x80 || CurrentKeys[DIK_RCONTROL] & 0x80) && CurrentKeys[DIK_D] & 0x80)
	{
		if(pCurPerson)
		{
			Creature *pCreature;
			pCreature = (Creature *)pCurPerson;
		
			if(PreludeWorld->GetArea(pCreature->GetAreaIn()))
				PreludeWorld->GetArea(pCreature->GetAreaIn())->RemoveFromUpdate((Object *)pCreature);
	
			delete pCreature;

			pCurPerson = NULL;

			ZSOldList *pList;
			pList = (ZSOldList *)GetChild(IDC_PEOPLE_EDIT_CREATUREWIN);
			pList->Clear();

			Thing *pThing;
			pThing = Creature::GetFirst();
			char IDString[64];

			while(pThing)
			{
				if(pThing->GetData(INDEX_BATTLEID).Value)
				{
					sprintf(IDString, "%s # %i", pThing->GetData(INDEX_NAME).String, pThing->GetData(INDEX_UID).String);
					pList->AddItem(IDString);
				}
				else
				{
					pList->AddItem(pThing->GetData(INDEX_NAME).String);
				}
				pThing = (Thing *)pThing->GetNext();
			}
		}
		return TRUE;
	}


	if(CurrentKeys[DIK_A] & 0x80 && !(LastKeys[DIK_A] & 0x80))
	{
		if(pCurPerson)
		{
			EditMode = PEDIT_ADD_LOCATOR;
		}
	}

	if(CurrentKeys[DIK_G] & 0x80 && !(LastKeys[DIK_G] & 0x80))
	{
		GetPerson();
	}

	return TRUE;
}

void PeopleEditWin::Show()
{
	Visible = TRUE;
	SetCursor(CURSOR_POINT);
	return;
}

void PeopleEditWin::GetPerson()
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
	vMouseNear.y = (1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->top/(float)(Engine->Graphics()->GetHeight()-100));
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

	Object *pOb;
	pOb = Valley->GetTarget(&vRayStart, &vRayEnd);

	if(pOb && pOb->GetObjectType() == OBJECT_CREATURE)
	{
		pCurPerson = pOb;
		SetupLocationEditors();
	}

}

PeopleEditWin::PeopleEditWin()
{
	Visible = TRUE;
	State = WINDOW_STATE_NORMAL;
	
	ZSWindow::GetMain()->AddChild(this);

	Bounds.left = 0;
	Bounds.top = 0;
	Bounds.right = Engine->Graphics()->GetWidth();
	Bounds.bottom = Engine->Graphics()->GetHeight();
	
	int n;
	int yn;
	yn = 0;
	for(n = 0; n < 8; n ++)
	{
		LocationEditors[n] = new LocatorEditWin(n+1000,0,yn, 128, yn+128);
		LocationEditors[n]->Hide();
		AddChild(LocationEditors[n]);
		yn += 128;
	}

	ShowTiles = FALSE;
	pCurPerson = FALSE,
	EditMode = PEDIT_MODE_PLACE_PERSON;

	ZSOldList *pList;
	pList = new ZSOldList(IDC_PEOPLE_EDIT_CREATUREWIN, Engine->Graphics()->GetWidth() - 192, 0, 192, 224);
	pList->SetText("Creatures");
	pList->Show();
	AddChild(pList);

	Thing *pThing;
	pThing = Creature::GetFirst();
	char IDString[64];

	while(pThing)
	{
		if(pThing->GetData(INDEX_BATTLEID).Value)
		{
			sprintf(IDString, "%s # %i", pThing->GetData(INDEX_NAME).String, pThing->GetData(INDEX_UID).String);
			pList->AddItem(IDString);
		}
		else
		{
			pList->AddItem(pThing->GetData(INDEX_NAME).String);
		}
		pThing = (Thing *)pThing->GetNext();
	}

}

void PeopleEditWin::AddLocator()
{
	if(!pCurPerson)
	{
		return;
	}
	Creature *pCreature;
	pCreature = (Creature *)pCurPerson;
	pCreature->AddLocator();
	Locator *pLocator;
	pLocator = pCreature->GetLocator(pCreature->GetNumLocators() - 1);
	RECT rLoc;
	rLoc.left = vLocatorStart.x;
	rLoc.right = vLocatorEnd.x;
	rLoc.top = vLocatorStart.y;
	rLoc.bottom = vLocatorEnd.y;

	pLocator->SetBounds(&rLoc);

	pLocator->SetArea(PreludeWorld->GetCurAreaNum());

	SetupLocationEditors();
}

void PeopleEditWin::RemoveLocator(int Num)
{
	if(!pCurPerson)
	{
		return;
	}
}

void PeopleEditWin::SetupLocationEditors()
{
	if(!pCurPerson)
		return;

	int n;
	Creature *pCreature;
	pCreature = (Creature *)pCurPerson;
	NumLocators = pCreature->GetNumLocators();

	for(n = 0; n < NumLocators; n++)
	{
		LocationEditors[n]->Show();
		LocationEditors[n]->SetLocator(pCreature->GetLocator(n));
	}
	for(;n < 8; n++)
	{
		LocationEditors[n]->Hide();
	}

}


