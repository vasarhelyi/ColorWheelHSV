# ColorWheelHSV

HSV Color Wheel, by Shervin Emami (shervin.emami@gmail.com), 6th Nov 2009.
HSVFiltering part added by Gabor Vasarhelyi (vasarhelyi@hal.elte.hu), since Jan 2011.

Current version: 1.1.1 Build 71

This code lets you select colors and color ranges in HSV color space for colored blob detection.


# install

Code tested on linux and Windows.

## prerequisites

* The code relies on OpenCv 2.x, you should install that first (yes, I know, it is old, I will upgrade only if you really need it).
Hint: [this github gist](https://gist.github.com/arthurbeggs/06df46af94af7f261513934e56103b30) seems to be a smooth way to go (on linux).

* Using [CUDA](https://developer.nvidia.com/cuda-zone) is also recommended for speedup.

## linux

On linux, run `./bootstrap.sh` which creates `build`, in which you can `make`.

## windows

For Windows, code was only tested in Visual Studio. There you need to setup your environment properly. The file called `user_macros.props` might be of help.


# usage

An input image or video file is needed as a first argument.

Click on the top Hue map, or the bottom Color graph to change values.

## mouse events

* **LEFT** button: change values to 3x3 neighbor average color. Do not change range.
* **Shift+LEFT** button: Average colors. Do not change range.
* **RIGHT** button: include this pixel to new range as tight as possible.
* **Ctrl+RIGHT** button: undo last mouseclick.
* **Ctrl+LEFT** button: save current color and range and draw it on the palette.
* **Ctrl+Shift+LEFT** button: clear all saved colors+ranges.

## keyboard shortcuts

* **nN** - next frame
* **fF** - 100 frame forward
* **h,s,v** - start writing a number in the console, on Enter it will update color.H, .S, .V, respectively
* **H,S,V** - start writing a number in the console, on Enter it will update color.rangeH, .rangeS, .rangeV, respectively
* **c,C**   - start writing three numbers in the console with space between, on Enter it will update all color.H, .S, .V
* **r,R**   - start writing three numbers in the console with space between, on Enter it will update all color.rangeH, .rangeS, .rangeV

