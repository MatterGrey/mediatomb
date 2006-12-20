/*MT*
    
    MediaTomb - http://www.mediatomb.org/
    
    sql_storage.cc - this file is part of MediaTomb.
    
    Copyright (C) 2005 Gena Batyan <bgeradz@mediatomb.org>,
                       Sergey Bostandzhyan <jin@mediatomb.org>
    Copyright (C) 2006 Gena Batyan <bgeradz@mediatomb.org>,
                       Sergey Bostandzhyan <jin@mediatomb.org>,
                       Leonhard Wimmer <leo@mediatomb.org>
    
    MediaTomb is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation.
    
    MediaTomb is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    version 2 along with MediaTomb; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    
    $Id$
*/

/// \file sql_storage.cc

#ifdef HAVE_CONFIG_H
    #include "autoconfig.h"
#endif

#include <limits.h>
#include "sql_storage.h"
#include "tools.h"
#include "update_manager.h"
#include "string_converter.h"

using namespace zmm;

#define LOC_DIR_PREFIX      'D'
#define LOC_FILE_PREFIX     'F'
#define LOC_VIRT_PREFIX     'V'
#define LOC_ILLEGAL_PREFIX  'X'
#define MAX_REMOVE_SIZE     10000
#define MAX_REMOVE_RECURSION 500

#define SQL_NULL             "NULL"

enum
{
    _id = 0,
    _ref_id,
    _parent_id,
    _object_type,
    _upnp_class,
    _dc_title,
    _location,
    _location_hash,
    _metadata,
    _auxdata,
    _resources,
    _update_id,
    _mime_type,
    _flags,
    _ref_upnp_class,
    _ref_location,
    _ref_metadata,
    _ref_auxdata,
    _ref_resources,
    _ref_mime_type,
    _as_id
};

/* table quote */
#define TQ(data)        QTB << data << QTE
/* table quote with dot */
#define TQD(data1, data2)        TQ(data1) << '.' << TQ(data2)

#define SEL_F_QUOTED        << TQ('f') <<
#define SEL_RF_QUOTED       << TQ("rf") <<

// end quote, space, f quoted, dot, begin quote
#define SEL_EQ_SP_FQ_DT_BQ  << QTE << ',' << TQ('f') << '.' << QTB <<
#define SEL_EQ_SP_RFQ_DT_BQ  << QTE << ',' << TQ("rf") << '.' << QTB <<

#define SELECT_DATA_FOR_STRINGBUFFER \
  TQ('f') << '.' << QTB << "id" \
    SEL_EQ_SP_FQ_DT_BQ "ref_id" \
    SEL_EQ_SP_FQ_DT_BQ "parent_id" \
    SEL_EQ_SP_FQ_DT_BQ "object_type" \
    SEL_EQ_SP_FQ_DT_BQ "upnp_class" \
    SEL_EQ_SP_FQ_DT_BQ "dc_title" \
    SEL_EQ_SP_FQ_DT_BQ "location" \
    SEL_EQ_SP_FQ_DT_BQ "location_hash" \
    SEL_EQ_SP_FQ_DT_BQ "metadata" \
    SEL_EQ_SP_FQ_DT_BQ "auxdata" \
    SEL_EQ_SP_FQ_DT_BQ "resources" \
    SEL_EQ_SP_FQ_DT_BQ "update_id" \
    SEL_EQ_SP_FQ_DT_BQ "mime_type" \
    SEL_EQ_SP_FQ_DT_BQ "flags" \
    SEL_EQ_SP_RFQ_DT_BQ "upnp_class" \
    SEL_EQ_SP_RFQ_DT_BQ "location" \
    SEL_EQ_SP_RFQ_DT_BQ "metadata" \
    SEL_EQ_SP_RFQ_DT_BQ "auxdata" \
    SEL_EQ_SP_RFQ_DT_BQ "resources" \
    SEL_EQ_SP_RFQ_DT_BQ "mime_type" << QTE \
    << ',' << TQD("as","id")
    
#define SQL_QUERY_FOR_STRINGBUFFER "SELECT " << SELECT_DATA_FOR_STRINGBUFFER << \
    " FROM " << TQ(CDS_OBJECT_TABLE) << ' ' << TQ('f') << " LEFT JOIN " \
    << TQ(CDS_OBJECT_TABLE) << ' ' << TQ("rf") << " ON " << TQD('f',"ref_id") \
    << '=' << TQD("rf","id") << " LEFT JOIN " << TQ(AUTOSCAN_TABLE) << ' ' \
    << TQ("as") << " ON " << TQD("as","obj_id") << '=' << TQD('f',"id") << ' '
    
#define SQL_QUERY       sql_query

/* enum for createObjectFromRow's mode parameter */

SQLStorage::SQLStorage() : Storage()
{
    table_quote_begin = '\0';
    table_quote_end = '\0';
}

void SQLStorage::init()
{
    if (table_quote_begin == '\0' || table_quote_end == '\0')
        throw _Exception(_("quote vars need to be overriden!"));
    
    Ref<StringBuffer> buf(new StringBuffer());
    *buf << SQL_QUERY_FOR_STRINGBUFFER;
    this->sql_query = buf->toString();
    //log_debug("using SQL: %s\n", this->sql_query.c_str());
    
    //objectTitleCache = Ref<DSOHash<CdsObject> >(new DSOHash<CdsObject>(OBJECT_CACHE_CAPACITY));
    //objectIDCache = Ref<DBOHash<int, CdsObject> >(new DBOHash<int, CdsObject>(OBJECT_CACHE_CAPACITY, -100));
   
/*    
    Ref<SQLResult> res = select(_("SELECT MAX(id) + 1 FROM cds_objects"));
    Ref<SQLRow> row = res->nextRow();
    nextObjectID = row->col(0).toInt();
    
    log_debug(("PRELOADING OBJECTS...\n"));
    res = select(getSelectQuery(SELECT_FULL));
    Ref<Array<CdsObject> > arr(new Array<CdsObject>());
    while((row = res->nextRow()) != nil)
    {
        Ref<CdsObject> obj = createObjectFromRow(row, SELECT_FULL);
        obj->optimize();
        objectTitleCache->put(String::from(obj->getParentID()) + '|' + obj->getTitle(), obj);
        objectIDCache->put(obj->getID(), obj);
    }
    log_debug(("PRELOADING OBJECTS DONE\n"));
*/
}

/*
SQLStorage::~SQLStorage()
{
}
*/

Ref<CdsObject> SQLStorage::checkRefID(Ref<CdsObject> obj)
{
    if (! obj->isVirtual()) throw _Exception(_("checkRefID called for a non-virtual object"));
    int refID = obj->getRefID();
    String location = obj->getLocation();
    if (! string_ok(location))
        throw _Exception(_("tried to check refID without a location set"));
    if (refID > 0)
    {
        try
        {
            Ref<CdsObject> refObj;
            refObj = loadObject(refID);
            if (refObj != nil && refObj->getLocation() == location)
                return refObj;
        }
        catch (Exception e)
        {
        }
    }
    
    return findObjectByPath(location);
}

Ref<Array<SQLStorage::AddUpdateTable> > SQLStorage::_addUpdateObject(Ref<CdsObject> obj, bool isUpdate, int *changedContainer)
{
    int objectType = obj->getObjectType();
    Ref<CdsObject> refObj = nil;
    bool hasReference = false;
    if (obj->isVirtual() && IS_CDS_PURE_ITEM(objectType))
    {
        hasReference = true;
        refObj = checkRefID(obj);
        if (refObj == nil)
            throw _Exception(_("tried to add or update a virtual object with illegal reference id and an illegal location"));
    }
    
    Ref<Array<AddUpdateTable> > returnVal(new Array<AddUpdateTable>(2));
    Ref<Dictionary> cdsObjectSql(new Dictionary());
    returnVal->append(Ref<AddUpdateTable> (new AddUpdateTable(_(CDS_OBJECT_TABLE), cdsObjectSql)));
    
    cdsObjectSql->put(_("object_type"), quote(objectType));
    
    if (hasReference)
        cdsObjectSql->put(_("ref_id"), quote(refObj->getID()));
    else if (isUpdate)
        cdsObjectSql->put(_("ref_id"), _(SQL_NULL));
    
    if (! hasReference || refObj->getClass() != obj->getClass())
        cdsObjectSql->put(_("upnp_class"), quote(obj->getClass()));
    else if (isUpdate)
        cdsObjectSql->put(_("upnp_class"), _(SQL_NULL));
    
    //if (!hasReference || refObj->getTitle() != obj->getTitle())
    cdsObjectSql->put(_("dc_title"), quote(obj->getTitle()));
    //else if (isUpdate)
    //    cdsObjectSql->put(_("dc_title"), _(SQL_NULL));
    
    cdsObjectSql->put(_("flags"), quote(obj->getFlags()));
    
    if (isUpdate)
        cdsObjectSql->put(_("metadata"), _(SQL_NULL));
    Ref<Dictionary> dict = obj->getMetadata();
    if (dict->size() > 0)
    {
        if (! hasReference || ! refObj->getMetadata()->equals(obj->getMetadata()))
        {
            cdsObjectSql->put(_("metadata"), quote(dict->encode()));
        }
    }
    
    if (isUpdate)
        cdsObjectSql->put(_("auxdata"), _(SQL_NULL));
    dict = obj->getAuxData();
    if (dict->size() > 0 && (! hasReference || ! refObj->getAuxData()->equals(obj->getAuxData())))
    {
        cdsObjectSql->put(_("auxdata"), quote(obj->getAuxData()->encode()));
    }
    
    if (! hasReference || (! obj->getFlag(OBJECT_FLAG_USE_RESOURCE_REF) && ! refObj->resourcesEqual(obj)))
    {
        // encode resources
        Ref<StringBuffer> resBuf(new StringBuffer());
        for (int i = 0; i < obj->getResourceCount(); i++)
        {
            if (i > 0)
                *resBuf << RESOURCE_SEP;
            *resBuf << obj->getResource(i)->encode();
        }
        String resStr = resBuf->toString();
        if (string_ok(resStr))
            cdsObjectSql->put(_("resources"), quote(resStr));
        else
            cdsObjectSql->put(_("resources"), _(SQL_NULL));
    }
    else if (isUpdate)
        cdsObjectSql->put(_("resources"), _(SQL_NULL));
    
    if (IS_CDS_CONTAINER(objectType))
    {
        if (! (isUpdate && obj->isVirtual()) )
            throw _Exception(_("tried to add a container or tried to update a non-virtual container via _addUpdateObject; is this correct?"));
        String dbLocation = addLocationPrefix(LOC_VIRT_PREFIX, obj->getLocation());
        cdsObjectSql->put(_("location"), quote(dbLocation));
        cdsObjectSql->put(_("location_hash"), quote(stringHash(dbLocation)));
    }
    
    if (IS_CDS_ITEM(objectType))
    {
        Ref<CdsItem> item = RefCast(obj, CdsItem);
        
        if (! hasReference)
        {
            String loc = item->getLocation();
            if (!string_ok(loc)) throw _Exception(_("tried to create or update a non-referenced item without a location set"));
            if (IS_CDS_PURE_ITEM(objectType))
            {
                Ref<Array<StringBase> > pathAr = split_path(loc);
                String path = pathAr->get(0);
                int parentID = ensurePathExistence(path, changedContainer);
                item->setParentID(parentID);
                String dbLocation = addLocationPrefix(LOC_FILE_PREFIX, loc);
                cdsObjectSql->put(_("location"), quote(dbLocation));
                cdsObjectSql->put(_("location_hash"), quote(stringHash(dbLocation)));
            }
            else 
            {
                // URLs and active items
                cdsObjectSql->put(_("location"), quote(loc));
                cdsObjectSql->put(_("location_hash"), _(SQL_NULL));
            }
        }
        else 
        {
            if (! IS_CDS_PURE_ITEM(objectType))
                throw _Exception(_("tried to create or update a non-pure item with refID set"));
            if (isUpdate)
            {
                cdsObjectSql->put(_("location"), _(SQL_NULL));
                cdsObjectSql->put(_("location_hash"), _(SQL_NULL));
            }
        }
        
        cdsObjectSql->put(_("mime_type"), quote(item->getMimeType()));
    }
    if (IS_CDS_ACTIVE_ITEM(objectType))
    {
        Ref<Dictionary> cdsActiveItemSql(new Dictionary());
        returnVal->append(Ref<AddUpdateTable> (new AddUpdateTable(_(CDS_ACTIVE_ITEM_TABLE), cdsActiveItemSql)));
        Ref<CdsActiveItem> aitem = RefCast(obj, CdsActiveItem);
        
        cdsActiveItemSql->put(_("id"), String::from(aitem->getID()));
        cdsActiveItemSql->put(_("action"), quote(aitem->getAction()));
        cdsActiveItemSql->put(_("state"), quote(aitem->getState()));
    }
    
    // check for a duplicate (virtual) object
    if (hasReference && ! isUpdate)
    {
        Ref<StringBuffer> qb(new StringBuffer());
        *qb << "SELECT " << TQ("id") 
            << " FROM " << TQ(CDS_OBJECT_TABLE)
            << " WHERE " << TQ("parent_id") 
            << '=' << quote(obj->getParentID())
            << " AND " << TQ("ref_id") 
            << '=' << quote(refObj->getID())
            << " AND " << TQ("dc_title")
            << '=' << quote(obj->getTitle())
            << " LIMIT 1";
        Ref<SQLResult> res = select(qb);
        // if duplicate items is found - ignore
        if (res != nil && (res->nextRow() != nil))
            return nil;
    }
    
    if (obj->getParentID() == INVALID_OBJECT_ID)
        throw _Exception(_("tried to create or update an object with an illegal parent id"));
    cdsObjectSql->put(_("parent_id"), String::from(obj->getParentID()));
    
    return returnVal;
}

void SQLStorage::addObject(Ref<CdsObject> obj, int *changedContainer)
{
    if (obj->getID() != INVALID_OBJECT_ID)
        throw _Exception(_("tried to add an object with an object ID set"));
    //obj->setID(INVALID_OBJECT_ID);
    Ref<Array<AddUpdateTable> > data = _addUpdateObject(obj, false, changedContainer);
    if (data == nil)
        return;
    int lastInsertID = INVALID_OBJECT_ID;
    for (int i = 0; i < data->size(); i++)
    {
        Ref<AddUpdateTable> addUpdateTable = data->get(i);
        String tableName = addUpdateTable->getTable();
        Ref<Array<DictionaryElement> > dataElements = addUpdateTable->getDict()->getElements();
        
        Ref<StringBuffer> fields(new StringBuffer(128));
        Ref<StringBuffer> values(new StringBuffer(128));
        for (int j = 0; j < dataElements->size(); j++)
        {
            Ref<DictionaryElement> element = dataElements->get(j);
            if (j != 0)
            {
                *fields << ',';
                *values << ',';
            }
            *fields << TQ(element->getKey());
            if (lastInsertID != INVALID_OBJECT_ID &&
                element->getKey() == "id" &&
                element->getValue().toInt() == INVALID_OBJECT_ID )
                *values << lastInsertID;
            else
            *values << element->getValue();
        }
        
        Ref<StringBuffer> qb(new StringBuffer(256));
        *qb << "INSERT INTO " << TQ(tableName) << " (" << fields->toString() <<
                ") VALUES (" << values->toString() << ')';
                
        log_debug("insert_query: %s\n", qb->toString().c_str());
        
        if ( lastInsertID == INVALID_OBJECT_ID && tableName == _(CDS_OBJECT_TABLE))
        {
            lastInsertID = exec(qb, true);
            obj->setID(lastInsertID);
        }
        else exec(qb);
    }
}

void SQLStorage::updateObject(zmm::Ref<CdsObject> obj, int *changedContainer)
{
    Ref<Array<AddUpdateTable> > data;
    if (obj->getID() == CDS_ID_FS_ROOT)
    {
        data = Ref<Array<AddUpdateTable> >(new Array<AddUpdateTable>(1));
        Ref<Dictionary> cdsObjectSql(new Dictionary());
        data->append(Ref<AddUpdateTable> (new AddUpdateTable(_(CDS_OBJECT_TABLE), cdsObjectSql)));
        cdsObjectSql->put(_("dc_title"), quote(obj->getTitle()));
        setFsRootName(obj->getTitle());
        cdsObjectSql->put(_("upnp_class"), quote(obj->getClass()));
    }
    else
    {
        if (IS_FORBIDDEN_CDS_ID(obj->getID()))
            throw _Exception(_("tried to update an object with a forbidden ID (")+obj->getID()+")!");
        data = _addUpdateObject(obj, true, changedContainer);
        if (data == nil)
            return;
    }
    for (int i = 0; i < data->size(); i++)
    {
        Ref<AddUpdateTable> addUpdateTable = data->get(i);
        String tableName = addUpdateTable->getTable();
        Ref<Array<DictionaryElement> > dataElements = addUpdateTable->getDict()->getElements();
        
        Ref<StringBuffer> qb(new StringBuffer(256));
        *qb << "UPDATE " << TQ(tableName) << " SET ";
        
        for (int j = 0; j < dataElements->size(); j++)
        {
            Ref<DictionaryElement> element = dataElements->get(j);
            if (j != 0)
            {
                *qb << ',';
            }
            *qb << TQ(element->getKey()) << '='
                << element->getValue();
        }
        
        *qb << " WHERE id = " << obj->getID();
        
        log_debug("upd_query: %s\n", qb->toString().c_str());
        
        exec(qb);
    }
}

Ref<CdsObject> SQLStorage::loadObject(int objectID)
{
/*
    Ref<CdsObject> obj = objectIDCache->get(objectID);
    if (obj != nil)
        return obj;
    throw _Exception(_("Object not found: ") + objectID);
*/
    Ref<StringBuffer> qb(new StringBuffer());
    
    //log_debug("sql_query = %s\n",sql_query.c_str());
    
    *qb << SQL_QUERY << " WHERE " << TQD('f',"id") << '=' << objectID;

    Ref<SQLResult> res = select(qb);
    Ref<SQLRow> row;
    if (res != nil && (row = res->nextRow()) != nil)
    {
        return createObjectFromRow(row);
    }
    throw _Exception(_("Object not found: ") + objectID);
}

Ref<Array<CdsObject> > SQLStorage::browse(Ref<BrowseParam> param)
{
    int objectID;
    int objectType;
    
    bool getContainers = param->getFlag(BROWSE_CONTAINERS);
    bool getItems = param->getFlag(BROWSE_ITEMS);
    
    objectID = param->getObjectID();
    
    Ref<SQLResult> res;
    Ref<SQLRow> row;
    
    Ref<StringBuffer> qb(new StringBuffer());
    *qb << "SELECT " << TQ("object_type")
        << " FROM " << TQ(CDS_OBJECT_TABLE)
        << " WHERE " << TQ("id") << '=' << objectID;
    res = select(qb);
    if(res != nil && (row = res->nextRow()) != nil)
    {
        objectType = row->col(0).toInt();
    }
    else
    {
        throw _StorageException(_("Object not found: ") + objectID);
    }
    
    row = nil;
    res = nil;
    
    if(param->getFlag(BROWSE_DIRECT_CHILDREN) && IS_CDS_CONTAINER(objectType))
    {
        param->setTotalMatches(getChildCount(objectID, getContainers, getItems));
    }
    else
    {
        param->setTotalMatches(1);
    }
    
    row = nil;
    res = nil;
    
    qb->clear();
    *qb << SQL_QUERY << " WHERE ";
    
    if(param->getFlag(BROWSE_DIRECT_CHILDREN) && IS_CDS_CONTAINER(objectType))
    {
        int count = param->getRequestedCount();
        bool doLimit = true;
        if (! count)
        {
            if (param->getStartingIndex())
                count = INT_MAX;
            else
                doLimit = false;
        }
        
        *qb << TQD('f',"parent_id") << '=' << objectID;
        if (! getContainers && ! getItems)
        {
            *qb << " AND 0=1";
        }
        else if (getContainers && ! getItems)
        {
            *qb << " AND " << TQD('f',"object_type") << '='
                << quote(OBJECT_TYPE_CONTAINER)
                << " ORDER BY " << TQD('f',"dc_title");
        }
        else if (! getContainers && getItems)
        {
            *qb << " AND " << TQD('f',"object_type") << " & "
                << quote(OBJECT_TYPE_ITEM)
                << " ORDER BY " << TQD('f',"dc_title");
        }
        else
        {
            *qb << " ORDER BY ("
            << TQD('f',"object_type") << '=' << quote(OBJECT_TYPE_CONTAINER)
            << ") DESC, "<< TQD('f',"dc_title");
        }
        if (doLimit)
            *qb << " LIMIT " << count << " OFFSET " << param->getStartingIndex();
    }
    else // metadata
    {
        *qb << TQD('f',"id") << '=' << objectID << " LIMIT 1";
    }
    //log_debug("QUERY: %s\n", qb->toString().c_str());
    res = select(qb);
    
    Ref<Array<CdsObject> > arr(new Array<CdsObject>());
    
    while((row = res->nextRow()) != nil)
    {
        Ref<CdsObject> obj = createObjectFromRow(row);
        arr->append(obj);
        row = nil;
    }
    
    row = nil;
    res = nil;
    
    // update childCount fields
    for (int i = 0; i < arr->size(); i++)
    {
        Ref<CdsObject> obj = arr->get(i);
        if (IS_CDS_CONTAINER(obj->getObjectType()))
        {
            Ref<CdsContainer> cont = RefCast(obj, CdsContainer);
            cont->setChildCount(getChildCount(cont->getID(), getContainers, getItems));
        }
    }
    
    return arr;
}

int SQLStorage::getChildCount(int contId, bool containers, bool items)
{
    if (! containers && ! items)
        return 0;
    Ref<SQLRow> row;
    Ref<SQLResult> res;
    Ref<StringBuffer> qb(new StringBuffer());
    *qb << "SELECT COUNT(*) FROM " << TQ(CDS_OBJECT_TABLE)
        << " WHERE " << TQ("parent_id") << '=' << contId;
    if (containers && ! items)
        *qb << " AND " << TQ("object_type") << '=' << OBJECT_TYPE_CONTAINER;
    else if (items && ! containers)
        *qb << " AND " << TQ("object_type") << " & " << OBJECT_TYPE_ITEM;
    res = select(qb);
    if (res != nil && (row = res->nextRow()) != nil)
    {
        return row->col(0).toInt();
    }
    return 0;
}

Ref<Array<StringBase> > SQLStorage::getMimeTypes()
{
    Ref<Array<StringBase> > arr(new Array<StringBase>());
    
    Ref<StringBuffer> qb(new StringBuffer());
    *qb << "SELECT DISTINCT " << TQ("mime_type")
        << " FROM " << TQ(CDS_OBJECT_TABLE)
        << " WHERE " << TQ("mime_type") << " IS NOT NULL ORDER BY "
        << TQ("mime_type");
    Ref<SQLResult> res = select(qb);
    if (res == nil)
        throw _Exception(_("db error"));
    
    Ref<SQLRow> row;
    
    while ((row = res->nextRow()) != nil)
    {
        arr->append(String(row->col(0)));
    }
    
    return arr;
}

Ref<SQLRow> SQLStorage::_findObjectByPath(String fullpath)
{
    //log_debug("fullpath: %s\n", fullpath.c_str());
    fullpath = fullpath.reduce(DIR_SEPARATOR);
    //log_debug("fullpath after reduce: %s\n", fullpath.c_str());
    Ref<Array<StringBase> > pathAr = split_path(fullpath);
    String path = pathAr->get(0);
    String filename = pathAr->get(1);
    
    Ref<StringBuffer> qb(new StringBuffer());
    bool file = string_ok(filename);
    
    String dbLocation;
    if (file)
        dbLocation = addLocationPrefix(LOC_FILE_PREFIX, fullpath);
    else
        dbLocation = addLocationPrefix(LOC_DIR_PREFIX, path);
    *qb << SQL_QUERY
            << " WHERE " << TQD('f',"location_hash") << '=' << quote(stringHash(dbLocation))
            << " AND " << TQD('f',"location") << '=' << quote(dbLocation)
            << " AND " << TQD('f',"ref_id") << " IS NULL "
            "LIMIT 1";
    
    Ref<SQLResult> res = select(qb);
    if (res == nil)
        throw _Exception(_("error while doing select: ") + qb->toString());
    return res->nextRow();
}

Ref<CdsObject> SQLStorage::findObjectByPath(String fullpath)
{
    Ref<SQLRow> row = _findObjectByPath(fullpath);
    if (row == nil)
        return nil;
    return createObjectFromRow(row);
}

int SQLStorage::findObjectIDByPath(String fullpath)
{
    Ref<SQLRow> row = _findObjectByPath(fullpath);
    if (row == nil)
        return INVALID_OBJECT_ID;
    return row->col(_id).toInt();
}

int SQLStorage::ensurePathExistence(String path, int *changedContainer)
{
    *changedContainer = INVALID_OBJECT_ID;
    String cleanPath = path.reduce(DIR_SEPARATOR);
    if (cleanPath == DIR_SEPARATOR)
        return CDS_ID_FS_ROOT;
    if (cleanPath.charAt(cleanPath.length() - 1) == DIR_SEPARATOR) // cut off trailing slash
        cleanPath = cleanPath.substring(0, cleanPath.length() - 1);
    return _ensurePathExistence(cleanPath, changedContainer);
}

int SQLStorage::_ensurePathExistence(String path, int *changedContainer)
{
    if (path == DIR_SEPARATOR)
        return CDS_ID_FS_ROOT;
    Ref<CdsObject> obj = findObjectByPath(path + DIR_SEPARATOR);
    if (obj != nil)
        return obj->getID();
    Ref<Array<StringBase> > pathAr = split_path(path);
    String parent = pathAr->get(0);
    String folder = pathAr->get(1);
    int parentID;
    parentID = ensurePathExistence(parent, changedContainer);
    
    Ref<StringConverter> f2i = StringConverter::f2i();
    if (changedContainer != NULL && *changedContainer == INVALID_OBJECT_ID)
        *changedContainer = parentID;
    return createContainer(parentID, f2i->convert(folder), path, false);
}

int SQLStorage::createContainer(int parentID, String name, String path, bool isVirtual)
{
    String dbLocation = addLocationPrefix((isVirtual ? LOC_VIRT_PREFIX : LOC_DIR_PREFIX), path);
    Ref<StringBuffer> qb(new StringBuffer());
    *qb << "INSERT INTO " << TQ(CDS_OBJECT_TABLE)
        << " (" << TQ("parent_id") << ',' << TQ("object_type")
        << ',' << TQ("upnp_class") << ',' << TQ("dc_title") << ','
        << TQ("location") << ',' << TQ("location_hash") << ") VALUES ("
        << parentID
        << ',' << OBJECT_TYPE_CONTAINER
        << ',' << quote(_(UPNP_DEFAULT_CLASS_CONTAINER))
        << ',' << quote(name)
        << ',' << quote(dbLocation)
        << ',' << quote(stringHash(dbLocation))
        << ')';
        
    return exec(qb, true);
    
}

String SQLStorage::buildContainerPath(int parentID, String title)
{
    //title = escape(title, xxx);
    if (parentID == CDS_ID_ROOT)
        return String(VIRTUAL_CONTAINER_SEPARATOR) + title;
    Ref<StringBuffer> qb(new StringBuffer());
    *qb << "SELECT " << TQ("location") << " FROM " << TQ(CDS_OBJECT_TABLE) <<
        " WHERE " << TQ("id") << '=' << parentID << " LIMIT 1";
     Ref<SQLResult> res = select(qb);
    if (res == nil)
        return nil;
    Ref<SQLRow> row = res->nextRow();
    if (row == nil)
        return nil;
    char prefix;
    String path = stripLocationPrefix(&prefix, row->col(0)) + VIRTUAL_CONTAINER_SEPARATOR + title;
    if (prefix != LOC_VIRT_PREFIX)
        throw _Exception(_("tried to build a virtual container path with an non-virtual parentID"));
    return path;
}

void SQLStorage::addContainerChain(String path, int *containerID, int *updateID)
{
    path = path.reduce(VIRTUAL_CONTAINER_SEPARATOR);
    if (path == VIRTUAL_CONTAINER_SEPARATOR)
    {
        *containerID = CDS_ID_ROOT;
        return;
    }
    Ref<StringBuffer> qb(new StringBuffer());
    String dbLocation = addLocationPrefix(LOC_VIRT_PREFIX, path);
    *qb << "SELECT " << TQ("id") << " FROM " << TQ(CDS_OBJECT_TABLE)
            << " WHERE " << TQ("location_hash") << '=' << quote(stringHash(dbLocation))
            << " AND " << TQ("location") << '=' << quote(dbLocation)
            << " LIMIT 1";
    Ref<SQLResult> res = select(qb);
    if (res != nil)
    {
        Ref<SQLRow> row = res->nextRow();
        if (row != nil)
        {
            if (containerID != NULL)
                *containerID = row->col(0).toInt();
            return;
        }
    }
    int parentContainerID;
    String newpath, container;
    stripAndUnescapeVirtualContainerFromPath(path, newpath, container);
    addContainerChain(newpath, &parentContainerID, updateID);
    if (updateID != NULL && *updateID == INVALID_OBJECT_ID)
        *updateID = parentContainerID;
    *containerID = createContainer(parentContainerID, container, path, true);
}

String SQLStorage::addLocationPrefix(char prefix, String path)
{
    return String(prefix) + path;
}

String SQLStorage::stripLocationPrefix(char* prefix, String path)
{
    if (path == nil)
    {
        *prefix = LOC_ILLEGAL_PREFIX;
        return nil;
    }
    *prefix = path.charAt(0);
    return path.substring(1);
}

String SQLStorage::stripLocationPrefix(String path)
{
    if (path == nil)
        return nil;
    return path.substring(1);
}

Ref<CdsObject> SQLStorage::createObjectFromRow(Ref<SQLRow> row)
{
    int objectType = row->col(_object_type).toInt();
    Ref<CdsObject> obj = CdsObject::createObject(objectType);

    /* set common properties */
    obj->setID(row->col(_id).toInt());
    obj->setRefID(row->col(_ref_id).toInt());
    
    obj->setParentID(row->col(_parent_id).toInt());
    obj->setTitle(row->col(_dc_title));
    obj->setClass(fallbackString(row->col(_upnp_class), row->col(_ref_upnp_class)));
    obj->setFlags(row->col(_flags).toUInt());
    
    String metadataStr = fallbackString(row->col(_metadata), row->col(_ref_metadata));
    Ref<Dictionary> meta(new Dictionary());
    meta->decode(metadataStr);
    obj->setMetadata(meta);
    
    String auxdataStr = fallbackString(row->col(_auxdata), row->col(_ref_auxdata));
    Ref<Dictionary> aux(new Dictionary());
    aux->decode(auxdataStr);
    obj->setAuxData(aux);
    
    String resources_str = fallbackString(row->col(_resources), row->col(_ref_resources));
    bool resource_zero_ok = false;
    if (string_ok(resources_str))                                            
    {
        Ref<Array<StringBase> > resources = split_string(resources_str,
                                                    RESOURCE_SEP);
        for (int i = 0; i < resources->size(); i++)
        {
            if (i == 0)
                resource_zero_ok = true;
            obj->addResource(CdsResource::decode(resources->get(i)));
        }
    }
    
    if ( (obj->getRefID() && IS_CDS_PURE_ITEM(objectType)) ||
        IS_CDS_ITEM(objectType) && ! IS_CDS_PURE_ITEM(objectType) )
        obj->setVirtual(true);
    else
        obj->setVirtual(false); // gets set to true for virtual containers below
    
    int matched_types = 0;
    
    if (IS_CDS_CONTAINER(objectType))
    {
        Ref<CdsContainer> cont = RefCast(obj, CdsContainer);
        cont->setUpdateID(row->col(_update_id).toInt());
        char locationPrefix;
        cont->setLocation(stripLocationPrefix(&locationPrefix, row->col(_location)));
        if (locationPrefix == LOC_VIRT_PREFIX)
            cont->setVirtual(true);
        cont->setAutoscanStart(string_ok(row->col(_as_id)));
        matched_types++;
    }
    
    if (IS_CDS_ITEM(objectType))
    {
        if (! resource_zero_ok)
            throw _Exception(_("tried to create object without at least one resource"));
        
        Ref<CdsItem> item = RefCast(obj, CdsItem);
        item->setMimeType(fallbackString(row->col(_mime_type), row->col(_ref_mime_type)));
        if (IS_CDS_PURE_ITEM(objectType))
        {
            if (! obj->isVirtual())
            item->setLocation(stripLocationPrefix(row->col(_location)));
                else
            item->setLocation(stripLocationPrefix(row->col(_ref_location)));
        }
        else // URLs and active items
        {
            item->setLocation(row->col(_location));
        }
        matched_types++;
    }
    
    if (IS_CDS_ACTIVE_ITEM(objectType))
    {
        Ref<CdsActiveItem> aitem = RefCast(obj, CdsActiveItem);
        
        Ref<StringBuffer> query(new StringBuffer());
        *query << "SELECT " << TQ("id") << ',' << TQ("action") << ','
            << TQ("state") << " FROM " << TQ(CDS_ACTIVE_ITEM_TABLE)
            << " WHERE " << TQ("id") << '=' << quote(aitem->getID());
        Ref<SQLResult> resAI = select(query);
        Ref<SQLRow> rowAI;
        if (resAI != nil && (rowAI = resAI->nextRow()) != nil)
        {
            aitem->setAction(rowAI->col(1));
            aitem->setState(rowAI->col(2));
        }
        else
            throw _Exception(_("Active Item in cds_objects, but not in cds_active_item"));
        
        matched_types++;
    }

    if(! matched_types)
    {
        throw _StorageException(_("unknown object type: ")+ objectType);
    }
    return obj;
}

int SQLStorage::getTotalFiles()
{
    Ref<StringBuffer> query(new StringBuffer());
    *query << "SELECT COUNT(*) FROM " << TQ(CDS_OBJECT_TABLE) << " WHERE "
           << TQ("object_type") << " != " << quote(OBJECT_TYPE_CONTAINER);
           //<< " AND is_virtual = 0";
    Ref<SQLResult> res = select(query);
    Ref<SQLRow> row;
    if (res != nil && (row = res->nextRow()) != nil)
    {
        return row->col(0).toInt();
    }
    return 0;
}

String SQLStorage::incrementUpdateIDs(int *ids, int size)
{
    if (size <= 0)
        return nil;
    Ref<StringBuffer> inBuf(new StringBuffer()); // ??? what was that: size * sizeof(int)));
    *inBuf << "IN (" << ids[0];
    for (int i = 1; i < size; i++)
        *inBuf << ',' << ids[i];
    *inBuf << ')';
    
    Ref<StringBuffer> buf(new StringBuffer());
    *buf << "UPDATE " << TQ(CDS_OBJECT_TABLE) << " SET " << TQ("update_id") << '=' << TQ("update_id") << " + 1 WHERE " << TQ("id") << ' ';
    *buf << inBuf;
    exec(buf);
    
    buf->clear();
    *buf << "SELECT " << TQ("id") << ',' << TQ("update_id") << " FROM " << TQ(CDS_OBJECT_TABLE) << " WHERE " << TQ("id") << ' ';
    *buf << inBuf;
    Ref<SQLResult> res = select(buf);
    if (res == nil)
        throw _Exception(_("Error while fetching update ids"));
    Ref<SQLRow> row;
    buf->clear();
    while((row = res->nextRow()) != nil)
        *buf << ',' << row->col(0) << ',' << row->col(1);
    if (buf->length() <= 0)
        return nil;
    return buf->toString(1);
}

/*
Ref<Array<CdsObject> > SQLStorage::selectObjects(Ref<SelectParam> param)
{
    Ref<StringBuffer> q(new StringBuffer());
    *q << SQL_QUERY << " WHERE ";
    switch (param->flags)
    {
        case FILTER_PARENT_ID:
            *q << "f.parent_id = " << param->iArg1;
            break;
        case FILTER_REF_ID:
            *q << "f.ref_id = " << param->iArg1;
            break;
        case FILTER_PARENT_ID_ITEMS:
            *q << "f.parent_id = " << param->iArg1 << " AND "
               << "f.object_type & " << OBJECT_TYPE_ITEM << " <> 0";
            break;
        case FILTER_PARENT_ID_CONTAINERS:
            *q << "f.parent_id = " << param->iArg1 << " AND "
               << "f.object_type & " << OBJECT_TYPE_CONTAINER << " <> 0";
            break;
        default:
            throw _StorageException(_("selectObjects: invalid operation: ") +
                                   param->flags);
    }
    *q << " ORDER BY f.object_type, f.dc_title";
    Ref<SQLResult> res = select(q->toString());
    Ref<SQLRow> row;
    Ref<Array<CdsObject> > arr(new Array<CdsObject>());

    while((row = res->nextRow()) != nil)
    {
        Ref<CdsObject> obj = createObjectFromRow(row);
        arr->append(obj);
    }
    return arr;
}
*/

Ref<DBRHash<int> > SQLStorage::getObjects(int parentID, bool withoutContainer)
{
    Ref<StringBuffer> q(new StringBuffer());
    *q << "SELECT " << TQ("id") << " FROM " << TQ(CDS_OBJECT_TABLE) << " WHERE ";
    if (withoutContainer)
        *q << TQ("object_type") << " != " << OBJECT_TYPE_CONTAINER << " AND ";
    *q << TQ("parent_id") << '=';
    *q << parentID;
    Ref<SQLResult> res = select(q);
    if (res == nil)
        throw _Exception(_("db error"));
    Ref<SQLRow> row;
    
    if (res->getNumRows() <= 0)
        return nil;
    int capacity = res->getNumRows() * 5 + 1;
    if (capacity < 521)
        capacity = 521;
    
    Ref<DBRHash<int> > ret(new DBRHash<int>(capacity, res->getNumRows(), INVALID_OBJECT_ID, INVALID_OBJECT_ID_2));
    
    while ((row = res->nextRow()) != nil)
    {
        ret->put(row->col(0).toInt());
    }
    return ret;
}

Ref<IntArray> SQLStorage::removeObjects(zmm::Ref<DBRHash<int> > list, bool all)
{
    hash_data_array_t<int> hash_data_array;
    list->getAll(&hash_data_array);
    int count = hash_data_array.size;
    int *array = hash_data_array.data;
    if (count <= 0)
        return nil;
    
    Ref<StringBuffer> idsBuf(new StringBuffer());
    *idsBuf << "SELECT " << TQ("id") << ',' << TQ("object_type")
        << " FROM " << TQ(CDS_OBJECT_TABLE)
        << " WHERE " << TQ("id") << " IN (";
    for (int i = 0; i < count; i++)
    {
        int id = array[i];
        if (IS_FORBIDDEN_CDS_ID(id))
            throw _Exception(_("tried to delete a forbidden ID (") + id + ")!");
        *idsBuf << ',' << id;
    }
    *idsBuf << ')';
    Ref<SQLResult> res = select(idsBuf);
    idsBuf = nil;
    if (res == nil)
        throw _Exception(_("sql error"));
    
    Ref<StringBuffer> items(new StringBuffer());
    Ref<StringBuffer> containers(new StringBuffer());
    Ref<SQLRow> row;
    while ((row = res->nextRow()) != nil)
    {
        int objectType = row->col(1).toInt();
        if (IS_CDS_CONTAINER(objectType))
            *containers << ',' << row->col_c_str(0);
        else
            *items << ',' << row->col_c_str(0);
    }
    Ref<StringBuffer> containerIDs = _recursiveRemove(items, containers, all);
    return _purgeEmptyContainers(containerIDs);
}

void SQLStorage::_removeObjects(String objectIDs)
{
    Ref<StringBuffer> q(new StringBuffer());
    *q << "DELETE FROM " << TQ(CDS_OBJECT_TABLE)
        << " WHERE " << TQ("id") << " IN (" << objectIDs << ')';
    exec(q);
    q->clear();
    
    #warning todo: fix
    
    *q << "DELETE FROM " << TQ(CDS_ACTIVE_ITEM_TABLE)
        << " WHERE " << TQ("id") << " IN (" << objectIDs << ')';
    exec(q);
    q->clear();
    *q << "DELETE FROM " << TQ(AUTOSCAN_TABLE)
        << " WHERE " << TQ("id") << " IN (" << objectIDs << ')';
    exec(q);
}

Ref<IntArray> SQLStorage::removeObject(int objectID, bool all)
{
    Ref<StringBuffer> q(new StringBuffer());
    *q << "SELECT " << TQ("object_type") << ',' << TQ("ref_id")
        << " FROM " << TQ(CDS_OBJECT_TABLE)
        << " WHERE " << TQ("id") << '=' << quote(objectID) << " LIMIT 1";
    Ref<SQLResult> res = select(q);
    if (res == nil)
        return nil;
    Ref<SQLRow> row = res->nextRow();
    if (row == nil)
        return nil;
    
    int objectType = row->col(0).toInt();
    bool isContainer = IS_CDS_CONTAINER(objectType);
    if (all && ! isContainer)
    {
        String ref_id_str = row->col(1);
        int ref_id;
        if (string_ok(ref_id_str))
        {
            ref_id = ref_id_str.toInt();
            if (! IS_FORBIDDEN_CDS_ID(ref_id))
                objectID = ref_id;
        }
    }
    if (IS_FORBIDDEN_CDS_ID(objectID))
        throw _Exception(_("tried to delete a forbidden ID (") + objectID + ")!");
    Ref<StringBuffer> containerIDs = nil;
    Ref<StringBuffer> idsBuf(new StringBuffer());
    *idsBuf << ',' << objectID;
    if (isContainer)
        containerIDs = _recursiveRemove(nil, idsBuf, all);
    else
        containerIDs = _recursiveRemove(idsBuf, nil, all);
    return _purgeEmptyContainers(containerIDs);
}

Ref<StringBuffer> SQLStorage::_recursiveRemove(Ref<StringBuffer> items, Ref<StringBuffer> containers, bool all)
{
    log_debug("start\n");
    Ref<StringBuffer> recurseItems(new StringBuffer());
    *recurseItems << "SELECT DISTINCT " << TQ("id") << ',' << TQ("parent_id")
        << " FROM " << TQ(CDS_OBJECT_TABLE) <<
            " WHERE " << TQ("ref_id") << " IN (";
    int recurseItemsLen = recurseItems->length();
    
    Ref<StringBuffer> recurseContainers(new StringBuffer());
    *recurseContainers << "SELECT DISTINCT " << TQ("id") 
        << ',' << TQ("object_type");
    if (all)
        *recurseContainers << ',' << TQ("ref_id");
    *recurseContainers << " FROM " << TQ(CDS_OBJECT_TABLE) <<
            " WHERE " << TQ("parent_id") << " IN (";
    int recurseContainersLen = recurseContainers->length();
    
    Ref<StringBuffer> removeAddParents(new StringBuffer());
    *removeAddParents << "SELECT DISTINCT " << TQ("parent_id")
        << " FROM " << TQ(CDS_OBJECT_TABLE)
        << " WHERE " << TQ("id") << " IN (";
    int removeAddParentsLen = removeAddParents->length();
        
    Ref<StringBuffer> remove(new StringBuffer());
    Ref<StringBuffer> changedContainers(new StringBuffer());
    
    if (items != nil && items->length() > 1)
    {
        *recurseItems << items;
        *removeAddParents << items;
    }
    
    if (containers != nil && containers->length() > 1)
    {
        *recurseContainers << containers;
        *removeAddParents << containers;
    }
    
    Ref<SQLResult> res;
    Ref<SQLRow> row;
    int count = 0;
    while(recurseItems->length() > recurseItemsLen 
        || removeAddParents->length() > removeAddParentsLen 
        || recurseContainers->length() > recurseContainersLen)
    {
        if (removeAddParents->length() > removeAddParentsLen)
        {
            // add ids to remove
            *remove << removeAddParents->c_str(removeAddParentsLen);
            // get rid of first ','
            removeAddParents->setCharAt(removeAddParentsLen, ' ');
            *removeAddParents << ')';
            res = select(removeAddParents);
            // reset length
            removeAddParents->setLength(removeAddParentsLen);
            while ((row = res->nextRow()) != nil)
                *changedContainers << ',' << row->col_c_str(0);
        }
        
        if (recurseItems->length() > recurseItemsLen)
        {
            recurseItems->setCharAt(recurseItemsLen, ' ');
            *recurseItems << ')';
            res = select(recurseItems);
            recurseItems->setLength(recurseItemsLen);
            while ((row = res->nextRow()) != nil)
            {
                *remove << ',' << row->col_c_str(0);
                *changedContainers << ',' << row->col_c_str(1);
                //log_debug("refs-add id: %s; parent_id: %s\n", id.c_str(), parentId.c_str());
            }
        }
        
        if (recurseContainers->length() > recurseContainersLen)
        {
            recurseContainers->setCharAt(recurseContainersLen, ' ');
            *recurseContainers << ')';
            res = select(recurseContainers);
            recurseContainers->setLength(recurseContainersLen);
            while ((row = res->nextRow()) != nil)
            {
                //containers->append(row->col(1).toInt());
                
                int objectType = row->col(1).toInt();
                if (IS_CDS_CONTAINER(objectType))
                {
                    *recurseContainers << ',' << row->col_c_str(0);
                    *remove << ',' << row->col_c_str(0);
                }
                else
                {
                    if (all)
                    {
                        String refId = row->col(2);
                        if (string_ok(refId))
                        {
                            *removeAddParents << ',' << refId;
                            *recurseItems << ',' << refId;
                            //*remove << ',' << refId;
                        }
                        else
                        {
                            *remove << ',' << row->col_c_str(0);
                            *recurseItems << ',' << row->col_c_str(0);
                        }
                    }
                    else
                    {
                        *remove << ',' << row->col_c_str(0);
                        *recurseItems << ',' << row->col_c_str(0);
                    }
                }
                //log_debug("id: %s; parent_id: %s\n", id.c_str(), parentId.c_str());
            }
        }
        
        if (remove->length() > MAX_REMOVE_SIZE) // remove->length() > 0) // )
        {
            _removeObjects(remove->toString(1));
            remove->clear();
        }
        
        if (count++ > MAX_REMOVE_RECURSION)
            throw _Exception(_("there seems to be an infinite loop..."));
    }
    
    if (remove->length() > 0)
        _removeObjects(remove->toString(1));
    log_debug("end\n");
    return changedContainers;
}

Ref<IntArray> SQLStorage::_purgeEmptyContainers(Ref<StringBuffer> containerIDs)
{
    //int size = containerIDs->size();
    log_debug("start\n");
    if (! string_ok(containerIDs))
        return nil;
    //if (size <= 0)
    //    return;
    
    Ref<StringBuffer> bufSel(new StringBuffer());
    *bufSel << "SELECT " << TQD('a',"id")
        << ", COUNT(" << TQD('b',"parent_id") 
        << ")," << TQD('a',"parent_id") << ',' << TQD('a',"flags")
        << " FROM " << TQ(CDS_OBJECT_TABLE) << ' ' << TQ('a')
        << " LEFT JOIN " << TQ(CDS_OBJECT_TABLE) << ' ' << TQ('b')
        << " ON " << TQD('a',"id") << '=' << TQD('b',"parent_id") 
        << " WHERE " << TQD('a',"object_type") << '=' << quote(1)
        << " AND " << TQD('a',"id") << " IN (";  //(a.flags & " << OBJECT_FLAG_PERSISTENT_CONTAINER << ") = 0 AND
    int bufSelLen = bufSel->length();
    String strSel2 = _(") GROUP BY a.id"); // HAVING COUNT(b.parent_id)=0");
    
    Ref<StringBuffer> bufDel(new StringBuffer());
    Ref<IntArray> changedContainers(new IntArray());
    
    Ref<SQLResult> res;
    Ref<SQLRow> row;
    
    *bufSel << containerIDs;
    /*
    for (int i = 0; i < size; i++)
        *bufSel << ',' << quote(containerIDs->get(i));
    */
    bool again;
    int count = 0;
    do
    {
        again = false;
        bufSel->setCharAt(bufSelLen, ' ');
        *bufSel << strSel2;
        log_debug("sql: %s\n", bufSel->c_str());
        res = select(bufSel);
        bufSel->setLength(bufSelLen);
        if (res == nil)
            throw _Exception(_("db error"));
        while ((row = res->nextRow()) != nil)
        {
            int flags = row->col(3).toInt();
            if (flags & OBJECT_FLAG_PERSISTENT_CONTAINER)
                changedContainers->append(row->col(0).toInt());
            else if (row->col(1) == "0")
            {
                *bufDel << ',' << row->col_c_str(0);
                *bufSel << ',' << row->col_c_str(2);
            }
            else
            {
                *bufSel << ',' << row->col_c_str(0);
            }
        }
        //log_debug("selecting: %s; removing: %s\n", bufSel->c_str(), bufDel->c_str());
        if (bufDel->length() > 0)
        {
            _removeObjects(bufDel->toString(1));
            bufDel->clear();
            if (bufSel->length() > bufSelLen)
                again = true;
        }
        if (count++ >= MAX_REMOVE_RECURSION)
            throw _Exception(_("there seems to be a recursion..."));
    }
    while (again);
    if (bufSel->length() > bufSelLen)
        changedContainers->addCSV(bufSel->toString(bufSelLen + 1));
    log_debug("end; changedContainers: %s\n", changedContainers->toCSV().c_str());
    return changedContainers;
}

String SQLStorage::getInternalSetting(String key)
{
    Ref<StringBuffer> q(new StringBuffer());
    *q << "SELECT " << TQ("value") << " FROM " << TQ(INTERNAL_SETTINGS_TABLE) << " WHERE " << TQ("key") << '='
        << quote(key) << " LIMIT 1";
    Ref<SQLResult> res = select(q);
    if (res == nil)
        return nil;
    Ref<SQLRow> row = res->nextRow();
    if (row == nil)
        return nil;
    return row->col(0);
}
/*
void SQLStorage::storeInternalSetting(String key, String value)
overwritten due to different SQL syntax for MySQL and SQLite3
*/

void SQLStorage::updateAutoscanPersistentList(scan_mode_t scanmode, Ref<AutoscanList> list)
{
    
    log_debug("setting persistent autoscans untouched - scanmode: %s;\n", AutoscanDirectory::mapScanmode(scanmode).c_str());
    Ref<StringBuffer> q(new StringBuffer());
    *q << "UPDATE " << TQ(AUTOSCAN_TABLE)
        << " SET " << TQ("touched") << '=' << mapBool(false)
        << " WHERE "
        << TQ("persistent") << '=' << mapBool(true)
        << " AND " << TQ("scan_mode") << '='
        << quote(AutoscanDirectory::mapScanmode(scanmode));
    exec(q);
    
    int listSize = list->size();
    log_debug("updating/adding persistent autoscans (count: %d)\n", listSize);
    for (int i = 0; i < listSize; i++)
    {
        log_debug("getting ad %d from list..\n", i);
        Ref<AutoscanDirectory> ad = list->get(i);
        if (ad == nil)
            continue;
        
        // only persistent asD should be given to getAutoscanList
        assert(ad->persistent());
        // the scanmode should match the given parameter
        assert(ad->getScanMode() == scanmode);
        
        String location = ad->getLocation();
        if (! string_ok(location))
            throw _Exception(_("AutoscanDirectoy with illegal location given to SQLStorage::updateAutoscanPersistentList"));
        
        q->clear();
        *q << "SELECT " << TQ("id") << " FROM " << TQ(AUTOSCAN_TABLE)
            << " WHERE ";
        int objectID = findObjectIDByPath(location + '/');
        log_debug("objectID = %d\n", objectID);
        if (objectID == INVALID_OBJECT_ID)
            *q << TQ("location") << '=' << quote(location);
        else
            *q << TQ("obj_id") << '=' << quote(objectID);
        *q << " LIMIT 1";
        Ref<SQLResult> res = select(q);
        if (res == nil)
            throw _StorageException(_("query error while selecting from autoscan list"));
        Ref<SQLRow> row;
        if ((row = res->nextRow()) != nil)
        {
            ad->setStorageID(row->col(0).toInt());
            updateAutoscanDirectory(ad);
        }
        else
            addAutoscanDirectory(ad);
    }
    
    q->clear();
    *q << "DELETE FROM " << TQ(AUTOSCAN_TABLE)
        << " WHERE " << TQ("touched") << '=' << mapBool(false);
    exec(q);
}

Ref<AutoscanList> SQLStorage::getAutoscanList(scan_mode_t scanmode)
{
    #define FLD(field) << TQD('a',field) <<
    Ref<StringBuffer> q(new StringBuffer());
    *q << "SELECT " FLD("id") ',' FLD("obj_id") ',' FLD("scan_level") ','
       FLD("scan_mode") ',' FLD("recursive") ',' FLD("hidden") ','
       FLD("interval") ',' FLD("last_modified") ',' FLD("persistent") ','
       FLD("location") ',' << TQD('t',"location")
       << " FROM " << TQ(AUTOSCAN_TABLE) << ' ' << TQ('a')
       << " LEFT JOIN " << TQ(CDS_OBJECT_TABLE) << ' ' << TQ('t')
       << " ON " FLD("obj_id") '=' << TQD('t',"id")
       << " WHERE " FLD("scan_mode") '=' << quote(AutoscanDirectory::mapScanmode(scanmode));
    Ref<SQLResult> res = select(q);
    if (res == nil)
        throw _StorageException(_("query error while fetching autoscan list"));
    Ref<AutoscanList> ret(new AutoscanList());
    Ref<SQLRow> row;
    while((row = res->nextRow()) != nil)
    {
        int objectID = INVALID_OBJECT_ID;
        String objectIDstr = row->col(1);
        if (string_ok(objectIDstr))
            objectID = objectIDstr.toInt();
        int storageID = row->col(0).toInt();
        
        String location;
        if (objectID == INVALID_OBJECT_ID)
            location = row->col(9);
        else
        {
            char prefix;
            location = stripLocationPrefix(&prefix, row->col(10));
            if (prefix != LOC_DIR_PREFIX)
                throw _Exception(_("mt_autoscan referred to an object, that was not a directory - id: ") + objectID + "; location: " + row->col(9) + "; prefix: " + prefix);
        }
        
        scan_level_t level = AutoscanDirectory::remapScanlevel(row->col(2));
        scan_mode_t mode = AutoscanDirectory::remapScanmode(row->col(3));
        bool recursive = remapBool(row->col(4));
        bool hidden = remapBool(row->col(5));
        bool persistent = remapBool(row->col(8));
        int interval = 0;
        if (mode == TimedScanMode)
            interval = row->col(6).toInt();
        time_t last_modified = row->col(7).toLong();
        
        log_debug("adding autoscan location: %s; recursive: %d\n", location.c_str(), recursive);
        
        Ref<AutoscanDirectory> dir(new AutoscanDirectory(location, mode, level, recursive, persistent, -1, interval, hidden));
        dir->setObjectID(objectID);
        dir->setStorageID(storageID);
        dir->setCurrentLMT(last_modified);
        dir->updateLMT();
        ret->add(dir);
    }
    
    return ret;
}

int SQLStorage::addAutoscanDirectory(Ref<AutoscanDirectory> adir)
{
    if (adir->getStorageID() >= 0)
        throw _Exception(_("tried to add autoscan directory with a storage id set"));
    int objectID = findObjectIDByPath(adir->getLocation() + DIR_SEPARATOR);
    if (! adir->persistent() && objectID < 0)
        throw _Exception(_("tried to add non-persistent autoscan directory with an illegal objectID or location"));
    
    _autoscanChangePersistentFlag(objectID, true);
    
    Ref<StringBuffer> q(new StringBuffer());
    *q << "INSERT INTO " << TQ(AUTOSCAN_TABLE)
        << " (" << TQ("obj_id") << ','
        << TQ("scan_level") << ','
        << TQ("scan_mode") << ','
        << TQ("recursive") << ','
        << TQ("hidden") << ','
        << TQ("interval") << ','
        << TQ("last_modified") << ','
        << TQ("persistent") << ','
        << TQ("location")
        << ") VALUES ("
        << (objectID >= 0 ? quote(objectID) : _(SQL_NULL)) << ','
        << quote(AutoscanDirectory::mapScanlevel(adir->getScanLevel())) << ','
        << quote(AutoscanDirectory::mapScanmode(adir->getScanMode())) << ','
        << mapBool(adir->getRecursive()) << ','
        << mapBool(adir->getHidden()) << ','
        << quote(adir->getInterval()) << ','
        << quote(adir->getPreviousLMT()) << ','
        << mapBool(adir->persistent()) << ','
        << (objectID >= 0 ? _(SQL_NULL) : quote(adir->getLocation()))
        << ')';
    return exec(q, true);
}

void SQLStorage::updateAutoscanDirectory(Ref<AutoscanDirectory> adir)
{
    log_debug("id: %d, obj_id: %d\n", adir->getStorageID(), adir->getObjectID());
    int objectID = adir->getObjectID();
    int objectIDold = _getAutoscanObjectID(adir->getStorageID());
    if (objectIDold != objectID)
    {
        _autoscanChangePersistentFlag(objectIDold, false);
        _autoscanChangePersistentFlag(objectID, true);
    }
    Ref<StringBuffer> q(new StringBuffer());
    *q << "UPDATE " << TQ(AUTOSCAN_TABLE)
        << " SET " << TQ("obj_id") << '=' << (objectID >= 0 ? quote(objectID) : _(SQL_NULL))
        << ',' << TQ("scan_level") << '='
        << quote(AutoscanDirectory::mapScanlevel(adir->getScanLevel()))
        << ',' << TQ("scan_mode") << '='
        << quote(AutoscanDirectory::mapScanmode(adir->getScanMode()))
        << ',' << TQ("recursive") << '=' << mapBool(adir->getRecursive())
        << ',' << TQ("hidden") << '=' << mapBool(adir->getHidden())
        << ',' << TQ("interval") << '=' << quote(adir->getInterval());
    if (adir->getPreviousLMT() > 0)
        *q << ',' << TQ("last_modified") << '=' << quote(adir->getPreviousLMT());
    *q << ',' << TQ("persistent") << '=' << mapBool(adir->persistent())
        << ',' << TQ("location") << '=' << (objectID >= 0 ? _(SQL_NULL) : quote(adir->getLocation()))
        << ',' << TQ("touched") << '=' << mapBool(true)
        << " WHERE " << TQ("id") << '=' << quote(adir->getStorageID());
    exec(q);
}

void SQLStorage::removeAutoscanDirectoryByObjectID(int objectID)
{
    if (objectID == INVALID_OBJECT_ID)
        return;
    Ref<StringBuffer> q(new StringBuffer());
    *q << "DELETE FROM " << TQ(AUTOSCAN_TABLE)
        << " WHERE " << TQ("obj_id") << '=' << quote(objectID);
    exec(q);
    
    _autoscanChangePersistentFlag(objectID, false);
}

void SQLStorage::removeAutoscanDirectory(int autoscanID)
{
    if (autoscanID == INVALID_OBJECT_ID)
        return;
    int objectID = _getAutoscanObjectID(autoscanID);
    Ref<StringBuffer> q(new StringBuffer());
    *q << "DELETE FROM " << TQ(AUTOSCAN_TABLE)
        << " WHERE " << TQ("id") << '=' << quote(autoscanID);
    exec(q);
    if (objectID != INVALID_OBJECT_ID)
        _autoscanChangePersistentFlag(objectID, false);
}

int SQLStorage::getAutoscanDirectoryType(int objectID)
{
    if (objectID == INVALID_OBJECT_ID)
        return 0;
    Ref<StringBuffer> q(new StringBuffer());
    *q << "SELECT " << TQ("persistent") << " FROM " << TQ(AUTOSCAN_TABLE)
        << " WHERE " << TQ("obj_id") << '=' << quote(objectID);
    Ref<SQLResult> res = select(q);
    Ref<SQLRow> row;
    if (res == nil || (row = res->nextRow()) == nil)
        return 0;
    if (! remapBool(row->col(0)))
        return 1;
    else
        return 2;
}

int SQLStorage::_getAutoscanObjectID(int autoscanID)
{
    Ref<StringBuffer> q(new StringBuffer());
    *q << "SELECT " << TQ("obj_id") << " FROM " << TQ(AUTOSCAN_TABLE)
        << " WHERE " << TQ("id") << '=' << quote(autoscanID)
        << " LIMIT 1";
    Ref<SQLResult> res = select(q);
    if (res == nil)
        throw _StorageException(_("error while doing select on "));
    Ref<SQLRow> row;
    if ((row = res->nextRow()) != nil && string_ok(row->col(0)))
        return row->col(0).toInt();
    return INVALID_OBJECT_ID;
}

void SQLStorage::_autoscanChangePersistentFlag(int objectID, bool persistent)
{
    Ref<StringBuffer> q(new StringBuffer());
    *q << "UPDATE " << TQ(CDS_OBJECT_TABLE)
        << " SET " << TQ("flags") << " = (" << TQ("flags")
        << (persistent ? _(" | ") : _(" & ~"))
        << OBJECT_FLAG_PERSISTENT_CONTAINER
        << ") WHERE " << TQ("id") << '=' << quote(objectID);
    exec(q);
}

void SQLStorage::autoscanUpdateLM(zmm::Ref<AutoscanDirectory> adir)
{
    /*
    int objectID = adir->getObjectID();
    if (IS_FORBIDDEN_CDS_ID(objectID))
    {
        objectID = findObjectIDByPath(adir->getLocation() + '/');
        if (IS_FORBIDDEN_CDS_ID(objectID))
            throw _Exception(_("autoscanUpdateLM called with adir with illegal objectID and location"));
    }
    */
    log_debug("id: %d; last_modified: %d\n", adir->getStorageID(), adir->getPreviousLMT());
    Ref<StringBuffer> q(new StringBuffer());
    *q << "UPDATE " << TQ(AUTOSCAN_TABLE)
        << " SET " << TQ("last_modified") << '=' << quote(adir->getPreviousLMT())
        << " WHERE " << TQ("id") << '=' << quote(adir->getStorageID());
    exec(q);
}

String SQLStorage::getFsRootName()
{
    if (string_ok(fsRootName))
        return fsRootName;
    setFsRootName();
    return fsRootName;
}

void SQLStorage::setFsRootName(String rootName)
{
    if (string_ok(rootName))
    {
        fsRootName = rootName;
    }
    else
    {
        Ref<CdsObject> fsRootObj = loadObject(CDS_ID_FS_ROOT);
        fsRootName = fsRootObj->getTitle();
    }
}
