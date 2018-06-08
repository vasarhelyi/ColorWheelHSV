
# OpenCV (activate the second set of lines in case you don't have pkg-config)
OPENCV_CPPFLAGS    := `pkg-config --cflags opencv`
OPENCV_LDLIBS      := `pkg-config --libs opencv`

override CPPFLAGS  += $(OPENCV_CPPFLAGS) -I/usr/include -I/usr/local/include/libswscale -g $(OPENCV_LDLIBS) -L/usr/local/lib


ColorWheelHSV: ColorWheelHSV.cpp
	g++ $(CPPFLAGS) -DON_LINUX -D__STDC_CONSTANT_MACROS ColorWheelHSV.cpp -lavformat -lavcodec -lavutil -o ColorWheelHSV

clean:
	echo 'clean'
	-rm ColorWheelHSV
