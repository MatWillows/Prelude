#include "ZSutilities.h"
#include <assert.h>

char *ExitErrorMessage = NULL;

const char *StatDescriptors[] = 
{
	"Worse than none", //less than zero
	"None",				// = 0
	"Bare Minimum",		// < 10
	"Awful",			// < 20
	"Very Bad",			// < 30
	"Bad",				// < 40
	"Below Average",	// < 50
	"Average",			// < 60
	"Above Average",	// < 70
	"Good",				// < 80
	"Very Good",		// < 90
	"Excellent",		// < 100
	"Perfect",			// = 100
	"Beyond Perfect"	// >100
};

const char *HealthDescriptors[] = 
{
	"Seriously Dead",		//less than zero
	"Dead",					// = 0
	"Almost Dead",			// < 10
	"Seriously Wounded",	// < 20
	"Wounded",				// < 30
	"Wounded",				// < 40
	"Injured",				// < 50
	"Bleeding",				// < 60
	"Bleeding",				// < 70
	"Bruised",				// < 80
	"Bruised",				// < 90
	"Scratched",			// < 100
	"Uninjured",			// = 100
	"Beyond Perfect"		// >100
};

const char *RestDescriptors[] = 
{
	"Unconscious", //less than zero
	"Collapsed",			// = 0
	"About to Collapse",	// < 10
	"Barely Standing",	// < 20
	"Very Bad",				// < 30
	"Bad",					// < 40
	"Heavily Winded",		// < 50
	"Winded",				// < 60
	"Winded",				// < 70
	"Breathing Hard",		// < 80
	"Barely Winded",		// < 90
	"Well Rested",			// < 100
	"Perfect",				// = 100
	"Beyond Perfect"		// >100
};

const char *SkillDescriptors[] = 
{
	"Worse than none",//less than zero
	"None",				// = 0
	"Minimal",			// < 10
	"Below Average",	// < 20
	"Average",			// < 30
	"Above Average",	// < 40
	"Good",				// < 50
	"Very Good",		// < 60
	"Exceptional",		// < 70
	"Master",			// < 80
	"GrandMaster",		// < 90
	"Perfect",			// < 100
	"SuperHuman",		// = 100
	"SuperHuman",		// >100
};

//attempts to open a file and exits if it fails

FILE *SafeFileOpen(const char *filename, const char *attributestring)
{
	assert(filename);

	FILE *fp;
	fp = fopen(filename, attributestring);

	if(!fp)
	{
		char blarg[128];
	
		sprintf(blarg,"Failed to open file: %s.  Aborting program.",filename);
		SafeExit(blarg);
	}

	return fp;
}

void SafeExit(char *ErrorMessage)
{
	ExitErrorMessage = new char[256];
	sprintf(ExitErrorMessage,"%s",ErrorMessage);
	exit(1);
}



////////////////
///
//  gets a generic data type 
//
///////////////////

float GetFloat(FILE *fp)
{
	char blarg[16];

	int ccount = 0;

	char c = '\0';;
	 
	c = (char)fgetc(fp);

	while(!isdigit(c) && c != '-')
	{
		c = (char)fgetc(fp);
	}

	while(isdigit(c) || c == '.' || c == '-')
	{
		blarg[ccount] = c;
		c = (char)fgetc(fp);
		ccount++;
	}
	
	blarg[ccount] = '\0';

	return (float)atof(blarg);
}

int GetInt(FILE *fp)
{
	char blarg[16];

	int ccount = 0;

	char c = '\0';
	 
	c = (char)fgetc(fp);

	while(!isdigit(c) && c != '-')
	{
		c = (char)fgetc(fp);
	}

	while(isdigit(c) || c == '-')
	{
		blarg[ccount] = c;
		c = (char)fgetc(fp);
		ccount++;
	}
	
	blarg[ccount] = '\0';

	fseek(fp, -1, 1);

	return atoi(blarg);

}

DATA_T GetNumber(FILE *fp, DATA_FIELD_T *dest)
{
	char blarg[16];	//string to hold character representation of number

	int ccount = 0; //counter

	char c;			//read character

	c = (char)fgetc(fp);

	while(!isdigit(c) && c != '-')
	{
		c = (char)fgetc(fp);
	}

	//read in the character representation
	while (c == '-' || isdigit(c) || c == '.')
	{
		blarg[ccount] = c;
		c = (char)fgetc(fp);
		ccount++;
	}

	//terminate the number string
	blarg[ccount] = '\0';

	//convert it to a number
	char *pc;

	//check for a decimal point
	pc = strchr(blarg,'.');

	//if there is a decimal point convert to a float
	if(pc)
	{
		dest->fValue = (float)atof(blarg);
		return DATA_FLOAT;
	}
	else //otherwise convert to an integer
	{
		dest->Value = atoi(blarg);
		return DATA_INT;
	}

	
	return DATA_ERROR;

}

char *GetString(FILE *fp, DATA_FIELD_T *dest)
{
	fpos_t start;
	//get string gets called one character into the string, so seek back one

	//store the start position of the string
	fgetpos(fp,&start);

	int length = 0;

	char c = '\0';

	//count how long the string is
	while (c != DELIMIT_CHARACTER && c != '\n' && c != '\r')
	{
		length++;
		c = (char)fgetc(fp);
	}

	//return to the start of the string
	fsetpos(fp,&start);

	char *returnstring;

	//allocate space for the string, do so in 8 byte increments to avoid memory block issues
	returnstring = new char[length];

	//get the actual string
	fgets(returnstring,length,fp);

	c = (char)fgetc(fp);

	assert((c == DELIMIT_CHARACTER) || (c == '\r') || (c == '\n'));

	//done
	dest->String = returnstring;
	return returnstring;
}

char *GetString(FILE *fp, char Delimitter)
{
	fpos_t start;

	//store the start position of the string
	fgetpos(fp,&start);

	int length = 0;

	char c = '\0';

	//count how long the string is
	do
	{
		length++;
		c = (char)fgetc(fp);
#ifndef NDEBUG
	//validate character;	
	if(c == '[' || c == '(' || (c == '^' && Delimitter != '^'))
	{
		DEBUG_INFO("Bad character found in delimitting string");
		break;
	}
#endif
	}while (c != Delimitter && c != '\n' && c != '\r' && !feof(fp));

	//return to the start of the string
	fsetpos(fp,&start);

	char *returnstring;

	//allocate space for the string, do so in 8 byte increments to avoid memory block issues
	returnstring = new char[length];

	//get the actual string
	fgets(returnstring,length,fp);

	c = (char)fgetc(fp);

	if(c != Delimitter)
	{
		DEBUG_INFO("Problem with delimitted string\n");
		DEBUG_INFO(returnstring);
		DEBUG_INFO("\n");
		delete[] returnstring;
		exit(1);
	}
	//done
	return returnstring;
}


char *GetString(FILE *fp, char *dest)
{
	int length = 0;
	char c = '\0';
	fpos_t start;

	while(c != '"')
	{
		if(c == EOF)
		{
			
		}
		c = fgetc(fp);
	}
	c = '\0';
	
	fgetpos(fp,&start);
		
	while (c != '"' && c != '\n' && c != '\r')
	{
		length++;
		c = fgetc(fp);
	}

	fsetpos(fp,&start);

	fgets(dest,length,fp);

	dest[length] = '\0';
	fgetc(fp);
	
	return dest;


}


char *GetString(FILE *fp)
{
	fpos_t start;
	//get string gets called one character into the string, so seek back one

	//store the start position of the string
	fgetpos(fp,&start);

	int length = 0;

	char c = '\0';

	//count how long the string is
	while (c != DELIMIT_CHARACTER && c != '\n' && c != '\r')
	{
		length++;
		c = (char)fgetc(fp);
	}

	//return to the start of the string
	fsetpos(fp,&start);

	char *returnstring;

	//allocate space for the string, do so in 8 byte increments to avoid memory block issues
	returnstring = new char[length];

	//get the actual string
	fgets(returnstring,length,fp);

	//done
	return returnstring;
}

D3DVECTOR *GetVector(FILE *fp, DATA_FIELD_T *dest)
{
	D3DVECTOR *NewVector;
	NewVector = new D3DVECTOR;
	NewVector->x = GetFloat(fp);
	NewVector->y = GetFloat(fp);
	NewVector->z = GetFloat(fp);
	dest->pVector = NewVector;
	char c;
	c = fgetc(fp);

	assert(c == DELIMIT_CHARACTER);
	return NewVector;

}

DATA_T GetFileData(FILE *fp, DATA_FIELD_T *dest)
{
	char c;
	DATA_T ReturnValue;

	c = fgetc(fp);

	fseek(fp,-1,1);

	if(c == '(')
	{
		dest->pVector = GetVector(fp, dest);
		return DATA_VECTOR;
	}
	
	if(isdigit(c) || c == '-')
	{
		ReturnValue = GetNumber(fp, dest);
		return ReturnValue;
	}

	if(isalpha(c))
	{
		dest->String = GetString(fp, dest);
		return DATA_STRING;
	}

	if(c == DELIMIT_CHARACTER)
	{
		fgetc(fp);
		dest->Value = 0;
		DEBUG_INFO("\n************\n\nDATA FIELD EMPTY\n\n******************\n\n");
		return DATA_NONE;
	}

	return DATA_ERROR;

}

int SaveInt(FILE *fp, int ToSave)
{
	fprintf(fp,"%i",ToSave);
	return TRUE;
}

int SaveFloat(FILE *fp, float ToSave)
{
	fprintf(fp,"%f",ToSave);
	return TRUE;
}

int SaveString(FILE *fp, const char *ToSave)
{
	fprintf(fp,"%s",ToSave);
	return TRUE;

}

int SaveVector(FILE *fp, D3DVECTOR *ToSave)
{
	fprintf(fp,"(%f,%f,%f)",(double)ToSave->x,(double)ToSave->y,(double)ToSave->z);
	return TRUE;
}

float GetDistance(D3DVECTOR *VA,D3DVECTOR *VB)
{
	float Length1;
	float Length2;

	Length1 = VA->x - VB->x;
	Length2 = VA->y - VB->y;
	
	return (float)sqrt((float)((Length1 * Length1) + (Length2 * Length2)));
}

int SeekTo(FILE *fp, const char *id)
{
	assert(fp);

	char c = '\0';
	int n;
	BOOL found = FALSE;

	while (!found && !feof(fp))
	{
	
		while (c != id[0] && !feof(fp))
		{
			c = fgetc(fp);
		}
		
		n = 0;
		while (c == id[n] &&!feof(fp))
		{
			n++;
			c = fgetc(fp);
		}

		if(id[n] == '\0')
		{
			found = TRUE;
		}
	}

	if(!found)
	{
		DEBUG_INFO("Failed to seek to: ");
		DEBUG_INFO(id);
		DEBUG_INFO("\n");
		return FALSE;
	}

	fseek(fp,-1,SEEK_CUR);

	return TRUE;
}

int SeekToSkip(FILE *fp, const char *id)
{
	assert(fp);

	char c = '\0';
	int n;
	BOOL found = FALSE;

	while (!found && !feof(fp))
	{
				
		while (c != id[0] && !feof(fp))
		{
			c = fgetc(fp);

			if(c == '[' && id[0] != '[')
			{
				while (c != ']' && !feof(fp))
				{
					c = fgetc(fp);
				}
			}
			else
			if(c == ';')
			{
				do
				{
					c = fgetc(fp);
				}while (c != ';' && !feof(fp));
			}

		}
		
		n = 0;
		while (c == id[n] &&!feof(fp))
		{
			n++;
			c = fgetc(fp);
		}

		if(id[n] == '\0')
		{
			found = TRUE;
		}
	}

	if(!found)
	{
		DEBUG_INFO("Failed to seek to: ");
		DEBUG_INFO(id);
		DEBUG_INFO("\n");
		return FALSE;
	}

	fseek(fp,-1,SEEK_CUR);

	return TRUE;
}

char *GetPureString(FILE *fp)
{
	fpos_t start;
	//get string gets called one character into the string, so seek back one

	int length = 0;

	char c = ' ';

	while(!isalpha(c))
	{
		c = (char)fgetc(fp);
	}

	fseek(fp,-1, SEEK_CUR);

	//store the start position of the string
	fgetpos(fp,&start);

	//count how long the string is
	while (isalpha((int)c) || isdigit((int)c))
	{
		length++;
		c = (char)fgetc(fp);
	}

	//return to the start of the string
	fsetpos(fp,&start);

	char *returnstring;

	//allocate space for the string, do so in 8 byte increments to avoid memory block issues
	returnstring = new char[length];

	//get the actual string
	fgets(returnstring,length,fp);

	//done
	return returnstring;
}


char *GetStringNoWhite(FILE *fp)
{
	fpos_t start;
	//get string gets called one character into the string, so seek back one

	int length = 0;

	char c = ' ';

	while(!isalpha(c))
	{
		c = (char)fgetc(fp);
	}

	fseek(fp,-1, SEEK_CUR);

	//store the start position of the string
	fgetpos(fp,&start);

	//count how long the string is
	while (!isspace((int)c))
	{
		length++;
		c = (char)fgetc(fp);
	}

	//return to the start of the string
	fsetpos(fp,&start);

	char *returnstring;

	//allocate space for the string
	returnstring = new char[length];

	//get the actual string
	fgets(returnstring, length, fp);

	//done
	return returnstring;
}

DIRECTION_T	FindFacing(D3DVECTOR *pvFrom, D3DVECTOR *pvTo)
{
	float angle;

	D3DVECTOR v1;
	D3DVECTOR v2;

	v1.x = 0.0f;
	v1.y = 1.0f;
	v1.z = 0.0f;
	
	v2.z = 0;

	v2.x = pvTo->x - pvFrom->x;
	v2.y = -(pvTo->y - pvFrom->y);

	angle = RadToDeg(GetAngle(&v1,&v2));

	int Angle = (int)angle;

	if(Angle < 23)
	{
		return NORTH;
	}
	else
	if(Angle < 68)
	{
		return NORTHEAST;
	}
	else
	if(Angle < 113)
	{
		return EAST;
	}
	else
	if(Angle < 158)
	{ 
		return SOUTHEAST;
	}
	else
	if(Angle < 203)
	{
		return SOUTH;
	}
	else
	if(Angle < 248)
	{
		return SOUTHWEST;
	}
	else
	if(Angle < 293)
	{
		return WEST;
	}
	else
	if(Angle < 337)
	{
		return NORTHWEST;
	}
	else
	{
		return NORTH;
	}
	
	return DIR_NONE;
}

DIRECTION_T	FindFacing(float Angle)
{
	int NewAngle;
	NewAngle = (int)RadToDeg(Angle);

	if(NewAngle < 23)
	{
		return NORTH;
	}
	else
	if(NewAngle < 68)
	{
		return NORTHEAST;
	}
	else
	if(NewAngle < 113)
	{
		return EAST;
	}
	else
	if(NewAngle < 158)
	{ 
		return SOUTHEAST;
	}
	else
	if(NewAngle < 203)
	{
		return SOUTH;
	}
	else
	if(NewAngle < 248)
	{
		return SOUTHWEST;
	}
	else
	if(NewAngle < 293)
	{
		return WEST;
	}
	else
	if(NewAngle < 337)
	{
		return NORTHWEST;
	}
	else
	{
		return NORTH;
	}

}


int ConvertToPercent(float f)
{

	return (int)(f * 100.0f);
}

float ConvertFromPercent(int n)
{
	return (float)n/100.0f;
}


float GetAngle(D3DVECTOR *v1, D3DVECTOR *v2)
{
//	float CosAngle = DotProduct(*v1,*v2)/((Magnitude(*v1)*Magnitude(*v2)));
//	float Angle = (float)acos(CosAngle);

	float DegAngle, Angle;

	D3DVECTOR vNorth(0.0f,1.0f,0.0f);
	float AngleA;
	float AngleB;

	AngleA = (float)acos(DotProduct(vNorth,*v1)/Magnitude(*v1));
	AngleB = (float)acos(DotProduct(vNorth,*v2)/Magnitude(*v2));

	if(v1->x < 0.0f) AngleA = PI_MUL_2 - AngleA;
	if(v2->x < 0.0f) AngleB = PI_MUL_2 - AngleB;
	 
	Angle = AngleB - AngleA;

	if(Angle < 0)
	{
		Angle = PI_MUL_2 + Angle;
	}
	
	DegAngle = (float)RadToDeg(Angle);

	return Angle;


//	return Angle;
}

//distance function
//point to point
float GetDist(float x1,float y1, float x2, float y2)
{
	return (float)sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
}

//point to line
//point to test, then start and end points of line
float GetDist(float x, float y, float x1, float y1, float x2, float y2)
{
	float dist;
	float angle;
	dist = GetDist(x,y,x1,y1);
	D3DVECTOR a;
	D3DVECTOR b;
	a.x = x2 - x1;
	a.y = y2 - y1;
	b.x = x - x1;
	b.y = y - y1;
	
	angle = GetAngle(&a,&b);

	float top, bottom, left, right;
	if(y1 > y2)
	{
		top = y2;
		bottom = y1;
	}
	else
	{
		top = y1;
		bottom = y2;
	}
	if(x1 > x2)
	{
		right = x1;
		left = x2;
	}
	else
	{
		right = x2;
		left = x1;
	}

	float nx = x1 + (dist *(float)sin(angle));
	float ny = y1 + (dist *(float)cos(angle));

	if(nx < left || nx > right || ny < top || ny > bottom)
	{
		return 10000.0f;
	}
	
	return dist*(float)sin(angle);

}


float AngleDifCW(const float Angle1, const float Angle2)
{
	if(Angle2 > Angle1)
	{
		return Angle2 - Angle1;
	}
	else
	{
		return Angle2 + (PI_MUL_2 - Angle1);
	}
}


float AngleDifCCW(const float Angle1, const float Angle2)
{
	if(Angle1 > Angle2)
	{
		return Angle1 - Angle2;
	}
	else
	{
		return Angle1 + (PI_MUL_2 - Angle2);
	}
}


//get next printable character from a file, ignoring comments
char GetChar(FILE *fp)
{
	char c;

	do
	{
		c = fgetc(fp);
		if(c == ';')
		{
			do
			{
				c = fgetc(fp);
			}
			while(c != ';');
			c = fgetc(fp);
		}
	}while(!feof(fp) && isspace((int)c));
	

	return c;
}


char *GetRangeDescriptor(int Num, int Min, int Max, RANGE_T RangeType)
{
	int RangeIndex;
	int RangeLength = Max - Min;
	int RangeNum = Num - Min;

	assert(RangeLength);

	//calculate where in the range the number is
	if(RangeNum < 0)
	{
		RangeIndex = RANGE_DESCRIPTOR_START;
	}
	else
	if(RangeNum == 0)
	{
		RangeIndex = RANGE_DESCRIPTOR_START + 1;
	}
	else
	if(RangeNum == RangeLength)
	{
		RangeIndex = RANGE_DESCRIPTOR_END - 1;
	}
	else
	if(RangeNum > RangeLength)
	{
		RangeIndex = RANGE_DESCRIPTOR_END;
	}
	else
	{
		RangeIndex = (int)(10.0f *((float)RangeNum/(float)RangeLength)) + 2;
	}

	switch(RangeType)
	{
		case RANGE_STAT:
			return (char *)StatDescriptors[RangeIndex];
		case RANGE_SKILL:
			return (char *)SkillDescriptors[RangeIndex];
		case RANGE_HEALTH:
			return (char *)HealthDescriptors[RangeIndex];
		case RANGE_REST:
			return (char *)RestDescriptors[RangeIndex];
		default:
			break;
	}

	return NULL;
}

void ConvertToCapitals(char *String)
{
	int n;
	n = 0;

	while(String[n] != '\0')
	{
		if(isalpha((int)String[n]))
			String[n] = (char)toupper((int)String[n]);
		n++;
	}
}

void ConvertToLowerCase(char *String)
{
	int n;
	n = 0;

	while(String[n] != '\0')
	{
		if(isalpha((int)String[n]))
			String[n] = (char)tolower((int)String[n]);
		n++;
	}
}

float PointToLine(D3DVECTOR *pPoint, D3DVECTOR *pStart, D3DVECTOR *pEnd)
{
	D3DVECTOR LineRay, PointRay;
	LineRay = *pEnd - *pStart;

//	LineRay = Normalize(LineRay);

	PointRay = *pPoint - *pStart;
//	float Distance;

//	Distance = DotProduct(PointRay, LineRay);

//	LineRay = LineRay * Distance;

//	LineRay += *pStart;

	float Angle;

	Angle = (float)acos(DotProduct(Normalize(PointRay),Normalize(LineRay)));

	float Hypotenuse = Magnitude(PointRay);

	return (float)sin(Angle)*Hypotenuse;

//	return GetDistance(&LineRay, pPoint);
}

int PointToLineIntersect(D3DVECTOR *vOut, D3DVECTOR *pPoint, D3DVECTOR *pStart, D3DVECTOR *pEnd)
{
	D3DVECTOR LineRay, PointRay;
	LineRay = *pEnd - *pStart;

	LineRay = Normalize(LineRay);

	PointRay = *pPoint - *pStart;

	float Distance;

	Distance = DotProduct(PointRay, LineRay);

	LineRay = LineRay * Distance;

	LineRay += *pStart;

	*vOut = LineRay; 

	return TRUE;

}

void ScaleRect(RECT *rToScale, RECT *Scalar, RECT *Base)
{
	float xfactor;

	xfactor = (float)(Base->right - Base->left) / (float)(Scalar->right - Scalar->left);

	float yfactor;

	yfactor = (float)(Base->bottom - Base->top) / (float)(Scalar->bottom - Scalar->top);
	

	rToScale->left = (int)((float)rToScale->left * xfactor);
	rToScale->right = (int)((float)rToScale->right * xfactor);
	rToScale->top = (int)((float)rToScale->top * yfactor);
	rToScale->bottom = (int)((float)rToScale->bottom * yfactor);
}

void LoadRect(RECT *rLoad, FILE *fp)
{
	rLoad->left = GetInt(fp);
	rLoad->top = GetInt(fp);
	rLoad->right = rLoad->left + GetInt(fp);
	rLoad->bottom = rLoad->top + GetInt(fp);
}

char *GetHelp(const char *HelpID)
{
	char *retstring = NULL;
	char UpCase[64];
	char Label[64];

	sprintf(Label,"[%s]",HelpID);

	FILE *fp;
	fp = SafeFileOpen("help.txt","rt");
	if(SeekTo(fp,Label))
	{
		SeekTo(fp,"\"");
		retstring = GetString(fp,'\"');
	}
	else
	{
		fseek(fp,0,0);
		strcpy(UpCase, HelpID);
		ConvertToCapitals(UpCase);
		sprintf(Label,"[%s]",UpCase);
		if(SeekTo(fp,Label))
		{
			SeekTo(fp,"\"");
			retstring = GetString(fp,'\"');
		}
	}

	fclose(fp);
	if(!retstring)
	{
		retstring = new char[128];
		sprintf(retstring,"help topic not available: %s\n", HelpID);
	}
	return retstring;
}

float ManhattanDistance(D3DVECTOR *VA, D3DVECTOR *VB)
{

	float XDif;
	float YDif;
	float ZDif;

	if(VA->x > VB->x)
	{
		XDif = VA->x - VB->x;
	}
	else
	{
		XDif = VB->x - VA->x;
	}

	if(VA->y > VB->y)
	{
		YDif = VA->y - VB->y;
	}
	else
	{
		YDif = VB->y - VA->y;
	}

	if(VA->z > VB->z)
	{
		ZDif = VA->z - VB->z;
	}
	else
	{
		ZDif = VB->z - VA->z;
	}

	return XDif + YDif + ZDif;
}

BOOL Triangle2DIntersect(D3DVECTOR *vPoint, D3DVERTEX *vxA, D3DVERTEX *vxB, D3DVERTEX *vxC)
{
	D3DVECTOR vTest;
	vTest = *vPoint;
	vTest.z = 0.0f;

	D3DVECTOR vA;
	vA.x = vxA->x - vTest.x;
	vA.y = vxA->y - vTest.y;
	vA.z = 0.0f;

	vA = Normalize(vA);

	D3DVECTOR vB;
	vB.x = vxB->x - vTest.x;
	vB.y = vxB->y - vTest.y;
	vB.z = 0.0f;

	vB= Normalize(vB);
	
	D3DVECTOR vC;
	vC.x = vxC->x - vTest.x;
	vC.y = vxC->y - vTest.y;
	vC.z = 0.0f;

	vC = Normalize(vC);

	float Angle = 0.0f;
	float Angle1 = 0.0f;
	float Angle2 = 0.0f;
	float Angle3 = 0.0f;

	//had to reverse my angle calculations from what I thought they should be...a-b-c to c-b-a???
	Angle1 = (float)acos(DotProduct(vA,vB));
	Angle2 = (float)acos(DotProduct(vB,vC));
	Angle3 = (float)acos(DotProduct(vC,vA));

	Angle = Angle1 + Angle2 + Angle3;

/*	if(Angle1 < 0.0f || Angle2 < 0.0f || Angle3 < 0.0f)
	{
		return FALSE;
	}
*/
	if(Angle > (PI_MUL_2 + 0.03f) || Angle < (PI_MUL_2 - 0.03f))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

BOOL Triangle2DIntersect(D3DVECTOR *vPoint, D3DVERTEX *VertArray, unsigned short *Indexes)
{
	return Triangle2DIntersect(vPoint, &VertArray[Indexes[0]], &VertArray[Indexes[1]], &VertArray[Indexes[2]]);
}

BOOL Triangle3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *vxA, D3DVERTEX *vxB, D3DVERTEX *vxC)
{
	D3DVECTOR vSideA;
	D3DVECTOR vSideB;
	D3DVECTOR vL1;
	D3DVECTOR vL2;
	D3DVECTOR vL3;
	D3DVECTOR vNormA;

	D3DVECTOR vA;
	vA.x = vxA->x;
	vA.y = vxA->y;
	vA.z = vxA->z;

	D3DVECTOR vB;
	vB.x = vxB->x;
	vB.y = vxB->y;
	vB.z = vxB->z;
	
	D3DVECTOR vC;
	vC.x = vxC->x;
	vC.y = vxC->y;
	vC.z = vxC->z;

	D3DVECTOR vIntersect;

	D3DVECTOR vRay;
	vRay = *vRayEnd - *vRayStart;

	float DistToPlane;
	float ProjectedLength;
	float Ratio;

	float AngleTotal;

	vSideA = vA - vB;

	vSideB = vC - vB;

	vNormA = Normalize(CrossProduct(vSideA,vSideB));
	
	//vNorm now contains the normal to the upper left triangle of this tile
	vL1 = vB - *vRayStart;

	DistToPlane = DotProduct(vL1,vNormA);
	
	ProjectedLength = DotProduct((*vRayEnd - *vRayStart),vNormA);
	Ratio = DistToPlane / ProjectedLength;

	vIntersect = *vRayStart + (vRay * Ratio);
	
	AngleTotal = 0.0f;
	//now check to see if the interect is in the polygon;
	vL1 = vA - vIntersect;
	vL1 = Normalize(vL1);

	vL2 = vB - vIntersect;
	vL2 = Normalize(vL2);

	vL3 = vC - vIntersect;
	vL3 = Normalize(vL3);

	AngleTotal += (float)acos(DotProduct(vL1,vL2));
	AngleTotal += (float)acos(DotProduct(vL2,vL3));
	AngleTotal += (float)acos(DotProduct(vL3,vL1));

	if(AngleTotal > 6.27f && AngleTotal < 6.29f)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL Triangle3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *VertArray, unsigned short *Indexes)
{
	return Triangle3DIntersect(vRayStart, vRayEnd, &VertArray[Indexes[0]], &VertArray[Indexes[1]], &VertArray[Indexes[2]]);
}

BOOL Quad3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *VertArray, unsigned short *Indexes)
{
	return Quad3DIntersect(vRayStart, vRayEnd, &VertArray[Indexes[0]], &VertArray[Indexes[1]], &VertArray[Indexes[2]], &VertArray[Indexes[4]]);
}

BOOL Quad3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *vxA, D3DVERTEX *vxB, D3DVERTEX *vxC, D3DVERTEX *vxD)
{
	
	return FALSE;
}


