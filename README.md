# ColorWheelHSV

HSV Color Wheel, by Shervin Emami (shervin.emami@gmail.com), 6th Nov 2009.
HSVFiltering part added by Gabor Vasarhelyi (vasarhelyi@hal.elte.hu), since Jan 2011.

Current version: 1.1.1 Build 71

This code lets you select HSV colors and color ranges (radius around base color in HSV color space) for colored blob detection.


# install

Code tested on linux and Windows.

## prerequisites

* The code relies on OpenCv 2.x/3.x, you should install that first (yes, I know, it is old, I will upgrade only if you really need it).
  I tested it with versions 2.4.13 and 3.2.0 and both work fine.

* Using [CUDA](https://developer.nvidia.com/cuda-zone) is also recommended for speedup.

## linux

On linux, run `./bootstrap.sh` which creates `build`, in which you can `make`.

If you want to experiment with different OpenCV versions, change the explicitely
given version in `CMakeLists.txt` in the line `find_package( OpenCV ... REQUIRED )`.

## windows

For Windows, code was only tested in Visual Studio. There you need to setup your environment properly. The file called `user_macros.props` might be of help.


# usage

An input image or video file is needed as a first argument.

Click on the top Hue map, or the bottom Color graph to change values.

## mouse events

* **LEFT** button: change values to 3x3 neighbor average color. Do not change range.
* **Shift+LEFT** button: Average colors. Do not change range.
* **Ctrl+LEFT** button: save current color and range and draw it on the palette.
* **Ctrl+Shift+LEFT** button: clear all saved colors+ranges.

* **RIGHT** button: include this pixel to new range as tight as possible.
* **Shift+RIGHT** button: undo last mouseclick or console color input
* **Ctrl+RIGHT** button: undo last mouseclick or console color input
* **Ctrl+Shift+RIGHT** button: reset inclusion of multiple colors

## keyboard shortcuts

Note: these work only when the image window is the active one.

* **nN** - next frame
* **fF** - 100 frame forward
* **h,s,v** - start writing a number in the console, on Enter it will update color.H, .S, .V, respectively
* **H,S,V** - start writing a number in the console, on Enter it will update color.rangeH, .rangeS, .rangeV, respectively
* **c,C**   - start writing three numbers in the console with space between, on Enter it will update all color.H, .S, .V
* **r,R**   - start writing three numbers in the console with space between, on Enter it will update all color.rangeH, .rangeS, .rangeV
* **d/D**   - draw red circles and show area/diameter around none/largest/all blobs
* **x/X**   - change highlight color (blue, green, red, white)
* **BackSpace** - undo last color or range selection (of mouse clicks or console input)

