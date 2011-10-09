<<<<<<< HEAD
=======

import pprint
from time import time, ctime
print "Ya Dawg, I heard you like bases?"

import mediatomb
mediatomb.log( "I'm running PYTHON! (%s)" % ctime(time()))

#print dir(mediatomb)

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


#print media

# mediatomb.log( "path:%s , location:%s" % (media.path(), media.location()) )

#mediatomb.log( "foo %s " % media.path )
#media.path = "kittens!"

#mediatomb.log( "foo %s " % media.path )
# mediatomb.log( "path:%s , location:%s" % (media.path, media.location) )


#mediatomb.log("bye")
# get the current object
# switch on mime


>>>>>>> upstream/python
