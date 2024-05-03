TARGET := libBitmaps.a
D_TARGET := libBitmaps.so
HDR_TARGET := Bitmaps.hpp

SRC_DIR := src
OBJ_DIR := obj
TST_DIR := tests

HEADERS := \
src/bitmaps.hpp\
src/class_RGBA_bitmap.hpp\
src/class_RGBA_sprite.hpp\
src/class_RGB_bitmap.hpp\
src/ppm.hpp\
src/struct_RGBA.hpp\
src/struct_RGB.hpp

SRC_FILES := \
src/bitmaps.cpp\
src/class_RGBA_bitmap.cpp\
src/class_RGBA_sprite.cpp\
src/class_RGB_bitmap.cpp\
src/ppm.cpp\

OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,$(SRC_FILES))

CXX = g++
CXXFLAGS = -g -fno-exceptions -Wall -Wpedantic -Wextra -Wparentheses -O2 

INCLUDE = -I./ -Isrc

#GL_INCL = -IOpenGL_C
#GL_LIBS = -lglut -lGL
#INT_LIBS = -LOpenGL_C -lOpenGLintC

BTM_LIBS = -L./ -lBitmaps

all: libs

libs: $(OBJ_FILES) $(HEADERS)
	ar rs $(TARGET) $(OBJ_FILES)

test: plot_on_rgb plot_on_rgba plot_on_spr test_read_sp4 test_read_ppm

plot_on_rgb: $(TST_DIR)/plot_on_rgb.cpp
	$(CXX) $(CXXFLAGS) -o $(TST_DIR)/test_plot_rgb $(TST_DIR)/plot_on_rgb.cpp $(BTM_LIBS) $(INCLUDE)

plot_on_rgba: $(TST_DIR)/plot_on_rgba.cpp
	$(CXX) $(CXXFLAGS) -o $(TST_DIR)/test_plot_rgba $(TST_DIR)/plot_on_rgba.cpp $(BTM_LIBS) $(INCLUDE)

plot_on_spr: $(TST_DIR)/plot_on_spr.cpp
	$(CXX) $(CXXFLAGS) -o $(TST_DIR)/test_plot_spr $(TST_DIR)/plot_on_spr.cpp $(BTM_LIBS) $(INCLUDE)

test_read_sp4: $(TST_DIR)/test_read_sp4.cpp
	$(CXX) $(CXXFLAGS) -o $(TST_DIR)/test_read_sp4 $(TST_DIR)/test_read_sp4.cpp $(BTM_LIBS) $(INCLUDE)

test_read_ppm: $(TST_DIR)/test_read_ppm.cpp
	$(CXX) $(CXXFLAGS) -o $(TST_DIR)/test_read_ppm $(TST_DIR)/test_read_ppm.cpp $(BTM_LIBS) $(INCLUDE)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDE) 


headers:
	cat $(SRC_DIR)/BitmapsC++_header > $(HDR_TARGET)
	awk '!/#include/' $(SRC_DIR)/struct_RGB.hpp >> $(HDR_TARGET)
	awk '!/#include/' $(SRC_DIR)/struct_RGBA.hpp >> $(HDR_TARGET)
	awk '!/#include/' $(SRC_DIR)/class_RGB_bitmap.hpp >> $(HDR_TARGET)
	awk '!/#include/' $(SRC_DIR)/class_RGBA_bitmap.hpp >> $(HDR_TARGET)
	awk '!/#include/' $(SRC_DIR)/class_RGBA_sprite.hpp >> $(HDR_TARGET)
	awk '!/#include/' $(SRC_DIR)/bitmaps.hpp >> $(HDR_TARGET)
	awk '!/#include/' $(SRC_DIR)/ppm.hpp >> $(HDR_TARGET)
	cat $(SRC_DIR)/BitmapsC++_footer >> $(HDR_TARGET)


clean:
	rm -f $(OBJ_DIR)/*.o $(TARGET)

.PHONY: clean all test headers_N