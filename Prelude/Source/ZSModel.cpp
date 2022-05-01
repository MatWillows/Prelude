

//Revision 3: Altered the Import2 routine to do vertex normal smoothing.
//Revision 4: Altered the smoothing and process face functions to better handle cases of 
//			   VID's with multiple UVID mappings.
//Revision 5: Began the switch to strided vertex format.
//				Previous vertex format still included!!!!!!!!
//Revision 6: Implemented Load and Save functions
//Revision 7: Fixed Destructor, Cleanup of internal functions. Extend Output to take care
//				of equipment info.
//Revision 8: Excised non strided vertex code. Excised old import routine.
//Revision 9: Switched to single frame normals


#ifndef D3D_OVERLOADS
#define D3D_OVERLOADS
#endif


#include "ZSModel.h"
#include "zsutilities.h" //for input helpers
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <fstream>
#include "zsengine.h"

//#define TLISTPRINT

//***************STRUCTS*****************************************************
//**
//These structures aid in the importing of data from truspace file format.
//They are intended to improve the readability and maintainability of the code.

struct VIDLookupEntry
{
	float	x;
	float	y;
	float	z;
};


struct UVIDLookupEntry
{
	float u;
	float v;
};


class Normal
{
public:
	float nx;
	float ny;
	float nz;

	Normal() { nx = 0; ny = 0; nz = 0;};
	bool operator==(const Normal& otherNormal) { return (nx == otherNormal.nx && ny == otherNormal.ny && nz == otherNormal.nz) ? true : false; };
	Normal& operator=(const Normal& otherNormal) { nx = otherNormal.nx; ny = otherNormal.ny; nz = otherNormal.nz; return *this;};
	void Normalize();
};

//this helper function normalizes the vector. 
//There are routines in the d3d helper stuff to do this, but for portability's sakes....
void Normal::Normalize()
{
	//first get the length:
	float length = sqrt( nx*nx + ny*ny + nz*nz);
	
	//avoid div zero errors
	if (length != 0.0f)
	{
		nx /= length;
		ny /= length;
		nz /= length;
	}
}


struct VertexListEntry
{
  
	int		Vid;
	int		UVid;
	Normal	normal;
	vector<int> facelist;

	VertexListEntry();
	void output(FILE* fp, int entry);
};

struct FaceListEntry
{
	int		Vid2;
	int		UVid2;
	int		Vid1;
	int		UVid1;
	int		Vid0;
	int		UVid0;	
	Normal  faceNormal;

	void output(FILE* fp, int entry);
};

void FaceListEntry::output(FILE* fp, int entry)
{
	fprintf(fp, "Entry: %d\nVid2:%4d\tUVid2:%4d\nVid1:%4d\tUVid1:%4d\nVid0:%4d\tUVid0:%4d\n", 
		                                                             entry, Vid2, UVid2,
																			Vid1, UVid1,
																			Vid0, UVid0);
	fprintf(fp, "faceNormal: Nx: %f  Ny %f  Nz: %f\n\n", faceNormal.nx, faceNormal.ny, faceNormal.nz);

}


//the following function just allows us to set default values in the array that we
// use for processing vertices. A flag of -1 on a UVid means that it hasn't been set,
// since all UVid's are positive integers.
VertexListEntry::VertexListEntry()
{
	Vid = -1;
	UVid = -1;

	//the following aren't necessary. but we include them as a reminder
	normal.nx = 0;
	normal.ny = 0;
	normal.nz = 0;
}


void VertexListEntry::output(FILE* fp, int entry)
{
	fprintf(fp, "VertexListEntry: %d\nVid:%4d\nUVid:%4d\nNormal: Nx:%f\tNy:%f\tNz:%f\n",
					entry, Vid, UVid, normal.nx, normal.ny, normal.nz);


	//*****print the face list*********************
	fprintf(fp, "Face list: \n    ");

	vector<int>::const_iterator pos;
	for (pos = facelist.begin() ; pos != facelist.end() ; ++pos)
	{
		fprintf(fp, "%d ", *pos);
	}

	fprintf(fp, "\n\n");
}

//**
//***************************************************************************


//********************FUNCTION DELCARATIONS**********************************
//void SeekTo(FILE *fp, const char *id);
//float GetFloat(FILE *fp);
//int	GetInt(FILE *fp);

void GetVertexIDList(vector<VIDLookupEntry> &VIDLookupTable, int numVertices, FILE* fp);

void GetUVIDList(vector<UVIDLookupEntry> &UVIDLookupTable, int numUVCoords, FILE* fp);

void GetFaceList(vector<FaceListEntry> &faceList, int numFaces, int numVertices, FILE* fp);

void InitializeVertexList(vector<VertexListEntry> &vertexArray, int numVertices);

int ProcessFaceVertex(int VID, int UVID, const Normal &faceNormal, int &currentVertexCount, int faceNumber,
					   int numVertices, vector<VertexListEntry> &vertexArray);

bool SmoothVertexNormals(vector<VertexListEntry> *vertexArray, int currentVertexCount, 
                         int numVertices, vector<FaceListEntry> * faceList);

//*******************************************************************************


HRESULT ZSModel::Draw(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, float scale, int frame)
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
		//Engine->ReportError("Frame out of bounds");
		return DDERR_INVALIDPARAMS;
	}
	
	if (stridedVertexArray[frame] == NULL)
		return DDERR_GENERIC;
		
	//set the data position info
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
	
	D3DXMatrixScaling(&mscale,scale,scale,scale);

	D3DXMatrixMultiply(&mtemp,&mscale,&mrotate);
	
	D3DXMATRIX matWorld;

	D3DXMatrixIdentity(&matWorld);

	D3DXMatrixMultiply(&matWorld,&mtemp,&mmove);

	D3DDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX *)&matWorld );

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




void ZSModel::Output(const char *filename)
{
	FILE *fp;
	fp = SafeFileOpen(filename,"wt");

	if(!fp) return;

	int n, m, o;
	fprintf(fp, "Number of frames: %i\n", this->numframes);
	fprintf(fp, "numvertex: %i\n",numvertex);
	fprintf(fp, "numtriangles: %i\n",numtriangles);
	
	//output info about equipment
	if (equipmentlist == NULL)
		fprintf(fp, "Equipment information not present\n");
	else
		fprintf(fp,"Equipment info should be present: %d slots are supported\n", numWeaponSlotsDefined);
	
	//FOR FUTURE..........
	//OUTPUT the equipment list here as well.
	//I leave it out right now because we don't have a good way of getting info in there.
	
	
	//calculate size
	int vertexsize = sizeof(float) * 8;	//2 from the uv, 6 from the vertex and normal
	int listsize = sizeof(unsigned short);
	int totalsize = vertexsize * numvertex *numframes + numtriangles*3*listsize;
	fprintf(fp,"size in bytes:  %i    in k:  %i\n\n",totalsize,totalsize/1024);
	
	
	
	//output triangle list
	for(n = 0; n < numtriangles; n++)
	{
		fprintf(fp, "%i:  %i -- %i -- %i\n",n,trianglelist[n*3],trianglelist[n*3+1],trianglelist[n*3+2]);
	} 


	
	//do the vertices
	for ( o = 0 ; o < numframes ; ++o)
	{
	
		fprintf(fp, "\n******Frame %d *******\n\n", o);
		
		for(n = 0, m = 0; n < numvertex * 3; n += 3, m += 2)
		{
			fprintf(fp, "%i:  location(%f, %f, %f)  uv(%f, %f)   normal(%f,%f,%f)\n",
						n / 3,
						stridedVertexArray[o][n],
						stridedVertexArray[o][n + 1],
						stridedVertexArray[o][n + 2],
						stridedUV[m],
						stridedUV[m + 1],
						stridedNormals[n],
						stridedNormals[n + 1],
						stridedNormals[n + 2]);
		}
	
	}
	fclose(fp);
}

bool ZSModel::Load(FILE *fp)
{
	this->Clear();
	
	//--------------------------------
	//confirm file extant
	assert(fp);
	
	//--------------------------------

	//load the headers into local structs
	ZSModelFileHeader fh;
	ZSModelFileInfoHeader fih;
	
	fread(&fh, sizeof(ZSModelFileHeader), 1, fp);
	fread(&fih, sizeof(ZSModelFileInfoHeader), 1, fp);

	//read the file identifier
	if (fh.fileIdentifier != ZSMFILEIDENTIFIER)
		return 0;

	//copy the information from the headers into the object
	this->numframes = fh.frameCount; 
	this->numvertex = fh.vertexCount;
	this->numtriangles = fh.faceCount;
	this->numWeaponSlotsDefined = fh.supportedEquipmentCount;
	
	//now we just have to read in the data.
	//start with the overlay--------------------
	if (fh.overlayEnabled == 1)
	{
		//initialize a new registration mark and read the data from the file into it.
		memcpy(&equipmentRegistrationMark, &fh.overlayLocator, sizeof(EquipmentLocator));
	}
	//else we don't do anything
	
	
	
	//---------equipment stuff-----------------
	if (fh.equipmentEnabled == 1)
	{
		this->equipmentlist = new EquipmentLocator[10];
		fread(this->equipmentlist,sizeof(EquipmentLocator),10,fp);
	}

	//now handle the triangle list--------------
	this->trianglelist = new unsigned short [fh.faceCount * 3];

		//error check for memory allocation
		if (this->trianglelist == NULL)
			return 0;	
	//read the data in
	fread(&trianglelist[0], sizeof(char), fih.faceArraySize, fp);
	
	//now the uv---------------------------------
	this->stridedUV = new float[fh.vertexCount * 2];
		//error check for memory allocation
		if (this->stridedUV == NULL)
			return 0;		
	//read the data in
	fread(&stridedUV[0], sizeof(char), fih.uvArraySize, fp);
	
	//now the normals-------------------------------
	this->stridedNormals = new float[fh.vertexCount * 3];
		//error check for memory allocation
		if (this->stridedNormals == NULL)
			return 0;		
	//read the data in
	fread(&stridedNormals[0], sizeof(char), fih.vertexArraySize, fp);

	//now the vertices---------------------------
	this->stridedVertexArray = new float*[fh.frameCount];
		//error check for memory allocation
		if (this->stridedVertexArray == NULL)
			return 0;
	//read the data in
	for (int i = 0 ; i < fh.frameCount ; i++)
	{
		this->stridedVertexArray[i] = new float[fh.vertexCount * 3];
			//we won't error check here just yet.....
			//i'd like to know if this is blowing... (could also use an assertion).
		
		fread(&stridedVertexArray[i][0], sizeof(char), fih.vertexArraySize, fp);
			
	}
		
	stridedDataInfo.normal.lpvData = stridedNormals;
	stridedDataInfo.textureCoords[0].lpvData = stridedUV;

	//after this step we should be done.
	return 1;

}



bool ZSModel::Load(const char *filename)
{

	this->Clear();
	
	//--------------------------------
	//try to open file for binary read
	FILE* fp = NULL;
	
	fp = SafeFileOpen(filename, "rb");

	//check to see if file opened
	if (fp == NULL)
	{
		return 0; 
	}
	
	//--------------------------------
	//load model using overloaded function
	Load(fp);

	fclose(fp);
	return 1;

}

//************SAVE*************************************88
bool ZSModel::Save(FILE *fp)
{
	//--------------------------------
	//confirm file extant
	assert(fp);
	
	//--------------------------------
	//if this object has no data, return an error.
	if (stridedVertexArray == 0)
		return 0;
	
	ZSModelFileHeader 		fh;
	ZSModelFileInfoHeader 	fih;
	
	
	//put the object's info into the file header.
	fh.fileIdentifier 	= ZSMFILEIDENTIFIER;
	fh.frameCount 		= this->numframes;
	fh.vertexCount 		= this->numvertex;
	fh.faceCount 		= this->numtriangles;
	if(equipmentlist != NULL)
		fh.equipmentEnabled = 1;
	else
		fh.equipmentEnabled = 0;
	fh.supportedEquipmentCount = this->numWeaponSlotsDefined;

	//check if the overlay is enabled, and if so, copy the data and set the flag:
	fh.overlayEnabled 	= 1;
	memcpy(&fh.overlayLocator, &(equipmentRegistrationMark), sizeof(EquipmentLocator));
	//if not we just set the flag to false and we don't care about the data inside the locator
	
	
	//calculate the sizes of the local data structures
	fih.equipmentArraySize 	= sizeof(EquipmentLocator) * 10;
	fih.uvArraySize 		= 2 * this->numvertex * sizeof(float);
	fih.faceArraySize 	= 3 * this->numtriangles * sizeof(unsigned short);
	fih.vertexArraySize 	= 3 * this->numvertex * sizeof(float);
	
	//using this size information, calculate the offsets from the end of the zsfileinfoheader
	// to the relevant data....
	if (fh.equipmentEnabled == 0 )
		fih.faceOffset = 0;
	else if (fh.equipmentEnabled == 1)
		fih.faceOffset = ( this->numframes * 10 )  *  sizeof(EquipmentLocator);
		
	fih.uvOffset = fih.faceOffset + fih.faceArraySize;
	fih.vertexOffset = fih.uvOffset + fih.uvArraySize;
	
	
	//now write these headers to file.
	fwrite(&fh, sizeof(ZSModelFileHeader), 1, fp);
	fwrite(&fih, sizeof(ZSModelFileInfoHeader), 1, fp);
	
	
	//Write the equipment list to file
	if (fh.equipmentEnabled == 1)
	{
		//we error check again here just to be sure.
		if (this->equipmentlist != NULL)
		{
			fwrite(equipmentlist, sizeof(EquipmentLocator), 10, fp);
		}
		//	else we return false because we want to be alerted to this failure.
		//means a class invariant has been violated.
		else return 0;
	}
	
	//write the facelist to file
	fwrite(&trianglelist[0], sizeof(char), fih.faceArraySize, fp);
	
	//write the UV list to file
	fwrite(&stridedUV[0], sizeof(char), fih.uvArraySize, fp);

	fwrite(&stridedNormals[0],sizeof(char), fih.vertexArraySize, fp);
	
	//finally write the vertex information to file
	
	for (int j = 0 ; j < numframes ; j++)
	{
		if (stridedVertexArray[j] != NULL)
		{
			fwrite(&stridedVertexArray[j][0], sizeof(char), fih.vertexArraySize, fp);	
		}
		else return 0;
	}
	
	
	return 1;
}



bool ZSModel::Save(const char *filename)
{
	//--------------------------------
	//try to open file for binary write
	FILE* fp = NULL;
	
	fp = SafeFileOpen(filename, "wb");

	//check to see if file opened
	if (fp == NULL)
	{
		return 0; 
	}

	//--------------------------------
	//Save data using overloaded function
	Save(fp);

	fclose(fp);
	return 1;
}









void ZSModel::Clear()
{
	//some of the safe delete checks might be redundant in C++;
	//However, for debugger visibility and for porting to C we wish to keep them
	// in for now.

	//delete the equipment rays
	if (equipmentlist != NULL)
	{
		//delete[]  equipmentlist;
	}


	if (trianglelist)
		delete[] trianglelist;


	if (stridedVertexArray)
	{
		for (int i = 0 ; i < numframes ; i++)
		{
			if (stridedVertexArray[i])
				delete [] stridedVertexArray[i];
		}
		
			delete[] stridedVertexArray;
	}	
	
	
	if (stridedUV)
		delete[] stridedUV;

	if (stridedNormals)
		delete[] stridedNormals;



	//set all members to their nominal values.
	numtriangles = 0;
	numvertex = 0;
	numframes = 0;
	numWeaponSlotsDefined = -1;

	trianglelist = NULL;
	equipmentlist = NULL;
	//ZeroMemory(&equipmentRegistrationMark,sizeof(equipmentRegistrationMark));
	
	stridedVertexArray = NULL;
	stridedUV = NULL;
	stridedNormals = NULL;

}





//-----------CONSTRUCTOR---------------------//
ZSModel::ZSModel()
{
	numtriangles = 0;
	numvertex = 0;
	numframes = 0;

	trianglelist = NULL;

	equipmentlist = NULL;
	ZeroMemory(&equipmentRegistrationMark, sizeof(equipmentRegistrationMark));
	numWeaponSlotsDefined = -1;

	stridedVertexArray = NULL;
	stridedUV		= NULL;
	stridedNormals = NULL;

	ZeroMemory(&stridedDataInfo, sizeof(D3DDRAWPRIMITIVESTRIDEDDATA));

	//set the strides.  
	stridedDataInfo.position.dwStride = sizeof(float)*3;
	stridedDataInfo.normal.dwStride = sizeof(float)*3;
	stridedDataInfo.textureCoords[0].dwStride = sizeof(float)*2;
}


ZSModel::~ZSModel()
{
	Clear();
}

bool 
ZSModel::DrawAsEquipment(LPDIRECT3DDEVICE7 D3DDevice, 
						 float scale, 
						 ZSModel* target, 
					 	float rotation, 
					 	int frame, 
					 	int targetFrame, 
					 	EQUIP_POSITION equip_position,
					 	float offset,
						D3DVECTOR *WorldPosition,
						float targetAngle)
{

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

	HRESULT hr;

	D3DVALUE  theta;
	
	
	//set up the strided data structure and error check
	//-------------------------------------------------
	//error check
/*	if (target->equipmentlist == NULL)
		return 0;			
		
	//if we are not initialized, return a generic error
	if (stridedVertexArray == NULL)
		return DDERR_GENERIC;
	
	//if the uV coords are missing, return a generic error
	if (stridedUV == NULL)
		return DDERR_GENERIC;
	
	if (frame < 0 || frame >= numframes)
	{
		Engine->ReportError("Mesh frame out of bounds.");
		return DDERR_INVALIDPARAMS;
	}	
	
	if (targetFrame < 0 || targetFrame >= target->numframes)
	{
		Engine->ReportError("Target Mesh frame out of bounds.");
		return DDERR_INVALIDPARAMS;
	}	

	if (stridedVertexArray[frame] == NULL)
		return DDERR_GENERIC;
*/		

	//set the data
	//only nead to set position data each draw
	stridedDataInfo.position.lpvData = &stridedVertexArray[frame][0];
	//---------------------------------------------------

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
	int MyLinkArrayOffset = this->equipmentRegistrationMark.LinkIndex * 3;

	LinkPoint.x = this->stridedVertexArray[frame][MyLinkArrayOffset];
	LinkPoint.y = this->stridedVertexArray[frame][MyLinkArrayOffset + 1];
	LinkPoint.z = this->stridedVertexArray[frame][MyLinkArrayOffset + 2];
		
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

	//objectNormal = this->equipmentRegistrationMark[equip_position];
	objectNormal = D3DVECTOR(0.0f,1.0f,0.0f);
	
		//get the axis of rotation 
	
		temp = CrossProduct(objectNormal, targetNormal);

		//get the angle of rotation
		//magnitude can be eliminated if both are normalized vectors

		theta = acos(DotProduct(objectNormal, targetNormal));

		//this rotates the main axis of the object (ie: weapon) to the proper direction 
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

	//now calculate the offset:
	//this should allow us to slide up and down the line which the 
	// target's normal spans.
/*		if (offset != 0.0)
		{

			//first grab the normal from the proper frame of the model
			//targetNormal = target->equipmentlist[targetFrame ][equip_position].normal;
			
			//calculate the proper vector for the projection
			temp = Normalize(targetNormal) * offset;

			//now calculate projections onto the orthogonal basis
			D3DVECTOR x(1.0, 0.0, 0.0);
			D3DVECTOR y(0.0, 1.0, 0.0);
			D3DVECTOR z(0.0, 0.0, 1.0);

			temp = (DotProduct(temp, x) * x);
				offsetx = temp.x;

			temp = DotProduct(temp, y) * y;
				offsety = temp.y;

			temp = DotProduct(temp, z) * z;
				offsetz = temp.z;

			//put them into the translation matrix		
			mmove.m30 += offsetx;
			mmove.m31 += offsety;
			mmove.m32 += offsetz;
		
			//done setting up the offset
		}
*/
	
	//do the final multiplication
	D3DXMatrixMultiply(&matWorld, &matWorld, &mmove);

	//the rest is basically the same as draw();
		// we now have the place we have to move to, the proper rotation to align to the 
		//  target model's normal, the offset amounts, and the scale factor.

	D3DDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX *)&matWorld );

	//might have to add something here about setting the current texturing state.
	//texture is independent of mesh, so draws with a single texture can occur with multiple meshes

	hr = D3DDevice->DrawIndexedPrimitiveStrided(D3DPT_TRIANGLELIST, 
												D3DFVF_VERTEX,
												&this->stridedDataInfo,
												this->numvertex,
												this->trianglelist,
												this->numtriangles * 3,
												0);
												

	return FALSE;
}




//*********************************************************************
//
//
//  The New Import Routine
//

//this routine makes heavy use of the STL and was engineered for maintainability.
// and readability.
//Subroutine perform vertex smoothing, loading in the UV list, the V list, the face list,
//There is a subroutine to process the face list and to process a single entry.


bool 
ZSModel::Import(const char *filename)
{

	//********************************************
	//         our local data structures.
	vector<VIDLookupEntry>*		VIDLookupTable = NULL;
	vector<UVIDLookupEntry>*	UVIDLookupTable = NULL;
	vector<unsigned short>*		triangleList = NULL;
	vector<VertexListEntry>*	vertexArray = NULL;
	vector<FaceListEntry>*		faceList	= NULL;



	//indexes into our data structures
	int numVertices = 0;
	int numUVCoords = 0;
	int numFaces = 0;
	int currentVertexCount = 0;
	int triangleListSize = 0;

	//other counters
	int numFrames = 0;
	//**
	//*********************************************

	//if we already have something loaded, we should clear out our data:
	//
	if (this->stridedVertexArray != NULL)
	{
		this->Clear();
	}
	//----------------------------
	
	//try to open the file for ascii read
	FILE* fp = NULL;
	
	fp = SafeFileOpen(filename, "r");
	
	if (fp == NULL)
		return 0;
	//---------------------------
	

	//read in the number of frames.
	SeekTo(fp, "NumFrames:");
	numFrames = GetInt(fp);

	if(!numFrames)
	{
		DEBUG_INFO("attemped to import file with 0 frames...\n");
		DEBUG_INFO(filename);
		DEBUG_INFO("\n");
		numFrames = 1;
	}


	//allocate class storage---------------------
	this->stridedVertexArray = new float*[numFrames];
	//-------------------------------------------
	
	
	//------------------------------------------------------
	//*******Process each Frame*******************************************
	for (int currentFrame = 0 ;  currentFrame < numFrames ; currentFrame++)
	{

		//Handle VertexID List:

			//read in the number of vertices
			SeekTo(fp, "Vertices:");
			numVertices = GetInt(fp);

			//allocate local storage--------------------------------------
			VIDLookupTable = new vector<VIDLookupEntry> (numVertices);
				//error check
				if (VIDLookupTable == NULL) return 0;
			//------------------------------------------------------	

			//read in the Vertex info
			GetVertexIDList(*VIDLookupTable, numVertices, fp);

		//Handle UVID List:

			//Read in number of UV entries

			SeekTo(fp, "NumUV:");
			numUVCoords = GetInt(fp);

			//allocate local storage--------------------------------------
			UVIDLookupTable = new vector<UVIDLookupEntry> (numUVCoords);
			//error check
			if (UVIDLookupTable == NULL) return 0;
			

			//------------------------------------------------------
			
			GetUVIDList(*UVIDLookupTable, numUVCoords, fp);
	
			//Handle Face List:

			//read in the number of faces

			SeekTo(fp, "Faces:");
			numFaces = GetInt(fp);

			//allocate storage

			faceList = new vector<FaceListEntry> (numFaces);
				//error check
				if (faceList == NULL) return 0;

			GetFaceList(*faceList, numFaces, numVertices, fp);

		//*****************************
		//Now start processing the data.

		//allocate a data structure to hold all the info. This will expand automatically if we need more
		// space. We're not real time critical so a bit of expansion doesn't really matter.
		vertexArray = new vector<VertexListEntry> ( numVertices);
		
		//error check
			if (vertexArray == NULL) return 0;

		//allocate a triangle list
		triangleList = new vector<unsigned short> (3 * numFaces);
			if (triangleList == NULL) return 0;

		//set the current number of vertices. (This will change over time, so store in separate var)
			currentVertexCount = numVertices;


		//set the VID values in the new array. It is also a direct Lookup Table.
		InitializeVertexList(*vertexArray, numVertices);


		//at this point we have all the data that we need. We can start processing the FaceList.
		//the face list is of fixed size (it will never change) so we can use iterators.

		vector<FaceListEntry>::const_iterator faceListPos;
		
		vector<unsigned short>::iterator triangleListPos = triangleList->begin();


		//---------Process the face list----------------------------------------------------
		int faceNumber = 0;
		for (faceListPos = faceList->begin() ; faceListPos < faceList->end() ; ++faceListPos, ++faceNumber )
		{
			*triangleListPos = 
			      ProcessFaceVertex( (*faceListPos).Vid2, 
				                     (*faceListPos).UVid2, 
				                     (*faceListPos).faceNormal,  
							         numVertices, currentVertexCount, faceNumber,
							         *vertexArray);

			++triangleListPos;

			*triangleListPos = 
			      ProcessFaceVertex( (*faceListPos).Vid1, 
				                     (*faceListPos).UVid1, 
				                     (*faceListPos).faceNormal,  
							         numVertices, currentVertexCount, faceNumber,
							         *vertexArray);

			++triangleListPos;

			*triangleListPos = 
			      ProcessFaceVertex( (*faceListPos).Vid0, 
				                     (*faceListPos).UVid0, 
				                     (*faceListPos).faceNormal,  
							         numVertices, currentVertexCount, faceNumber,
							         *vertexArray);

			++triangleListPos;

		}//end process frame specific face data.

		
		
	
	//we set all our relevant info after parsing the first frame.
	if (currentFrame == 0)
	{
		
		//************SMOOTH*****************************
		//smooth the vertex normals that we receive since we are converting from facenormals
		// to vertex normals.
		// only for frame one.  All Frames will use same normal list
		// definitely not optimal visual quality, but almost halves size and load time
		//SmoothVertexNormals(vertexArray, currentVertexCount, numVertices,faceList);
		
		this->numframes = numFrames;
		this->numtriangles = numFaces;
		this->numvertex = numVertices;

		//set the triangle list. We assume that subsequent frames use the same list.
		// (Might want to change this in future).
		this->trianglelist = new unsigned short[3 * numFaces];
		int dest = 0;
		for (triangleListPos = triangleList->begin() ; triangleListPos != triangleList->end() ; ++ triangleListPos)
		{
			trianglelist[dest] = *triangleListPos;
			++dest;
		}
	}

	//---------copy data into the private members.
	//****************************************************************************************

	//DEBUG: if this fails we are allocating more/less memory than necessary in the 
	//         ZSModel to hold the vertex data
	assert(vertexArray->size() == numVertices);
	
	
	//copy in the vertex list
	//allocate class memory

	if (currentFrame == 0)
	{
		//allocate uv memory.
		stridedUV = new float[2 * numVertices];
		stridedNormals = new float[3 * numVertices];
	}
	
	//initialize this part of the stridedvertexarray.
	
		//we keep it really verbose for debugging purposes.
		// An optimizing compiler will see through this anyhow.
	
	int vertexArraySize = vertexArray->size() * 3;
	this->stridedVertexArray[currentFrame] = new float[vertexArraySize];
		
	//a few helper variables
	vector<VertexListEntry>::iterator vertexPos;
	int stridedVertexPosition = 0;
	int stridedUVPosition = 0;
	int stridedNormalsPosition = 0;
	
	for ( vertexPos = vertexArray->begin(); vertexPos != vertexArray->end() ; ++vertexPos)
	{
		VIDLookupEntry  vle;
		UVIDLookupEntry uvle;
		VertexListEntry sourceEntry;
	
		//retrieve the info from the vertex array, 
		//  We do this explicitly largely for debugging convenience
		 sourceEntry = *vertexPos;
		
			//if the UVID entry in the vertex that we are looking at is -1, or the facelist is empty,
			// we can set to a dummy value VID0, UVID0/
		
			if (sourceEntry.UVid == -1 || sourceEntry.facelist.size() == 0)
			{
				sourceEntry.UVid = 0;
				sourceEntry.Vid = 0;
			}	
		
		 vle = VIDLookupTable->at(sourceEntry.Vid);
		 uvle = UVIDLookupTable->at(sourceEntry.UVid);
		
		//copy the x, y , z's in
		stridedVertexArray[currentFrame][stridedVertexPosition] = vle.x;
		++stridedVertexPosition;
		stridedVertexArray[currentFrame][stridedVertexPosition] = vle.y;
		++stridedVertexPosition;
		stridedVertexArray[currentFrame][stridedVertexPosition] = vle.z;
		++stridedVertexPosition;
		
		//handle the UV stuff here on first frame only.
		//We only need one UV list in one pass strided format.
		if (currentFrame == 0)
		{
			stridedUV[stridedUVPosition] = uvle.u;
			stridedUVPosition++;
			stridedUV[stridedUVPosition] = uvle.v;
			stridedUVPosition++;

			//copy the normal stuff in
			stridedNormals[stridedNormalsPosition] = sourceEntry.normal.nx;
			++stridedNormalsPosition;
			stridedNormals[stridedNormalsPosition] = sourceEntry.normal.ny;
			++stridedNormalsPosition;
			stridedNormals[stridedNormalsPosition] = sourceEntry.normal.nz;
			++stridedNormalsPosition;
			
		}//end uv entry
	
	}//end for
	
	//--------------------------------------------------------------------------

#ifdef PRINT_ZSMODEL_DEBUG
	if (currentFrame == 0)
	{
	//****DEBUG********************
		vector<FaceListEntry>::iterator faceListEnd;
		vector<FaceListEntry>::iterator faceListStart;

	FILE* fp = NULL;
	fp = SafeFileOpen ("FaceListOutput.txt", "w");
	if (!fp)
		return 0;


	faceListEnd = faceList->end();
	int i = 0;
	for (faceListStart = faceList->begin() ; faceListStart != faceListEnd ; ++faceListStart)
	{
		(*faceListStart).output(fp, i);//faceList->at(i).output(fp, i);
		i++;
	}
	fclose(fp);
	//******************************
	vector<VertexListEntry>::const_iterator vertexListStart;
	
	fp = NULL;
	fp = SafeFileOpen ("VertexListOutput.txt", "w");
	if (!fp)
		return 0;
	i = 0;
	for (vertexListStart = vertexArray->begin() ; vertexListStart != vertexArray->end() ; ++vertexListStart)
	{
		vertexArray->at(i).output(fp, i);
		i++;
	}
	fclose(fp);
	
	//******************************
	vector<unsigned short>::iterator triangleListStart;
	fp = NULL;
	fp = SafeFileOpen("TriangleListOutput.txt", "w");
	if (!fp)
		return 0;
	i = 0;
	for (triangleListStart = triangleList->begin() ; triangleListStart != triangleList->end() ; ++triangleListStart)
	{
		fprintf(fp, "%d\n", *triangleListStart);

	}
	fclose(fp);

	//********************************
	vector<UVIDLookupEntry>::iterator UVIDListPos;
	fp = NULL;
	fp = SafeFileOpen("UVLookupTableOutput.txt","w");
	if(!fp)
		return 0;
	i = 0;
	
	for(UVIDListPos = UVIDLookupTable->begin();  UVIDListPos != UVIDLookupTable->end(); ++UVIDListPos)
	{
		fprintf(fp, "UV: %d:    U: %f   V: %f\n", i, (*UVIDListPos).u, (*UVIDListPos).v);
		i++;
	}

	fclose(fp);

	//********************************
	vector<VIDLookupEntry>::iterator VIDListPos;
	fp = NULL;
	fp = SafeFileOpen("VIDLookupTableOutput.txt", "w");
	if (!fp)
		return 0;
	i = 0;
	for (VIDListPos = VIDLookupTable->begin() ; VIDListPos != VIDLookupTable->end() ; ++VIDListPos)
	{
		fprintf(fp, "VID: %d: X: %f\tY: %f\tZ: %f\n", i, (*VIDListPos).x, (*VIDListPos).y, (*VIDListPos).z);
		i++;
	}
	
	fclose(fp);
	}

#endif


	//safe delete our temporary data structures.
	if (VIDLookupTable)
		delete  VIDLookupTable;
	if (UVIDLookupTable)
		delete  UVIDLookupTable;
	if (triangleList)
		delete  triangleList;
	if (vertexArray)
		delete vertexArray;
	if (faceList)
		delete  faceList;

	}//end process each frame

	//---------cleanup------------------------
	fclose(fp);

	Output("blarg.txt");

	//set our strided info properly
	stridedDataInfo.normal.lpvData = stridedNormals;
	stridedDataInfo.textureCoords[0].lpvData = stridedUV;


	return 1;
}


void GetVertexIDList(vector<VIDLookupEntry> &VIDLookupTable, int numVertices, FILE* fp)
{
	vector<VIDLookupEntry>::iterator pos;


	//start at 0 and go down the list of vertexID's and their x,y,z entries in the file.
	// Assign VID X to LookupTable[X].

	for (pos = VIDLookupTable.begin(); pos != VIDLookupTable.end() ; ++pos)
	{

		SeekTo(fp, "Vertex:");
		//skip past the VID
		GetInt(fp);

		(*pos).x = GetFloat(fp);
		(*pos).y = GetFloat(fp);
		(*pos).z = GetFloat(fp);
	}	
}




void GetUVIDList(vector<UVIDLookupEntry> &UVIDLookupTable, int numUVCoords, FILE* fp)
{

	vector<UVIDLookupEntry>::iterator pos;

	//start at 0 and go down the list of UVID's and their u and v entries.
	// Assign UVIX N to UVIDLookupTable[N]

	for (pos = UVIDLookupTable.begin(); pos != UVIDLookupTable.end(); ++pos)
	{
		SeekTo(fp, "UV:");
		//skip past the UVID number
		GetInt(fp);

		(*pos).u = GetFloat(fp);
		(*pos).v = GetFloat(fp);
	}

}


void GetFaceList(vector<FaceListEntry> &faceList, int numFaces, int numVertices, FILE* fp)
{
	vector<FaceListEntry>::iterator pos;

	for (pos = faceList.begin() ; pos != faceList.end() ; ++pos)
	{


		SeekTo(fp, "FACE:");
		//ignore FaceID
		GetInt(fp);
		//skip past the faceId's colon
		fseek(fp, 1, SEEK_CUR);

		//we pass in the converse of what the VID value should be because some idiot
		// wrote a pretty peculiar file exporter. entry X for a vertex in the face list actually references
		// the xth entry back from the end of the list of vertices. 

		SeekTo(fp, "2:");
		(*pos).Vid0 = GetInt(fp);//numVertices - 1 - GetInt(fp);
		(*pos).UVid0 = GetInt(fp);

		SeekTo(fp, "1:");
		(*pos).Vid1 = GetInt(fp);//numVertices - 1 - GetInt(fp);
		(*pos).UVid1 = GetInt(fp);

		SeekTo(fp, "0:");
		(*pos).Vid2 = GetInt(fp); //numVertices - 1 - GetInt(fp);
		(*pos).UVid2 = GetInt(fp);
		
		SeekTo(fp, "normal:");
		(*pos).faceNormal.nx = GetFloat(fp);
		(*pos).faceNormal.ny = GetFloat(fp);
		(*pos).faceNormal.nz = GetFloat(fp);
	}

}


inline void InitializeVertexList(vector<VertexListEntry> &vertexArray, int numVertices)
{
	vector<VertexListEntry>::iterator pos;
	int index = 0;

	for (pos = vertexArray.begin() ; index < numVertices  ; pos++, index++)
	{
		(*pos).Vid = index;
	}

}


int ProcessFaceVertex(int VID, 
					  int UVID, 
					  const Normal &faceNormal, 
					  int &currentVertexCount,
					  int numVertices, 
					  int faceNumber,
					  vector<VertexListEntry> &vertexArray)
{

		//basically we try to add the UV and normal data to the proper slot.
		//finding the proper slot is a hassle. We may have to add an entry to the end of 
		// the vertex array. In this case, we increment currentVertexCount which is passed
		// in by alias.


		VertexListEntry vle, vle2;

		//lookup what is already at the VID slot in the Vertex array.
		vle = vertexArray.at(VID);

		if (vle.UVid == -1)
			//we have not seen this vertex before
		{
			vertexArray.at(VID).Vid = VID;
			vertexArray.at(VID).UVid = UVID;
			vertexArray.at(VID).normal = faceNormal;
			vertexArray.at(VID).facelist.push_back(faceNumber);
			return VID;
		}
		else
		if (vle.UVid == UVID )//&& vle.normal == faceNormal)
			//we have found a vertex record that is equivalent. Do nothing.
			//Just push the faceNumber in and return the current vertex id location.
		{
			vertexArray.at(VID).facelist.push_back(faceNumber);
			return VID;
		}
		else
		{
			//we have found a VID with a differing UVID. It could be that we have already
			// added this VID, UVID pair. We have to find out by scanning.
			//We need to do a run to see if there was an identical vertex added elsewhere. 
			//if not, we need to add one.

			//scan to the end and see if we find a duplicate.

					// [Since we use the STL we can replace currentVertex count with a call
					//  to size() of the vertexArray].
			int index;
			for (  index = numVertices; index < currentVertexCount ; index++)
			{
				if (vertexArray[index].Vid == VID && vertexArray[index].UVid == UVID)
				{
					//we have found a match
					//push the face number in
					vertexArray.at(index).facelist.push_back(faceNumber);
					
					// return the Location (which is also the VID)
					//   since we are using direct lookup table. 
					return index;
				}
			}
			//if we escape the for loop without returning we have found no such value. 
			vle2.Vid = VID;
			vle2.UVid = UVID;
			vle2.normal = faceNormal;
			vle2.facelist.push_back(faceNumber);
			vector<VertexListEntry>::iterator insertPosition = vertexArray.begin() + currentVertexCount;
			vertexArray.insert(insertPosition, vle2);
			
			//increment and return.
			currentVertexCount++;
			return (index);
		}
 
}


bool SmoothVertexNormals(vector<VertexListEntry> *vertexArray, int currentVertexCount, 
                         int numVertices, vector<FaceListEntry> * faceList)
{

		//now we calculate the average normal for each of the original vertices by summing up
		// the normals in the internal list and dividing by the number.
		// (There are other measures and more sophisticated means, but.....)

		vector<int>::iterator faceNormalPos;
		
		for (int position = 0 ; position < numVertices ; ++position)
		{
			Normal total;
			vector<int>* currentFaceList = &vertexArray->at(position).facelist;
	
			
			for (faceNormalPos = currentFaceList->begin() ; faceNormalPos != currentFaceList->end() ; ++faceNormalPos)
			{
					Normal tempNormal = faceList->at(*faceNormalPos).faceNormal;
					total.nx += tempNormal.nx;
					total.ny += tempNormal.ny;
					total.nz += tempNormal.nz;			
			}
			
			//now divide by the number of normals
			//(we can do this because we know that TRUSPACE outputs normalized vectors, and
			// hence the length of each vector is 1.

			int numNormals = currentFaceList->size();
			total.nx /= numNormals;
			total.ny /= numNormals;
			total.nz /= numNormals;
			
			//normalize the result
			total.Normalize();
			
			//finally we copy this value into the structure
			vertexArray->at(position).normal = total;
		}

	return 1;
}

