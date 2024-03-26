#include "Bmp.h"

#include <fstream>
#include <iostream>
#include <ostream>

Image3x8 create_3x8_from_bmp(const char *path)
{
    std::ifstream ifs;
    ifs.open(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "File could not be opened\n";
        ifs.close();
        throw std::exception();
    }
    uint8_t file_header[file_header_size];
    ifs.read(reinterpret_cast<char *>(file_header), file_header_size);
    if (file_header[0] != 'B' || file_header[1] != 'M') {
        std::cout << "The specified path is not a bitmap image\n";
        ifs.close();
        throw std::exception();
    }
    const int offset = file_header[10] + (file_header[11] << 8) + (file_header[12] << 16)
                       + (file_header[13] << 24);
    uint8_t file_info[file_info_size];
    ifs.read(reinterpret_cast<char *>(file_info), file_info_size);
    int width = file_info[4] + (file_info[5] << 8) + (file_info[6] << 16) + (file_info[7] << 24);
    int height = file_info[8] + (file_info[9] << 8) + (file_info[10] << 16) + (file_info[11] << 24);
    bool isTopDown = false;
    if (height < 0) {
        height = -height;
        isTopDown = true;
    }
    if (file_header_size + file_info_size < offset)
        ifs.ignore(offset - file_header_size - file_info_size);
    std::vector<uint8_t> data(width * height * 3);
    int64_t index = 0;
    for (int32_t row = 0; row < height; ++row) {
        for (int32_t col = 0; col < width; ++col) {
            uint8_t pixel[3];
            ifs.read(reinterpret_cast<char *>(pixel), 3);
            data[index + 0] = pixel[2];
            data[index + 1] = pixel[1];
            data[index + 2] = pixel[0];
            index += 3;
        }
    }
    std::cout << "File read\n";
    Image3x8 result(data, height, width);
    if (isTopDown)
        result.reflect_vertical();
    return result;
}

void write_bmp_file(const Image3x8 &image, const char *path)
{
    std::ofstream ofs;
    ofs.open(path, std::ios::out | std::ios::binary);
    if (!ofs.is_open()) {
        std::cout << "File cannot be opened";
        return;
    }
    const uint8_t padding_amount = (4 - (image.width() * 3) % 4) % 4;
    const size_t file_size = file_header_size + file_info_size
                             + image.height() * (image.width() * 3 + padding_amount);
    uint8_t file_header[file_header_size];
    write_file_header(file_size, file_header);
    ofs.write(reinterpret_cast<char *>(file_header), file_header_size);
    uint8_t file_info[file_info_size];
    write_information_header(file_info, image.width(), image.height());
    ofs.write(reinterpret_cast<char *>(file_info), file_info_size);
    uint8_t bmpPad[3] = {0, 0, 0};
    for (int32_t row = 0; row < image.height(); ++row) {
        for (int32_t col = 0; col < image.width(); ++col) {
            const uint8_t r = image[row][col].red;
            const uint8_t g = image[row][col].green;
            const uint8_t b = image[row][col].blue;
            uint8_t color[3] = {b, g, r};
            ofs.write(reinterpret_cast<char *>(color), 3);
        }
        ofs.write(reinterpret_cast<char *>(bmpPad), padding_amount);
    }
    ofs.close();
    std::cout << "File created\n";
}

static void write_file_header(const size_t file_size, uint8_t *file_header)
{
    // file type
    file_header[0] = 'B';
    file_header[1] = 'M';
    // file size
    file_header[2] = file_size;
    file_header[3] = file_size >> 8;
    file_header[4] = file_size >> 16;
    file_header[5] = file_size >> 24;
    // Reserved (not used)
    file_header[6] = 0;
    file_header[7] = 0;
    // Reserved 1 (not used)
    file_header[8] = 0;
    file_header[9] = 0;
    // Pixel data offset
    file_header[10] = file_header_size + file_info_size;
    file_header[11] = 0;
    file_header[12] = 0;
    file_header[13] = 0;
}

void write_information_header(uint8_t *information_header, int32_t width, int32_t height)
{
    // Header size
    information_header[0] = file_info_size;
    information_header[1] = 0;
    information_header[2] = 0;
    information_header[3] = 0;
    // Image width
    information_header[4] = width;
    information_header[5] = width >> 8;
    information_header[6] = width >> 16;
    information_header[7] = width >> 24;
    // Image height
    information_header[8] = height;
    information_header[9] = height >> 8;
    information_header[10] = height >> 16;
    information_header[11] = height >> 24;
    // Planes
    information_header[12] = 1;
    information_header[13] = 0;
    // Bits per pixel (RGB)
    information_header[14] = 24;
    information_header[15] = 0;
    // Compression (No compression)
    information_header[16] = 0;
    information_header[17] = 0;
    information_header[18] = 0;
    information_header[19] = 0;
    // Image size (No compression)
    information_header[20] = 0;
    information_header[21] = 0;
    information_header[22] = 0;
    information_header[23] = 0;
    // X pixel per meter (Not specified)
    information_header[24] = 0;
    information_header[25] = 0;
    information_header[26] = 0;
    information_header[27] = 0;
    // Y pixel per meter (Not specified)
    information_header[28] = 0;
    information_header[29] = 0;
    information_header[30] = 0;
    information_header[31] = 0;
    // Total colors (Color palette not used)
    information_header[32] = 0;
    information_header[33] = 0;
    information_header[34] = 0;
    information_header[35] = 0;
    // Important colors (Generally ignored)
    information_header[36] = 0;
    information_header[37] = 0;
    information_header[38] = 0;
    information_header[39] = 0;
}
