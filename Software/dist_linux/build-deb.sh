#
# lightpack/Software/build-deb.sh
#
#     Author: brunql
# Created on: 25.12.11
#

#TODO: get the version from a centralized point, eg. version.h
VERSION=5.11.2.6

if [ -z $1 ];
then
    echo "usage: $0 <arch>"
    exit 1
fi

perl prepare_deb.pl $VERSION $1 || exit 1;

chmod a+x deb/DEBIAN/control || exit 1;

if [ -e "deb/usr/bin/Prismatik" ];
then
	echo "Renaming 'deb/usr/bin/Prismatik' to 'deb/usr/bin/prismatik'."
	mv deb/usr/bin/Prismatik deb/usr/bin/prismatik
fi

if [ ! -e "deb/usr/bin/prismatik" ];
then
	echo "File 'deb/usr/bin/prismatik' not found."
	exit 2
fi


if [ -x "`which md5deep 2>/dev/null`" ]; 
then
	# Update MD5 sums
	md5deep -r deb/usr > deb/DEBIAN/md5sums
else
	echo "Please install 'md5deep' package first."
	exit 5
fi

# Build package in format "name_version_arch.deb"
fakeroot dpkg-deb --build deb/ .
