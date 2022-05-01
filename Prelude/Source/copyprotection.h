

//******************************************************************
// CopyProtection.h
//************************

#ifndef _ALCATRAZ_INCLUDE_
#define _ALCATRAZ_INCLUDE_
    #include "Alcatraz.h"
#endif

bool InstallFullLicense(Alcatraz::eRETURNCODE& eRetCode);
Alcatraz::eRETURNCODE CheckLicense();
Alcatraz::eRETURNCODE TransferLicense(char const* sEvalLicenseFile);
//*****************************************************************
