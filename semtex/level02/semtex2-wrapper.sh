#!/bin/sh
export LD_PRELOAD=./intercept.so.1.0.1
exec /semtex/whoisme
