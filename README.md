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
- Under `Assignments > Project` enable each texture and type in the appropriate texture id for each Texture ID field. Note, You must have a texture and a normal texture for parallax or displacement mapping to work (This is about the location of the textures since it is hard coded).
- After that feel free to mess with the controls for the displacement scale and parallax layers.

### Testing Tessellation

- On the source code itself in `Project.cpp` on `line 71` feel free to flip the variable `useTessellation` to `true`. Then rerun the program.
- As for adding the model and adding textures please follow all the steps above but the model that should be loaded is `plane_4x4.obj` since we only want a model to have a few vertices.
- Once the model and textures are loaded, feel free to mess with the `Tessellation inner` and `Tessellation outer` levels.

### Testing cube mapping

- On the source code in `Project.cpp` on `line 71` set `useTessellation` to `false` after setting it to `true`, then on `line 75` set `useCubemapping` to `true`.
- Rerun the program.
- After the program is running, feel free to hold your mouse then move around.


### Demos

### Normal Mapping
![Screenshot 2023-12-15 at 9 11 38 AM](https://github.com/Tagaca19e/texture-mapping/assets/85138779/baa65588-b33b-43ac-8468-7a09a59bed7f)

### Parallax Occlusion
![Screenshot 2023-12-15 at 9 32 33 AM](https://github.com/Tagaca19e/texture-mapping/assets/85138779/c0a5198a-2366-4989-9585-2e6cfdb3285a)


### Displacement Mapping
https://github.com/Tagaca19e/texture-mapping/assets/85138779/ee367c66-36b9-4508-81f7-cf6fb3466f1a

### Tessellation

https://github.com/Tagaca19e/texture-mapping/assets/85138779/537c83e7-7342-49c9-95d8-27fc70cdbc9f

https://github.com/Tagaca19e/texture-mapping/assets/85138779/5ffb7265-f6c1-49a1-8d7f-d6f40806299a

### Skybox
https://github.com/Tagaca19e/texture-mapping/assets/85138779/07ca1d60-afce-45c9-a932-09ca88f51c58




