#pragma once

#ifndef IMAGES_BMP_H
#define IMAGES_BMP_H

#include "Image3x8.h"

#include <cstdint>
#include <ostream>

struct FileHeader {
    uint16_t header_field;
    uint32_t size;
    uint16_t idk_1;
    uint16_t idk_2;
    uint32_t offset;
};

struct InfoHeader {
    int32_t size_info_header;
    int32_t width;
    int32_t height;
};

static constexpr uint8_t file_header_size = 14;
static constexpr uint8_t file_info_size = 40;

[[nodiscard]] Image3x8 create_3x8_from_bmp(const char* path);
void write_bmp_file(const Image3x8& image, const char* path);
static void write_information_header(uint8_t* information_header, int32_t width, int32_t height);
static void write_file_header(size_t file_size, uint8_t* file_header);

#endif //IMAGES_BMP_H
