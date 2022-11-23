echo make solution
cd build/mac
sudo chmod 774 premake5
./premake5 --os=macosx xcode4
sudo chmod 774 bin
sudo chmod -R 774 bin/*

echo compile project
xcodebuild -project bin/LightFX.xcodeproj -configuration Release
sudo chmod -R 774 bin/Release/*

cd ../../
echo compress to target
datestr=`date +%Y%m%d`
echo $datestr
filename=build/lightmap-tools-darwin-$datestr
zip -j $filename build/mac/bin/Release/LightFX
