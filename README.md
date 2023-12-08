# Texture Mapping and Tesselation

## Description

This project is about Normal Mapping, Parallax Mapping, Displacement Mapping, Tessellation, and Cube Mapping. Written in C++ and glsl and using OpenGL.

## Running the program

Please see the steps below on how to run the program.

#### Running on MacOS

There was a problem with loading some of the shaders in macOS. For you to fix this, please uncomment the code on `line 98` in `Project.cpp`.

```bash
# Run compile and execute file
./blaze_mac.sh
```

#### Running on Windows

For Windows: You would need to use Visual Studio 2019 to run the program.

- Install Visual Studio 2019 (Community Edition).
- Select "Desktop development with C++."
- Once Visual Studio 2019 is open, select `Open a project or solution` and select `winbuild/codebase.sln`.
- After opening the project, go to `Build > Build Solution` to build the program.
- After building, run the program by going to `Debug > Start Debugging`.

### Testing for Normal Mapping, Parallax Mapping, and Displacement Mapping

- Load model under `Assignments > Project`. Under the `models` directory use the model `plane_face_front.obj`. The scene will be dark since the lighting is not in the right direction.
- Switch the lighting direction under `Assignments > Project > Lighting` to `0.000`, `-1.000`, `-1.000` to light up the surface of the plane.
- Under `OpenGL Renderer > Textures` load new textures. Specifically `wall_diffuse.png`, `wall_normal.png`, and `wall_displacement.png`. These textures are all under the textures directory.
- Under `Assignments > Project` enable each texture and type in the appropriate textuer id for each Texture ID field. Note, You must have a texture and a normal texture for parallax or displacement mapping to work (This is about the location of the textures since it is hard coded).
- After that feel free to mess with the controls for the displacement scale and parallax layers.

### Testing Tessellation

- On the source code itself in `Project.cpp` on `line 71` feel free to flip the variable `useTessellation` to `true`. Then run the recompile and execute bash script.
- As for adding the model and adding textures please follow all the steps above but the model that should be loaded is `plane_4x4.obj` to make sure that we only have a few vertices for the model.
- Once the model and textures are loaded, feel free to mess with the `Tessellation inner` and `Tessellation outer` levels.

### Testing cube mapping

- On the source code in `Project.cpp` on `line 71` set `useTessellation` to `false` after setting it to `true`, then on `line 75` set `useCubemapping` to `true`.
- Then run and compile using the provided bash script.
- After the program is running, feel free to hold your mouse then move around.
