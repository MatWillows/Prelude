/*****************************************************************
|
|      MPEG audio decoder. Audio Driver Output Module
|
|      (c) 1996-2000 Xaudio Corporation
|      Author: Gilles Boccon-Gibod (gilles@xaudio.com)
|
 ****************************************************************/

#ifndef __AUDIO_OUTPUT_H__
#define __AUDIO_OUTPUT_H__

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "decoder.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
#define XA_DSOUND_PITCH_ENVIRONMENT "OUTPUT.DIRECTSOUND.PITCH"

/*----------------------------------------------------------------------
|       prototypes
+---------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" 
{
#endif

int XA_EXPORT audio_output_module_register(XA_OutputModule *module);
#ifdef WIN32
int XA_EXPORT dsound_output_module_register(XA_OutputModule *module);
#endif /* WIN32 */

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_OUTPUT_H__ */

