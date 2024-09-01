# Flight & Fight

Work in progress

## compile

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
