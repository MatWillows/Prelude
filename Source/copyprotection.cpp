

//******************************************************************
// CopyProtection.cpp
//************************

//license checking code
#include "CopyProtection.h"
#define LICENSE_FILE  "Prelude.lic.txt"   //Replace with your license file name
#define ENCRYPTION_KEY  "17CFC541DAB011D6"   //Replace with your encryption key (from GenKey.exe)
const double VERSION = 1.0;

bool InstallFullLicense(Alcatraz::eRETURNCODE& eRetCode)
{
    char sFullLicenseFile[MAX_PATH] = "c:\\";
    strcat(sFullLicenseFile, LICENSE_FILE);

    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile(sFullLicenseFile, &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE)
        return false;
    
    char sLicenseFile[MAX_PATH] = "";
    strcat(sLicenseFile, getenv("WINDIR"));
    strcat(sLicenseFile, "\\");
    strcat(sLicenseFile, LICENSE_FILE);

    eRetCode = Alcatraz::InstallFullLicense(sFullLicenseFile, sLicenseFile);
    if (Alcatraz::IS_OK == eRetCode)
        return true;

    return false;
}

Alcatraz::eRETURNCODE CheckLicense()
{
    char sLicenseFile[MAX_PATH] = "";
    strcat(sLicenseFile, LICENSE_FILE);

////////////////////////////////////////////////////////////////////
    Alcatraz alcatraz;
    Alcatraz::eRETURNCODE eRetCode =
        alcatraz.LicenseIsValid(sLicenseFile, TEXT(ENCRYPTION_KEY), VERSION);

    if (Alcatraz::EVALUATION_LICENSE == eRetCode && InstallFullLicense(eRetCode))
        eRetCode = alcatraz.LicenseIsValid(sLicenseFile, TEXT(ENCRYPTION_KEY), VERSION);
//////////////////////
// if you want to use the User-Locked licensing scheme, the above lines will be
//  Alcatraz::eRETURNCODE eRetCode =
//      alcatraz.LicenseIsValid(sLicenseFile, TEXT(ENCRYPTION_KEY), VERSION, sCustomerIdentifier, USER_LOCK);

//    if (Alcatraz::EVALUATION_LICENSE == eRetCode && InstallFullLicense(eRetCode))
//        eRetCode = alcatraz.LicenseIsValid(sLicenseFile, TEXT(ENCRYPTION_KEY), VERSION, sCustomerIdentifier, USER_LOCK);
////////////////////////////////////////////////////////////////////

    char sRetCodeMsg[ALCATRAZ_MAX_RETCODEMSG] = "";
    if (Alcatraz::EVALUATION_LICENSE == eRetCode)
    {
        int nDays = -1;
        Alcatraz::eRETURNCODE eRetCode1 = alcatraz.DaysToLicenseExpiry(sLicenseFile, TEXT(ENCRYPTION_KEY), &nDays);
        if (Alcatraz::IS_OK == eRetCode1)
        {
            if (6 > nDays)		// Warn the user
            {
                if (1 > nDays)
                    sprintf(sRetCodeMsg, "License Has Expired");
                else
                {
                    sprintf(sRetCodeMsg, "License Expires In %d Day", nDays);
                    if (1 < nDays) strcat(sRetCodeMsg, "s");
                }
                strcat(sRetCodeMsg, "\nPlease visit www.mysite.com to purchase the application");
            }
        }
        else
            Alcatraz::GetCopyProtectionMsg(eRetCode1, sRetCodeMsg);		
    }
    else
    {
        if (Alcatraz::LICENSE_IS_VALID != eRetCode)
            Alcatraz::GetCopyProtectionMsg(eRetCode, sRetCodeMsg);
    }

    if (0 != strcmp(sRetCodeMsg, ""))
    {
        ::MessageBox(NULL,
		   sRetCodeMsg,
		   "Copy-Protection Message",
		   MB_OK | MB_ICONERROR);
    }

    return eRetCode;
}

Alcatraz::eRETURNCODE TransferLicense(char const* sEvalLicenseFile)
{
    char sFullLicenseFile[MAX_PATH] = "";
    strcat(sFullLicenseFile, LICENSE_FILE);

    Alcatraz alcatraz;
    Alcatraz::eRETURNCODE eRetCode =
        alcatraz.TransferLicense(sFullLicenseFile, TEXT(ENCRYPTION_KEY), sEvalLicenseFile);

    char sRetCodeMsg[ALCATRAZ_MAX_RETCODEMSG] = "License Transfer Succeeded";
    if (Alcatraz::IS_OK != eRetCode)
    {
        Alcatraz::GetCopyProtectionMsg(eRetCode, sRetCodeMsg);
        strcat(sRetCodeMsg, "\nLicense Transfer Failed");
    }

    ::MessageBox(
        NULL,
        sRetCodeMsg,
        "Copy-Protection Message",
        MB_OK | MB_ICONERROR);

    return eRetCode;
}

