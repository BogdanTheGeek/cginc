![image](https://user-images.githubusercontent.com/8824186/114960870-cb9bd000-9e5f-11eb-9a91-66c328208c13.png)

# CGinC
CGinC (See G-code (written) in C) is a small gcode visualiser that can also import stl files.

I have created it for myself as a tool to verify gcode generated by [libccam](https://github.com/BogdanTheGeek/libccam)(write gcode with c functions).

The main advantage over anything else out there is that you can pass the gcode and stl files as arguments to it in the command line, making it (almost) an instant preview.

Everything is written in C, with the help of [raylib](https://www.raylib.com/), which makes it possible to port to virtually any platform.

# Installation
This is still in the early days of development, but I am planning on releasing binaries.

# Compile
To compile make sure you have downloaded, compiled and installed raylib with default parameters.

Run:
```
./compile.sh
```

# Running & Features
To run simply pass the path to either a gcode file(.nc, .ngc, .gcode, .gc) and/or a binary stl file(OpenSCAD only does ASCII stl's do you will need to convert it to binary format):
```
./cginc test.nc resource/test.stl
```
Passing in the `--msaa` parameter enables antialiasing.

Use `Left Mouse` button to orbit and `Right Mouse` button to pan.

Pressing `Home` on the keyboard returns you to home location.

Pressing `c` on the keyboard toggles between Perspective and Orthogonal View.

Pressing `g` on the keyboard toggles the grid.

Pressing `o` on the keyboard toggles the origin.

Pressing `m` on the keyboard toggles the 3D model.

Pressing `d` on the keyboard toggles dark mode.


# Thanks to:
- raysan5 for [raylib](https://www.raylib.com/)
- WEREMSOFT for the [stl_loader](https://github.com/WEREMSOFT/stl-loader-for-raylib)
