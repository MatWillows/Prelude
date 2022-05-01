//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSModel.h           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    1.	Aug 22th, 2000                                    *                                                    *
//*Author:      James Williams            
//*					Mat Williams			                               *
//*Purpose:                                                           *
//*				This file provides a declaration of a class that      *
//*				 encapsulates ZSModel data for a 3d entity.		      *
//*             It supports multiple meshes, giving multiple frames   *
//*              of animation.                                        *
//*********************************************************************
//*Outstanding issues:                                                                                                      *
//*	
//*********************************************************************
//*********************************************************************
//Revision 5:  updated to strided vertex format.
//				Deleted unnecessary data members
//Revision 6:  added load and save implementations, plus
//              structures for the ZSModel file format.
//Revision 7: Fixed Destructor, Cleanup of internal functions. Extend Output to take care
//				of equipment info.
//Revision 8: Removed Identity matrix.  Changed implementation of equipment registration to 
//				index vertices in the mesh rather than be independent of mesh
//Revision 9: Tested strided vertex format
//Revision 10:  Implemented equipment linkage
//Revision 11:  Switch to Single Frame normals.


#ifndef ZSModel_H
#define ZSModel_H

#define D3D_OVERLOADS

#include <vector>
#include <math.h>
#include <d3d.h>
#include <d3dx.h>
#include <d3dutil.h>
#include <d3dmath.h>

using namespace std;

#define MAX_EQUIPMENT_POSITIONS	10
#define MAX_MODEL_FILENAME			32


//------------------------Local Data Structures-----------------------------------------
//**************************************************************************************

//Struct to hold the info about equipment vectors. Used in DrawAsEquipment(..)
struct EquipmentLocator
{
	int LinkIndex;
	int RayFromIndex;
};


//------------------------Stuff for File I/O--------------------------------------------

#define 	ZSMFILEIDENTIFIER  0x02010345		//the identifier at the start of zms files.

//The file format is basically:
//
//	ZSModelFileHeader			-gives data about the model contained in the file
//  ZSModelFileInfoHeader		-indicates how the file is stored and gives the sizes of data runs to speed up loading
//	Equipment Info		(opt)	-optional section including the equipment info for the model. May be nonexistent if model
//								  is not equipment activated. There are 10 entries for equipment rays per frame. Not all
//                                will be used in a given model, but we normalize to 10 for convenience.
//  Triangle List				-a run of ushort ints
//  UV list						-a run of floats. u1 v1 u2 v2 u3 v3..... packed into a bytestream.
//  Vertex list					-a run of floats for each frame in the model. vx1 vy1 vz1 nx1 ny1 nz1 vx1 vy1 vz1 nx1 ny1 nz1

//Struct for the file format
struct ZSModelFileHeader
{
	DWORD	fileIdentifier;		//should be 0x02010345 for a ZSModel file
	DWORD	frameCount;			//the number of frames in this file
	DWORD	vertexCount;		//the number of vertices per frame
	DWORD	faceCount;			//the number of triangles
	DWORD	equipmentEnabled;	//does this model contain any equipmentlocators for placing other models on it?
	DWORD 	overlayEnabled;		//does this struct contain an Equipment Locator for placing it ON other models?
	DWORD	supportedEquipmentCount;	//the number of weapon slots supported in this model
	DWORD	reserved;			//reserved	
	EquipmentLocator overlayLocator;//the equipmentlocator which is used to position this model on other models. 
									//If the model doesn't support overlaying, this member is undefined. (don't access it).
									//we embed this data element in the header for convenience
									//we require a bit more control flow in the load and save routines if we embed it
									// with the rest of the data. This saves us a bit and makes offset computation more easy.
};

struct ZSModelFileInfoHeader
{
	DWORD	equipmentArraySize; //size (bytes) of a run of 10 equipment_locators
	DWORD	uvArraySize;		//size (bytes) of the uv array
	DWORD	faceArraySize;	//size (bytes) of the triangle list
	DWORD	vertexArraySize;	//size (bytes) of ONE FRAME's worth of vertex data
	DWORD	faceOffset;			//offset fromt he end of the struct to the face listing
	DWORD	uvOffset;		//offset from the end of the struct to the UV listing.
	DWORD	vertexOffset;	//offset from the end of the struct to the vertex listing
	DWORD modefierInfoSize;		//size (bytes) of the modifier info struct
};


//-------------------------------------------------------------------------------------
//*************************************************************************************


//enumeration for equipment positions. We use the enumeration to access the multidimensional
//    array in the ZSModel class directly. See below.
typedef enum { 
				EQUIP_POS_RHAND = 0,
			   EQUIP_POS_LHAND,
			   EQUIP_POS_HEAD,
			   EQUIP_POS_LEFTARM,
			   EQUIP_POS_SCABBARD,
			   EQUIP_POS_BACK,
				EQUIP_POS_CUSTOM1,
			   EQUIP_POS_CUSTOM2,
			   EQUIP_POS_CUSTOM3,
			   EQUIP_POS_CUSTOM4 } EQUIP_POSITION;








//****************************CLASS************************************
//**************                                  *********************
//**                         ZSModel                                 **
//*********************************************************************
//*Purpose:     See header banner                                     *
//*********************************************************************
//*Invariants:                                                                                                                    *
//*                                                                   *
//*********************************************************************


class ZSModel
{
public:

	//***************MEMBERS****************************************************
	//**
	int numframes;		//number of individual meshes
	
	int numvertex;		//number of vertices in each mesh (must be constant for all frames)

	int numtriangles;	//number of faces

	EquipmentLocator *equipmentlist; 

			//singe array. Equip_position[n] gives two indices into vertex array
			//to use perframe info for where to attach equipment.

	EquipmentLocator equipmentRegistrationMark; 
			//for objects like swords. Gives the registration
			// point which DrawAsEquipment() uses to match
			// up the object with the proper place on the model.

	int numWeaponSlotsDefined;
	
			//this gives the number of weapon slots supported.
			//The order of weapon slots must be respected, and if 5 slots are supported they must be slots 0 thru 4
			// (rh, lh, head, la, scabbard).

	unsigned short *trianglelist;	//the face list used as an index into the strided vertex arrays

	float**	stridedVertexArray;		// multidimensionl array of vertex positions.
												//  starting with vx vy vz...... The size of each frame is 
												//   3 * numvertices.
	
	float*  stridedNormals;			//single array of normal data nx,ny,nx
	float*  stridedUV;				//single array of uv data, u v u v..... size is 2 * numVertices

	
	D3DDRAWPRIMITIVESTRIDEDDATA stridedDataInfo;
									//nasty struct needed to use strided vertices.
									//embedded here for efficiency. (we don't want to push more on the stack than necessary).
	//----------------------------------------
	//**
	//******************************************************************************




	//********************METHODS***************************************************
	//**
	
	HRESULT Draw(LPDIRECT3DDEVICE7 D3DDevice, float x, float y, float z, float angle, float scale, int frame);
	//Invokes DDraw routines to draw the specified frame of the model. 
	//	Returns the typical ddraw results for drawprimitiveindexedstrided or whatever it's called.

	void Output(const char *filename);
	//outputs file info to ascii text file. 
	// Outputs ENTIRE contents of the model, as just the first frame is not suitable for debugging.

	bool Import(const char *filename);
	//Reads a TRUSPACE export file (mod)
	// Parses the file, performs vertex smoothing and sets up the strided vertex format.
	// A preprocessor macro controls the printing out of intermediate data. (see the source)
	// Very verbose, and not optimized for speed.

	bool Load(FILE *fp);
	bool Load(const char *filename);
	//Loads a ZSM file.
	//if the ZSModel already has info, the info is erased and replaced with that encoded in the binary ZSM file.
	//Loading from binary is MUCH MUCH faster than importing.
	
	bool Save(FILE *fp);
	bool Save(const char *filename);
	//Saves the contents of the ZSModel in binary format to a ZSM file.

	void Clear();
	//deletes all internal data and sets members to their initial values.

	bool DrawAsEquipment(LPDIRECT3DDEVICE7 D3DDevice, float scale, ZSModel* target, 
					     float rotation, int frame, int targetFrame, 
					     EQUIP_POSITION equip_position, float offset,
						 D3DVECTOR *WorldPosition,
						 float targetAngle);
	//Allows one to draw the current model as an overlay on another model which has equipment info defined.
	// The other model must have equipment information for the given position at each Frame in it's set of frames.
	// See the documentation for more detail.


	ZSModel();
	~ZSModel();
	
	bool operator=(const ZSModel& otherModel);
	bool operator==(const ZSModel& otherModel);
	bool operator!=(const ZSModel& otherModel);

	//**
	//*******************************************************************************
};

#endif