## Building
Use `glslc` from the Vulkan SDK to compile all shaders in the `res` directory.

For C++:
    Build the executable using CMake with `GENERATE_HAXE_BINDINGS` set to `OFF`.

For Haxe:
    Change the `HASHLINK_PATH` in `CMakeLists.txt` to point to where you store the HashLink files on your system.
    Build the library using CMake with `GENERATE_HAXE_BINDINGS` set to `ON`.
    Copy `res` and the built `.dll` into `haxe/bin/hl` and rename the `.dll` to `PxlIO.hdll`.
    If dynamic linking is enabled (the default in `CMakeLists.txt` is static for VCPKG), the `dll`s for SDL2, SDL2_image, and SDL2_mixer also need to be placed in `haxe/bin/hl`.
    Build the Haxe example by running `haxe build.hxml` in the `haxe` directory.