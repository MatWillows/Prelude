#include "zsmodelex.h"
#include "defs.h"
#include "zsengine.h"
#include "zsutilities.h"

#ifndef NDEBUG
extern float HeightLevels[1600][1600];
#else
static float HeightLevels[1][1];
#endif

void ZSModelEx::LoadEx(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"rb");

	LoadEx(fp);

	fclose(fp);
}

void ZSModelEx::LoadEx(FILE *fp)
{
	Load(fp);

	char temp[32];

	//get the mesh name
	fread(filename,MESH_NAME_LENGTH,1,fp);

	ConvertToLowerCase(filename);

	//get the texture name
	fread(temp,TEXTURE_NAME_LENGTH,1,fp);

	char *pC;
	
	pC = strchr(temp,'.');

	if(pC)
		*pC = '\0';

	pTexture = Engine->GetTexture(temp);

	//read in the boundary info
	fread((LPBYTE)&leftbound, sizeof(leftbound),1,fp);
	fread((LPBYTE)&rightbound, sizeof(rightbound),1,fp);
	fread((LPBYTE)&topbound, sizeof(topbound),1,fp);
	fread((LPBYTE)&bottombound, sizeof(bottombound),1,fp);
	fread((LPBYTE)&frontbound, sizeof(frontbound),1,fp);
	fread((LPBYTE)&backbound, sizeof(backbound),1,fp);

	fread((LPBYTE)&xscale,		sizeof(xscale),	1,	fp);
	fread((LPBYTE)&yscale,		sizeof(yscale),	1,	fp);
	fread((LPBYTE)&zscale,		sizeof(zscale),	1,	fp);

	fread((LPBYTE)&rangemin,		sizeof(rangemin),	1,	fp);
	fread((LPBYTE)&rangemax,		sizeof(rangemax),	1,	fp);

	fread((LPBYTE)&Blocking, sizeof(Blocking), 1, fp);
	return;
}

void ZSModelEx::SaveEx(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"wb");

	SaveEx(fp);

	fclose(fp);
}

void ZSModelEx::SaveEx(FILE *fp)
{
	Save(fp);
	
	//write out the mesh name
	fwrite((LPBYTE)filename, 32, 1, fp);
	//write the texture name if there is one
	if(pTexture)
	{
		fwrite((LPBYTE)pTexture->GetName(), 32, 1, fp);
	}
	else
	{
		//otherwise
		fwrite(filename, 32, 1, fp);
	}
	//write out the number of frames, etc...
	
	//write out the boundary info
	fwrite((LPBYTE)&leftbound,		sizeof(leftbound),	1,	fp);
	fwrite((LPBYTE)&rightbound,	sizeof(rightbound),	1,	fp);
	fwrite((LPBYTE)&topbound,		sizeof(topbound),		1,	fp);
	fwrite((LPBYTE)&bottombound,	sizeof(bottombound),	1,	fp);
	fwrite((LPBYTE)&frontbound,	sizeof(frontbound),	1,	fp);
	fwrite((LPBYTE)&backbound,		sizeof(backbound),	1,	fp);

	fwrite((LPBYTE)&xscale,		sizeof(xscale),	1,	fp);
	fwrite((LPBYTE)&yscale,		sizeof(yscale),	1,	fp);
	fwrite((LPBYTE)&zscale,		sizeof(zscale),	1,	fp);

	fwrite((LPBYTE)&rangemin,		sizeof(rangemin),	1,	fp);
	fwrite((LPBYTE)&rangemax,		sizeof(rangemax),	1,	fp);

	fwrite((LPBYTE)&Blocking, sizeof(Blocking), 1, fp);

	//done
	return;
}

void ZSModelEx::GetBounds(float *pLeft, float *pRight, float *pTop, float *pBottom, float *pFront, float *pBack, int Frame)
{
	if(Frame > numframes) return;

	int n;
	*pLeft	= stridedVertexArray[Frame][0];
	*pRight	= stridedVertexArray[Frame][0];
	
	*pFront	= stridedVertexArray[Frame][1];
	*pBack	= stridedVertexArray[Frame][1];
	
	*pTop		= stridedVertexArray[Frame][2];
	*pBottom = stridedVertexArray[Frame][2];
	
	for(n = 0; n < numvertex  * 3; n+= 3)
	{
		if(stridedVertexArray[Frame][n] < *pLeft)		*pLeft	= stridedVertexArray[Frame][n];
		if(stridedVertexArray[Frame][n] > *pRight)	*pRight	= stridedVertexArray[Frame][n];

		if(stridedVertexArray[Frame][n+1] < *pFront) *pFront	= stridedVertexArray[Frame][n+1];
		if(stridedVertexArray[Frame][n+1] > *pBack)	*pBack	= stridedVertexArray[Frame][n+1];
	
		if(stridedVertexArray[Frame][n+2] > *pTop)	*pTop			= stridedVertexArray[Frame][n+2];
		if(stridedVertexArray[Frame][n+2] < *pBottom) *pBottom	= stridedVertexArray[Frame][n+2];
	}
}


void ZSModelEx::GetBounds()
{
	int n;
	leftbound	= stridedVertexArray[0][0];
	rightbound	= stridedVertexArray[0][0];
	
	frontbound	= stridedVertexArray[0][1];
	backbound	= stridedVertexArray[0][1];
	
	topbound		= stridedVertexArray[0][2];
	bottombound = stridedVertexArray[0][2];
	
	for(n = 0; n < numvertex  * 3; n+= 3)
	{
		if(stridedVertexArray[0][n] < leftbound) leftbound			= stridedVertexArray[0][n];
		if(stridedVertexArray[0][n] > rightbound) rightbound		= stridedVertexArray[0][n];

		if(stridedVertexArray[0][n+1] < frontbound) frontbound	= stridedVertexArray[0][n+1];
		if(stridedVertexArray[0][n+1] > backbound) backbound		= stridedVertexArray[0][n+1];
	
		if(stridedVertexArray[0][n+2] > topbound) topbound			= stridedVertexArray[0][n+2];
		if(stridedVertexArray[0][n+2] < bottombound) bottombound = stridedVertexArray[0][n+2];
	}

}

void ZSModelEx::Scale(float xfactor, float yfactor, float zfactor)
{
	//center the object
	float cx = 0.0f;
	float cy = 0.0f;
	float cz = 0.0f;
	int n, fn;

	//the center can be considered the average of all the vertices;
	for(n = 0; n < numvertex * 3; n+= 3)
	{
		cx += stridedVertexArray[0][n];
		cy += stridedVertexArray[0][n+1];
		cz += stridedVertexArray[0][n+2];
	}
	cx = cx/(float)numvertex;
	cy = cy/(float)numvertex;
	cz = cz/(float)numvertex;

	for(fn = 0; fn < numframes; fn++)
	for(n = 0; n < numvertex * 3; n += 3)
	{
		stridedVertexArray[fn][n]	 =	((stridedVertexArray[fn][n] - cx)	* xfactor) + cx;
		stridedVertexArray[fn][n+1] = ((stridedVertexArray[fn][n+1] - cy)	* yfactor) + cy;
		stridedVertexArray[fn][n+2] = ((stridedVertexArray[fn][n+2] - cz)	* zfactor) + cz;
	}
	GetBounds();
}

void ZSModelEx::Move(float xfactor, float yfactor, float zfactor)
{
	int n, fn;
	for(fn = 0; fn < numframes; fn++)
	for(n = 0; n < numvertex * 3; n+= 3)
	{
		stridedVertexArray[fn][n]	 += xfactor;
		stridedVertexArray[fn][n+1] += yfactor;
		stridedVertexArray[fn][n+2] += zfactor;
	}

	topbound += zfactor;
	bottombound += zfactor;

	frontbound += yfactor;
	backbound += yfactor;

	leftbound += xfactor;
	rightbound += xfactor;

}


void ZSModelEx::Center()
{

	float xfactor;
	float yfactor;

	float Width = rightbound - leftbound;
	float Depth = backbound - frontbound;

	xfactor = (Width / 2) - rightbound;
	
	yfactor = (Depth / 2) - backbound;

	Move(xfactor,yfactor,0.0f);

}

D3DVECTOR ZSModelEx::GetPointVector(int Point, int Frame, float Angle)
{
//	stridedVertexArray[Frame][Point*3];

	D3DVECTOR Vector = D3DVECTOR(stridedVertexArray[Frame][Point*3],stridedVertexArray[Frame][Point*3+1],stridedVertexArray[Frame][Point*3+2]);

	D3DXMATRIX matRotate;
	
	D3DXVECTOR4 PointVector;

	D3DXMatrixRotationZ(&matRotate, Angle + PI);
	
	D3DXVec3Transform(&PointVector, (D3DXVECTOR3 *)&Vector, &matRotate);

	return (D3DVECTOR)PointVector;

}

HRESULT ZSModelEx::Draw(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, int frame)
{
	//our return value
	HRESULT hr;

	//set up the strided data structure and error check
	//-------------------------------------------------
	
	//if we are not initialized, return a generic error
	if (stridedVertexArray == NULL)
		return DDERR_GENERIC;
	
	//if the uV coords are missing, return a generic error
	if (stridedUV == NULL)
		return DDERR_GENERIC;
	
	if (frame < 0 || frame >= numframes)
	{
		//Engine->ReportError("Mesh frame out of bounds.");
		return DDERR_INVALIDPARAMS;
	}	
	if (stridedVertexArray[frame] == NULL)
		return DDERR_GENERIC;

	//set the data
	stridedDataInfo.position.lpvData = &stridedVertexArray[frame][0];


	//set up rotation matrix
	//set up transformation matrix
	//concatentate
	D3DXMATRIX mmove;
	D3DXMATRIX mrotate; 
	
	D3DXMatrixRotationZ(&mrotate,angle + PI);
	
	D3DXMatrixTranslation(&mmove,x,y,z);
	
	D3DXMATRIX matWorld;

	D3DXMatrixIdentity(&matWorld);

	D3DXMatrixMultiply(&matWorld,&mrotate,&mmove);

	D3DMATRIX finworld;

	finworld(0,0) = matWorld.m[0][0];
	finworld(0,1) = matWorld.m[0][1];
	finworld(0,2) = matWorld.m[0][2];
	finworld(0,3) = matWorld.m[0][3];

	finworld(1,0) = matWorld.m[1][0];
	finworld(1,1) = matWorld.m[1][1];
	finworld(1,2) = matWorld.m[1][2];
	finworld(1,3) = matWorld.m[1][3];	
	
	finworld(2,0) = matWorld.m[2][0];
	finworld(2,1) = matWorld.m[2][1];
	finworld(2,2) = matWorld.m[2][2];
	finworld(2,3) = matWorld.m[2][3];	
	
	finworld(3,0) = matWorld.m[3][0];
	finworld(3,1) = matWorld.m[3][1];
	finworld(3,2) = matWorld.m[3][2];
	finworld(3,3) = matWorld.m[3][3];
	
	D3DDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &finworld );

	//might have to add something here about setting the current texturing state.
	//texture states should be independent of model


	hr = D3DDevice->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
												D3DFVF_VERTEX,
												&this->stridedDataInfo,
												numvertex,
												this->trianglelist,
												numtriangles*3,
												0);
   if(hr != D3D_OK)
	{

	}
												
	return hr;
	
}

//lit version
HRESULT ZSModelEx::DrawLit(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, float xscale, float yscale, float zscale, int frame)
{
	//our return value
	HRESULT hr;
	D3DDRAWPRIMITIVESTRIDEDDATA LitDataInfo;

	//set up the strided data structure and error check
	//-------------------------------------------------
	
	//if we are not initialized, return a generic error
	if (stridedVertexArray == NULL)
		return DDERR_GENERIC;
	
	//if the uV coords are missing, return a generic error
	if (stridedUV == NULL)
		return DDERR_GENERIC;
	
	if (frame < 0 || frame >= numframes)
	{
		//Engine->ReportError("Mesh frame out of bounds.");
		return DDERR_INVALIDPARAMS;
	}		
	if (stridedVertexArray[frame] == NULL)
		return DDERR_GENERIC;
		
	//set the data
	LitDataInfo = stridedDataInfo;

	LitDataInfo.position.lpvData = &stridedVertexArray[frame][0];
	LitDataInfo.normal.lpvData = NULL;
	LitDataInfo.diffuse.lpvData = NULL;
	LitDataInfo.specular.lpvData = NULL;
	

	//set up rotation matrix
	//set up transformation matrix
	//concatentate
	D3DXMATRIX mmove;
	D3DXMATRIX mrotate; 
	D3DXMATRIX mscale;
	D3DXMATRIX mtemp;

	D3DXMatrixRotationZ(&mrotate,angle + PI);
	
	D3DXMatrixTranslation(&mmove,x,y,z);
	
	D3DXMatrixScaling(&mscale,xscale,yscale,zscale);

	D3DXMatrixMultiply(&mtemp,&mscale,&mrotate);
	
	D3DXMATRIX matWorld;

	D3DXMatrixIdentity(&matWorld);

	D3DXMatrixMultiply(&matWorld,&mtemp,&mmove);

	D3DMATRIX finworld;

	finworld(0,0) = matWorld.m[0][0];
	finworld(0,1) = matWorld.m[0][1];
	finworld(0,2) = matWorld.m[0][2];
	finworld(0,3) = matWorld.m[0][3];

	finworld(1,0) = matWorld.m[1][0];
	finworld(1,1) = matWorld.m[1][1];
	finworld(1,2) = matWorld.m[1][2];
	finworld(1,3) = matWorld.m[1][3];	
	
	finworld(2,0) = matWorld.m[2][0];
	finworld(2,1) = matWorld.m[2][1];
	finworld(2,2) = matWorld.m[2][2];
	finworld(2,3) = matWorld.m[2][3];	
	
	finworld(3,0) = matWorld.m[3][0];
	finworld(3,1) = matWorld.m[3][1];
	finworld(3,2) = matWorld.m[3][2];
	finworld(3,3) = matWorld.m[3][3];
	
	D3DDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &finworld );

	//might have to add something here about setting the current texturing state.
	//texture states should be independent of model


	hr = D3DDevice->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
												D3DFVF_XYZ | D3DFVF_TEX1,
												&LitDataInfo,
												numvertex,
												this->trianglelist,
												numtriangles*3,
												0);
   if(hr != D3D_OK)
	{

	}
												


	return hr;
	
}

HRESULT ZSModelEx::Draw(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, float xscale, float yscale, float zscale, int frame)
{
	//our return value
	HRESULT hr;

	//set up the strided data structure and error check
	//-------------------------------------------------
	
	//if we are not initialized, return a generic error
	if (stridedVertexArray == NULL)
		return DDERR_GENERIC;
	
	//if the uV coords are missing, return a generic error
	if (stridedUV == NULL)
		return DDERR_GENERIC;
	
	if (frame < 0 || frame >= numframes)
	{
		//Engine->ReportError("Mesh frame out of bounds.");
		return DDERR_INVALIDPARAMS;
	}		
	if (stridedVertexArray[frame] == NULL)
		return DDERR_GENERIC;
		
	//set the data
	stridedDataInfo.position.lpvData = &stridedVertexArray[frame][0];


	//set up rotation matrix
	//set up transformation matrix
	//concatentate
	D3DXMATRIX mmove;
	D3DXMATRIX mrotate; 
	D3DXMATRIX mscale;
	D3DXMATRIX mtemp;

	D3DXMatrixRotationZ(&mrotate,angle + PI);
	
	D3DXMatrixTranslation(&mmove,x,y,z);
	
	D3DXMatrixScaling(&mscale,xscale,yscale,zscale);

	D3DXMatrixMultiply(&mtemp,&mscale,&mrotate);
	
	D3DXMATRIX matWorld;

	D3DXMatrixIdentity(&matWorld);

	D3DXMatrixMultiply(&matWorld,&mtemp,&mmove);

	D3DMATRIX finworld;

	finworld(0,0) = matWorld.m[0][0];
	finworld(0,1) = matWorld.m[0][1];
	finworld(0,2) = matWorld.m[0][2];
	finworld(0,3) = matWorld.m[0][3];

	finworld(1,0) = matWorld.m[1][0];
	finworld(1,1) = matWorld.m[1][1];
	finworld(1,2) = matWorld.m[1][2];
	finworld(1,3) = matWorld.m[1][3];	
	
	finworld(2,0) = matWorld.m[2][0];
	finworld(2,1) = matWorld.m[2][1];
	finworld(2,2) = matWorld.m[2][2];
	finworld(2,3) = matWorld.m[2][3];	
	
	finworld(3,0) = matWorld.m[3][0];
	finworld(3,1) = matWorld.m[3][1];
	finworld(3,2) = matWorld.m[3][2];
	finworld(3,3) = matWorld.m[3][3];
	
	D3DDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &finworld );

	//might have to add something here about setting the current texturing state.
	//texture states should be independent of model


	hr = D3DDevice->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
												D3DFVF_VERTEX,
												&this->stridedDataInfo,
												numvertex,
												this->trianglelist,
												numtriangles*3,
												0);
   if(hr != D3D_OK)
	{

	}
												


	return hr;
	
}

D3DVERTEX ZSModelEx::GetPoint(int Point, int Frame)
{

	int pi;
	pi = Point * 3;

	D3DVERTEX	Vert;

	Vert.x = stridedVertexArray[Frame][pi ];
	Vert.y = stridedVertexArray[Frame][pi + 1];
	Vert.z = stridedVertexArray[Frame][pi + 2];
	
	Vert.nx = stridedNormals[pi];
	Vert.ny = stridedNormals[pi + 1];
	Vert.nz = stridedNormals[pi + 2];

	return Vert;
}
	
HRESULT ZSModelEx::Draw(LPDIRECT3DDEVICE7 D3DDevice, int frame)
{
	//our return value
	HRESULT hr;

	//set up the strided data structure and error check
	//-------------------------------------------------
	
	//if we are not initialized, return a generic error
	if (stridedVertexArray == NULL)
		return DDERR_GENERIC;
	
	//if the uV coords are missing, return a generic error
	if (stridedUV == NULL)
		return DDERR_GENERIC;
	
	if (frame < 0 || frame >= numframes)
	{
		//Engine->ReportError("Mesh frame out of bounds.");
		return DDERR_INVALIDPARAMS;
	}		
	if (stridedVertexArray[frame] == NULL)
		return DDERR_GENERIC;
		
	//set the data
	stridedDataInfo.position.lpvData = &stridedVertexArray[frame][0];

	//might have to add something here about setting the current texturing state.
	//texture states should be independent of model


	hr = D3DDevice->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
												D3DFVF_VERTEX,
												&this->stridedDataInfo,
												numvertex,
												this->trianglelist,
												numtriangles*3,
												0);
   if(hr != D3D_OK)
	{

	}
												
	return hr;
	
}

void ZSModelEx::Rotate(float yaw, float pitch, float roll)
{
	D3DXVECTOR3 Center;

	D3DXVECTOR3 InVector;

	D3DXVECTOR4 OutVector;

	D3DXMATRIX matRotate;

	D3DXMatrixRotationYawPitchRoll(&matRotate, yaw, pitch, roll);

	int n;
	int fn;

	ZeroMemory(&Center,sizeof(Center));

	//first get the center point
	for(n = 0; n < numvertex * 3; n+= 3)
	{
		Center.x += stridedVertexArray[0][n];
		Center.y += stridedVertexArray[0][n+1];
		Center.z += stridedVertexArray[0][n+2];
	}
	Center.x = Center.x/(float)numvertex;
	Center.y = Center.z/(float)numvertex;
	Center.z = Center.z/(float)numvertex;


	for(fn = 0; fn < numframes; fn ++)
	{
		for(n = 0; n < numvertex * 3; n+= 3)
		{
			InVector.x = stridedVertexArray[fn][n] - Center.x;
			InVector.y = stridedVertexArray[fn][n+1] - Center.y;
			InVector.z = stridedVertexArray[fn][n+2] - Center.z;
		
			D3DXVec3Transform(&OutVector, &InVector, &matRotate);
		
			stridedVertexArray[fn][n]   = OutVector.x + Center.x;
			stridedVertexArray[fn][n+1] = OutVector.y + Center.y;
			stridedVertexArray[fn][n+2]  = OutVector.z + Center.z;

			if(fn == 0)
			{
				InVector.x = stridedNormals[n];
				InVector.y = stridedNormals[n+1];
				InVector.z = stridedNormals[n+2];
			
				D3DXVec3Transform(&OutVector, &InVector, &matRotate);
			
				stridedNormals[n]   = OutVector.x;
				stridedNormals[n+1] = OutVector.y;
				stridedNormals[n+2]  = OutVector.z;
			}		
		
		}
	}
}

void ZSModelEx::FixNormals()
{
	D3DVECTOR vNormal;
	
	int a;
	int b;
	int c;
	int n;

	for(n = 0; n < numvertex * 3; n+= 3)
	{
		stridedNormals[n ] = -stridedNormals[n];
		stridedNormals[n+1] = -stridedNormals[n+1];
		stridedNormals[n+2] = -stridedNormals[n+2];
	}
	
	for(n = 0; n < numtriangles * 3; n += 3)
	{
		


		a = trianglelist[n * 3];
		b = trianglelist[n * 3 + 1];
		c = trianglelist[n * 3 + 2];

		trianglelist[n * 3] = c;
		trianglelist[n * 3 + 2] = a;
	}

	return;
}

//assumes that the ray has already been transformed into model coordinates
BOOL ZSModelEx::Intersect(int Frame, D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	D3DVERTEX vA,vB,vC;
	int n;
	int TriangleOffset = 0;
	for(n = 0; n < numtriangles; n ++)
	{
		//convert from the strided format to normal vertices
		vA.x = stridedVertexArray[Frame][trianglelist[TriangleOffset]*3];
		vA.y = stridedVertexArray[Frame][trianglelist[TriangleOffset]*3+1];
		vA.z = stridedVertexArray[Frame][trianglelist[TriangleOffset]*3+2];
		
		vB.x = stridedVertexArray[Frame][trianglelist[TriangleOffset + 1]*3];
		vB.y = stridedVertexArray[Frame][trianglelist[TriangleOffset + 1]*3+1];
		vB.z = stridedVertexArray[Frame][trianglelist[TriangleOffset + 1]*3+2];
		
		vC.x = stridedVertexArray[Frame][trianglelist[TriangleOffset + 2]*3];
		vC.y = stridedVertexArray[Frame][trianglelist[TriangleOffset + 2]*3+1];
		vC.z = stridedVertexArray[Frame][trianglelist[TriangleOffset + 2]*3+2];

		//check for intersection
		if(Triangle3DIntersect(vRayStart,vRayEnd,&vA,&vB,&vC))
		{
			return TRUE;
		}
		TriangleOffset += 3;
	}
	return FALSE;
}

//assumes that the line has already been transformed into model coordinates
BOOL ZSModelEx::LineIntersect(int Frame, D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	D3DVERTEX vA,vB,vC;
	int n;
	int TriangleOffset = 0;
	for(n = 0; n < numtriangles; n ++)
	{
		//convert from the strided format to normal vertices
		vA.x = stridedVertexArray[Frame][trianglelist[TriangleOffset]*3];
		vA.y = stridedVertexArray[Frame][trianglelist[TriangleOffset]*3+1];
		vA.z = stridedVertexArray[Frame][trianglelist[TriangleOffset]*3+2];
		
		vB.x = stridedVertexArray[Frame][trianglelist[TriangleOffset + 1]*3];
		vB.y = stridedVertexArray[Frame][trianglelist[TriangleOffset + 1]*3+1];
		vB.z = stridedVertexArray[Frame][trianglelist[TriangleOffset + 1]*3+2];
		
		vC.x = stridedVertexArray[Frame][trianglelist[TriangleOffset + 2]*3];
		vC.y = stridedVertexArray[Frame][trianglelist[TriangleOffset + 2]*3+1];
		vC.z = stridedVertexArray[Frame][trianglelist[TriangleOffset + 2]*3+2];

		//check for intersection
		if(Triangle3DIntersect(vRayStart,vRayEnd,&vA,&vB,&vC))
		{
			return TRUE;
		}
		TriangleOffset += 3;
	}
	return FALSE;
}

void ZSModelEx::Sharpen()
{
	D3DVECTOR vNorm;
	D3DVECTOR vA, vB, vC,vSideOne, vSideTwo;

	int n;

	for(n = 0; n < numtriangles * 3; n += 3)
	{
		vA.x = stridedVertexArray[0][trianglelist[n]*3];
		vA.y = stridedVertexArray[0][trianglelist[n]*3+1];
		vA.z = stridedVertexArray[0][trianglelist[n]*3+2];

		vB.x = stridedVertexArray[0][trianglelist[n+1]*3];
		vB.y = stridedVertexArray[0][trianglelist[n+1]*3+1];
		vB.z = stridedVertexArray[0][trianglelist[n+1]*3+2];

		vC.x = stridedVertexArray[0][trianglelist[n+2]*3];
		vC.y = stridedVertexArray[0][trianglelist[n+2]*3+1];
		vC.z = stridedVertexArray[0][trianglelist[n+2]*3+2];

		vSideOne = Normalize(vB - vA);
		vSideTwo = Normalize(vC - vA);

		vNorm = CrossProduct(vSideOne,vSideTwo);

		stridedNormals[trianglelist[n]*3] = vNorm.x;
		stridedNormals[trianglelist[n]*3+1] = vNorm.y;
		stridedNormals[trianglelist[n]*3+2] = vNorm.z;

		stridedNormals[trianglelist[n+1]*3] = vNorm.x;
		stridedNormals[trianglelist[n+1]*3+1] = vNorm.y;
		stridedNormals[trianglelist[n+1]*3+2] = vNorm.z;

		stridedNormals[trianglelist[n+2]*3] = vNorm.x;
		stridedNormals[trianglelist[n+2]*3+1] = vNorm.y;
		stridedNormals[trianglelist[n+2]*3+2] = vNorm.z;
	}

}

void ZSModelEx::SmoothNormals()
{
	float vx,vy,vz;
	D3DVECTOR vNorm;

	int n,sn,noff,snoff;
	float vertcount;

	noff = 0;
	for(n = 0; n < numvertex; n++)
	{
		vx = stridedVertexArray[0][noff];
		vy = stridedVertexArray[0][noff+1];
		vz = stridedVertexArray[0][noff+2];
		vNorm = _D3DVECTOR(0.0f,0.0f,0.0f);
		snoff = 0;
		vertcount = 0.0f;
		for(sn = 0; sn < numvertex;	sn++)	
		{
			if(vx == stridedVertexArray[0][snoff] &&
				vy == stridedVertexArray[0][snoff+1] &&
				vz == stridedVertexArray[0][snoff+2])
			{
				vNorm.x += stridedNormals[snoff];
				vNorm.y += stridedNormals[snoff+1];
				vNorm.z += stridedNormals[snoff+2];
				vertcount += 1.0f;
			}
			snoff +=3;	
		}
		
		vNorm.x /= vertcount;
		vNorm.y /= vertcount;
		vNorm.z /= vertcount;
		vNorm = Normalize(vNorm);
		
		stridedNormals[noff] = vNorm.x;
		stridedNormals[noff+1] = vNorm.y;
		stridedNormals[noff+2] = vNorm.z;
		noff+=3;
	}
}

//assumes normals are set up properly.

float ZSModelEx::GetZ(float x, float y)
{
	float zVal = 0.0f;
	float xn;
	float yn;
	float foffset;
	float fleft, fright, ftop, fbottom;
	int xh;
	int yh;
	RECT rTriangle;
	foffset = 1.0f / 1600.0f;

	D3DVECTOR vPoint;
	vPoint = _D3DVECTOR(x,y,0.0f);

	D3DVERTEX vxA,vxB,vxC;
	int n;
	for(xh  = 0; xh < 1600; xh++)
	for(yh = 0; yh < 1600; yh++)
	{
		HeightLevels[xh][yh] = 0.0f;
	}
		
	int TriangleOffset = 0;
	float a,b,c,d;
	D3DVECTOR vA, vB, vC, vNorm, vSideOne, vSideTwo;
	
	for(n = 0; n < numtriangles; n++)
	{
		//convert from the strided format to normal vertices
		vA.x = vxA.x = stridedVertexArray[0][trianglelist[TriangleOffset]*3];
		vA.y = vxA.y = stridedVertexArray[0][trianglelist[TriangleOffset]*3+1];
		vA.z = vxA.z = stridedVertexArray[0][trianglelist[TriangleOffset]*3+2];
		
		vB.x = vxB.x = stridedVertexArray[0][trianglelist[TriangleOffset + 1]*3];
		vB.y = vxB.y = stridedVertexArray[0][trianglelist[TriangleOffset + 1]*3+1];
		vB.z = vxB.z = stridedVertexArray[0][trianglelist[TriangleOffset + 1]*3+2];
		
		vC.x = vxC.x = stridedVertexArray[0][trianglelist[TriangleOffset + 2]*3];
		vC.y = vxC.y = stridedVertexArray[0][trianglelist[TriangleOffset + 2]*3+1];
		vC.z = vxC.z = stridedVertexArray[0][trianglelist[TriangleOffset + 2]*3+2];

		vSideOne = Normalize(vB - vA);
		vSideTwo = Normalize(vC - vA);

		vNorm = CrossProduct(vSideOne,vSideTwo);

		a = vNorm.x;
		b = vNorm.y;
		c = vNorm.z;

		fleft = fright = vxA.x;
		if(fleft > vxB.x) fleft = vxB.x;
		if(fleft > vxC.x) fleft = vxC.x;
		if(fright < vxB.x) fright = vxB.x;
		if(fright < vxC.x) fright = vxC.x;

		ftop = fbottom = vxA.y;
		if(ftop > vxB.y) ftop = vxB.y;
		if(ftop > vxC.y) ftop = vxC.y;
		if(fbottom < vxB.y) fbottom = vxB.y;
		if(fbottom < vxC.y) fbottom = vxC.y;

		rTriangle.left   = (int)(fleft / foffset);
		rTriangle.right  = (int)(fright / foffset);
		rTriangle.top	 = (int)(ftop / foffset);
		rTriangle.bottom = (int)(fbottom / foffset);

		for(yh = rTriangle.top; yh <= rTriangle.bottom; yh++)
		for(xh = rTriangle.left; xh <= rTriangle.right; xh++)
		{
			xn = (float)xh * foffset;
			yn = (float)yh * foffset;

		//	vPoint.x = xn;
		//	vPoint.y = yn;
		
			//if(Triangle2DIntersect(&vPoint,&vxA,&vxB,&vxC))
			//{
			d = -1 * (a * vA.x +
					 b * vA.y +
					 c * vA.z);

			zVal = -1 * (a * xn + b * yn + d)/c;
			HeightLevels[xh][yh] = zVal * 256.0f;
			//}
		}
		TriangleOffset += 3;
	}
	
/*	
	
	for(n = 0; n < numtriangles; n ++)
	{
		//convert from the strided format to normal vertices
		vxA.x = stridedVertexArray[0][trianglelist[TriangleOffset]*3];
		vxA.y = stridedVertexArray[0][trianglelist[TriangleOffset]*3+1];
		
		vxB.x = stridedVertexArray[0][trianglelist[TriangleOffset + 1]*3];
		vxB.y = stridedVertexArray[0][trianglelist[TriangleOffset + 1]*3+1];
		
		vxC.x = stridedVertexArray[0][trianglelist[TriangleOffset + 2]*3];
		vxC.y = stridedVertexArray[0][trianglelist[TriangleOffset + 2]*3+1];
		
		//check for intersection
		if(Triangle2DIntersect(&vPoint,&vxA,&vxB,&vxC))
		{
			D3DVECTOR vA;
		//	D3DVECTOR vB;
		//	D3DVECTOR vC;

			vA.x = vxA.x;
			vA.y = vxA.y;
		
		//	vB.x = vxB.x;
		//	vB.y = vxB.y;

		//	vC.x = vxC.x;
		//	vC.y = vxC.y;

			vA.z = stridedVertexArray[0][trianglelist[TriangleOffset]*3+2];
		//	vB.z = stridedVertexArray[0][trianglelist[TriangleOffset + 1]*3+2];
		//	vC.z = stridedVertexArray[0][trianglelist[TriangleOffset + 2]*3+2];

			float a,b,c,d,zn;

			a = stridedNormals[trianglelist[TriangleOffset]*3];
			b = stridedNormals[trianglelist[TriangleOffset]*3+1];
			c = stridedNormals[trianglelist[TriangleOffset]*3+2];
		
		//	a = vNorm.x;
		//	b = vNorm.y;
		//	c = vNorm.z;
			d = -1 * (a * vA.x +
						 b * vA.y +
						 c * vA.z);

			zVal = -1 * (a * x + b * y + d)/c;
		
			return zVal;
		}
		TriangleOffset += 3;
	}

//	DEBUG_INFO("Failed to get z from mesh\n");
*/
	return 0.0f;
}

