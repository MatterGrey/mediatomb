
import mediatomb
import re
import os 
import datetime


def getYear(date):
	# place holder 
	return (date[:4])

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

    chain =  ('Video', 'All Video');
    mediatomb.log("Adding : /%s/%s"  % ("/".join(chain), media.title) )
    mediatomb.addCdsObject(media, createContainerChain(chain) );

    (head, tail) =  os.path.split(media.location);
    parts = head.split('/')
    
    chain = ['Video', 'Directories']
    if (len(parts) > 3):        
        chain = chain + parts[-3:]
    elif (len(parts) > 2):        
        chain = chain + parts[-2:]
    elif (len(parts) > 1):        
        chain = chain + parts[-1:]
    else:
        return ()
    
    mediatomb.log("Adding : /%s/%s"   % ( "/".join(chain), media.title) )
    mediatomb.addCdsObject(media, createContainerChain(chain))
    return ();

def addVideoByDate(media):
    
    # grab some date referene. Ctime seems like a start.    
    statinfo   = os.stat(media.location)
    ctime      = datetime.datetime.fromtimestamp(statinfo.st_ctime)
    
    # Calculate how recent this date market was.
    now        = datetime.datetime.now()
    delta =  now - ctime
    
    #mediatomb.log("[%-40s] created %4d days ago"  % (media.title,delta.days) )
    
    # Most UPnP client sort using alphabet/numbers so give it some hints on 
    # sortting.
    ## Note that the following will add the same video to MULTIPLE locations.
    
    chain = ('Video', 'Added')
    
    # exactly 0, 1, or 2 days since creation. If it matches one of these 
    # criteria the media will be added.
    if delta.days == 0 :
        chain = ('Video', 'Added', '00 Today' );
        mediatomb.log("Adding : /%s/%s"  % ("/".join(chain), media.title) )
        mediatomb.addCdsObject(media, createContainerChain(chain));         

    if delta.days == 1 :
        chain = ('Video', 'Added', '01 Yesterday' );
        mediatomb.log("Adding : /%s/%s"  % ("/".join(chain), media.title) )
        mediatomb.addCdsObject(media, createContainerChain(chain)); 

    if delta.days == 2 :
        chain = ('Video', 'Added', '02 Two Days Ago' );
        mediatomb.log("Adding : /%s/%s"  % ("/".join(chain), media.title) )
        mediatomb.addCdsObject(media, createContainerChain(chain)); 

    # one of "less than a wee" XOR "less than two weeks" XOR 
    # "less than one month".
    if delta.days <= 7 :
        chain = ('Video', 'Added', '03 In the Last One Week' );
        mediatomb.log("Adding : /%s/%s"  % ("/".join(chain), media.title) )
        mediatomb.addCdsObject(media, createContainerChain(chain)); 
    
    elif delta.days <= 14 :
        chain = ('Video', 'Added', '04 In the Last Two Week' );
        mediatomb.log("Adding : /%s/%s"  % ("/".join(chain), media.title) )
        mediatomb.addCdsObject(media, createContainerChain(chain)); 
    
    elif delta.days <= 30 :
        chain = ('Video', 'Added', '05 In the Last Month' );
        mediatomb.log("Adding : /%s/%s"  % ("/".join(chain), media.title) )
        mediatomb.addCdsObject(media, createContainerChain(chain)); 
    else:
        # we don't want to polute the top level directory with all the videos,
        # so we skip over the rest.
        return ();
    return ();



def addAudio(media):
	#Gather Information
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
	
	mediatomb.log("Description: %s " % desc);
	## Start to add media
	
	""" Add track to album view , TODO rewrite this horrible bit of code 
	track = media.meta['upnp:originalTrackNumber'];
	if track == '':
		track = '';   # TODO
	else:
	track='00';
	if (len(track) == 1)
                track = '0' + track;
			track = track + ' ';
	"""
	"""
    chain =  ['Video', 'All Video'];
    mediatomb.log("Chain : %s" % createContainerChain(chain) )   
    mediatomb.addCdsObject(media, createContainerChain(chain) );
    """
	addCdsObject = mediatomb.addCdsObject;
	track = '' ;   # part of above todo
	chain = ['Audio,All,Audio'];
	media.title = title; ## why
	addCdsObject(media,createContainerChain(chain));	
	
	chain = ['Audio','Artists',artist,'All Songs'];
	addCdsObject(media,createContainerChain(chain));	
	
	chain = ['Audio','All - full name'];
	if (artist_full):
		temp = artist_full;
	if (album_full):
		temp = temp + ' - ' + album_full + ' - ';
	else:
		temp = temp +' - ';
	media.title= temp + title;
	addCdsObject(media,createContainerChain(chain));
	
	chain = ['Audio' ,'Artists' , artist , 'All - full name'];
	addCdsObject(media,createContainerChain(chain));
	
	chain = ['Audio' , 'Artists' , artist , album];
	addCdsObject(media,createContainerChain(chain)); 
	media.title = track + title;
	addCdsObject(media,createContainerChain(chain)); # UPNP_CLASS_CONTAINER_MUSIC ALBUM need to find out about this?
	
	chain = ['Audio' , 'Albums' , album];
	media.title = track + title;
	addCdsObject(media,createContainerChain(chain)); # UPNP_CLASS_CONTAINER_MUSIC_ALBUM
	
	chain = ['Audio' , 'Genres' , genre];
	addCdsObject(media,createContainerChain(chain)); # UPNP_CLASS_CONTAINER_MUSIC_GENRE
	
	chain = ['Audio' , 'Year' , date];
	addCdsObject(media,createContainerChain(chain));


def dumpMedia(media):
    for key in dir(media):
            #mediatomb.log("%20s -> %40s" % (key, "" ) )
            mediatomb.log("%20s -> %-s" % (key, getattr(media,key) ) )


if __name__ == '__main__':
    # grab the current media
    media = mediatomb.MediaTomb()

    # can I haz video?
    if getPlaylistType( media.mimetype) is None:
        mime = media.mimetype.split('/')[0]
        #mediatomb.log("mimetype : %s" % mime )

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
            addVideoByDate(media)
            
        #dumpMedia(media)

    
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



