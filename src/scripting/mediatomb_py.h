
/// \file mediatomb_py.h

#ifndef __MEDIATOMB_PY_H__
#define __MEDIATOMB_PY_H__

#include <Python.h>
#include "structmember.h"

typedef struct 
{
        PyObject_HEAD
        /* Type-specific fields go here. */
        PyObject *objectType;
        PyObject *id;
        PyObject *parentID;
        PyObject *title;
        PyObject *upnpclass;
        PyObject *location;

        PyObject *path;

        
} mediatomb_MediaTombObject;



#endif // __MEDIATOMB_PY_H__
