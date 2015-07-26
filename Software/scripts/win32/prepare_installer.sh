#!/bin/bash
# call this from $repo/Software
cp src/bin/*.dll dist_windows/content/
cp src/bin/*.exe dist_windows/content/

# Use windeployqt to copy Qt dependencies for running Prismatik on Windows
$QTDIR/bin/windeployqt --release --no-angle dist_windows/content/Prismatik.exe

# This assumes the build happens on a x64 machine
if [ "$1" = "x86" ]
then
	cp $WINDIR/SysWOW64/msvcr120.dll dist_windows/content/
	cp $WINDIR/SysWOW64/msvcp120.dll dist_windows/content/
else
	cp $WINDIR/System32/msvcr120.dll dist_windows/content/
	cp $WINDIR/System32/msvcp120.dll dist_windows/content/
fi
