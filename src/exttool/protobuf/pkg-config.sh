#!/bin/bash

pkg-config protobuf --libs &>/dev/null
if [ $? -eq 0 ]
then
	pkg-config protobuf $1
	exit
fi

R="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ "$1" = '--cflags' ]
then
	echo '-I'$R/include '-pthread'
elif [ "$1" = '--libs' ]
then
	if [ ! -f $R/lib/libprotobuf.so ]; then ln -s /usr/lib/libprotobuf.so.7 $R/lib/libprotobuf.so; fi
	echo '-L'$R/lib' -lprotobuf -lz -pthread -lpthread'
fi



