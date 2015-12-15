#ifndef IMAGE_OPERATIONS_H_
#define IMAGE_OPERATIONS_H_

const int POOL5_DIM = 9216;

// Frame datastructure
struct Frame {
  Frame()
  : width(0), height(0), channels(0), element_size(0), data(nullptr) {}

  Frame(int width, int height, int channels, int element_size)
  : width(width),
    height(height),
    channels(channels),
    element_size(element_size) {
    data = new char[width * height * channels * element_size];
  }

  int width;
  int height;
  int channels;
  int element_size;
  char* data;
};

int to_conv_input(Frame* in, Frame* out, Frame* mean);

#endif // IMAGE_OPERATIONS_H_
