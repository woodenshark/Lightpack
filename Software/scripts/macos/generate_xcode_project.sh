#!/usr/bin/env bash

folder=$(basename $(pwd))
if [[ "$folder" != "Software" ]]
then
    echo "Execute this from Lightpack/Software folder"
    exit
fi

QMAKE=$(which qmake)

if [[ "$QMAKE" == "" ]]
then
    echo "qmake not found in \$PATH"
    exit
fi

$QMAKE -spec macx-xcode -recursive

if [[ "$?" != "0" ]]
then
    echo "Failed to generate Xcode Project"
    exit
fi

function add_to_scheme()
{
    depfolder=$1
    depname=$2
    depxcodeproj=$depfolder/$depname.xcodeproj
    blueprintId=$(grep -m 1 -Eo 'BlueprintIdentifier = "[^"]+"' $depxcodeproj/xcshareddata/xcschemes/$depname.xcscheme)
    buildableName=$(grep -m 1 -Eo 'BuildableName = "[^"]+"' $depxcodeproj/xcshareddata/xcschemes/$depname.xcscheme)
    blueprintName=$(grep -m 1 -Eo 'BlueprintName = "[^"]+"' $depxcodeproj/xcshareddata/xcschemes/$depname.xcscheme)
    referenceContainer=$(grep -m 1 -Eo 'ReferencedContainer = "[^"]+"' $depxcodeproj/xcshareddata/xcschemes/$depname.xcscheme | sed -E 's#(container\:)#\1../'"$depfolder"'/#g')

    prismatikScheme=src/Prismatik.xcodeproj/xcshareddata/xcschemes/Prismatik.xcscheme

    presenceCheck=$(grep -o "$blueprintId" $prismatikScheme)

    if [[ "$presenceCheck" != "" ]]
    then
	echo "$depname is already added, skipping"
	return 0
    fi

    entry="<BuildActionEntry buildForTesting = \"YES\" buildForRunning = \"YES\" buildForProfiling = \"YES\" buildForArchiving = \"YES\" buildForAnalyzing = \"YES\"><BuildableReference BuildableIdentifier = \"primary\" $blueprintId $buildableName $blueprintName $referenceContainer></BuildableReference></BuildActionEntry>"

    sed -i '' -E 's#(\<BuildActionEntries\>)#\1'"$entry"'#g' $prismatikScheme && echo -e "\nAdded $depname"
}

function remove_optimizations_from_debug()
{
    depfolder=$1
    depname=$2
    depxcodeproj=$depfolder/$depname.xcodeproj/project.pbxproj
    
    # replace first 2 occurencies (debug section should be first) of O2
    # qt bug, never fixed https://bugreports.qt.io/browse/QTBUG-54791
    sed -i '' -E '1,/"-O[1-9]",/s/"-O[1-9]",/"-O0","-g",/' $depxcodeproj && sed -i '' -E '1,/"-O[1-9]",/s/"-O[1-9]",/"-O0","-g",/' $depxcodeproj && \
    # disable QT_NO_DEBUG
    sed -i '' -E '1,/"-DQT_NO_DEBUG",/s/"-DQT_NO_DEBUG",//' $depxcodeproj && sed -i '' -E '1,/"-DQT_NO_DEBUG",/s/"-DQT_NO_DEBUG",//' $depxcodeproj
}

remove_optimizations_from_debug grab grab && \
remove_optimizations_from_debug math prismatik-math && \
remove_optimizations_from_debug src Prismatik && \
add_to_scheme grab grab && add_to_scheme math prismatik-math && \
echo -e "\n\nOpen Lightpack.xcodeproj\n\n" || \
echo -e "\n\nFailed to add grab and prismatik-math dependencies\ntry manually through Xcode (Prismatik > Edit scheme > Build > +, put them above Prismatik)"
