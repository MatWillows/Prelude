#ifndef REGISTRATION_H
#define REGISTRAITON_H

#include <windows.h>

#define LockKey "abcd0123efgh4567"
#define HardWareMaskID "qwertasdflkjhzxa"

extern unsigned char HardwareID[];
extern unsigned char RegistrationKey[];

void GetHardwareID();
void GetOldHardwareID();
BOOL ValidateHardwareID(unsigned char *tovalidate);
inline void GenerateKey(unsigned char *Destination, unsigned char *From);
void GetRegisteredKey();
void RegisterKey(unsigned char *toregister);
inline BOOL ValidateKey(unsigned char *keytovalidate);
void MaskStrings(unsigned char *string, unsigned char *mask, unsigned char *result);

HRESULT Register();


#endif
