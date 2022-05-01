#include "equipobject.h"
#include "zsengine.h"

void EquipObject::Draw()
{
	int Frame;

	Frame = pLink->GetData(INDEX_FRAME).Value;

	ZSModelEx *pLinkMesh;

	pLinkMesh = pLink->GetMesh();

	pItem->GetMesh()->DrawAsEquipment(Engine->Graphics()->GetD3D(),1.0f,(ZSModel *)pLinkMesh,0,0,Frame, LinkSlot,0.0f,pLink->GetData(INDEX_POSITION).pVector,pLink->GetData(INDEX_ROTATION).fValue);

}

EquipObject::EquipObject(Thing *pNewLink, Thing *pNewItem, EQUIP_POSITION NewLinkSlot)
{
	LinkSlot = NewLinkSlot;
	pLink = pNewLink;
	pItem = pNewItem;
	pTexture = pItem->GetTexture();
	Frame = 0;
	pMesh = pItem->GetMesh();
}
