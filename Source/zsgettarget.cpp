#include "zsgettarget.h"
#include "zsengine.h"
#include "objects.h"
#include "world.h"
#include "creatures.h"
#include "gameitem.h"

int ZSTargetWin::Draw()
{
	Engine->Graphics()->GetD3D()->BeginScene();
	Valley->Draw();

	if(ValidTarget)
	{
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

		switch(TType)
		{
			case TARGET_LOCATION:
				Engine->GetMesh("sphere")->Draw(Engine->Graphics()->GetD3D(),vMouse.x,vMouse.y,vMouse.z,0.0f,0);
				break;
			
			case TARGET_CREATURE:
				((Creature *)pTarget)->HighLight(COLOR_GREEN);
				((Creature *)pTarget)->AltHighLight();
				break;

			case TARGET_ITEM:
				((GameItem *)pTarget)->HighLight(COLOR_BLUE);
				((GameItem *)pTarget)->AltHighLight();
				break;
		
			case TARGET_DIRECTION:
				break;
			
			case TARGET_NONE:
			default:
				break;
		}
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	}
	Engine->Graphics()->GetD3D()->EndScene();

	GetFocus()->DrawMouse(Engine->Input()->GetMouseRect()->left, Engine->Input()->GetMouseRect()->top);


	if(pChild)
	{
		pChild->Draw();
	}

	if(pSibling)
	{
		pSibling->Draw();
	}

	return TRUE;
}

int ZSTargetWin::GoModal()
{
	while(State != WINDOW_STATE_DONE && ZSWindow::GetMain()->GetState() != WINDOW_STATE_DONE)
	{
		pMainWindow->GetChild()->Draw();
		Engine->Graphics()->Flip();
		Engine->Input()->Update(this);
	}

	if(!ValidTarget)
	{
		if(TType == TARGET_LOCATION)
		{
			delete (D3DVECTOR *)pTarget;
		}
		else
		if(TType == TARGET_DIRECTION)
		{
			delete (int *)pTarget;
		}
	}
	return ReturnCode;
}

int ZSTargetWin::MoveMouse(long *x, long *y, long *z)
{
	//the meat and bones of gettarget
	//constrain to world
	if(*x >= Bounds.left && *x <= Bounds.right &&
		*y >= Bounds.top && *y <= Bounds.bottom)
	{
		PreludeWorld->ConvertMouse(*x, *y, &this->vMouse.x,&this->vMouse.y, &this->vMouse.z);
		
		GetMouseTarget();
	}
	
	D3DXMATRIX mRotation;
	D3DXVECTOR4 vResult;
	D3DVECTOR vOffset;
	
	if(*x <= Bounds.left)
	{
		if(!LBDown)
		{
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.x = -0.1f;
			vOffset.y = 0.0f;
			vOffset.z = 0.0f;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			//move to tope of screen
			PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
		}
		else
		{
			PreludeWorld->RotateCamera(ROTATION_ANGLE);
			PreludeWorld->ChangeCamera();	
		}
		*x = Bounds.left;
	}
	else
	if(*x >= Bounds.right)
	{
		if(!LBDown)
		{
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.x = 1.0f;
			vOffset.y = 0.0f;
			vOffset.z = 0.0f;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			//move to tope of screen
			PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
		}
		else
		{
			PreludeWorld->RotateCamera(-ROTATION_ANGLE);
			PreludeWorld->ChangeCamera();	
		}
		*x = Bounds.right;
	}

	if(*y <= Bounds.top)
	{
		if(!LBDown)
		{
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.y = -1.0f;
			vOffset.x = 0.0f;
			vOffset.z = 0.0f;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			//move to tope of screen
			PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
		}
		else
		{
			PreludeWorld->ArcCamera(-ROTATION_ANGLE);
			PreludeWorld->ChangeCamera();
		}
		*y = Bounds.top;
	}
	else
	if(*y >= Bounds.bottom)
	{
		if(!LBDown)
		{
			D3DXMatrixRotationZ(&mRotation, PreludeWorld->GetCameraAngle());
			vOffset.y = 1.0f;
			vOffset.x = 0.0f;
			vOffset.z = 0.0f;
			D3DXVec3Transform(&vResult,(D3DXVECTOR3 *)&vOffset,&mRotation);
			//move to tope of screen
			PreludeWorld->UpdateCameraOffset((D3DVECTOR)vResult);
		}
		else
		{
			PreludeWorld->ArcCamera(ROTATION_ANGLE);
			PreludeWorld->ChangeCamera();
		}
		*y = Bounds.bottom;
	}

	MouseScreenX = *x;
	MouseScreenY = *y;

	return TRUE;
}

int ZSTargetWin::LeftButtonDown(int x, int y)
{
	LBDown = TRUE;
	SetFocus(this);
	return TRUE;
}

int ZSTargetWin::LeftButtonUp(int x, int y)
{
	LBDown = FALSE;
	ReleaseFocus();
	if(ValidTarget)
	{
		State = WINDOW_STATE_DONE;
		ReturnCode = 1;
	}
	return TRUE;
}

int ZSTargetWin::RightButtonUp(int x, int y)
{
	ReleaseFocus();
	State = WINDOW_STATE_DONE;
	if(ValidTarget)
	{
		ReturnCode = 1;
	}
	else
	{
		ReturnCode = 0;
	}

	return TRUE;
}

int ZSTargetWin::HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys)
{
	if(CurrentKeys[DIK_ESCAPE] & 0x80 && !(LastKeys[DIK_ESCAPE] & 0x80))
	{
		State = WINDOW_STATE_DONE;
		if(ValidTarget)
		{
			ReturnCode = 1;
		}
		else
		{
			ReturnCode = 0;
		}
	}
	return TRUE;
}

void ZSTargetWin::GetMouseTarget()
{
	//check range
	float fDistance;
	fDistance = GetDistance(&vSource, &vMouse);
	if(fDistance > frange)
	{
		ValidTarget = FALSE;
		return;
	}

	Object *pOb = NULL;
	
	//mouse is in range
	switch(TType)
	{
		case TARGET_LOCATION:
			((D3DVECTOR *)pTarget)->x = vMouse.x = (float)(int)vMouse.x + 0.5f;
			((D3DVECTOR *)pTarget)->y = vMouse.y = (float)(int)vMouse.y + 0.5f;
			((D3DVECTOR *)pTarget)->z = vMouse.z = Valley->GetTileHeight(vMouse.x, vMouse.y);

			ValidTarget = TRUE;
			return;
		case TARGET_CREATURE:
		case TARGET_ITEM:
		{
			D3DVECTOR vRayStart, vRayEnd;

			//create the test ray
			vRayEnd = _D3DVECTOR(this->vMouse.x,this->vMouse.y,this->vMouse.z);

			D3DVECTOR vMouseNear;
			
			//convert mouse to camera space
			vMouseNear.x = -(1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->left/(float)Engine->Graphics()->GetWidth());
			vMouseNear.y = (1.0f - 2.0f * (float)Engine->Input()->GetMouseRect()->top/(float)(Engine->Graphics()->GetHeight() - 100));
			vMouseNear.z = 0.0f;

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

			pOb = Valley->GetTarget(&vRayStart, &vRayEnd);
			if(!pOb)
			{
				ValidTarget = FALSE;
				return;
			}
		}
		break;
		case TARGET_DIRECTION:
			


			break;
		case TARGET_NONE:
		default:
			ValidTarget = FALSE;
			return;
			break;
	}

	if(pOb->GetObjectType() == OBJECT_CREATURE)
	{
		if(TType == TARGET_ITEM)
		{
			ValidTarget = FALSE;
			return;
		}
	}
	else
	if(pOb->GetObjectType() != OBJECT_ITEM)
	{
		ValidTarget = FALSE;
		return;
	}

	pTarget = (void *)pOb;
	ValidTarget = TRUE;
	return;

}


ZSTargetWin::ZSTargetWin(int NewID, TARGET_T TargetType, D3DVECTOR vTargetSource, float fTargetRange, char *Message)
{
	Visible = FALSE;
	Moveable = FALSE;
	State = WINDOW_STATE_NORMAL;
	ReturnCode = FALSE;

	Bounds.left = 0;
	Bounds.top = 0;
	Bounds.bottom = Engine->Graphics()->GetHeight() - 103;
	Bounds.right = Engine->Graphics()->GetWidth();

	TType = TargetType;
	vSource = vTargetSource;
	frange = fTargetRange;
	ValidTarget = FALSE;

	switch(TargetType)
	{
	case TARGET_DIRECTION:
		pTarget = new int;
		break;
	case TARGET_LOCATION:
		pTarget = new D3DVECTOR;
		break;
	default:
		break;
	}

	SetText(Message);
	Describe(Message);
}

BOOL GetTarget(void **Target, TARGET_T Type, D3DVECTOR vSource, float frange, char *Message)
{
	ZSTargetWin *pWin;
	pWin = new ZSTargetWin(-1, Type, vSource, frange, Message);

	pWin->Show();
	ZSWindow::GetMain()->AddTopChild(pWin);

	pWin->SetFocus(pWin);

	BOOL TargetFound;

	TargetFound = pWin->GoModal();
	
	pWin->ReleaseFocus();

	if(!TargetFound)
	{
		ZSWindow::GetMain()->RemoveChild(pWin);
		return FALSE;
	}

	*Target = pWin->GetTarget();

	ZSWindow::GetMain()->RemoveChild(pWin);

	return TRUE;
}
