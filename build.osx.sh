if [ "${1}" = "clean" ] ; then
    xcodebuild -configuration Release clean
    xcodebuild -configuration Debug clean
elif [ "${2}" = "64" ] ; then
    xcodebuild -project rasp.xcodeproj -configuration ${1} arch=x86_64
else
    xcodebuild -project rasp.xcodeproj -configuration ${1}
fi
    
