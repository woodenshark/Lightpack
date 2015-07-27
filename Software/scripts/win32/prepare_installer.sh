#!/bin/bash
# call this from $repo/Software
cp bin/*.dll dist_windows/content/
cp bin/*.exe dist_windows/content/

cp -r bin/platforms dist_windows/content/
