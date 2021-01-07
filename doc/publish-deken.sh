#!/bin/bash -e

#to be called from the doc/ directory (here)

VERSION=`git describe`

rm -Rf publish-deken/
mkdir -p publish-deken/
cd ..

git archive --prefix=Fraise/ HEAD | (cd doc/publish-deken && tar xf -)

cd doc/publish-deken

echo $VERSION > Fraise/VERSION.txt

deken package --version $VERSION Fraise

mv "Fraise[v$VERSION](Sources).dek"        "Fraise[v$VERSION].dek"
mv "Fraise[v$VERSION](Sources).dek.asc"    "Fraise[v$VERSION].dek.asc"
mv "Fraise[v$VERSION](Sources).dek.sha256" "Fraise[v$VERSION].dek.sha256"

deken upload --no-source-error "Fraise[v$VERSION].dek"

