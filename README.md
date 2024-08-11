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
