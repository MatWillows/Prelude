//********************************************************************* 
//********************************************************************* 
//**************                things.cpp          ******************* 
//********************************************************************* 
//********************************************************************* 
//********************************************************************* 
//*                                                                                                                                      * 
//*Revision:    7/16                                                   * 
//*Revisor:     mat williams                                           
//*Purpose:     functionality for base thing class                     
//********************************************************************* 
//*Outstanding issues:                                                                                                       * 
//*     
//*		
//********************************************************************* 
//*********************************************************************

#include "things.h"	
#include "ZSModelEx.h"
#include "zsengine.h"
#include "zsutilities.h"
#include "world.h"
#include "gameitem.h"
#include <assert.h>

float ArrowAngle = 0.0f;
float ArrowAlphaOffset = 0.03f;
float ArrowAlpha = 0.1f;

//************** static thing Members *********************************

int Thing::NextUniqueID = 0;
int Thing::NumThings = 0;


//************** Constructors  ****************************************

//simple constructor
Thing::Thing()
{
	//initialize all member variables
	DataFieldNames = NULL;
	DataFields = NULL;
	DataTypes = NULL;
	pNext = NULL;
	pTexture = NULL;
	 
	NumThings++;

	//set the unique ID (UID) to be equal to the next unique ID
	UniqueID = NextUniqueID;

	//increment the next unique ID
	NextUniqueID++;

	//done
}

//copy constructor
Thing::Thing(Thing *pFrom)
{
	DataFieldNames = pFrom->DataFieldNames;
	NumFields = pFrom->NumFields;
	//assign member variables directly where possible
	DataFields = new DATA_FIELD_T[NumFields];
	DataTypes = new DATA_T[NumFields];
	//create space for the data fields
	for(int n =0;n< NumFields;n++)
	{
		switch(DataTypes[n])
		{
		case DATA_INT:
		case DATA_FLOAT:
			DataFields[n] = pFrom->DataFields[n];
			break;
		case DATA_STRING:
			DataFields[n].String = new char[strlen(pFrom->DataFields[n].String) + 1];
			strcpy(DataFields[n].String,pFrom->DataFields[n].String);
			break;
		case DATA_VECTOR:
			DataFields[n].pVector = new D3DVECTOR;
			memcpy(DataFields[n].pVector, pFrom->DataFields[n].pVector,sizeof(D3DVECTOR));
			break;
		default:
			DataFields[n] = DataFields[n];
			break;
		}
	}
	//copy the data fields from the source into the new object
	//set the unique ID (UID) to be equal to the next unique ID
	UniqueID = NextUniqueID;
	
	//increment the next unique ID
	NextUniqueID++;
	//done
}

//end:  Constructors ***************************************************



//*************** Destructor *******************************************

Thing::~Thing()
{
	//delete all data fields
	delete[] DataFields;
	//because things may cross reference data field names, do not delete them
}

//end:  Destructor *****************************************************



//************  Accessors  *********************************************

D3DVECTOR *Thing::GetPosition()
{
	return GetData(INDEX_POSITION).pVector;
}

int Thing::GetFrame()
{
	return GetData(INDEX_FRAME).Value;
}

int Thing::SetFrame(int NewFrame)
{
	Frame = NewFrame;
	this->SetData(INDEX_FRAME, NewFrame);
	return TRUE;
}

float Thing::GetMyAngle()
{
	return GetData(INDEX_ROTATION).fValue;
}

int Thing::SetAngle(float NewAngle)
{
	Angle = NewAngle;
	this->SetData(INDEX_ROTATION, NewAngle);
	return TRUE;
}

float Thing::GetScale()
{
	return GetData(INDEX_SCALE).fValue;
}

float Thing::GetRadius()
{
	return BlockingRadius;
}

DATA_FIELD_T Thing::GetData(int fieldnum)
{
	//reference the data fields by the number passed
	return DataFields[fieldnum];
}

DATA_FIELD_T Thing::GetData(char *fieldname)
{
	//compare each data field name to the field name passed
	//when a match is found return the value at that fieldname
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			return DataFields[n];
		}
		fn += 32;
	}

	//display error
	
	DEBUG_INFO("GetData: Failed to find field: ");
	DEBUG_INFO(fieldname);
	DEBUG_INFO("\n");
	SafeExit("GetData failed");

	return DataFields[n];
}
/*
Thing *Thing::GetNext()
{
	return pNext;
}
*/	

char *Thing::GetFieldNames()
{
	return DataFieldNames;
}
	
int Thing::GetNumFields()
{
	return NumFields;
}

Thing *Thing::Find(Thing *SearchStart, int ID, int UID)
{
	Thing *pThing = SearchStart;

	while(pThing)
	{
		if(pThing->GetData(INDEX_ID).Value == ID)
		{
			if(!UID || (UID && pThing->GetData(INDEX_UID).Value == UID))
			{
				break;
			}
			else
			{ 
				pThing = (Thing *)pThing->GetNext();
			}
		}
		else
		{
			pThing = (Thing *)pThing->GetNext();
		}
	}

	return pThing;
}

Thing *Thing::Find(Thing *SearchStart, const char *ThingName)
{
	Thing *pThing = SearchStart;

	while(pThing)
	{
		if(!strcmp(pThing->GetData(INDEX_NAME).String, ThingName))
		{
			break;
		}
		else
		{
			pThing = (Thing *)pThing->GetNext();
		}
	}

	return pThing;
}

int Thing::GetIndex(char *FieldName)
{
	//compare each data field name to the field name passed
	//when a match is found return the value at that fieldname
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(FieldName,&DataFieldNames[fn]))
		{
			return n;
		}
		fn += 32;
	}

	//display error
	
	DEBUG_INFO("Failed to find field Index: ");
	DEBUG_INFO(FieldName);
	DEBUG_INFO("\n");
	return -1;
}

char *Thing::GetName(int FieldNum)
{
	return &DataFieldNames[FieldNum * 32];
}

DATA_T Thing::GetType(int FieldNum)
{
	return DataTypes[FieldNum];
}

void Thing::GetXY(int *X, int *Y)
{
	D3DVECTOR *pVector;

	pVector = GetData(INDEX_POSITION).pVector;

	*X = (int)pVector->x;
	*Y = (int)pVector->y;
}
/*
BOOL Thing::RayIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd)
{
	D3DVECTOR *pPosition;

	pPosition = GetData(INDEX_POSITION).pVector;

	float Rotation;

	Rotation = GetData(INDEX_ROTATION).fValue;

	int CurFrame;

	CurFrame = GetData(INDEX_FRAME).Value;
	//transform the ray into object coordinates;
	//first check if ray is close enough to matter
	//point to line distance
	float Distance;

	Distance = PointToLine(pPosition,vRayStart,vRayEnd);

	float CurScale;

	CurScale = GetData(INDEX_SCALE).fValue;

	CurScale = 1.0f/CurScale;

	if(Distance < (pMesh->GetHeight()))
	{
		//convert the ray to object coordinates
		//first translate then rotate
		D3DXMATRIX matRotate, matScale, matTransform;
		D3DXMatrixRotationZ( &matRotate, -(Rotation + PI));
		D3DXMatrixScaling(&matScale, CurScale, CurScale, CurScale);
		D3DXMatrixMultiply(&matTransform, &matRotate, &matScale);
		D3DXVECTOR4 vTransStart, vTransEnd;
		D3DVECTOR vMoveStart, vMoveEnd;
		vMoveStart = *vRayStart - *pPosition;
		vMoveEnd = *vRayEnd - *pPosition;
		D3DXVec3Transform(&vTransStart,(D3DXVECTOR3 *)&vMoveStart, &matTransform);
		D3DXVec3Transform(&vTransEnd,(D3DXVECTOR3 *)&vMoveEnd, &matTransform);
		return pMesh->Intersect(CurFrame, (D3DVECTOR *)&vTransStart,(D3DVECTOR *)&vTransEnd);
	}
	else
	{
		return FALSE;
	}
}
*/
//end: Accessors *******************************************************



//************  Display Functions  *************************************
void Thing::Draw(void)
{
	//get a pointer to the mesh indicated by the things mesh field
	//set the current texture to the texture specified in the texture field
	//call the mesh's  draw function with the proper position, rotation,
	//and scaling factors
	//done

	
	D3DVECTOR *pPosition;

	pPosition = GetPosition();

	float Rotation;

	Rotation = GetMyAngle();

	int Frame;

	Frame = GetFrame();

	Engine->Graphics()->SetTexture(pTexture);

	if(pMesh)
	{
		pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x,pPosition->y,pPosition->z,Rotation,Scale,Scale,Scale,Frame);

//cartoon rendering		
/*		Engine->Graphics()->ClearTexture();
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_BLACK));
	//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);
		pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x,pPosition->y,pPosition->z,Rotation,1.03f,1.03f,1.025f, Frame);
		Engine->Graphics()->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	//	Engine->Graphics()->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);
		Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
*/	
  }
	return;
}

BOOL Thing::AdvanceFrame()
{
/*	SetData(INDEX_FRAME, GetData(INDEX_FRAME).Value + 1);
	if(GetData(INDEX_FRAME).Value > pMesh->numframes)
	{
		SetData(INDEX_FRAME, 0);
	}
*/	return TRUE;
}

/*
int Thing::HighLight(COLOR_T Color)
{

	Engine->Graphics()->ClearTexture();//Engine->Active.GetSurface());

//	D3DMATERIAL7 TempMat;
//	ZeroMemory(&TempMat,sizeof(TempMat));

//	TempMat.diffuse = Color; 
	//TempMat.specular = 0;
//	TempMat.ambient = Color;
//	TempMat.power = 1.0f;
	
//	Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);
	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(Color));
	D3DVECTOR *pPosition;

	pPosition = GetData(INDEX_POSITION).pVector;

	float Rotation;

	Rotation = GetData(INDEX_ROTATION).fValue;

	int Frame;

	Frame = GetData(INDEX_FRAME).Value;

	if(pMesh)
		pMesh->Draw(Engine->Graphics()->GetD3D(),pPosition->x,pPosition->y,pPosition->z,Rotation,Frame);

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));

	return TRUE;
}
*/
int Thing::Circle(COLOR_T Color)
{
/*	D3DVECTOR At;
	float CircleRadius;
	At = *GetData(INDEX_POSITION).pVector;
	At.z += 0.05f;
	CircleRadius = (pMesh->GetWidth() + pMesh->GetHeight())/4.0f;
	Engine->Graphics()->Circle(&At, Color, CircleRadius);
	return Engine->Graphics()->Circle(&At, Color, CircleRadius);
*/
/*
	D3DVECTOR *vAt;
	vAt = GetData(INDEX_POSITION).pVector;
	Valley->HighlightTile(CircleTile((int)vAt->x,(int)vAt->y,Color);
	
	Valley->CircleTile(CircleTile((int)vAt->x,(int)vAt->y,Color);
	return TRUE;
*/
	return TRUE;

}

void Thing::WordBalloon()
{
	if(!pMesh)
		return;
	
	D3DVECTOR *pVect;
	
	pVect = GetData(INDEX_POSITION).pVector;
	
	float x;
	float y;
	float z;

	float RotationAngle;

	RotationAngle = PreludeWorld->GetCameraAngle();

	x = pVect->x;
	y = pVect->y;
	z = pVect->z + pMesh->GetHeight() + (Engine->GetMesh(1)->GetHeight()/4);

	Engine->Graphics()->ClearTexture();

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));
	
	Engine->GetMesh("wordballoon")->Draw(Engine->Graphics()->GetD3D(),x,y,z,RotationAngle,0);
	
	
	return;
}

int Thing::Arrow(D3DCOLORVALUE Color)
{
	if(!pMesh) 
		return FALSE;
	D3DVECTOR *pVect;
	
	pVect = GetData(INDEX_POSITION).pVector;
	
	float x;
	float y;
	float z;

	x = pVect->x;
	y = pVect->y;
	z = pVect->z + pMesh->GetHeight() + (Engine->GetMesh(1)->GetHeight()/4);

	Engine->Graphics()->ClearTexture();

	D3DMATERIAL7 TempMat;
	ZeroMemory(&TempMat,sizeof(TempMat));

	TempMat.diffuse = Color; 
	TempMat.diffuse.a = ArrowAlpha;
	TempMat.ambient = Color;
	TempMat.ambient.a = ArrowAlpha;
	TempMat.power = 1.0f;
	
	Engine->Graphics()->GetD3D()->SetMaterial(&TempMat);

	Engine->GetMesh(1)->Draw(Engine->Graphics()->GetD3D(),x,y,z,ArrowAngle,0);
	
	
	ArrowAngle += ROTATION_ANGLE;
	if(ArrowAngle > PI_MUL_2)
	{
		ArrowAngle = 0;
	}

	ArrowAlpha += ArrowAlphaOffset;

	if(ArrowAlpha < 0)
	{
		ArrowAlphaOffset = 0.03f;
	}
	else
	if(ArrowAlpha > 0.9f)
	{
		ArrowAlphaOffset = -0.03f;
	}

	Engine->Graphics()->GetD3D()->SetMaterial(Engine->Graphics()->GetMaterial(COLOR_DEFAULT));

	return TRUE;
}



//end: Display functions ***********************************************



//************ Mutators ************************************************
/*
int Thing::SetNext(Thing *NewNext)
{
	pNext = NewNext;
	return TRUE;
}
*/

int Thing::SetFieldNames(char *NewNames)
{
	DataFieldNames = NewNames;
	return TRUE;
}

int Thing::SetNumFields(int NewNumFields)
{
	NumFields = NewNumFields;
	return TRUE;
}

int Thing::SetData(int fieldnum, int NewValue)
{
	//set the value at the index provide to be equal to the value passed
	DataFields[fieldnum].Value = NewValue;
	//done
	return TRUE;
}

int Thing::SetData(char *fieldname, int NewValue)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			DataFields[n].Value = NewValue;
			return TRUE;
		}
		fn += 32;
	}

	//display error
	
	DEBUG_INFO("SetData: Failed to find field: " );
	DEBUG_INFO(fieldname);
	DEBUG_INFO("\n");
	SafeExit("SetData failed");

	return TRUE;
}

int Thing::SetData(int fieldnum, float NewfValue)
{
	//set the value at the index provide to be equal to the value passed
	DataFields[fieldnum].fValue = NewfValue;
	//done
	return TRUE;
}

int Thing::SetData(char *fieldname, float NewfValue)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			DataFields[n].fValue = NewfValue;
			return TRUE;
		}
		fn += 32;
	}
	
	DEBUG_INFO("SetData: Failed to find field: ");
	DEBUG_INFO(fieldname);
	DEBUG_INFO("\n");
	SafeExit("SetDat Failed");

	return TRUE;
}

int Thing::SetData(int fieldnum, char *NewString)
{
	//set the value at the index provide to be equal to the value passed
	if(DataFields[fieldnum].String)
		delete[] DataFields[fieldnum].String;
	DataFields[fieldnum].String = NewString;
	//done
	return TRUE;
}

int Thing::SetData(char *fieldname, char *NewString)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			if(DataFields[n].String)
				delete[] DataFields[n].String;
			DataFields[n].String = NewString;
			return TRUE;
		}
		fn += 32;
	}


	DEBUG_INFO("SetData: Failed to find field: ");
	DEBUG_INFO(fieldname);
	DEBUG_INFO("\n");
	SafeExit("SetData Failed");
	return TRUE;
}

int Thing::SetData(int fieldnum, D3DVECTOR *NewpVector)
{
	//set the value at the index provide to be equal to the value passed
	DataFields[fieldnum].pVector = NewpVector;
	//done
	return TRUE;
}

int Thing::SetData(char *fieldname, D3DVECTOR *NewpVector)
{
	//compare each data field name to the field name passed
	//when a match is found set that field to equal to value passed
	//if no match is found display an error message
	int fn = 0;
	for(int n = 0; n < NumFields; n++)
	{
		if(!strcmp(fieldname,&DataFieldNames[fn]))
		{
			DataFields[n].pVector = NewpVector;
			return TRUE;
		}
		fn += 32;
	}

	//display error

	DEBUG_INFO("SetData: Failed to find field: ");
	DEBUG_INFO(fieldname);
	DEBUG_INFO("\n");
	SafeExit("SetData Failed");

	return TRUE;
}

int Thing::LoadFieldNames(FILE *fp, char *destination)
{
	//check out input parameters

	assert(fp);

	//mark our start position
	fpos_t start;
	fgetpos(fp, &start);

	int numfields = 0;

	char c;

	c = fgetc(fp);

	while (c != '\r' && c != '\n' )
	{
		if(c == DELIMIT_CHARACTER)  
		{
			numfields++;
		}
		c = fgetc(fp);
	}
	numfields++;
	fsetpos(fp, &start);

	//read the field names one at a time into the array passed from
	//read in the names
	BOOL done = FALSE;
	int ccount = 0;
	int fn = 0;
	while (!done)
	{
		ccount = 0;

		do
		{
			c = (char)fgetc(fp);
			if(c != DELIMIT_CHARACTER && c != '\n' && c != '\r')
				destination[fn + ccount] = c;
			else
				destination[fn + ccount] = '\0';
			ccount++;
		}while (c != DELIMIT_CHARACTER && c != '\n' && c != '\r');

		fn += 32;
		if( c == '\n' || c == '\r')
		{
			done = TRUE;
		}

	}
	
	NumFields = numfields;
	//printf("number of fields: %i\n",numfields);

	DataFieldNames = destination;

	DEBUG_INFO("Loaded FieldNames\n");

	return TRUE;
}

int Thing::LoadBin(FILE *fp)
{
	fread(&Position,	sizeof(Position),1,fp);
	fread(&Frame, sizeof(Frame),1,fp);
	fread(&Data, sizeof(Data),1,fp);
	fread(&Angle, sizeof(Angle),1,fp);
	fread(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fread(&Scale, sizeof(Scale),1,fp);
	fread(&MeshNum, sizeof(MeshNum),1,fp);
	fread(&TextureNum, sizeof(TextureNum),1,fp);

	if(MeshNum)
	{
		pMesh = Engine->GetMesh(MeshNum);
	}
	else
	{
		pMesh = NULL;
	}
	if(TextureNum)
	{
		pTexture = Engine->GetTexture(TextureNum);
	}
	else
	{
		pTexture = NULL;
	}

	DataFields = new DATA_FIELD_T[NumFields];

	int Length;
	for(int n = 0; n < NumFields; n++)
	{
		switch(DataTypes[n])
		{
			case DATA_INT:
				fread(&DataFields[n].Value,sizeof(int),1,fp);
				break;
			case DATA_FLOAT:
				fread(&DataFields[n].fValue,sizeof(float),1,fp);
				break;
			case DATA_STRING:
				fread(&Length,sizeof(int),1,fp);
				DataFields[n].String = new char[Length + 1];
				DataFields[n].String[Length] = '\0';
				fread(DataFields[n].String,sizeof(char),Length,fp);
				break;
			case DATA_VECTOR:
				DataFields[n].pVector = new D3DVECTOR;
				fread(DataFields[n].pVector,sizeof(D3DVECTOR),1,fp);
				break;
			case DATA_NONE:
				fprintf(fp,"Blarg, error");
				break;
		}
	}

	int NumItems = 0;
	//save their inventory/contents;
	fread(&NumItems,sizeof(int),1,fp);
	for(n = 0; n < NumItems; n++)
	{
		GameItem *pGI;
		pGI = new GameItem();
		OBJECT_T OT;
		fread(&OT,sizeof(OBJECT_T),1,fp);
		pGI->Load(fp);
		pGI->SetNext(pContents);
		if(this->GetObjectType() == OBJECT_CREATURE)
			pGI->SetLocation(LOCATION_PERSON,this);
		else
			pGI->SetLocation(LOCATION_CONTAINER, this);
		pContents = (Object *)pGI;
	}

	return TRUE;
}

int Thing::LoadData(FILE *fp)
{
	//read in the data fields one at a time from the file passed 
	//allocate space for the data
	DataFields = new DATA_FIELD_T[NumFields];
	if(!DataTypes)
	{
		DataTypes = new DATA_T[NumFields];
	}
	char *pString;
	//read in each field one at a time
	for(int n = 0; n < NumFields; n++)
	{
		DataTypes[n] = GetFileData(fp, &DataFields[n]);
		if(DataTypes[n] == DATA_STRING)
		{
			if(!strcmp(&DataFieldNames[n * 32], "MESH"))
			{
				pString = DataFields[n].String;
				DataTypes[n] = DATA_INT;
				pMesh = Engine->GetMesh(pString);
				DataFields[n].Value = 0;
				delete[] pString;
			}
			else
			if(!strcmp(&DataFieldNames[n * 32], "TEXTURE"))
			{
				pString = DataFields[n].String;
				DataTypes[n] = DATA_INT;
				pTexture = Engine->GetTexture(pString);
				DataFields[n].Value = 0;
				delete[] pString;
			}
		}
	}
	//seek to the end of the line.
	//GOT_HERE("%s: Loaded Data \n",GetData("NAME").String);
	return TRUE;
}

int Thing::DestroyFieldNames()
{
	DEBUG_INFO("Field names destroyed\n");

	return TRUE;
}


//end: Mutators ********************************************************



//************ Outputs *************************************************

int Thing::SaveFieldNames(FILE *fp)
{
	//write out each field name one at a time to the file
	for(int n = 0; n < NumFields; n++)
	{
		SaveString(fp,&DataFieldNames[n*32]);
		//put a delimitting character between each field name
		fputc(DELIMIT_CHARACTER,fp);
	}
	fseek(fp,-1,1);
	fputc('\r',fp);
	//after all field names are outputted put an endline

	//GOT_HERE("Field names saved\n",0);
	return TRUE;
}

int Thing::SaveBin(FILE *fp)
{
	//write out the value of each field one at a time to the file
	//write out each field name one at a time to the file
	if(pMesh)
		MeshNum = Engine->GetMeshNum(pMesh);
	else
		MeshNum = 0;
	TextureNum = Engine->GetTextureNum(pTexture);
	fwrite(&Position,	sizeof(Position),1,fp);
	fwrite(&Frame, sizeof(Frame),1,fp);
	fwrite(&Data, sizeof(Data),1,fp);
	fwrite(&Angle, sizeof(Angle),1,fp);
	fwrite(&BlockingRadius, sizeof(BlockingRadius),1,fp);
	fwrite(&Scale, sizeof(Scale),1,fp);
	fwrite(&MeshNum, sizeof(MeshNum),1,fp);
	fwrite(&TextureNum, sizeof(TextureNum),1,fp);

	int Length;
	
	for(int n = 0; n < NumFields; n++)
	{
		switch(DataTypes[n])
		{
			case DATA_INT:
				fwrite(&DataFields[n].Value,sizeof(int),1,fp);
				break;
			case DATA_FLOAT:
				fwrite(&DataFields[n].fValue,sizeof(float),1,fp);
				break;
			case DATA_STRING:
				Length = strlen(DataFields[n].String);
				fwrite(&Length,sizeof(int),1,fp);
				fwrite(DataFields[n].String,sizeof(char),Length,fp);
				break;
			case DATA_VECTOR:
				fwrite(DataFields[n].pVector,sizeof(D3DVECTOR),1,fp);
				break;
			case DATA_NONE:
				fprintf(fp,"Blarg, error");
				break;
		}
		//put a delimitting character between each field name
	}
	
	int NumItems = 0;
	//save their inventory/contents;
	Object *pOb;
	pOb = GetContents();
	while(pOb)
	{
		NumItems++;
		pOb = pOb->GetNext();
	}
	fwrite(&NumItems,sizeof(int),1,fp);
	pOb = GetContents();
	for(n = 0; n < NumItems; n++)
	{
		((GameItem *)pOb)->Save(fp);
		pOb = pOb->GetNext();
	}


	return TRUE;
}

int Thing::SaveData(FILE *fp)
{
	//write out the value of each field one at a time to the file
		//write out each field name one at a time to the file

	for(int n = 0; n < NumFields; n++)
	{
		switch(DataTypes[n])
		{
			case DATA_INT:
				SaveInt(fp,DataFields[n].Value);
				break;
			case DATA_FLOAT:
				SaveFloat(fp,DataFields[n].fValue);
				break;
			case DATA_STRING:
				SaveString(fp,DataFields[n].String);
				break;
			case DATA_VECTOR:
				SaveVector(fp,DataFields[n].pVector);
				break;
			case DATA_NONE:
				fprintf(fp,"Blarg, error");
				break;
		}
		//put a delimitting character between each field name
		fputc(DELIMIT_CHARACTER,fp);
	}
	//backup and replace the delimitting character with an endline
	fseek(fp,-1,1);
	fputc('\r',fp);

	//GOT_HERE("%s: Data Saved\n",GetData("NAME").String);
	return TRUE;
}

//

bool Thing::operator == (Thing &ThingOne)
{
	if(NumFields != ThingOne.GetNumFields())
	{
		return false;
	}
	for(int n = INDEX_DAMAGE_OVERRIDE + 1; n < NumFields; n++)
	{
		if(GetData(n).Value != ThingOne.GetData(n).Value)
		{
			return false;
		}
	}
	return true;
}

Thing::operator = (Thing &OtherThing)
{
	for(int n = 0; n < NumFields; n++)
	{
		switch(DataTypes[n])
		{
		case DATA_NONE:
			break;
		case DATA_INT:
			DataFields[n].Value = OtherThing.GetData(n).Value;
			break;
		case DATA_FLOAT:
			DataFields[n].fValue = OtherThing.GetData(n).fValue;
			break;
		case DATA_VECTOR:
			if(DataFields[n].pVector)
			{
				delete DataFields[n].pVector;
			}
			DataFields[n].pVector = new D3DVECTOR;
			memcpy(DataFields[n].pVector,OtherThing.GetData(n).pVector,sizeof(D3DVECTOR));
			break;
		case DATA_STRING:
			if(DataFields[n].String)
			{
				delete[] DataFields[n].String;
			}
			DataFields[n].String = new char[strlen(OtherThing.GetData(n).String) + 1];
			memcpy(DataFields[n].String,OtherThing.GetData(n).String,strlen(OtherThing.GetData(n).String) + 1);
			break;
		}
	}
}


//

//end: Outputs ********************************************************



//************ Debug ***************************************************

int Thing::OutputDebugInfo(FILE *fp)
{
	//write out the value of each field one at a time to the file
	//write out the name of the field
	//write out the value of the field
	return TRUE;
}

int Thing::OutputDebugInfo(const char *FileName)
{
	FILE *fp;
	fp = SafeFileOpen(FileName,"wt");
	fseek(fp,0,0);


	int n;
	for(n = 0; n < NumFields; n++)
	{
		fprintf(fp,"%s: ",DataFieldNames[n]);
		switch(DataTypes[n])
		{
		case DATA_FLOAT:
			fprintf(fp, "%d\n",DataFields[n].fValue);
			break;
		case DATA_INT:
			fprintf(fp, "%i\n",DataFields[n].Value);
			break;
		case DATA_STRING:
			fprintf(fp, "%s\n",DataFields[n].String);
			break;
		case DATA_VECTOR:
			fprintf(fp, "(%d,%d,%d)\n",DataFields[n].pVector->x,DataFields[n].pVector->y,DataFields[n].pVector->z);
			break;
		case DATA_ERROR:
			fprintf(fp, "ERROR\n");
			break;
		}
	}

	fclose(fp);

	return TRUE;

}

//end: Debug ***********************************************************

int Thing::Has(Thing *ToTest)
{
	return FALSE;
}
int Thing::Take(Thing *ToTake, int Quantity)
{
	return FALSE;
}
int Thing::Give(Thing *ToReceive, int Quantity)
{
	return FALSE;
}

int Thing::AltHighLight()
{
	//display text in the appropriate location
	if(pMesh)
	{
		float MeshTop;
		MeshTop = pMesh->GetHeight();
		MeshTop *= GetScale();
		D3DVECTOR TextVector;
		TextVector = *GetPosition();
		TextVector.z += MeshTop;

		Engine->Graphics()->DrawText(&TextVector,this->GetData(INDEX_NAME).String);
	}

	return TRUE;
}

