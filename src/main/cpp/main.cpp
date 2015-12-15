#include "jpeg/JPEGReader.h"
#include "jpeg/JPEGWriter.h"

#include "image_operations.h"
#include "base64.h"

// Caffe
#include "boost/algorithm/string.hpp"
#include "google/protobuf/text_format.h"

#include "caffe/blob.hpp"
#include "caffe/common.hpp"
#include "caffe/net.hpp"
#include "caffe/proto/caffe.pb.h"
#include "caffe/util/db.hpp"
#include "caffe/util/io.hpp"

using caffe::Blob;
using caffe::BlobProto;
using caffe::Caffe;
using caffe::Net;
using boost::shared_ptr;
using std::string;

// Global variables
const int BATCH_SIZE = 1;
const int DIM = 227;
Frame mean(256, 256, 3, sizeof(float));
shared_ptr<Net<float> > feature_extraction_net;

void init_neural_net() {
  std::string model_path =
    "/opt/features/hybridCNN/hybridCNN_deploy_upgraded.prototxt";
  std::string model_weights_path =
    "/opt/features/hybridCNN/hybridCNN_iter_700000_upgraded.caffemodel";
    //"features/places205VGG16/snapshot_iter_765280.caffemodel";
  std::string mean_proto_path =
    "/opt/features/hybridCNN/hybridCNN_mean.binaryproto";

  // Initialize our network
  feature_extraction_net =
    shared_ptr<Net<float>>(new Net<float>(model_path, caffe::TEST));
  feature_extraction_net->CopyTrainedLayersFrom(model_weights_path);
  const shared_ptr<Blob<float>> data_blob =
    feature_extraction_net->blob_by_name("data");
  data_blob->Reshape({BATCH_SIZE, 3, DIM, DIM});

  // Load mean image
  Blob<float> data_mean;
  BlobProto blob_proto;
  bool result = ReadProtoFromBinaryFile(mean_proto_path, &blob_proto);
  data_mean.FromProto(blob_proto);
  memcpy(mean.data, data_mean.cpu_data(), sizeof(float) * 256 * 256 * 3);

  // MIT Places VGG-16 mean image
  // for (int i = 0; i < 224 * 224; ++i) {
  //   mean.data[i + (224 * 224) * 0] = 105.487823486f;
  //   mean.data[i + (224 * 224) * 1] = 113.741088867f;
  //   mean.data[i + (224 * 224) * 2] = 116.060394287f;
  // }
}

int main(int argc, char** argv) {
  init_neural_net();

  const int BATCH_SIZE = 1;

  // Load data from stdin
  JPEGReader reader;
  Frame frame;
  size_t data_size = 0;

  Frame conv_input(DIM, DIM, 3, sizeof(float));

  Blob<float> input{BATCH_SIZE, 3, DIM, DIM};
  float* input_data = input.mutable_cpu_data();

  int i = 0;
  while (std::cin.good()) {
    // Read new data
    std::string image_data;
    std::getline(std::cin, image_data);
    if (image_data.size() < 1) break;
    fprintf(stderr, "input (%d) size %d\n", i, image_data.size());
    fflush(stderr);

    std::vector<BYTE> bytes = base64_decode(image_data);

    // Parse data into jpeg
    reader.header_mem(reinterpret_cast<uint8_t*>(bytes.data()),
                      bytes.size());
    int width = reader.width();
    int height = reader.height();
    int components = reader.components();

    frame.width = width;
    frame.height = height;
    frame.channels = components;
    frame.element_size = sizeof(char);

    if (frame.data == nullptr) {
      data_size = sizeof(char) * width * height * components;
      frame.data = (char*)malloc(data_size);
    } else if (width * height * components > data_size) {
      data_size = sizeof(char) * width * height * components;
      frame.data = (char*)realloc(frame.data, data_size);
    }

    {
      std::vector<uint8_t*> rows;
      for (int i = 0; i < height; ++i) {
        rows.push_back(reinterpret_cast<uint8_t*>
                       (frame.data + i * width * components));
      }

      reader.load(rows.begin());
    }

    const shared_ptr<Blob<float>> data_blob =
      feature_extraction_net->blob_by_name("data");

    // Pack image into blob
    to_conv_input(&frame, &conv_input, &mean);
    memcpy(input_data, conv_input.data, DIM * DIM * 3 * sizeof(float));

    // Evaluate network
    feature_extraction_net->Forward({&input});

    // Extract features from Blob
    // TODO(abp): I can probably just allocate my own giant set of memory,
    // use it as the output for the net, and then create a bunch of images
    // backed by the features output into that memory
    const shared_ptr<Blob<float>> features_data =
      feature_extraction_net->blob_by_name("pool5");

    std::string output_bytes =
      base64_encode(reinterpret_cast<const uint8_t*>(features_data->cpu_data()),
                    sizeof(float) * POOL5_DIM);

    // Write out data
    std::cout << output_bytes;
    std::cout << std::endl;
    ++i;
  }
}
