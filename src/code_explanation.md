## Implemented Features

### 1. Solar System Simulation

- **Sun, Earth, and Moon**: Represented as spheres. I also defined a ring mesh (Saturn's rings) and a cube mesh (Skybox).
- **Orbital Mechanics**: Earth orbits the sun, and the moon orbits the earth.

### 2. Texturing

- **Earth and Moon Textures**: Applied realistic textures to the earth and moon.
- **Fixed Textures**: Textures remain fixed relative to the planets due to self-rotation.

### 3. Lighting

- **Sun as Light Source**: The sun emits light that illuminates the earth and moon.
- **Phong Lighting Model**: Used in the fragment shader to calculate ambient, diffuse, and specular components.

### 4. Camera Controls

- **Movement**: Move freely in 3D space using ZQSD (WASD) keys.
- **Looking Around**: Mouse movement allows looking in different directions.
- **Zooming**: Zoom in and out using `A` and `Z` keys, and/or the scroll wheel.

### 5. Simulation Control

- **Pause and Resume**: Pressing the `F` key freezes or unfreezes the simulation time.

### 6. Skybox

- **Cubemap Implementation**: Surrounds the scene with a skybox to simulate space (easy to change in the code).

## User Interactions

- **Keyboard Shortcuts**:
  - `Z`, `Q`, `S`, `D`: Move the camera.
  - `SPACE`, `LEFT_SHIFT`: Move the camera up and down.
  - `A`: Zoom in.
  - `E`: Zoom out.
  - `F`: Freeze/unfreeze the simulation.
  - `ESC`: Exit the application.
- **Mouse Controls**:
  - **Movement**: Look around.
  - **Scroll Wheel**: Zoom in and out.

## Shortcomings

- For **Saturn's rings**, the texture is not applied properly. Also, it does not have any thickness, so it dissepeared when viewed from the side.
- Upon moving, the **stars** in the back dim in brightness. I do not really know why this happens, so I don't know how to fix it.
- The lighting is very basic. There are multiple obvious issues:
  - **There is no shadow** from one object to another.
  - **The appearance of the sun is quite bad.** It is just a textured sphere, we do not really see the light coming from it. I tried to implement some kind of lens flare, but it did not work. A **better lightning model**, like PBR, would be a good improvement.
- When I write **texture**, I only mean a simple image. I did not implement any kind of bump mapping or normal mapping. This would be a good improvement.
- The "animations" are simply a few spheres rotating. More objects and a few different animations (clouds, better orbiting, ...) would be a good improvement.


## Sources:

- **Textures**: https://planetpixelemporium.com/saturn.html
- **More Textures**: https://www.solarsystemscope.com/textures/
- **SkyBox**: https://tools.wwwtyro.net/space-3d/index.html
