#!/bin/bash

#$2=pdsend port
#$1=directory to make 

$1 | sed 's/$/;/' | sed 's/^/make/' | pdsend $2

