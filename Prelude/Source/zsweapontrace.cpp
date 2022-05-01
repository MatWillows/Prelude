#include "zsweapontrace.h"
#include "equipobject.h"
#include "creatures.h"

void WeaponTracer::Draw()
{
	if(NumVerts > 2)
	{
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
		int Start;
		Start = NumVerts - 8;
		int End = 8;
		if(Start < 0)
		{
			End = NumVerts;
			Start = 0;
		}

	/*Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, AlphaOp);

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, SourceBlend);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, DestBlend);
	Engine->Graphics()->GetD3D()->SetMaterial(&Material);

	hr = Engine->Graphics()->GetD3D()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, D3DFVF_VERTEX, Particles, MAX_PARTICLES * 3, ParticleDrawList, DrawLength, 0);

	if(hr != D3D_OK)
	{
		Engine->ReportError("problem drawing particle system");
	}

	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	Engine->Graphics()->GetD3D()->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
*/

		Engine->Graphics()->SetTexture(NULL);
		
		Engine->Graphics()->GetD3D()->SetMaterial(&Material);

		Engine->Graphics()->GetD3D()->SetTransform(D3DTRANSFORMSTATE_WORLD, Engine->Graphics()->GetIdentity());

		Engine->Graphics()->GetD3D()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

		Engine->Graphics()->GetD3D()->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_VERTEX, &Verts[Start], End, NULL);

		Engine->Graphics()->GetD3D()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
		
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_WHITE));
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	}
}

void WeaponTracer::AddLine()
{
	//get weapon mesh
	ZSModelEx *pMesh;
	pMesh = pLink->GetItem()->GetMesh();

	//get the two points
	int Start;
	int End;

	if(pMesh->equipmentlist)
	{
		Start = pMesh->equipmentlist[0].RayFromIndex;
		End = pMesh->equipmentlist[0].LinkIndex;
	}
	else
	{
		Start = pMesh->equipmentRegistrationMark.LinkIndex;
		End = (pMesh->numvertex - 1);
		if(Start == End) End --;
	}

	D3DVERTEX vxStart;
	D3DVERTEX vxEnd;
	vxStart = pMesh->GetPoint(Start, 0);
	vxEnd = pMesh->GetPoint(End, 0);

	D3DXVECTOR3 vStart;
	D3DXVECTOR3 vEnd;
	vStart.x = vxStart.x;
	vStart.y = vxStart.y;
	vStart.z = vxStart.z;

	vEnd.x = vxEnd.x;
	vEnd.y = vxEnd.y;
	vEnd.z = vxEnd.z;

	ZSModelEx* target;
 	float rotation;
 	int frame;
 	int targetFrame; 
 	EQUIP_POSITION equip_position;
 	D3DVECTOR *WorldPosition;
	float targetAngle;
 
	target = pLink->GetLink()->GetMesh();
	rotation = pLink->GetLink()->GetMyAngle();
	WorldPosition = pLink->GetLink()->GetPosition();
	equip_position = pLink->GetEquipPosition();
	targetAngle = pLink->GetLink()->GetMyAngle();
	frame = 0;
	targetFrame =  pLink->GetLink()->GetFrame();
	
	//the rotation value gives the rotation around the normal axis found in the target's frame.
	//  (should our sword be held with guards in front of the fingers or beside the thumb

	//the rotation of the object to the proper normal on the model is handled automatically.

	//frame gives the frame of animation of the object (ie: flail) to render onto the target
	//targetFrame gives the current frame of the target's (ie: swordsman's) animation to grab
	//  the equipmentlocator information from.

	D3DXMATRIX mmove;
	D3DXMATRIX mrotate; 
	D3DXMATRIX mtargetrotate;
	D3DXMATRIX mmyrotate;
	D3DXMATRIX mtemp;
	D3DXMATRIX matWorld;
	D3DVECTOR targetNormal;
	D3DVECTOR targetOrigin;
	D3DVECTOR objectNormal;
	D3DVECTOR LinkPoint;
	D3DVECTOR TargetLink;
	D3DVECTOR TargetRayFrom;
	D3DVECTOR temp;

	D3DVALUE  theta;

	//set the world matrix to identity. Might not be needed.
	//not needed as we override it later
	D3DXMatrixIdentity(&matWorld);
	
	//Set the target's rotation Matrix;
	D3DXMatrixRotationZ(&mtargetrotate, targetAngle + PI);
	
	D3DXMatrixRotationY(&mmyrotate, -PI_DIV_2);
	
	//Set Translate matrix;
	//lots of de'refing going on here could be optimized with heavier use of pointers

	//translate the current model to the origin:
	int LinkArrayOffset = target->equipmentlist[equip_position].LinkIndex * 3;
	int RayArrayOffset = target->equipmentlist[equip_position].RayFromIndex * 3;
	int MyLinkArrayOffset = pMesh->equipmentRegistrationMark.LinkIndex * 3;

	LinkPoint.x = pMesh->stridedVertexArray[frame][MyLinkArrayOffset];
	LinkPoint.y = pMesh->stridedVertexArray[frame][MyLinkArrayOffset + 1];
	LinkPoint.z = pMesh->stridedVertexArray[frame][MyLinkArrayOffset + 2];
		
	TargetLink.x = target->stridedVertexArray[targetFrame][LinkArrayOffset];
	TargetLink.y = target->stridedVertexArray[targetFrame][LinkArrayOffset + 1];
	TargetLink.z = target->stridedVertexArray[targetFrame][LinkArrayOffset + 2];
	
	TargetRayFrom.x = target->stridedVertexArray[targetFrame][RayArrayOffset];
	TargetRayFrom.y = target->stridedVertexArray[targetFrame][RayArrayOffset + 1];
	TargetRayFrom.z = target->stridedVertexArray[targetFrame][RayArrayOffset + 2];
	
	D3DXMatrixTranslation( &mmove, -(LinkPoint.x), -(LinkPoint.y), -(LinkPoint.z) );

	//set the rotations
	//grab the info from the other model and figure out where to rotate.
	// to make the axes of both models line up.
	targetNormal = TargetLink - TargetRayFrom;
	
	targetNormal = Normalize(targetNormal);

	//objectNormal = pMesh->equipmentRegistrationMark[equip_position];
	objectNormal = D3DVECTOR(0.0f,1.0f,0.0f);
	
		//get the axis of rotation 
	
		temp = CrossProduct(objectNormal, targetNormal);

		//get the angle of rotation
		//magnitude can be eliminated if both are normalized vectors

		theta = acos(DotProduct(objectNormal, targetNormal));

		//pMesh rotates the main axis of the object (ie: weapon) to the proper direction 
		// on the model.
		D3DXMatrixRotationAxis(&mrotate,&(D3DXVECTOR3)temp, theta);  //check degrees or radians in other calcs.

	//now handle rotation about the proper axis of the weapon
		D3DXMatrixMultiply(&mrotate,&mmyrotate,&mrotate);
	
		D3DXMatrixRotationAxis(&mtemp, &(D3DXVECTOR3)objectNormal, rotation);

		D3DXMatrixMultiply(&mrotate,&mrotate,&mtemp);

		D3DXMatrixMultiply(&matWorld, &mmove, &mrotate);
		
	//translate the object to match the target's object origin for the current frame.
		//grab the equipmentLocation origin from the model.
		//  we reference the proper equipment slot directly thanks to the enum
		//   and use the targetFrame to get the right frame of the model's animation.
	
			//set translate matrix

			D3DXMatrixTranslation(&mmove, TargetLink.x, TargetLink.y, TargetLink.z);

			D3DXMatrixMultiply(&mmove, &mmove, &mtargetrotate);
			
			D3DXMatrixTranslation(&mtemp, WorldPosition->x, WorldPosition->y, WorldPosition->z);

			D3DXMatrixMultiply(&mmove, &mmove, &mtemp);

	
	//do the final multiplication
	D3DXMatrixMultiply(&matWorld, &matWorld, &mmove);

	//the rest is basically the same as draw();
		// we now have the place we have to move to, the proper rotation to align to the 
		//  target model's normal, the offset amounts, and the scale factor.

	D3DXVECTOR4 vOut;

	D3DXVec3Transform(&vOut,(D3DXVECTOR3 *)&vStart, &matWorld);

	Verts[NumVerts].x = vOut.x;
	Verts[NumVerts].y = vOut.y;
	Verts[NumVerts].z = vOut.z;
	Verts[NumVerts].nx = 0.0f;
	Verts[NumVerts].ny = 0.0f;
	Verts[NumVerts].nz = 1.0f;
	Verts[NumVerts].tu = NumVerts;
	Verts[NumVerts].tv = 0.0;

	NumVerts++;

	D3DXVec3Transform(&vOut,(D3DXVECTOR3 *)&vEnd, &matWorld);

	Verts[NumVerts].x = vOut.x;
	Verts[NumVerts].y = vOut.y;
	Verts[NumVerts].z = vOut.z;
	Verts[NumVerts].nx = 0.0f;
	Verts[NumVerts].ny = 0.0f;
	Verts[NumVerts].nz = 1.0f;
	Verts[NumVerts].tu = NumVerts -1;
	Verts[NumVerts].tv = 1.0f;

	NumVerts++;
}

BOOL WeaponTracer::AdvanceFrame()
{
	//check to see if there's a change
	//if so add a line to our arc
	Creature *pCreature;
	pCreature = (Creature *)pLink->GetLink();

	if(pCreature->GetFrame() != LastFrame ||
	   pCreature->GetMyAngle() != LastAngle)
	{
		LastFrame = pCreature->GetFrame();
		LastAngle = pCreature->GetMyAngle();
		AddLine();
	}

	return TRUE;
}






