CC = g++

GLLIBS = -lglut -lGLEW -lGL

all: main.cpp light.cpp ambient.cpp diffuse.cpp specular.cpp phong.cpp
	$(CC) main.cpp ../lib/utils.cpp -o teste $(GLLIBS)
	$(CC) light.cpp ../lib/utils.cpp -o light $(GLLIBS)
	$(CC) ambient.cpp ../lib/utils.cpp -o ambient $(GLLIBS)
	$(CC) diffuse.cpp ../lib/utils.cpp -o diffuse $(GLLIBS)
	$(CC) specular.cpp ../lib/utils.cpp -o specular $(GLLIBS)
	$(CC) phong.cpp ../lib/utils.cpp -o phong $(GLLIBS)

clean:
	rm -f teste light ambient diffuse specular phong
