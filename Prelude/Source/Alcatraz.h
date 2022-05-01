// Alcatraz Class Declaration
#ifndef _ALCATRAZ__A6F87598_23E0_11D2_BF20_000000000000__INCLUDED_ 
#define _ALCATRAZ__A6F87598_23E0_11D2_BF20_000000000000__INCLUDED_

#include "defs.h"

#define ALCATRAZ_MAX_ENCRYPTION_KEYSIZE		16
#define ALCATRAZ_MAX_USERNAME				50
#define ALCATRAZ_MAX_LICFILESIZE			800
#define ALCATRAZ_MAX_RETCODEMSG				500

class Alcatraz
{
public:
	Alcatraz() {};
	~Alcatraz() {};

	enum eRETURNCODE
	{
		LICENSE_IS_VALID,
		EVALUATION_LICENSE,
		CANNOT_READ_LOCKFILE,
		CPU_ID_IS_INVALID,
		SYSTEM_TIME_IS_INVALID,
		LOCK_FILE_IS_INVALID,
		LOCK_DATE_IS_INVALID,
		LICENSE_EXPIRES_IN_A_WEEK,	// Deprecated functionality
		INVALID_ENCRYPTED_DATA,
		INVALID_SOFTWARE_VERSION,
		IS_OK,
		USER_IS_INVALID,
		INVALID_ALCATRAZ_LIC,
		INTERNAL_ERROR_101,
		INTERNAL_ERROR_102,
		INTERNAL_ERROR_103,
	};

	enum eLOCK_TYPE
	{
		MACHINE_LOCK,
		USER_LOCK,
	};

	// API for checking machine locked licenses
	eRETURNCODE LicenseIsValid(char const* sLicFileName, 
							   char const* sEncryptionKey, 
							   double dProgVersion, 
							   char const* sUser = NULL, 
							   eLOCK_TYPE eLockType = MACHINE_LOCK);

	eRETURNCODE DaysToLicenseExpiry(char const* sLicFileName, 
									char const* sEncryptionKey,
									int* pnDays);

	// API for facilitating installation of full license
	static eRETURNCODE InstallFullLicense(char const* sFullLicFileName, 
										  char const* sEvalLicFileName); 

	// API for checking runtime licenses
	static eRETURNCODE GetLicFileData(char const* sLicFileName, 
							   char* sLicFileData);

	eRETURNCODE RuntimeLicenseIsValid(char const* sLicFileData, 
									  char const* sEncryptionKey, 
									  double dProgVersion,
									  char const* sUser = NULL);

	eRETURNCODE DaysToRuntimeLicenseExpiry(char const* sLicFileData, 
										   char const* sEncryptionKey,
										   int* pnDays);

	// API for converting the return code into a human-readable message
	static void GetCopyProtectionMsg(eRETURNCODE eRetCode, 
									 char* sRetCodeMsg);

	// API for transfering machine locked licenses
	eRETURNCODE TransferLicense(char const* sFullLicFileName, 
							    char const* sEncryptionKey, 
							    char const* sEvalLicFileName); 

	// deprecated interfaces
	eRETURNCODE LicenseExpiresInOneWeek(char const* sLicFileName, 
										char const* sEncryptionKey);

	eRETURNCODE	RuntimeLicenseExpiresInOneWeek(char const* sLicFileData, 
											   char const* sEncryptionKey);
};

#endif	// _ALCATRAZ__A6F87598_23E0_11D2_BF20_000000000000__INCLUDED_

