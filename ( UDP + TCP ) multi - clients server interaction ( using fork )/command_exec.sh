#!/bin/sh

cat $1
$2 > $1 2>&1
