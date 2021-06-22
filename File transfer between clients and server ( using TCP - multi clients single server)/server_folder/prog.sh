#!/bin/sh

if [ ! -d "$1" ]; then
mkdir $1
fi
mv $2 $1
