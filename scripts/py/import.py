
import pprint
from time import time, ctime
print "Ya Dawg, I heard you like bases?"

import mediatomb
mediatomb.log( "I'm running PYTHON! (%s)" % ctime(time()))

'''
mediatomb.log( "mediatomb is in the house")

for k in dir(mediatomb):
    print "%20s -> %s" % (k, getattr(mediatomb,k))

mediatomb.log( "stweeie said 'media'")
media = mediatomb.MediaTomb()

#media.meta['kittens']  = 'foo'

for k in dir(media):
    print "%20s -> " % (k),
    pprint.pprint( getattr(media,k) )

mediatomb.log("bye")
'''

#print media

# mediatomb.log( "path:%s , location:%s" % (media.path(), media.location()) )

#mediatomb.log( "foo %s " % media.path )
#media.path = "kittens!"

#mediatomb.log( "foo %s " % media.path )
# mediatomb.log( "path:%s , location:%s" % (media.path, media.location) )


#mediatomb.log("bye")
# get the current object
# switch on mime

import re
def escapeSlash(name):

    name = re.sub( r'\\',   r'\\\\', name );
    name = re.sub( r'/\//', r'\\/',  name );
    return name;


def createContainerChain(arr):
    return '/' + '/'.join([ escapeSlash(x)  for x in arr ]) 


def getPlaylistType(mimetype):
    if mimetype == 'audio/x-mpegurl':
        return 'm3u';
    if mimetype == 'audio/x-scpls':
        return 'pls';
    return None;

def addVideo(media):

    chain =  ['Video', 'All Video'];
    mediatomb.log("Chain : %s" % createContainerChain(chain) )   
    mediatomb.addCdsObject(media, createContainerChain(chain) );

    mediatomb.addCdsObject(media, "" );
    mediatomb.addCdsObject(media, None );
    
"""
    var dir = getRootPath(object_root_path, obj.location);

    if (dir.length > 0)
    {
        chain = new Array('Video', 'Directories');
        chain = chain.concat(dir);

        addCdsObject(obj, createContainerChain(chain));
    }
"""



if __name__ == '__main__':
    # grab the current media
    media = mediatomb.MediaTomb()

    # can I haz video?
    if getPlaylistType( media.mimetype) is None:
        mime = media.mimetype.split('/')[0]
        mediatomb.log("mimetype : %s" % mime )

        ## media.refID = media.id
        if mime == 'audio':
            mediatomb.log("audio: ignoring (%s)" % media.title )
            
        if mime == 'image':
            mediatomb.log("image: ignoring (%s)" % media.title )
            
        if media.mimetype == 'application/ogg' and media.theora is False:
            mediatomb.log("audio: (%s)" % media.title )

        if mime == 'video' or media.mimetype == 'application/ogg':
            mediatomb.log("I haz viddeo : (%s)" % media.title )
            addVideo(media)
            
'''
if (getPlaylistType(orig.mimetype) == '')
{
    var arr = orig.mimetype.split('/');
    var mime = arr[0];
    
    // var obj = copyObject(orig);
    
    var obj = orig; 
    obj.refID = orig.id;
    
    if (mime == 'audio')
    {
        if (obj.onlineservice == ONLINE_SERVICE_WEBORAMA)
            addWeborama(obj);
        else
            addAudio(obj);
    }
    
    if (mime == 'video')
    {
        if (obj.onlineservice == ONLINE_SERVICE_YOUTUBE)
            addYouTube(obj);
        else if (obj.onlineservice == ONLINE_SERVICE_APPLE_TRAILERS)
            addTrailer(obj);
        else
            addVideo(obj);
    }
    
    if (mime == 'image')
    {
        addImage(obj);
    }

    if (orig.mimetype == 'application/ogg')
    {
        if (orig.theora == 1)
            addVideo(obj);
        else
            addAudio(obj);
    }
}
'''



