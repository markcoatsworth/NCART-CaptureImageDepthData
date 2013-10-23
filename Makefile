all: CaptureImageDepthData
	./CaptureImageDepthData

CaptureImageDepthData: CaptureImageDepthData.cpp
	g++ -Wall -o CaptureImageDepthData -MD -MP -MT -c -msse3 -DUNIX -DGLX_GLXEXT_LEGACY -Wall -O2 -DNDEBUG -I../OpenNI-2.1.0-x86/Include -I../OpenNI-2.1.0-x86/ThirdParty/GL/ -fPIC -fvisibility=hidden CaptureImageDepthData.cpp -L. -lglut -lGL -lOpenNI2 -lncurses `pkg-config opencv --cflags --libs` -w -Wl,-rpath ./

clean:
	rm -rf *.o *.d CaptureImageDepthData

	
