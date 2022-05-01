/*****************************************************************
|
|      File Output Modules
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef __FILE_OUTPUT_H__
#define __FILE_OUTPUT_H__

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "decoder.h"

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

int XA_EXPORT file_output_module_register(XA_OutputModule *module);

#ifdef __cplusplus
}
#endif
 
#endif /* __FILE_OUTPUT_H__ */
