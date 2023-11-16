#!/bin/bash -e

#to be called from the doc/ directory (here)

VERSION=`git describe --abbrev=0`

#rm -Rf publish-deken/
mkdir -p publish-deken/

OSes="linux64 linuxarm macos windows"

declare -A deken_arch
deken_arch[linux64]=Linux-amd64-32
deken_arch[linuxarm]=Linux-armv7-32
deken_arch[macos]=Darwin-amd64-32
deken_arch[windows]=Windows-amd64-32

declare -A extra_ext
extra_ext[linux64]=".l_amd64"
extra_ext[linuxarm]=".l_arm"
extra_ext[macos]=".d_amd64 .pd_darwin .d_fat"
extra_ext[windows]=".w64 .dll shell.pd system-help.pd"

for os in $OSes; do
	rm -Rf publish-deken/Fraise
	cd ..

	git archive --prefix=Fraise/ HEAD | (cd doc/publish-deken && tar xf -)

	cd doc/publish-deken

	echo $VERSION > Fraise/VERSION.txt

	mv Fraise/bin Fraise/bin_tmp
	mkdir Fraise/bin
	mv Fraise/bin_tmp/$os Fraise/bin_tmp/share Fraise/bin
	rm -Rf Fraise/bin_tmp

	mv Fraise/bin/$os/bin_config.txt Fraise/bin/
	DEKEN_ARCH=${deken_arch[$os]}

	rm -f Fraise/extra/*.l_arm64 # no linux arm64 target yet
	for otheros in $OSes; do
		if [ $otheros != $os ] ; then
			for ext in ${extra_ext[$otheros]}; do
				rm -f Fraise/extra/*$ext
			done
		fi
	done

	deken package --version $VERSION Fraise

	mv "Fraise[v$VERSION](Sources).dek"        "Fraise[v$VERSION]($DEKEN_ARCH).dek"
	mv "Fraise[v$VERSION](Sources).dek.sha256" "Fraise[v$VERSION]($DEKEN_ARCH).dek.sha256"
	if [ -e "Fraise[v$VERSION](Sources).dek.asc" ]; then
		mv "Fraise[v$VERSION](Sources).dek.asc"    "Fraise[v$VERSION]($DEKEN_ARCH).dek.asc"
	fi

	if [ x$1 != xnoupload ] ; then
		deken upload --no-source-error "Fraise[v$VERSION]($DEKEN_ARCH).dek"
	fi
	
	cd ..
done


