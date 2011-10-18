
import mediatomb
import re

def getYear(date):
	return ()

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

def getRootPath(rootpath,location):
    return (media.location);

def addVideo(media):

    chain =  ['Video', 'All Video'];
    mediatomb.log("Chain : %s" % createContainerChain(chain) )   
    mediatomb.addCdsObject(media, createContainerChain(chain) );

 #   mediatomb.addCdsObject(media, "" );
 #   mediatomb.addCdsObject(media, None );
    Dir = getRootPath('',media.location);
    
    if (len(Dir) > 0):
        chain = ('Video', 'Directories');
        #chain = chain.concat(dir);

        mediatomb.addCdsObject(media, createContainerChain(chain));
       
	return ();
	
def addAudio(media):
	#Gather Information
	"""	
	Keys ['upnp:originalTrackNumber', 'dc:title', 'upnp:artist', 'dc:date', 'upnp:genre', 'upnp:album', 'dc:description']
	"""
	
	desc = ''; 
	artist_full = '';
	album_full = '' ; 
	
	if  not (media.meta['dc:title']) : 
		print "no Title";
		title = media.title;
	else:
		title = media.meta['dc:title'];
	
	if not (media.meta['upnp:artist']):
		artist = "unknow";
		artist_full = None;
	else:
		artist = media.meta['upnp:artist']
		artist_full = artist;
		desc = artist;
	
	if not (media.meta['upnp:album']):
		album = 'UnKnown';
		album_full = None;
	else:
		album = media.meta['upnp:album'];
		desc = desc + album;
		album_full = album;
	if desc:
		desc = desc + ',';
		desc = desc + title;
		mediatomb.log("Description: %s " % desc);
	
	if not media.meta['dc:date']:
		date = 'unknow';
	else:
		date = getYear(media.meta['dc:date']);
		desc = desc + ', ' + str(date);
	if not media.meta['upnp:genre']:
		genre = "UnKnown";
	else:
		genre = media.meta['upnp:genre'];
		desc = desc + ', ' + genre;
		
	if not media.meta['dc:description']:
		media.meta['dc:description'] = desc;
		
	"""
				
	"""
"""
	track = media.meta['upnp:originalTrackNumber'];
	if track == '':
		track = '';   # TODO
	else:
	track='00';
	if (len(track) == 1)
                track = '0' + track;
			track = track + ' ';
	"""
	#track = '' ;
	#chain = arry('Audio,All,Audio');
	
"""
function addAudio(obj)
{
       
*/
    // comment the following line out if you uncomment the stuff above  :)
    var track = '';

    var chain = new Array('Audio', 'All Audio');
    obj.title = title;
    addCdsObject(obj, createContainerChain(chain));
    
    chain = new Array('Audio', 'Artists', artist, 'All Songs');
    addCdsObject(obj, createContainerChain(chain));
    
    chain = new Array('Audio', 'All - full name');
    var temp = '';
    if (artist_full)
        temp = artist_full;
    
    if (album_full)
        temp = temp + ' - ' + album_full + ' - ';
    else
        temp = temp + ' - ';
   
    obj.title = temp + title;
    addCdsObject(obj, createContainerChain(chain));
    
    chain = new Array('Audio', 'Artists', artist, 'All - full name');
    addCdsObject(obj, createContainerChain(chain));
    
    chain = new Array('Audio', 'Artists', artist, album);
    obj.title = track + title;
    addCdsObject(obj, createContainerChain(chain), UPNP_CLASS_CONTAINER_MUSIC_ALBUM);
    
    chain = new Array('Audio', 'Albums', album);
    obj.title = track + title; 
    addCdsObject(obj, createContainerChain(chain), UPNP_CLASS_CONTAINER_MUSIC_ALBUM);
    
    chain = new Array('Audio', 'Genres', genre);
    addCdsObject(obj, createContainerChain(chain), UPNP_CLASS_CONTAINER_MUSIC_GENRE);
    
    chain = new Array('Audio', 'Year', date);
    addCdsObject(obj, createContainerChain(chain));
}"""

if __name__ == '__main__':
    # grab the current media
    media = mediatomb.MediaTomb()

    # can I haz video?
    if getPlaylistType( media.mimetype) is None:
        mime = media.mimetype.split('/')[0]
        mediatomb.log("mimetype : %s" % mime )

        ## media.refID = media.id
        if mime == 'audio':
            mediatomb.log("I Haz Auddio : (%s)" % media.title)
            addAudio(media)
            
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



