/*MT*
    
    MediaTomb - http://www.mediatomb.cc/
    
    import_script.cc - this file is part of MediaTomb.
    
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

/// \file import_script.cc

#ifdef HAVE_CONFIG_H
    #include "autoconfig.h"
#endif

#ifdef HAVE_JS

#include <jsapi.h>
#include "import_script.h"
#include "config_manager.h"
#include "js_functions.h"

using namespace zmm;

ImportScript::ImportScript(Ref<Runtime> runtime) : Script(runtime)
{
    String scriptPath = ConfigManager::getInstance()->getOption(CFG_IMPORT_SCRIPTING_IMPORT_SCRIPT); 

#ifdef JS_THREADSAFE
    JS_SetContextThread(cx);
    JS_BeginRequest(cx);
#endif

    try 
    {
        load(scriptPath);
#ifndef JS_MOZLIB185 
        root = JS_NewScriptObject(cx, script);
        JS_AddNamedRoot(cx, &root, "ImportScript");
#else
        root = script;
        JS_AddNamedObjectRoot(cx, &root, "ImportScript");
#endif
    }
    catch (Exception ex)
    {
#ifdef JS_THREADSAFE
        JS_EndRequest(cx);
        JS_ClearContextThread(cx);
#endif
        throw ex;
    }
#ifdef JS_THREADSAFE
        JS_EndRequest(cx);
        JS_ClearContextThread(cx);
#endif
}

void ImportScript::processCdsObject(Ref<CdsObject> obj, String rootpath)
{
#ifdef JS_THREADSAFE
    JS_SetContextThread(cx);
    JS_BeginRequest(cx);
#endif
    processed = obj;
    try 
    {
        JSObject *orig = JS_NewObject(cx, NULL, NULL, glob);
        setObjectProperty(glob, _("orig"), orig);
        cdsObject2jsObject(obj, orig);
        setProperty(glob, _("object_root_path"), rootpath);
        execute();
    }
    catch (Exception ex)
    {
        processed = nil;
#ifdef JS_THREADSAFE
        JS_EndRequest(cx);
        JS_ClearContextThread(cx);
#endif
        throw ex;
    }

    processed = nil;

    gc_counter++;
    if (gc_counter > JS_CALL_GC_AFTER_NUM)
    {
        JS_MaybeGC(cx);
        gc_counter = 0;
    }
#ifdef JS_THREADSAFE
    JS_EndRequest(cx);
    JS_ClearContextThread(cx);
#endif
}

ImportScript::~ImportScript()
{
#ifdef JS_THREADSAFE
    JS_SetContextThread(cx);
    JS_BeginRequest(cx);
#endif
    
    if (root)
#ifndef JS_MOZLIB185 
        JS_RemoveRoot(cx, &root);
#else
        JS_RemoveObjectRoot(cx, &root);
#endif

#ifdef JS_THREADSAFE
    JS_EndRequest(cx);
    JS_ClearContextThread(cx);
#endif

}

#endif // HAVE_JS



#ifdef HAVE_PYTHON

#include "import_script.h"
#include "config_manager.h"
#include "runtime.h"

using namespace zmm;

ImportScript::ImportScript(Ref<Runtime> runtime) : Script(runtime)
{


    String scriptPath = ConfigManager::getInstance()->getOption(CFG_IMPORT_SCRIPTING_IMPORT_SCRIPT);
    
    //log_info("ImportScript::ImportScript(%s)\n", scriptPath.c_str());
    try 
    {
        load(scriptPath);
    }
    catch (Exception ex)
    {
            throw ex;
    }
}
void ImportScript::processCdsObject(Ref<CdsObject> obj, String rootpath)
{
	zmm::Ref<Runtime> runtime = Runtime::getInstance();
    processed = obj;
	
	runtime->setProcessedObject(obj);
	
    try 
    {
        
        //log_info("ImportScript::processCdsObject(%s)\n", rootpath.c_str());
		setPyObj(obj);
/*
        JSObject *orig = JS_NewObject(cx, NULL, NULL, glob);
        setObjectProperty(glob, _("orig"), orig);
        cdsObject2jsObject(obj, orig);
        setProperty(glob, _("object_root_path"), rootpath);
*/
        execute();

    }
    catch (Exception ex)
    {
		runtime->setProcessedObject(nil);
        processed = nil;
        throw ex;
    }

	runtime->setProcessedObject(nil);
    processed = nil;
/*
    gc_counter++;
    if (gc_counter > JS_CALL_GC_AFTER_NUM)
    {
        JS_MaybeGC(cx);
        gc_counter = 0;
    }
*/
}

ImportScript::~ImportScript()
{
    
/*    if (root)
        JS_RemoveRoot(cx, &root);
*/

    
}

#endif //HAVE_PYTHON        
