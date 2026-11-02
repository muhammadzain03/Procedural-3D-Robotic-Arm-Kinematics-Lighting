# Procedural 3D Robotic Arm

CPSC 453 Homework 3. A kinematic arm built from unit shapes in `src/geometry.cpp`, drawn with the shared mesh and shader code.

SFML opens the window. OpenGL does the rendering. Audio, networking, and SFML graphics are off. GLAD loads OpenGL, and GLM supplies the matrices.

## Layout

- `src/geometry.h` and `src/geometry.cpp` generate the unit cylinder, box, and sphere.
- `src/mesh.h` and `src/mesh.cpp` upload a mesh and draw it. One mesh holds one texture.
- `src/shader.h` and `src/shader.cpp` compile `mesh.vert` and `mesh.frag`.
- `assets/texture_arm_linkage.png` textures the upper arm and forearm.
- `assets/texture_base_top.png` textures the top face of the base.

The robot program is added in `CMakeLists.txt` once `src/robot.cpp` exists.
