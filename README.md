# Procedural 3D Robotic Arm

CPSC 453 Homework 3. Built a kinematic arm from unit shapes in `src/geometry.cpp`, drawn with the shared mesh and shader code.

SFML opens the window. OpenGL does the rendering. Left audio, networking, and SFML graphics off. GLAD loads OpenGL, and GLM supplies the matrices.

## Build

From this folder, with CMake and Ninja available:

```
cmake -B build -G Ninja
cmake --build build
```

Run `build/bin/robot.exe`. The build copies the shaders and textures next to that executable. The program loads them from its own folder, so the working directory does not matter.

## Controls

The keyboard is polled every frame. A joint keeps moving while the key is held, at 75 degrees per second. Escape closes the window.

| Joint | Keys | Range |
|---|---|---|
| Base roll | A / Left, D / Right | full turn, wraps at 360° |
| Shoulder pitch | W / S | −10° to +78° |
| Elbow pitch | R / F | −150° to +10° |
| Wrist pitch | T / G | −90° to +90° |
| Gripper | O open, C close | −22° to +45° |
| Reset | Space | every joint back to 0° |

The shoulder stops at 78°. The upper arm is a cylinder of radius 0.22, hinged at the centre of a base of radius 1.10. That cylinder is tangent to the rim of the base when

```
cos α = 0.22 / 1.10 = 0.2
α = arccos(0.2) ≈ 78.5°
```

Past that, the upper arm cuts the top of the base. 78° stays just inside the tangent angle.

At 0° the jaws are parallel. Negative closes them until the inner faces nearly meet. Positive opens them past parallel.

## Layout

- `src/geometry.h` and `src/geometry.cpp` generate the unit cylinder, disc, box, and sphere.
- `src/robot.cpp` builds the kinematic chain, the camera, and the controls.
- `src/mesh.h` and `src/mesh.cpp` upload a mesh and draw it. One mesh holds one texture and one material.
- `src/shader.h` and `src/shader.cpp` compile `mesh.vert` and `mesh.frag`.
- `assets/texture_arm_linkage.png` textures the upper arm and forearm.
- `assets/texture_base_top.png` textures the top face of the base.

The base disc sits 0.005 above the cylinder cap. A gap of 0.001 is only a few times the depth resolution at this camera distance, and the two faces flickered. 0.005 is still too small to see, and it keeps them apart. The end caps of the arm links use the same texture as the walls, because one mesh holds one texture.
