#!/bin/bash -e

#to be called from the doc/ directory (here)

VERSION=`git describe --abbrev=0`

mkdir -p publish-deken/

rm -Rf publish-deken/Fraise
cd ..

git archive --prefix=Fraise/ HEAD | (cd doc/publish-deken && tar xf -)

cd doc/publish-deken

echo $VERSION > Fraise/VERSION.txt

deken package --version $VERSION Fraise

filename=Fraise[v$VERSION]
mv "$filename(Sources).dek"        "$filename.dek"
mv "$filename(Sources).dek.sha256" "$filename.dek.sha256"
if [ -e "$filename(Sources).dek.asc" ]; then
	mv "$filename(Sources).dek.asc"    "$filename.dek.asc"
fi

if [ x$1 != xnoupload ] ; then
	deken upload --no-source-error "$filename.dek"
fi

cd ..

