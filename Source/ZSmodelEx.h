#ifndef ZSModelEx_H
#define ZSModelEx_H

#include "ZSModel.h"
#include "ZStexture.h"

#define MESH_NAME_LENGTH		32
#define TEXTURE_NAME_LENGTH	32

class ZSModelEx : public ZSModel
{
public:

	char filename[MESH_NAME_LENGTH];

	float xscale, yscale, zscale; //base scalars
	int rangemin, rangemax; //percent scalars for random placement

	BYTE	Blocking;

	ZSTexture *pTexture; //the texture used to draw this mesh

	//coordinates which define the cube which encompasses the mesh
	float leftbound;
	float rightbound;
	float topbound;
	float bottombound;
	float frontbound;
	float backbound;

	void GetBounds();
	void GetBounds(float *pLeft, float *pRight, float *pTop, float *pBottom, float *pFront, float *pBack, int Frame);

	void Scale(float xfactor, float yfactor, float zfactor);
	void Move(float xfactor, float yfactor, float zfactor);
	void Rotate(float xaxis, float yaxis, float zaxis);
	void Flip(BOOL x, BOOL y, BOOL z);

	void Center();

	void LoadEx(const char *filename);
	void LoadEx(FILE *fp);

	void SaveEx(const char *filename);
	void SaveEx(FILE *fp);

	int GetNumFrames() { return numframes; }

	inline char *GetName()
	{
		return filename;
	}

	float GetHeight() { return topbound - bottombound; }
	float GetWidth() { return rightbound - leftbound; }
	float GetDepth() { return backbound - frontbound; }

	float GetLeftBound() { return leftbound; }
	float GetRightBound() { return rightbound; }
	float GetTopBound() { return topbound; }
	float GetBottomBound() { return bottombound; }
	float GetFrontBound() { return frontbound; }
	float GetBackBound() { return backbound; }

	D3DVECTOR GetPointVector(int Point, int Frame, float Angle);

	D3DVERTEX GetPoint(int Point, int Frame);
	
   HRESULT Draw(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, int frame);
	HRESULT Draw(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, float xscale, float yscale, float zscale, int frame);
	HRESULT Draw(LPDIRECT3DDEVICE7 D3DDevice, int frame);

	HRESULT DrawLit(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, float xscale, float yscale, float zscale, int frame);

	void FixNormals();

	ZSTexture *GetTexture() { return pTexture; }
	void SetTexture(ZSTexture *pNewTexture) { pTexture = pNewTexture; }

	BYTE GetBlocking() { return Blocking; }
	void SetBlocking(BYTE Value) { Blocking = Value; }

	BOOL Intersect(int Frame, D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd);
	BOOL LineIntersect(int Frame, D3DVECTOR *vLineStart, D3DVECTOR *vLineEnd);


	float GetZ(float x, float y);

	void SmoothNormals();
	void Sharpen();

};

#endif