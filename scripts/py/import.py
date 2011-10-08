
from time import time, ctime
print "Ya Dawg, I heard you like bases?"

import mediatomb
mediatomb.log( "I'm running PYTHON! (%s)" % ctime(time()))



def CheckObjectInput():
	mediatomb.log( "Checking Object Input")
	media = mediatomb.MediaTomb()
	print dir(media)
	mediatomb.log ("title %s" % media.title)
	mediatomb.log ("Path %s" % media.path)
	mediatomb.log ("Location %s" % media.location)
	mediatomb.log ("Upnpclass %s" % media.upnpclass)
	mediatomb.log( mediatomb.getObj() )
	

CheckObjectInput();
