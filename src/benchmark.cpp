//-----------------------------------------------------------------------------
// HIMG, by Marcus Geelnard, 2015
//
// This is free and unencumbered software released into the public domain.
//
// See LICENSE for details.
//-----------------------------------------------------------------------------

#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>

#include <FreeImage.h>

#include "decoder.h"
#include "encoder.h"

namespace {

const int kNumIterations = 30;

enum BenchmarkMode {
  Decode,
  Encode
};

class TimeMeasure {
 public:
  void Start() { m_start = std::chrono::system_clock::now(); }

  double Duration() const {
    std::chrono::system_clock::time_point t = std::chrono::system_clock::now();
    return static_cast<double>(
               std::chrono::duration_cast<std::chrono::microseconds>(
                   t - m_start).count()) /
           1000.0;
  }

 private:
  std::chrono::system_clock::time_point m_start;
};

bool IsHimg(const std::vector<uint8_t> &buffer) {
  return buffer.size() >= 12 && buffer[0] == 'R' && buffer[1] == 'I' &&
         buffer[2] == 'F' && buffer[3] == 'F' && buffer[8] == 'H' &&
         buffer[9] == 'I' && buffer[10] == 'M' && buffer[11] == 'G';
}

void ShowUsage(const char *arg0) {
  std::cout << "Usage: " << arg0 << " [-d][-e] image" << std::endl;
  std::cout << "  -d Decode (default)" << std::endl;
  std::cout << "  -e Encode" << std::endl;
}

bool LoadFile(const std::string &file_name, std::vector<uint8_t> *buffer) {
  // Open file.
  std::ifstream f(file_name.c_str(), std::ifstream::in | std::ofstream::binary);
  if (!f.good()) {
    std::cout << "Unable to read file " << file_name << std::endl;
    return false;
  }

  // Get file size.
  f.seekg(0, std::ifstream::end);
  int file_size = static_cast<int>(f.tellg());
  f.seekg(0, std::ifstream::beg);
  std::cout << "File size: " << file_size << std::endl;

  // Read the file data into our buffer.
  buffer->resize(file_size);
  f.read(reinterpret_cast<char *>(buffer->data()), file_size);

  return true;
}

}  // namespace

int main(int argc, const char **argv) {
  // Parse arguments.
  BenchmarkMode benchmark_mode = Decode;
  std::string file_name;
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if (arg[0] == '-' && arg[1] != 0 && arg[2] == 0) {
      if (arg[1] == 'd')
        benchmark_mode = Decode;
      else if (arg[1] == 'e')
        benchmark_mode = Encode;
    } else if (file_name.empty()) {
      file_name = std::string(arg);
    } else {
      ShowUsage(argv[0]);
      return 0;
    }
  }

  if (file_name.empty()) {
    ShowUsage(argv[0]);
    return 0;
  }

  // Load the data from a file into memory.
  std::vector<uint8_t> buffer;
  LoadFile(file_name, &buffer);

  FreeImage_Initialise();
  himg::Decoder himg_decoder;

  double min_dt = -1.0, max_dt = -1.0, total_t = 0.0;
  for (int iteration = 1; iteration <= kNumIterations; ++iteration) {
    std::cout << "Iteration " << iteration << "/" << kNumIterations
              << std::endl;

    TimeMeasure one_measure;
    one_measure.Start();

    if (benchmark_mode == Decode) {
      // Decode the image.
      if (IsHimg(buffer)) {
        // Use the HIMG decoder.
        if (!himg_decoder.Decode(buffer.data(), buffer.size())) {
          std::cout << "Unable to decode image." << std::endl;
          return -1;
        }
      } else {
        // Use FreeImage to decode.
        // TODO(m): Add support for other decoders (libjpeg-turbo!).
        FIMEMORY *mem = FreeImage_OpenMemory(static_cast<BYTE *>(buffer.data()),
                                             buffer.size());
        FREE_IMAGE_FORMAT format =
            FreeImage_GetFIFFromFilename(file_name.c_str());
        FIBITMAP *bitmap = FreeImage_LoadFromMemory(format, mem);
        FreeImage_Unload(bitmap);
        FreeImage_CloseMemory(mem);
      }
    } else {
      // TODO(m): Implement me!
    }

    double dt = one_measure.Duration();
    if (min_dt < 0.0 || dt < min_dt) {
      min_dt = dt;
    }
    if (max_dt < 0.0 || dt > max_dt) {
      max_dt = dt;
    }
    total_t += dt;
  }

  double average = total_t / static_cast<double>(kNumIterations);
  std::cout << "    Min: " << min_dt << " ms\n";
  std::cout << "    Max: " << max_dt << " ms\n";
  std::cout << "Average: " << average << " ms\n";

  FreeImage_DeInitialise();

  return 0;
}
