#!/bin/bash

set -e

IsWindows=false
IsMacOS=false
IsLinux=false
is64BitOperatingSystem=false
CurrentDir=$(dirname "$0")
OutPathPrefix=build/bin

ArtifactPath=''
IncludeDebug=false
Version=''

# check platform
unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     IsLinux=true;;
    Darwin*)    IsMacOS=true;;
    CYGWIN*)    IsWindows=true;;
    MINGW*)     IsWindows=true;;
    *)          ;;
esac

parseArgs() {
    args=("$@")
    for ((i=0; i<${#args[@]}; i++)); do
        case ${args[i]} in
            '-ArtifactPath')
                ArtifactPath=${args[++i]}
                ;;
            '-IncludeDebug')
                IncludeDebug=true
                ;;
            '-Version')
                Version=${args[++i]}
                ;;
            *)
                ;;
        esac
    done
}

printEnvironments() {
    cat << EOF
IsWindows: $IsWindows
IsMacOS: $IsMacOS
IsLinux: $IsLinux
Is64BitOperatingSystem: $is64BitOperatingSystem
Current working directory: $(pwd)
ArtifactPath: $ArtifactPath
IncludeDebug: $IncludeDebug
Version: $Version
EOF
}

installVcpkg() {
    vcpkgUrl='https://github.com/microsoft/vcpkg.git'
    git clone "$vcpkgUrl"
    
    if [ "$IsWindows" = true ]; then
        ./vcpkg/bootstrap-vcpkg.bat
    elif [ "$IsMacOS" = true ] || [ "$IsLinux" = true ]; then
        ./vcpkg/bootstrap-vcpkg.sh
        if [ "$IsMacOS" = true ]; then
            # xcode-select --install
            :
        fi
    else
        echo 'vcpkg is not available on target platform.'
        exit 1
    fi
}

downloadFile() {
    url="$1"
    dest="$2"
    
    file=$(basename "$dest")
    dir=$(dirname "$dest")
    mkdir -p "$dir"
    
    while true; do
        curl -L --user-agent "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36" -o "$file" "$url"
        ret=$?
        if [ $ret -eq 0 ]; then
            mv "$file" "$dest"
            break
        fi
    done
}

installDependenciesForMacOS() {
    # Download both x86-64 and arm-64 libs and merge them into a uniform binary.
    # https://www.f-ax.de/dev/2022/11/09/how-to-use-vcpkg-with-universal-binaries-on-macos/
    dependencies=('boost' 'openssl')
    for libName in "${dependencies[@]}"; do
        ./vcpkg/vcpkg install --triplet=x64-osx "$libName"
        ./vcpkg/vcpkg install --triplet=arm64-osx "$libName"
    done

    python3 ./CI/lipo-dir-merge.py ./vcpkg/installed/arm64-osx ./vcpkg/installed/x64-osx ./vcpkg/installed/uni-osx
}

installDependenciesForWindows() {
    dependencies=('boost' 'openssl')
    for libName in "${dependencies[@]}"; do
        ./vcpkg/vcpkg install "$libName" --triplet x64-windows-static
    done
}


installDependencies() {
    if [ "$IsMacOS" = true ]; then
        installDependenciesForMacOS
    elif [ "$IsWindows" = true ]; then
        installDependenciesForWindows
    fi
}

build_mac() {
    buildType="${1}"
    exePath=build/bin/$buildType/LightFx

    premake5 --os=macosx xcode4 --file=build/premake5.lua

    xcodebuild -project build/bin/LightFX.xcodeproj -configuration $buildType
    
    if [ ! -d $exePath ]; then
        echo "Can't find ${exePath}"
        return 1;
    fi

    datestr=`date +%Y%m%d`
    filename=build/lightmap-tools-darwin-${datestr}.zip
    zip -j $filename $exePath
}

function extract_zip() {
    if ! command -v unzip &> /dev/null; then
        echo "Error: unzip is not installed. Please install it and retry."
        exit 1
    fi

    local zip_file=$1
    local dest_dir=$2

    echo "Extracting $zip_file to $dest_dir..."
    unzip -n -d "$dest_dir" "$zip_file"
}

build_windows() {
    buildType="${1}"
    exePath=build/bin/$buildType/LightFx.ext

    extract_zip "3rd/embree/lib/embree.zip" "3rd/embree/lib"
    extract_zip "3rd/embree/lib/embree_avx.zip" "3rd/embree/lib"
    extract_zip "3rd/embree/lib/embree_avx2.zip" "3rd/embree/lib"
    extract_zip "3rd/embree/lib/embree_sse42.zip" "3rd/embree/lib"

    premake5 --os=windows vs2019 --file=build/premake5.lua

    if [ -f "/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/devenv.exe" ]; then
        "/c/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/devenv.exe" build/bin/LightFX.sln /Project LightFX /Build $buildType /Out build.log
    else
        echo "Error: Visual Studio 2019 Community Edition is not installed or not in the expected location."
        exit 1
    fi

    if [ ! -d $exePath ]; then
        echo "Can't find ${exePath}"
        return 1;
    fi

    datestr=`date +%Y%m%d`
    filename=build/lightmap-tools-win32-$datestr.zip
    zip -j $filename $exePath
}

check_premake5() {
    if command -v premake5 &> /dev/null; then
        premake5 --os=windows vs2019
    else
        echo "Error: premake5 is not installed or not in PATH. Please install it and retry."
        exit 1
    fi
}

do_build() {
    buildType="${1}"
    if [ "$IsWindows" = true ]; then
        build_windows buildType
    else
        build_mac buildType
    fi
}

build() {
    check_premake5

    cmakeBuildTypes=('Release')
    if [ "$IncludeDebug" = true ]; then
        cmakeBuildTypes+=('Debug')
    fi

    for buildType in "${cmakeBuildTypes[@]}"; do
        do_build "$buildType"
    done
    
    if [ -n "$ArtifactPath" ]; then
        # Remove all .lib and .a files
        find $OutPathPrefix -type f \( -name '*.lib' -o -name '*.a' \) -delete
        # Pack the installation directory
        tar -czvf $ArtifactPath -C $OutPathPrefix .
    fi
}

parseArgs "$@"
printEnvironments
installVcpkg
installDependencies
build