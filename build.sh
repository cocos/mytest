echo make solution
cd build/mac
chmod 774 premake5
./premake5 --os=macosx xcode4
chmod 774 bin
chmod -R 774 bin/*

echo compile project
xcodebuild -project bin/LightFX.xcodeproj -configuration Release
chmod -R 774 bin/Release/*

cd ../../
echo compress to target
datestr=`date +%Y%m%d`
echo $datestr
filename=build/lightmap-tools-darwin-$datestr
zip -j $filename build/mac/bin/Release/LightFX
