#
# lightpack/Software/build-deb.sh
#
#     Author: brunql
# Created on: 25.12.11
#

VERSION=`cat ../VERSION`

if [ -z $1 ];
then
    echo "usage: $0 <arch>"
    exit 1
fi

perl prepare_deb.pl $VERSION $1 || exit 1;

chmod 0644 deb/DEBIAN/control || exit 1;
chmod 0755 deb/DEBIAN/postinst || exit 1;

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


if [ -x "`which hashdeep 2>/dev/null`" ]; 
then
	# Update MD5 sums
	hashdeep -r deb/usr > deb/DEBIAN/md5sums
	chmod 0644 deb/DEBIAN/md5sums
else
	echo "Please install 'hashdeep' package first."
	exit 5
fi

# Build package in format "name_version_arch.deb"
fakeroot dpkg-deb --build deb/ .
