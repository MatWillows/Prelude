//the equip editor window

#ifndef EQUIPEDITWIN_H
#define EQUIPEDITWIN_H

#include "ZSMODELEX.H"
#include "ZSwindow.h"

class EEWin : public ZSWindow
{
	private:
		int CurMesh;
		int CurSlot;
		EquipmentLocator  Equipslots[10];
		EquipmentLocator Link;
		int PointLink;
		int CurLink;
		int CurFrame;
		float Angle;
		D3DXMATRIX matProjection;
		D3DXMATRIX matCamera;
		D3DXMATRIX matRotation;
		int ScalePercent;
		
		void DrawPoints();
		void DrawMesh();
		void Rotate(DIRECTION_T DirectionType);
		void UpdateCurSlotText();
		void UpdateSlotText();
		void SyncToMesh();
		
	public:
		int Command(int IDFrom, int Command, int Param);
		int Draw();
		int HandleKeys(BYTE *CurrentKeys, BYTE *LastKeys);
		void Show();

		EEWin(ZSWindow *newParent);

};





#endif