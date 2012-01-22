/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    runtime.h - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.cc>,
                       Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>
    
    Copyright (C) 2006-2010 Gena Batyan <bgeradz@mediatomb.cc>,
                            Sergey 'Jin' Bostandzhyan <jin@mediatomb.cc>,
                            Leonhard Wimmer <leo@mediatomb.cc>
    
    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
    
    $Id$
*/

/// \file runtime.h

#ifndef __SCRIPTING_RUNTIME_H__
#define __SCRIPTING_RUNTIME_H__

#define XP_UNIX 1
#ifdef HAVE_JS

#include <jsapi.h>
#include <pthread.h>
#include "common.h"
#include "sync.h"
#include "singleton.h"

/// \brief Runtime class definition.
class Runtime : public Singleton<Runtime>
{
protected:
    JSRuntime *rt;

public:
    Runtime();
    virtual ~Runtime();
    
    /// \brief Returns the runtime for script execution.
    JSRuntime *getRT() { return rt; }
};

#endif // HAVA_JS
#ifdef HAVE_PYTHON


#include <pthread.h>
#include "common.h"
#include "sync.h"
#include "singleton.h"
#include "cds_objects.h"
//#include "script.h"

typedef enum
{
    S_IMPORT = 0,
    S_PLAYLIST,
    S_DVD
} script_class_t;

/// \brief Runtime class definition.
class Runtime : public Singleton<Runtime>
{
protected:
        // JSRuntime *rt;
        // int *rt;
        zmm::Ref<CdsObject> obj;
		//zmm::Ref<Script> script;

		zmm::Ref<CdsObject> processed;

public:
    Runtime();
    virtual ~Runtime();
    
    /// \brief Returns the runtime for script execution.
    //JSRuntime *getRT() { return rt; }
    zmm::Ref<CdsObject> getCdsObj() { return obj; };
    void setCdsObj(  zmm::Ref<CdsObject> cdsObj ) { obj = cdsObj; };
	
	zmm::Ref<CdsObject> getProcessedObject() { return processed ; }; 
	void setProcessedObject(zmm::Ref<CdsObject> cdsObj) { processed = processed; };	
	/*
    zmm::Ref<Script> getScript() { return script; }
    void setScript(  zmm::Ref<Script> cur_script ) { script = cur_script; }
 	*/

    script_class_t whoami ;
    
};


#endif

#endif // __SCRIPTING_RUNTIME_H__
