sudo chmod 774 premake5
./premake5 --os=macosx xcode4
sudo chmod 774 bin
sudo chmod -R 774 bin/*
