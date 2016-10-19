#!/usr/bin/python
#Rewrite in python
#So, apparently python's bignumber performance is very very fast compared to perl's. Good to know!
import sys
import re
import time
from odict import odict

startTime=time.time()
print startTime
addvalue=27137
hash=92156295871308407838808214521283596197005567493826981266515267734732800
attempts=odict()
match=0
tries=0
verbose =  (len(sys.argv)>1 and re.search("v",sys.argv[1]))

print "Semtex level 7 brute forcer (python version), by Chris Stufflebeam.\nTip: To see what the script is actually doing, try the -v param.\n"

while(1):
    for i in range(match, 127):
        match = 32
        tries = tries + 1
        if hash % (addvalue + i) == 0:
            attempts[hash] = i
            hash = hash / (addvalue + i)
            if hash==1:
                timeDelta=time.time()-startTime
                print "Winner found in %d attempts (%0.3f seconds elapsed) (%d attempts per second):" % (tries, timeDelta,tries/timeDelta)
                tmp=""
                for k, v in attempts.iteritems():
                    tmp +=  "%c" % v
                print tmp
                sys.exit()
            match  = i
            break

    if match == 32:
        try:
            (hash, match) = attempts.popitem()
        except KeyError:
            print "Failed to break hash. :(."
            sys.exit()
        match = match + 1

    #Intentionally not resetting match.
    if verbose:
        tmp=""
        for k,v in attempts.iteritems():
            tmp += "%d," % v
        print "%d: Depth(%d) Match(%d) Attempts(%s)" % (tries, len(attempts), match, tmp)
