## TODO
- Textures do not always face the same way (changes with camera, very bad)
- Lightning does not update when time is frozen

## Implemented Features

### 1. Solar System Simulation

- **Sun, Earth, and Moon**: Represented as spheres.
- **Orbital Mechanics**: Earth orbits the sun, and the moon orbits the earth.

### 2. Texturing

- **Earth and Moon Textures**: Applied realistic textures to the earth and moon.
- **Fixed Textures**: Textures remain fixed relative to the planets due to self-rotation.

### 3. Lighting

- **Sun as Light Source**: The sun emits light that illuminates the earth and moon.
- **Phong Lighting Model**: Used in the fragment shader to calculate ambient, diffuse, and specular components.

### 4. Camera Controls

- **Movement**: Move freely in 3D space using WASD keys.
- **Looking Around**: Mouse movement allows looking in different directions.
- **Zooming**: Zoom in and out using `A` and `Z` keys, and/or the scroll wheel.

### 5. Simulation Control

- **Pause and Resume**: Pressing the `F` key freezes or unfreezes the simulation time.
- **Custom Simulation Time**: Allows precise control over the simulation's progression.

### 6. Skybox

- **Cubemap Implementation**: Surrounds the scene with a skybox to simulate space.
- **High-Quality Textures**: Enhances the visual realism of the simulation.

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