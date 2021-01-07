#!/bin/bash -e

#to be called from the doc/ directory (here)

VERSION=`git describe`

rm -Rf publish-deken/
mkdir -p publish-deken/
cd ..

git archive --prefix=Fraise/ HEAD | (cd doc/publish-deken && tar xf -)

cd doc/publish-deken

echo $VERSION > Fraise/VERSION.txt

deken upload --version $VERSION Fraise

