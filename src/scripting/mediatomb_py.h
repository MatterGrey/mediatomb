
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
    
    PyObject *restricted;
    PyObject *theora;
    
    PyObject *onlineservice;
    
    PyObject *meta;
    PyObject *aux;
    
    PyObject *mimetype;
    PyObject *serviceID;
    PyObject *action;
    PyObject *state;
    
    PyObject *updateID;
    PyObject *searchable;
    
    PyObject *path;
    
    int whoami ;
        
} mediatomb_MediaTombObject;



#endif // __MEDIATOMB_PY_H__
