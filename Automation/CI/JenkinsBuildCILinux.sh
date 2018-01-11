# Do a debug build
mkdir DebugBuild
cd DebugBuild
cmake -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" -D CMAKE_PREFIX_PATH="${QtPath}" ../Source
cmake --build .

cd ..

# Do a release build
mkdir ReleaseBuild
cd ReleaseBuild
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" -D CMAKE_PREFIX_PATH="${QtPath}" ../Source
cmake --build .
