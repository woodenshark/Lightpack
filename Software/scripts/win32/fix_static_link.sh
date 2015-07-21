#!/bin/bash
# qmake only allows shared linkage of the msvcr* runtime
# for our lightweiht, non-qt components we want to inject into other processes this is bad
# so after qmake is done, run this little script to rewrite the vcxproj's to use static linking
# call this from $repo/Software

function fix_static {
	for f in $1
	do
	  if [ -f $f -a -r $f ]; then
	   sed "s/MultiThreadedDLL/MultiThreaded/g" "$f" | sed "s/MultiThreadedDebugDLL/MultiThreadedDebug/g" > "$f.tmp" && mv "$f.tmp" "$f"
	  else
	   echo "Error: Cannot read $f"
	  fi
	done
}

fix_static "hooks/*.vcxproj"
fix_static "unhook/*.vcxproj"
fix_static "offsetfinder/*.vcxproj"