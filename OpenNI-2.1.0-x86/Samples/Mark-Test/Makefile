all: test
	./test

test: test.cpp
	g++ -Wall -o test -MD -MP -MT -c -msse3 -DUNIX -DGLX_GLXEXT_LEGACY -Wall -O2 -DNDEBUG -I../../Include -I../../ThirdParty/GL/ -fPIC -fvisibility=hidden test.cpp -lglut -lGL -L../Bin -lOpenNI2 `pkg-config opencv --cflags --libs` -w

clean:
	rm -rf *.o *.d test
	
