#include "registration.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include "resource.h"

unsigned char HardwareID[256];
unsigned char RegistrationKey[256];

void GetHardwareID()
{
	unsigned char TempID[256];
	
	DWORD dwID;
	GetVolumeInformation("C:\\",NULL,0,&dwID,NULL,NULL,NULL, 0);

	char blarg[256];
	int first = 0;

	memset(blarg,'\0',256);
	memset(TempID,'0',256);
	sprintf(blarg,"%i",dwID);
	for(int n = 0; n < 16; n++)
	{
		if(blarg[n] == '\0')
		{
			if(!first)
				first = n;
			TempID[n] = blarg[n - first];
		}
		else
		{
			TempID[n] = blarg[n];
		}
	}
	TempID[16] = '\0';
	HardwareID[16] = '\0';

	MaskStrings(TempID,(unsigned char *)HardWareMaskID,HardwareID);

	return;
}


BOOL ValidateHardwareID(unsigned char *tovalidate)
{

	return TRUE;
}

inline void GenerateKey(unsigned char *Destination, unsigned char *From)
{
	MaskStrings(From, (unsigned char *)LockKey, Destination);

	for(int n = 1; n < 15; n++)
	{
		Destination[n] = (Destination[n] * HardWareMaskID[n]) % 10 + 48;
	}

}

void GetRegisteredKey()
{
	HKEY WindowsIDKey;
	DWORD Length;
	HRESULT hResult;

	FILE *fp;
	
	
	for(int n = 0; n < 16; n++)
	{
		RegistrationKey[n] = '0';
	}
	RegistrationKey[16] = '\0';

	hResult = 
		RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			"Software\\ZeroSum\\Prelude",
		  0,
		  KEY_QUERY_VALUE, // security access mask
		  &WindowsIDKey    // address of handle to open key
		  );

	if(hResult != ERROR_SUCCESS)
	{
		fp = fopen ("key.bin","rb");\
		if(fp)
		{
			fread(RegistrationKey,sizeof(char),16,fp);
			fclose(fp);
		}
		return;
	}
	
	RegQueryValueEx(
		WindowsIDKey,	
	   	"RegistrationCode",
		0,
		NULL,
		RegistrationKey,
		&Length
	);


	RegCloseKey(WindowsIDKey);

	if(!strcmp((char *)RegistrationKey,"0000000000000000"))
	{
		fp = fopen ("key.bin","rb");
		if(fp)
		{
			fread(RegistrationKey,sizeof(char),16,fp);
			fclose(fp);
		}
	}

}

void RegisterKey(unsigned char *toregister)
{
	HKEY WindowsIDKey;
	DWORD Length;
	HRESULT hResult;

	hResult = 
		RegCreateKeyEx( 
				HKEY_LOCAL_MACHINE,
				"Software\\ZeroSum\\Prelude",
			  0,
			  NULL,
			  REG_OPTION_NON_VOLATILE,
			  KEY_WRITE, // security access mask
			  NULL,
			  &WindowsIDKey,    // address of handle to open key
			  &Length 
			  );
	if(hResult != ERROR_SUCCESS)
	{
		for(int n = 0; n < 16; n++)
		{
			RegistrationKey[n] = 0;
		}
		RegistrationKey[16] = '\0';
		return;
	}
	
	RegSetValueEx(
		WindowsIDKey,	
	   	"RegistrationCode",
		0,
		REG_SZ,
		toregister,
		17
	);

	RegFlushKey(WindowsIDKey);
	
	RegCloseKey(WindowsIDKey);

	FILE *fp;
	fp = fopen("key.bin","wb");
	if(fp)
	{
		fwrite(RegistrationKey,sizeof(char),16,fp);
		fclose(fp);
	}

}

inline BOOL ValidateKey(unsigned char *keytovalidate)
{
   unsigned char TempKey[64];
   unsigned char *vkey;
   if(!keytovalidate)
		vkey = RegistrationKey;
   else
		vkey = keytovalidate;

   GenerateKey(TempKey,HardwareID);
   if(!strcmp((char *)TempKey,(char *)vkey))
   {
		return TRUE;
   }
     
   return TRUE;
}

void MaskStrings(unsigned char *string, unsigned char *mask, unsigned char *result)
{
	int StringCount;
	int adder;
	int Length;
	Length = strlen((char *)string);
	int MaskLength;
	MaskLength = strlen((char *)mask);

	for(StringCount = 0; StringCount < Length; StringCount++)
	{
		if(string[StringCount] % 2)
			adder = string[StringCount] + mask[StringCount % MaskLength];
		else
			adder = string[StringCount] - mask[StringCount % MaskLength];
		if(adder < 0) adder *= -1;
		result[StringCount] = (char)((adder % 10) + 48);
		if((!isdigit(result[StringCount]) && !isalpha(result[StringCount])))
		{
			result[StringCount] = string[StringCount];
		}
	}
	result[Length] = '\0';
}

//---------------------------------------------------------------------
LRESULT CALLBACK DlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hCtl;

	switch (msg) {

		case WM_INITDIALOG:
		
			hCtl = GetDlgItem(hwnd,IDC_CUSTOMERID);
			Edit_SetText(hCtl, (char *)HardwareID);


	     	 return TRUE;

		case WM_CLOSE:

			 EndDialog(hwnd, IDCANCEL);

			 break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) 
			{

		        case IDCANCEL:
                     EndDialog(hwnd, IDCANCEL);
					 break;

				case IDREGISTER:
					::ShellExecute(NULL, NULL, "http://www.zero-sum.com", NULL, NULL, SW_SHOWNORMAL); 
					break;

				case IDPLAY:
					//play without registering
					hCtl = GetDlgItem(hwnd,IDC_REGISTRATIONKEY);
					Edit_GetText(hCtl, (char *)RegistrationKey, 17);
					RegistrationKey[16] = '\0';
					if(ValidateKey(RegistrationKey))
					{
						RegisterKey(RegistrationKey);
						MessageBox(NULL, "Registration Successful!","Registration valid.", MB_OK);
					}
					else
					{
						MessageBox(NULL, "Continuing with trial version","Registration not valid.",MB_OK);
					}
					

					EndDialog(hwnd, IDOK);

					break;

				default:
					break;

            }

            break;
    }

    return FALSE;
}




HRESULT Register()
{
	HRESULT hres;
	
	GetHardwareID();
	GetRegisteredKey();
	if(!ValidateKey(RegistrationKey))
		hres = DialogBox(NULL,(LPSTR)MAKEINTRESOURCE(IDD_DIALOG1),NULL,(DLGPROC)DlgProc);

	if(hres == IDCANCEL)
	{
		exit(1);
	}
	return hres;
}

