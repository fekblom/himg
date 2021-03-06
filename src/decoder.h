//-----------------------------------------------------------------------------
// HIMG, by Marcus Geelnard, 2015
//
// This is free and unencumbered software released into the public domain.
//
// See LICENSE for details.
//-----------------------------------------------------------------------------

#ifndef DECODER_H_
#define DECODER_H_

#include <cstdint>
#include <vector>

#include "downsampled.h"
#include "huffman_dec.h"
#include "mapper.h"
#include "quantize.h"

namespace himg {

class Decoder {
 public:
  Decoder(int max_threads = 0);

  bool Decode(const uint8_t *packed_data, int packed_size);

  const uint8_t *unpacked_data() const { return m_unpacked_data.data(); }
  int unpacked_size() const { return static_cast<int>(m_unpacked_data.size()); }

  int width() const { return m_width; }
  int height() const { return m_height; }
  int num_channels() const { return m_num_channels; }

 private:
  bool HasChroma() const;

  bool DecodeRIFFStart();
  bool DecodeHeader();
  bool DecodeLowResMappingFunction();
  bool DecodeLowRes();
  bool DecodeQuantizationConfig();
  bool DecodeFullResMappingFunction();
  bool DecodeFullRes();

  bool DecodeFullResBlockRow(const HuffmanDec &huffman_dec, int y);

  bool DecodeRIFFChunk(uint32_t *fourcc, int *size);
  bool FindRIFFChunk(uint32_t fourcc, int *size);

  int m_max_threads;

  Quantize m_quantize;
  LowResMapper m_low_res_mapper;
  FullResMapper m_full_res_mapper;
  std::vector<Downsampled> m_downsampled;
  std::vector<uint8_t> m_unpacked_data;

  const uint8_t *m_packed_data;
  int m_packed_size;
  int m_packed_idx;

  int m_width;
  int m_height;
  int m_num_channels;
  bool m_use_ycbcr;
};

}  // namespace himg

#endif  // DECODER_H_
