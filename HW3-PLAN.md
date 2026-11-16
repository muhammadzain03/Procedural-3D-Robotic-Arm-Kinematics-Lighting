# HW3 - Procedural 3D Robotic Arm: As Built

**Course:** CPSC 453 (Fall 2026) · **Due:** Nov 6, 11:55 PM · **Demo:** scheduled tutorial, MS 239

Notes on the arm in `src/`, for the demo walkthrough. The AI declaration is in `DECLARATION AND REFLECTION.md`.

---

## 0. Grading map

| Item | Points | Where it lives |
|---|---|---|
| Model transformations + interactions | **7** | Phase 3, Phase 5 |
| Procedural geometry generation | 5 | Phase 1 |
| Texturing + lighting and shading | 5 | Phase 6, Phase 7 |
| Camera | 3 | Phase 4 |
| Declaration and reflection | 2 | Phase 8 |
| Demo | 5 (+5 bonus for demoing **early**) | Phase 9 |

---

## 1. Architecture

### 1.1 Stateless draw

The mesh does not store a model matrix. `draw` takes one, and every matrix is rebuilt from the joint angles each frame:

```cpp
void draw(const glm::mat4& modelMatrix,
          const glm::mat4& view,
          const glm::mat4& projection,
          const glm::vec3& lightPositionView) const;
```

Removed `applyTransform` and the old `draw(view, projection)`. Nothing called them. The old draw also hardcoded the light at $(4, 9, 6)$, which duplicated the light already computed in `robot.cpp`.

### 1.2 Unit shapes

Each shape is generated once, at unit size. Size and placement are a matrix on the draw line, so a child does not inherit its parent's scale.

| Shape | Unit definition |
|---|---|
| Cylinder | radius $1$, height $1$, base on the XZ plane at the origin, extending to $y = 1$ |
| Disc | radius $1$, in the XZ plane at $y = 0$, normal $+Y$ |
| Sphere | radius $1$, centred at the origin |
| Box | $1 \times 1 \times 1$, centred at the origin, so each axis spans $[-1/2, +1/2]$ |

The cylinder base sits at the origin on purpose. Translating by the segment length along $+Y$ lands on the next joint.

$$
M_{\mathrm{draw}} = M_{\mathrm{joint}} \, T_{\mathrm{local}} \, S
$$

$T_{\mathrm{local}}$ is the identity for the cylinders, spheres, and the disc. A centred box needs a half-length translate so one end sits on the joint.

### 1.3 Axes

- A segment extends along $+Y$ in its own frame.
- Shoulder, elbow, wrist, and the finger joints pitch about $X$, so the arm and the jaws bend in the YZ plane.
- The base rolls about $Y$.

### 1.4 Five meshes

`Mesh` stores one texture and one material, so the arm is five meshes.

| Mesh | Texture | Material | Used by |
|---|---|---|---|
| `cylinderTextured` | `texture_arm_linkage.png` | link, shininess $32$ | upper arm, forearm |
| `cylinderPlain` | none | flat, shininess $8$ | base body, both stems |
| `discTextured` | `texture_base_top.png` | link, shininess $32$ | base top only |
| `sphere` | none | joint, shininess $96$ | shoulder, elbow, wrist, both finger joints |
| `box` | none | flat, shininess $8$ | palm, both fingers |

The base texture belongs on the top face only. The wall comes from `cylinderPlain` and the top from a separate disc. The cylinder still has its own top cap, in the same plane as that disc, so the disc is lifted by

$$
\delta = 0.005
$$

which is `arm::discGap`. The first gap was $0.001$. At a depth of about $20$, with near $0.1$ and far $60$, the depth step is roughly $2.4 \times 10^{-4}$, so $0.001$ was only about four steps and the top flickered. $0.005$ is still too small to see. The link caps show the linkage texture as well, because they belong to the same mesh as the wall. The joint spheres cover most of those ends.

---

## 2. Phase 0 - Setup

Done. `CMakeLists.txt` builds the `robot` target (`project(RoboticArm)`). The post-build step copies `mesh.vert`, `mesh.frag`, and both PNGs to `build/bin/shaders` and `build/bin/assets`. stb is pinned to commit `d14d46f6b44456eb88beac58cba833a4135d4c89`. SFML is window and system only. Paths come from `argv[0]`, so the working directory does not matter.

Deleted `build/`, configured again, and rebuilt. `build/bin/robot.exe` starts with both shaders and both textures beside it.

---

## 3. Phase 1 - Geometry

The generators are in `geometry.h` / `geometry.cpp`. Every vertex has a position, a normal, and a texture coordinate. `makeVertex` lives in the anonymous namespace.

### 3.1 Cylinder

`makeCylinder(segments, aroundRepeats = 1, alongRepeats = 1)`. The plain mesh calls `makeCylinder(48)`. The textured mesh calls `makeCylinder(48, 4, 2)`.

Three vertex groups, because the normals differ. They do not share vertices.

**(a) Wall.** For $i = 0, \ldots, N$ inclusive,

$$
u = \frac{i}{N}, \qquad \theta = 2\pi u, \qquad \mathbf{d} = (\cos\theta,\ 0,\ \sin\theta).
$$

The bottom vertex is $\mathbf{d}$ with normal $\mathbf{d}$ and texture coordinate $(u \, r_u,\ 0)$. The top is $\mathbf{d} + (0, 1, 0)$ with the same normal and texture coordinate $(u \, r_u,\ r_v)$. $\theta$ comes from the unscaled $u$. Scaling $u$ before $\theta$ wraps the vertices around the cylinder more than once.

$i = N$ repeats the position of $i = 0$ and carries $u \, r_u = r_u$. `GL_REPEAT` treats an integer $r_u$ as the same texel as $0$, so the seam closes. Two triangles per quad: $(b_0, t_0, b_1)$ and $(t_0, t_1, b_1)$. At $\theta = 0$ the first of those has geometric normal $(+\Delta, 0, 0)$, which points outward.

**(b) Bottom cap.** Normal $(0, -1, 0)$ on every vertex. Centre $(0, 0, 0)$, rim $(\cos\theta,\ 0,\ \sin\theta)$,

$$
(u, v) = \bigl(0.5 + 0.5\cos\theta,\ 0.5 + 0.5\sin\theta\bigr).
$$

Fan order $\{C,\ r_i,\ r_{i+1}\}$.

**(c) Top cap.** Normal $(0, +1, 0)$. Centre $(0, 1, 0)$, same disc coordinates. Fan order $\{C,\ r_{i+1},\ r_i\}$.

The two fans wind opposite each other, and both face outward. The first version had them opposite but pointing into the cylinder. Face culling is on (`glEnable(GL_CULL_FACE)`), and both caps stay visible, which is the check. For the top fan, with $\mathbf{a} = r_{i+1} - C$ and $\mathbf{b} = r_i - C$,

$$
(\mathbf{a} \times \mathbf{b})_y = \sin\Delta > 0,
$$

so the normal is $+Y$. The bottom fan checks the same way:

$$
(1, 0, 0) \times (0.991, 0, 0.131) = (0, -0.131, 0),
$$

which is $-Y$.

### 3.2 Disc

`makeDisc(48)` is the top-cap fan alone, lying in the XZ plane at $y = 0$. Same coordinates, same winding $\{C,\ r_{i+1},\ r_i\}$, normal $+Y$. It exists so the base top can have its own texture.

### 3.3 Box

24 vertices, 12 triangles. Each face has its own four corners and one axis normal, so no vertex is shared across an edge. Texture coordinates run from $(0, 0)$ to $(1, 1)$ on each face.

### 3.4 Sphere

`makeSphere(32, 16)`. On the unit sphere the normal equals the position. The texture coordinate is $(u,\ 1 - v)$, with $u$ going around once. The index reserve is $\mathrm{stacks} \cdot \mathrm{slices} \cdot 6$. The first and last bands contain degenerate triangles at the poles. That is normal for this construction.

The single-cylinder checkpoint passed: a smooth wall, flat caps, and a shading break at the rim.

---

## 4. Phase 2 - Dimensions

All of these are `constexpr` in `namespace arm` in `robot.h`.

| Component | Size | Draw transform |
|---|---|---|
| Base body | height $0.70$, radius $1.10$ | $S(1.10,\ 0.70,\ 1.10)$ |
| Base disc | radius $1.10$ | $T(0,\ 0.70 + \delta,\ 0)\, S(1.10,\ 1,\ 1.10)$ |
| Shoulder | radius $0.24$ | $S(0.24)$ |
| Upper arm | length $2.80$, radius $0.22$ | $S(0.22,\ 2.80,\ 0.22)$ |
| Elbow | radius $0.20$ | $S(0.20)$ |
| Forearm | length $2.25$, radius $0.17$ | $S(0.17,\ 2.25,\ 0.17)$ |
| Wrist | radius $0.17$ | $S(0.17)$ |
| Palm | length $0.82$, thickness $0.36$, height $0.68$ | $T(0,\ 0.41,\ 0)\, S(0.36,\ 0.82,\ 0.68)$ |
| Stems | length $0.58$, radius $0.045$ | $S(0.045,\ 0.58,\ 0.045)$ |
| Finger joints | radius $0.06$ | $S(0.06)$ |
| Fingers | length $0.80$, thickness $0.085$, height $0.07$ | $T(0,\ 0.40,\ 0)\, S(0.085,\ 0.80,\ 0.07)$ |

Palm length is $+Y$, thickness is $X$, and height is $Z$. The stems sit at

$$
z = \pm \frac{0.68}{2} = \pm 0.34.
$$

The unit box is centred, so the palm is translated by $0.82 / 2 = 0.41$ and each finger by $0.80 / 2 = 0.40$ before the scale.

---

## 5. Phase 3 - Kinematic chain

The chain is rebuilt every frame. GLM post-multiplies, and `glm::rotate` takes radians. Angles are stored in degrees and converted at the call.

$$
M_{\mathrm{child}} = M_{\mathrm{parent}} \, T_{\mathrm{offset}} \, R_{\mathrm{joint}}.
$$

$T$ comes first so the child pivots at the joint. $R$ first would orbit the parent's origin. The usual $TRT^{-1}$ does not need the final $T^{-1}$ here, because the child geometry already starts at its own origin.

$$
\begin{aligned}
M_{\mathrm{base}} &= R_y(\theta_{\mathrm{base}}) \\
M_{\mathrm{shoulder}} &= M_{\mathrm{base}} \, T(0,\ 0.70,\ 0) \, R_x(\theta_{\mathrm{shoulder}}) \\
M_{\mathrm{elbow}} &= M_{\mathrm{shoulder}} \, T(0,\ 2.80,\ 0) \, R_x(\theta_{\mathrm{elbow}}) \\
M_{\mathrm{wrist}} &= M_{\mathrm{elbow}} \, T(0,\ 2.25,\ 0) \, R_x(\theta_{\mathrm{wrist}})
\end{aligned}
$$

For each side $s \in \{+1, -1\}$,

$$
\begin{aligned}
M_{\mathrm{stem}} &= M_{\mathrm{wrist}} \, T(0,\ 0.82,\ s \cdot 0.34) \\
M_{\mathrm{knuckle}} &= M_{\mathrm{stem}} \, T(0,\ 0.58,\ 0) \\
M_{\mathrm{finger}} &= M_{\mathrm{knuckle}} \, R_x(s \, \theta_{\mathrm{jaw}}).
\end{aligned}
$$

The disc uses $M_{\mathrm{base}}$ only, plus its own translate and scale. That translate is not written into $M_{\mathrm{base}}$, or the whole arm would rise by $\delta$.

Positive $\theta_{\mathrm{jaw}}$ opens the jaw. $R_x(\theta)$ sends the local $+Y$ axis to $(0,\ \cos\theta,\ \sin\theta)$. On the finger at $z = +0.34$ the tip moves toward $+Z$, away from the centre. The finger at $z = -0.34$ uses $s = -1$, so it moves toward $-Z$. The two rotations are mirrors.

With every angle at $0$ the arm is straight: base, column, palm, and two parallel fingers, with the joints sitting in the links.

---

## 6. Phase 4 - Camera

The straight arm is about

$$
0.70 + 2.80 + 2.25 + 0.82 + 0.58 + 0.80 = 7.95
$$

tall, and a folded pose reaches farther from the look target than the straight pose does. The fit uses a sphere of radius $R = 8$ about $(0,\ 3,\ 0)$. For a vertical field of view $f = 45^\circ$,

$$
d = \frac{R}{\sin(f/2)} = \frac{8}{\sin 22.5^\circ} \approx 20.9.
$$

The view direction is $\hat{\mathbf{v}} = \mathrm{normalize}(0.30,\ 0.15,\ 1)$, and

$$
\mathbf{eye} = (0,\ 3,\ 0) + d\,\hat{\mathbf{v}} \approx (5.95,\ 5.97,\ 19.82),
$$

with up $(0, 1, 0)$. Near plane $0.1$, far plane $60$. The aspect ratio is the window width over its height, and a zero height uses aspect $1$. `glViewport` is set from the window size every frame. There is no resize-event handler. A numerical check put the worst fingertip $18.5$ from the target and $18.7^\circ$ off the view axis, inside the $22.5^\circ$ half-angle, so the arm stays in frame.

---

## 7. Phase 5 - Controls

The loop calls `sf::Keyboard::isKeyPressed` every frame, not `KeyPressed` events. With speed $\omega = 75^\circ/\mathrm{s}$ and the SFML frame time $\Delta t$,

$$
\theta \leftarrow \theta + \omega \, \Delta t.
$$

| Control | Keys | Range |
|---|---|---|
| Base roll | A / Left, D / Right | unrestricted, wrapped with `fmod` into $[0^\circ, 360^\circ)$ |
| Shoulder | W / S | $[-10^\circ,\ +78^\circ]$ |
| Elbow | R / F | $[-150^\circ,\ +10^\circ]$ |
| Wrist | T / G | $[-90^\circ,\ +90^\circ]$ |
| Gripper | O opens, C closes | $[-22^\circ,\ +45^\circ]$ |
| Reset | Space | all five angles to $0$ |

Escape closes the window. The four pitch joints use `std::clamp`. The base uses `fmod` instead.

The shoulder was first clamped at $+95^\circ$. That lets the upper arm pitch through the base. The cylinder of radius $0.22$, hinged at the centre of a disc of radius $1.10$, meets the rim when

$$
\cos\alpha = \frac{0.22}{1.10} = 0.2, \qquad \alpha = \arccos(0.2) \approx 78.5^\circ.
$$

At $95^\circ$ the lower surface is about $0.096$ below the base top where it crosses $r = 1.10$. The clamp is $78^\circ$, just inside the tangent angle. The spec says the upper arm cannot pitch below the base.

$0^\circ$ is the parallel jaw, with a $0.68$ gap. Positive opens past parallel, which lets the gripper clear a part wider than that gap. Negative closes.

A centreline estimate, $0.80\sin\theta = 0.34$, gives

$$
\theta = \arcsin\bigl(0.34 / 0.80\bigr) \approx 25.2^\circ.
$$

The fingers are $0.07$ thick along $Z$, so the inner faces meet earlier. With $\alpha > 0$ the close angle,

$$
0.80\sin\alpha + 0.035\cos\alpha = 0.34, \qquad \alpha \approx 22.6^\circ.
$$

The clamp is $-22^\circ$. On screen the tips meet and do not cross. Past $22.6^\circ$ they interpenetrate.

---

## 8. Phase 6 - Textures

- Upper arm and forearm: `cylinderTextured`, linkage image, wall coordinates repeated $4$ times around and $2$ times along.
- Base top: `discTextured`, base-top image, one copy of the disc coordinates.
- Base wall, stems, palm, fingers, and all five spheres: no texture. `Mesh` sets `useTexture = 0`, and `mesh.frag` substitutes white.

The linkage image is repeated so a joint rotation is readable. One copy over a length of $2.80$ and a radius of $0.22$ is too stretched to show the turn.

---

## 9. Phase 7 - Phong lighting

`mesh.frag` already evaluated ambient, diffuse, and specular per fragment. Two things were wrong at the start: the light direction was a constant, and every mesh shared one material.

### 9.1 Point light

`fragmentPosition` is view space, because `mesh.vert` sets it to $(VM\mathbf{p})_{xyz}$. The view direction $\mathrm{normalize}(-\mathbf{p}_{\mathrm{frag}})$ is only valid in that space, so the light is transformed there too. The light sits at $(4, 9, 6)$, above and to the side of the arm. The trailing $1$ makes it a point:

$$
\mathbf{p}_{\mathrm{view}} = \bigl( V \begin{bmatrix} 4 \\ 9 \\ 6 \\ 1 \end{bmatrix} \bigr)_{xyz}, \qquad
\mathbf{l} = \mathrm{normalize}(\mathbf{p}_{\mathrm{view}} - \mathbf{p}_{\mathrm{frag}}).
$$

`view` does not change, so $\mathbf{p}_{\mathrm{view}}$ is computed once, next to `lookAt`, and passed into every draw. There is no distance attenuation. Pitching a joint moves the highlight, because the light stays fixed while the normal changes.

### 9.2 Materials

`baseColor` is the vertex colour times the texel. The reflection is

$$
\mathbf{R} = \mathrm{reflect}(-\mathbf{L}, \mathbf{N}) = 2(\mathbf{N}\cdot\mathbf{L})\,\mathbf{N} - \mathbf{L}.
$$

When $\mathbf{N}\cdot\mathbf{L} < 0$ the surface faces away from the light, but $\mathbf{R}\cdot\mathbf{V}$ can still be positive, so the specular term is dropped there:

$$
s =
\begin{cases}
\bigl(\max(\mathbf{V}\cdot\mathbf{R},\ 0)\bigr)^{n} & \text{if } \mathbf{N}\cdot\mathbf{L} > 0, \\
0 & \text{otherwise.}
\end{cases}
$$

The lit colour is

$$
\mathbf{c} = \mathbf{c}_{\mathrm{base}} \bigl( \mathbf{k}_a + \mathbf{k}_d \max(\mathbf{N}\cdot\mathbf{L},\ 0) \bigr) + \mathbf{k}_s \, s.
$$

| Material | $\mathbf{k}_a$ | $\mathbf{k}_d$ | $\mathbf{k}_s$ | $n$ | Meshes |
|---|---|---|---|---|---|
| Link | $(0.15, 0.15, 0.15)$ | $(0.70, 0.70, 0.70)$ | $(0.25, 0.25, 0.25)$ | $32$ | textured cylinder, disc |
| Joint | $(0.08, 0.08, 0.08)$ | $(0.35, 0.35, 0.35)$ | $(0.80, 0.80, 0.80)$ | $96$ | sphere |
| Flat | $(0.18, 0.18, 0.18)$ | $(0.65, 0.65, 0.65)$ | $(0.05, 0.05, 0.05)$ | $8$ | plain cylinder, box |

Lighting stays on for all three. The shader does not know which surface is curved. Flat faces have one normal per triangle. The sphere and the cylinder wall interpolate radial normals, so the joint highlight slides. The gate is what keeps that highlight off the dark limb of the joint spheres.

---

## 10. Phase 8 - Submission package

Done locally, except the Gradescope upload.

1. Filled in `DECLARATION AND REFLECTION.md`: the pledge, Katherine Hou, the four sources, and the reflection, including the cap winding, the jaw limit, and the light upload.
2. `README.md` lists the build, the keys, the jaw range, the shoulder limit, and the disc offset.
3. Deleted `build/`, configured, and rebuilt. `robot.exe` ran, with both shaders and both textures copied beside it.
4. Shaders and textures are in the repository. CMake fetches SFML, GLM, and stb. `build/` is gitignored.
5. This plan and `Robot Arm Frames.html` stay untracked.
6. The repository still has to go to Gradescope.

---

## 11. What actually broke, and the cause

| What happened | Cause |
|---|---|
| Sphere indices ran off the end of the vertex list | `topLeft` omitted `+ slice`, and `bottomLeft` multiplied by the row length |
| A child orbited instead of bending | rotated and then translated, or the elbow offset used the joint radius instead of the segment length |
| Stems were as wide as the base | the stem's $Z$ scale used `baseRadius` |
| Caps vanished once culling was turned on | both fans wound inward |
| Gripper opened away from the centre, then the tips crossed | the sign of $s\,\theta_{\mathrm{jaw}}$, and a close limit taken from the centreline instead of the inner faces |
| Base top flickered | disc and cylinder cap in the same plane; lifted the disc, now by $\delta = 0.005$ |
| Upper arm cut the base top | shoulder clamp was $+95^\circ$; the rim is tangent at about $78.5^\circ$, so the clamp is $78^\circ$ |
| Texture call threw before the first frame | PNG path was not beside the executable, or the uniform name did not match the shader copied into `build/bin/shaders` |
| Light upload did not compile | `glm::vdc3`, `setMatrix4` on a `vec3`, and draw calls that never received the light |
| Bright crescent on the dark side of a joint | specular ran when $\mathbf{N}\cdot\mathbf{L} < 0$; $s = 0$ there |

---

## 12. Demo answers

- **Hierarchy.** $M_{\mathrm{child}} = M_{\mathrm{parent}} \, T \, R$, recomputed from the five angles. No matrix is stored on the mesh.
- **Why $T$ before $R$.** The rotation is about the joint. The other order orbits the parent's origin.
- **Cylinder.** Three vertex groups. Wall normals are radial. Cap normals are $(0,\ \pm 1,\ 0)$. The seam vertex is duplicated so $u$ can be $0$ and $r_u$ at the same position. Both fans face outward; culling is the check.
- **One shader, flat and curved.** The normals differ. The shader does not.
- **Why `Mesh::draw` takes a matrix.** One unit cylinder is the base, both links, and both stems. A stored matrix would accumulate.
- **Shoulder limit.** The upper arm meets the base rim when $\cos\alpha = 0.22 / 1.10$. The clamp is $78^\circ$.
- **Jaw limit.** The inner faces meet when $0.80\sin\alpha + 0.035\cos\alpha = 0.34$, about $22.6^\circ$. The close stop is $-22^\circ$. Positive opens past parallel so the jaw can clear a part wider than $0.68$.
- **Light.** A point at world $(4, 9, 6)$, sent through $V$ with homogeneous coordinate $1$, then $\mathbf{l} = \mathrm{normalize}(\mathbf{p}_{\mathrm{view}} - \mathbf{p}_{\mathrm{frag}})$. Specular is zero wherever $\mathbf{N}\cdot\mathbf{L} \le 0$.

---

## 13. What is left

Upload to Gradescope. The demo can walk the chain, then the shoulder and jaw limits, then the view-space light.
