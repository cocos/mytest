# LightFX
A self-contained cross-platform lightmap baker,<br>
based on Embree and Websocket++.

All the necessary repos are embedded as git submodules,<br>
integrated as one CMake project. No additional dependencies required.

# Build
* clone this repo with --recursive flag, or if you didn't, run:<br>
`git submodule update --init --recursive`
* run the `build_boost` script that suits your system, if you are building for 32-bit platforms,<br>
pass `32` or `32_64`(Apple Darwin gcc) as the first parameter. Don't misspell.
* if everything builds without hiccups, you should be able to use CMake to configure the project environment smoothly.
