CC = emcc
CFLAGS = -c -O2 -fPIC
LDFLAGS = -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s BUILD_AS_WORKER=1 --post-js build/glue.js
SOURCES = lensfun/auxfun.cpp lensfun/camera.cpp lensfun/database.cpp \
			lensfun/lens.cpp lensfun/mod-color.cpp lensfun/mod-coord.cpp \
			lensfun/mod-pc.cpp lensfun/mod-subpix.cpp lensfun/modifier.cpp \
			lensfun/mount.cpp
OBJECTS = $(SOURCES:.cpp=.o)
EXECUTABLE = dist/lensfun_wasm.html

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	python ../emsdk-portable/emscripten/1.37.21/tools/webidl_binder.py bindings/bindings.idl build/glue
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) bindings/glue_wrapper.cpp

%.o: %.cpp 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm lensfun/*.o 
	rm dist/*.html dist/*.js
	rm build/*.js build/*.cpp