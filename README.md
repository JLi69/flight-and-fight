# flightsim

Work in progress

## compile

Dependencies:
[stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) |
[glad](https://github.com/Dav1dde/glad) |
[glfw](https://github.com/glfw/glfw) |
[glm](https://github.com/g-truc/glm)

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
