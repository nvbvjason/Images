#pragma once

#ifndef PNG_HELPERS_H
#define PNG_HELPERS_H

#include "Image3x8.h"

Image3x8 interlace(const std::vector<Image3x8>& images, int32_t height, int32_t width);
static std::vector<std::vector<int8_t> > make_interlace_kernel(int32_t size);
std::vector<uint8_t> filter(const Image3x8& image, uint8_t fitler_type);
static size_t index_(int32_t row, int32_t col, int32_t width);
static uint8_t filter_one_two(uint8_t x, uint8_t ab);
static uint8_t filter_three(uint8_t x, uint8_t a, uint8_t b);
static uint8_t peath(uint8_t a, uint8_t b, uint8_t c);
Image3x8 defilter(const std::vector<uint8_t>& data, int32_t height, int32_t width);
static uint8_t defilter_one_two(uint8_t x, uint8_t ab);
static uint8_t defilter_three(uint8_t x, uint8_t a, uint8_t b);


#endif //PNG_HELPERS_H
