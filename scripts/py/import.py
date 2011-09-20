
from time import time, ctime
import mediatomb
mediatomb.log( "I'm running PYTHON! (%s)" % ctime(time()))

mediatomb.log( mediatomb.getObj() )
print dir(mediatomb)
media = mediatomb.MediaTomb()
print dir(media)

print media

# mediatomb.log( "path:%s , location:%s" % (media.path(), media.location()) )

mediatomb.log( "foo %s " % media.path )
media.path = "kittens!"

mediatomb.log( "foo %s " % media.path )
# mediatomb.log( "path:%s , location:%s" % (media.path, media.location) )


mediatomb.log("bye")
# get the current object
# switch on mime
