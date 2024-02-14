#!/bin/bash
# call this from $repo/Software
set -e

# build UpdateElevate
echo "#define NAME L\"Prismatik\"" > UpdateElevate/UpdateElevate/command.h
echo "#define EXT L\".exe\"" >> UpdateElevate/UpdateElevate/command.h
echo "#define ARGS L\" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART\"" >> UpdateElevate/UpdateElevate/command.h
cmd //c scripts\\win32\\build_UpdateElevate.bat
cp UpdateElevate/x64/Release/UpdateElevate.exe dist_windows/content/



cp bin/*.dll dist_windows/content/
cp bin/*.exe dist_windows/content/

cp -r bin/platforms dist_windows/content/
cp -r bin/styles dist_windows/content/
cp -r bin/tls dist_windows/content/
