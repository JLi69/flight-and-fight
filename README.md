# Flight & Fight

You can download this game on [itch.io](https://nullptr-error.itch.io/flight-fight)

## About

This is meant to be a simple arcade flight simulator/plane fighting game.


### Casual Mode

Just fly around and enjoy looking at the terrain (just avoid crashing into it).

### Fight Mode

Shoot down as many enemies as possible before you yourself get shot down to get 
the highest possible score. Use your minimap to locate where enemies are and avoid 
crashing into the terrain. Good luck!

## Controls

 - Space Bar/Left Mouse Button - Shoot
 - A, D - Turn
 - S - Pitch up
 - W - Pitch down
 - Left Shift/Scroll up - Speed up
 - Left Ctrl/Scroll down - Slow down
 - You can also move your mouse to rotate the plane (move mouse up = pitch up, move mouse down = pitch down, move mouse left = rotate left, move mouse right = rotate right)
 - T - toggle crosshair in fight mode

## Compile

Dependencies:

[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) (included)

[glad](https://github.com/Dav1dde/glad) (included)

[glfw](https://github.com/glfw/glfw)

[glm](https://github.com/g-truc/glm)

[nuklear](https://github.com/Immediate-Mode-UI/Nuklear) (included)

[fast_obj](https://github.com/thisistherk/fast_obj) (included)

[dr_wav](https://github.com/mackron/dr_libs/blob/master/dr_wav.h) (included)

[openal-soft](https://github.com/kcat/openal-soft)

openal-soft, GLFW, and glm are provided as submodules that you can clone using
`git clone --recursive` and you can then manually build and install them yourself
should you need them. If it's possible you can also install them through a
package manager if they are available for your distro.

Linux:

```
make -j$(nproc)
./flightsim
```

Windows:

Use [mingw-w64](https://sourceforge.net/projects/mingw-w64) to compile:

```
mingw32-make -j$(nproc)
.\flightsim.exe
```

## Credits

for a full list of all the 3rd party assets used, go to `assets/credits.txt`.
