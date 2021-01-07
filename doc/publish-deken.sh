#!/bin/bash -e

VERSION=`git describe`

rm -Rf publish-deken/
mkdir -p publish-deken/
cd ..
#git archive HEAD > doc/publish-deken/Fraise.tar
git archive --prefix=Fraise/ HEAD | (cd doc/publish-deken && tar xf -)
cd doc/publish-deken
deken upload --version $VERSION Fraise

