//meshfx.h
//an fx structure used to do funky effects requiring a bas mesh, expanding spheres, etc...
#ifndef MESHFX_H
#define MESHFX_H

#include "objects.h"

class ZSMeshFX : public Object
{
private:
	float fExpansionRate;
	float fAlpha;
	float fAlphaRate;
	float fRed;
	float fRedRate;
	float fGreen;
	float fGreenRate;
	float fBlue;
	float fBlueRate;
	int Start;
	int End;
	Object *pLink;
	BOOL Lighting;
	D3DMATERIAL7 Material;

public:
//accessors
	float GetExpansionRate() { return fExpansionRate; };
	float GetAlpha() { return fAlpha; };
	float GetAlphaRate() { return fAlphaRate; };
	float GetRed() { return fRed; };
	float GetRedRate() { return fRedRate; };
	float GetGreen() { return fGreen; };
	float GetGreenRate() { return fGreenRate; };
	float GetBlue() { return fBlue; };
	float GetBlueRate() { return fBlueRate; };
	int GetStart() { return Start; };
	int GetEnd() { return End; };
	Object *GetLink() { return pLink; };
	BOOL IsLit() { return Lighting; }

//mutators
	void SetExpansionRate(float fNewValue) {  fExpansionRate = fNewValue; };
	void SetAlpha(float fNewValue) {  fAlpha = fNewValue; };
	void SetAlphaRate(float fNewValue) {  fAlphaRate = fNewValue; };
	void SetRed(float fNewValue) {  fRed = fNewValue; };
	void SetRedRate(float fNewValue) {  fRedRate = fNewValue; };
	void SetGreen(float fNewValue) {  fGreen = fNewValue; };
	void SetGreenRate(float fNewValue) {  fGreenRate = fNewValue; };
	void SetBlue(float fNewValue) {  fBlue = fNewValue; };
	void SetBlueRate(float fNewValue) {  fBlueRate = fNewValue; };
	void SetStart(int NewValue) {  Start = NewValue; };
	void SetEnd(int NewValue) {  End = NewValue; };
	void SetLink(Object *pNewLink) {  pLink = pNewLink; };
	void SetLighting(BOOL bNewLit) { Lighting = bNewLit; };

	D3DMATERIAL7* GetMaterial() { return &Material; }

	BOOL AdvanceFrame();

//display
	void Draw();

//constructor
	ZSMeshFX();

};

#endif