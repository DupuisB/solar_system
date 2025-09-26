# 🪐 Solar System — OpenGL Demo

> This is my first project using OpenGL. Enjoy!

A simple OpenGL-based solar system demo (Sun, Earth, Moon, Saturn + rings, skybox).

## Demo

![Solar System Demo](./demo.gif)

## Highlights / Features
- Rendered spheres representing Sun, Earth, Moon and Saturn.
- Saturn has a separate ring mesh (texture for rings included, though ring rendering has issues).
- Textures applied for planets and skybox (in `src/media/`).
- Sun acts as the light source; fragment shader implements a basic Phong-style lighting model.
- Camera with free movement, mouse-look and zoom.
- Pause/resume simulation time (`F` key).
- Skybox implemented with cubemap textures.
- Uses GLFW, GLAD, GLM and stb_image (under `src/dep/` and `src/`).

## Controls (default)
- Movement: Z / Q / S / D (ZQSD for AZERTY; W/A/S/D on QWERTY)
- Vertical: `SPACE` (up), `LEFT_SHIFT` (down)
- Look: Mouse movement
- Zoom: `A` (zoom in), `E` (zoom out) and mouse scroll
- Pause/resume simulation: `F`
- Exit: `ESC`

## Build & Run (Unix-like)

```bash
# from repository root
./src/build.sh
# then run the produced executable
./src/tpOpenGL
```

## Shortcomings
- Saturn rings texture doesn't apply perfectly and rings lack thickness (disappear when viewed exactly edge-on).
- Stars in the skybox can appear to dim when moving — lighting/sampling interaction.
- Lighting is basic: no shadows and the sun's visual appearance is a textured sphere rather than an emissive/glow effect.
- No normal or bump mapping; textures are simple color maps.

## Improvements
- Make the sun render emissive and add bloom / lens flare post-processing.
- Implement shadow mapping and improved lighting (PBR) for better realism.
- Improve Saturn rings by using a thin 3D mesh or double-sided textured geometry and alpha blending.
- Add normal maps and specular maps to the planet shaders.

## Disclaimer

This project is an IGR201 practical by Kiwon Um. It is my first project, so please don't expect too much.

