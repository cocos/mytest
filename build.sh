cd build/mac
sudo chmod 774 premake5
./premake5 --os=macosx xcode4
sudo chmod 774 bin
sudo chmod -R 774 bin/*
xcodebuild -project bin/LightFX.xcodeproj -configuration Release
sudo chmod -R 774 bin/Release/*