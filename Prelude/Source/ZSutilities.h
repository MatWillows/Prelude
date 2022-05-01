//utilities.h 
//generic utility functions
//get something from a file, etc...
//should probably be broken into two files, file i/o and 
//		graphics utilities
//
//
/////////////
#ifndef UTILITIES_H
#define UTILITIES_H

#include "defs.h"
#include "registration.h"

#ifndef D3D_OVERLOADS
#define D3D_OVERLOADS
#endif

#include <d3d.h>

#define RANGE_DESCRIPTOR_START	0
#define RANGE_DESCRIPTOR_END		13

#define XYWH(x)	x.left, x.top, x.right - x.left, x.bottom - x.top

extern const char *StatDescriptors[]; 

extern const char *HealthDescriptors[];

extern const char *RestDescriptors[];

extern const char *SkillDescriptors[];


typedef enum
{
	RANGE_NONE,
	RANGE_STAT,
	RANGE_HEALTH,
	RANGE_SKILL,
	RANGE_REST,
} RANGE_T;

FILE *SafeFileOpen(const char *filename, const char *attributestring);
void SafeExit(char *ErrorMessage);

int GetInt(FILE *fp);

float GetFloat(FILE *fp);

DATA_T GetNumber(FILE *fp, DATA_FIELD_T *dest);

char *GetString(FILE *fp);

char *GetString(FILE *fp, DATA_FIELD_T *dest);

char *GetString(FILE *fp, char *dest);

char *GetString(FILE *fp, char Delimitter);

char *GetString(FILE *fp, char *dest, char Delimitter);

D3DVECTOR *GetVector(FILE *fp);

D3DVECTOR *GetVector(FILE *fp, DATA_FIELD_T *dest);

DATA_T GetFileData(FILE *fp, DATA_FIELD_T *dest);

int SaveInt(FILE *fp, int ToSave);

int SaveFloat(FILE *fp, float ToSave);

int SaveString(FILE *fp, const char *ToSave);

int SaveVector(FILE *fp, D3DVECTOR *ToSave);

float GetDistance(D3DVECTOR *VA, D3DVECTOR *VB);

int SeekTo(FILE *fp, const char *id);
int SeekToSkip(FILE *fp, const char *id);

char *GetStringNoWhite(FILE *fp);
char *GetPureString(FILE *fp);

DIRECTION_T	FindFacing(D3DVECTOR *pvFrom, D3DVECTOR *pvTo);
DIRECTION_T	FindFacing(float Angle);

int ConvertToPercent(float f);

float ConvertFromPercent(int n);

float GetAngle(D3DVECTOR *v1, D3DVECTOR *v2);

char GetChar(FILE *fp);

char *GetRangeDescriptor(int Num, int Min, int Max, RANGE_T RangeType);

float GetDistance(D3DVECTOR *VA,D3DVECTOR *VB);

float AngleDifCW(const float Angle1, const float Angle2);
float AngleDifCCW(const float Angle1, const float Angle2);

int ConvertToPercent(float f);

float ConvertFromPercent(int n);

void Describe(char *DescribeText);
void ClearDescribe();

void ConvertToCapitals(char *String);
void ConvertToLowerCase(char *String);

float PointToLine(D3DVECTOR *pPoint, D3DVECTOR *pStart, D3DVECTOR *pEnd);

int PointToLineIntersect(D3DVECTOR *vOut, D3DVECTOR *pPoint, D3DVECTOR *pStart, D3DVECTOR *pEnd);

void ScaleRect(RECT *rToScale, RECT *Base, RECT *Scalar);

void LoadRect(RECT *rToLoad, FILE *fp);

char *GetHelp(const char *HelpID);

float ManhattanDistance(D3DVECTOR *VA, D3DVECTOR *VB);

BOOL Triangle2DIntersect(D3DVECTOR *vPoint, D3DVERTEX *vxA, D3DVERTEX *vxB, D3DVERTEX *vxC);
BOOL Triangle2DIntersect(D3DVECTOR *vPoint, D3DVERTEX *VertArry, unsigned short *Indexes);

BOOL Triangle3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *VertArry, unsigned short *Indexes);
BOOL Triangle3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *vxA, D3DVERTEX *vxB, D3DVERTEX *vxC);

BOOL Quad3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *VertArry, unsigned short *Indexes);
BOOL Quad3DIntersect(D3DVECTOR *vRayStart, D3DVECTOR *vRayEnd, D3DVERTEX *vxA, D3DVERTEX *vxB, D3DVERTEX *vxC, D3DVERTEX *vxD);

#endif
