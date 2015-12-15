OUT := evaluate_caffe

GCC= g++

.PHONY: default
default: $(OUT)

SOURCE_FILES := \
  main.cpp \
  image_operations.cpp \
  base64.cpp \
  jpeg/JPEGReader.cpp \
  jpeg/JPEGWriter.cpp

HALIDE_SRC := \
  to_conv_patch.cpp

OBJECTS := $(SOURCE_FILES:%.cpp=src/main/cpp/%.o)

#Third party include and library paths
HALIDE_INC_PATH=`echo ~`/repos/Halide/include
HALIDE_LIB_PATH=`echo ~`/repos/Halide/bin

CAFFE_INC_PATH=`echo ~`/repos/caffe/include
CAFFE_LIB_PATH=`echo ~`/repos/caffe/build/lib

HDF5_INC_PATH=/usr/include/hdf5/serial
HDF5_LIB_PATH=/usr/lib/x86_64-linux-gnu/hdf5/serial

LD_FLAGS += \
  -ljpeg \
  -lz -lm -ldl \
  -L$(CAFFE_LIB_PATH) -lcaffe -L$(HDF5_LIB_PATH) -lhdf5 -lglog \
  -pthread

INCLUDE_FLAGS += \
  -I./src/main/cpp/ \
  -I$(CAFFE_INC_PATH) \
  -I$(HDF5_INC_PATH)

GCC_FLAGS := -std=c++11 -g
# For Caffe
GCC_FLAGS += -DCPU_ONLY -DUSE_MKL

HALIDE_GEN := $(HALIDE_SRC:%.cpp=src/main/cpp/halide/%_gen)
HALIDE_OBJS := $(HALIDE_SRC:%.cpp=src/main/cpp/halide/%.o)

$(OUT): $(HALIDE_OBJS) $(OBJECTS)
	$(GCC) -o $@ -std=c++11 -I./src/main/cpp/ $^ $(LD_FLAGS)

$(OBJECTS): src/main/cpp/%.o : src/main/cpp/%.cpp
	$(GCC) -o $@ -c $< $(GCC_FLAGS) $(INCLUDE_FLAGS)

$(HALIDE_OBJS) : %.o : %.cpp
	$(GCC) -o $(@:%.o=%_gen) $< -g -ggdb -std=c++11 \
	-I$(HALIDE_INC_PATH) -L$(HALIDE_LIB_PATH) -lHalide && \
	cd ./src/main/cpp/halide && ../../../../$(@:%.o=%_gen)

cleanthis:
	rm -fr $(OUT) $(OBJECTS) $(HALIDE_OBJS) $(HALIDE_GEN)
