/*****************************************************************
|
|      XAUDIO SDK. Sample Input Module
|
|      (c) 1997 Gilles Boccon-Gibod. bok@bok.net
|
 ****************************************************************/

/*
** This is a sample input module.
** This module shows a simple implementation of an input module
** that reads the bitstream from a file on disk.
** It also prints messages or shows message windows for each of the
** functions called, to show the order in which the Xaudio library
** makes calls to the module's methods.
*/

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "decoder.h"
#include "xaudio.h"

/*----------------------------------------------------------------------
|       constants and macros
+---------------------------------------------------------------------*/
#ifdef WIN32
#include <windows.h>
#define TRACE(x) MessageBox(NULL, x, "Input Module", MB_OK)
#else
#define TRACE(x) printf("INPUT MODULE: " x "\n")
#endif

#define NOTIFY_DEBUG(device, level, message, reason)     \
XA_NOTIFY_DEBUG(&(device)->decoder->notification_client, \
                (device)->class_info->id,                \
                level, message, reason)

/*----------------------------------------------------------------------
|       types
+---------------------------------------------------------------------*/
typedef struct XA_InputInstance {
    const char              *name;
    XA_InputModuleClassInfo *class_info;
    XA_DecoderInfo          *decoder;
    FILE                    *file;
} FileInput;

/*----------------------------------------------------------------------
|       myfile_input_module_probe
+---------------------------------------------------------------------*/
static int
myfile_input_module_probe(const char *name)
{
    //TRACE("input_module_probe");

    /* accept all names */
    return  XA_SUCCESS;
}

/*----------------------------------------------------------------------
|       myfile_input_module_query
+---------------------------------------------------------------------*/
static int  
myfile_input_module_query(XA_InputModuleQuery *query, 
                        unsigned long        query_flags)
{
    //TRACE("input_module_query");

    if (query_flags & XA_DECODER_INPUT_QUERY_MODULE_NAME) {
        strcpy(query->name, "Input From File");
        strcpy(query->description, "Version 1.0");
    }

    if (query_flags & XA_DECODER_INPUT_QUERY_NB_DEVICES) {
        query->index = 2;
    }
    
    /* if we're asked for the module name, stop here */
    if (query_flags & XA_DECODER_INPUT_QUERY_MODULE_NAME) return XA_SUCCESS;

    if (query_flags & XA_DECODER_INPUT_QUERY_DEVICE_NAME) {
        switch (query->index) {
            default: return XA_ERROR_INVALID_PARAMETERS;
          case 0:
            strcpy(query->name, "*");
            strcpy(query->description, "any file name");
            query->flags = XA_DECODER_INPUT_QUERY_NAME_IS_GENERIC;
            break;
          case 1:
            strcpy(query->name, "-");
            strcpy(query->description, "standard input");
            query->flags = 0;
            break;
        }
    }
    
    return XA_SUCCESS;
}

/*----------------------------------------------------------------------
|       myfile_input_new
+---------------------------------------------------------------------*/
static int
myfile_input_new(FileInput             **input, 
               const char              *name,
               XA_InputModuleClassInfo *class_info,
               XA_DecoderInfo          *decoder)
{
    /* allocate a file object */
    *input = (FileInput *)malloc(sizeof(FileInput));
    if (*input == NULL) return XA_ERROR_OUT_OF_MEMORY;

    //TRACE("input_new");

    /* store the parameters */
    (*input)->class_info = class_info;
    (*input)->decoder    = decoder;

    /* parameter checking */
    if (!name || strlen(name) == 0) {
        NOTIFY_DEBUG((*input), 0, 
                     "internal error", 
                     "null params or null/empty input name");
        free((void*)(*input));
        return XA_ERROR_INVALID_PARAMETERS;
    }
    
    /* store the name */
    (*input)->name = strdup(name);

    return XA_SUCCESS;
}

/*----------------------------------------------------------------------
|       myfile_input_delete
+---------------------------------------------------------------------*/
static int
myfile_input_delete(FileInput *input) 
{
    //TRACE("input_delete");

    if (input) {
        free((void *)input->name);
        free((void *)input);
    }

    return XA_SUCCESS;
}

/*----------------------------------------------------------------------
|       myfile_input_open
+---------------------------------------------------------------------*/
static int
myfile_input_open(FileInput *input)
{
    //TRACE("input_open");

    /* if file is named '-' it means stdin */
    if (input->name[0] == '-' && input->name[1] == '\0') {
        input->file = stdin;
        return XA_SUCCESS;
    }

    input->file = fopen(input->name, "rb");
    if (!input->file) {
        char *message = "cannot open input file";
         switch (errno) {
          case ENOENT:
            NOTIFY_DEBUG(input, 0, message, "file not found");
            return XA_ERROR_NO_SUCH_FILE;
            
          case EACCES:
            NOTIFY_DEBUG(input, 0, message, "permission denied");
            return XA_ERROR_PERMISSION_DENIED;

          default:
            NOTIFY_DEBUG(input, 0, message, strerror(errno));
            return XA_ERROR_OPEN_FAILED;
        }
    }

    return XA_SUCCESS;
}

/*----------------------------------------------------------------------
|       myfile_input_close
+---------------------------------------------------------------------*/
static int
myfile_input_close(FileInput *input)
{
    //TRACE("input_close");

    /* do not close stdin */
    if (input->file != stdin) {
        fclose(input->file);
    }

    return XA_SUCCESS;
}

/*----------------------------------------------------------------------
|       myfile_input_read
+---------------------------------------------------------------------*/
static int
myfile_input_read(FileInput *input, void *buffer, unsigned long size)
{
    int nb_read;

    //TRACE("input_read");

    nb_read = fread((void*)((char *)buffer),
                    size, 1, input->file);
    if (nb_read == 0) {
        if (feof(input->file)) return 0;
        if (ferror(input->file)) return 0;
        return 0;
    }
    return nb_read;
}

/*----------------------------------------------------------------------
|       myfile_input_seek
+---------------------------------------------------------------------*/
static int
myfile_input_seek(FileInput *input, unsigned long offset)
{
    //TRACE("input_seek");

    return fseek(input->file, offset, SEEK_SET);
}

/*----------------------------------------------------------------------
|       myfile_input_get_caps
+---------------------------------------------------------------------*/
static long
myfile_input_get_caps(FileInput *input)
{
    //TRACE("input_get_caps");

    if (input->file != stdin) {
        return XA_DECODER_INPUT_SEEKABLE;
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|       myfile_input_get_size
+---------------------------------------------------------------------*/
static long
myfile_input_get_size(FileInput *input)
{
    int filedes;
    struct stat stat_buffer;

    //TRACE("input_get_size");

    filedes = fileno(input->file);
    if (filedes > 0 && !fstat(filedes, &stat_buffer)) {
        return stat_buffer.st_size;
    }
    return 0;
}

/*----------------------------------------------------------------------
|       myfile_input_module_register
+---------------------------------------------------------------------*/
int XA_EXPORT
myfile_input_module_register(XA_InputModule *module)
{

    //TRACE("input_module_register");

    module->api_version_id     = XA_SYNC_API_VERSION;
    module->input_module_probe = myfile_input_module_probe;
    module->input_module_query = myfile_input_module_query;
    module->input_new          = myfile_input_new;
    module->input_delete       = myfile_input_delete;
    module->input_open         = myfile_input_open;
    module->input_close        = myfile_input_close;
    module->input_read         = myfile_input_read;
    module->input_seek         = myfile_input_seek;
    module->input_get_caps     = myfile_input_get_caps;
    module->input_get_size     = myfile_input_get_size;

    return XA_SUCCESS;
} 

