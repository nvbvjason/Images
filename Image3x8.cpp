#include "Image3x8.h"

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>

Pixel::Pixel()
    : red(0), green(0), blue(0)
{
}

Pixel::Pixel(const uint8_t r, const uint8_t g, const uint8_t b)
    : red(r), green(g), blue(b)
{
}

Pixel::Pixel(const Pixel &other) = default;

PixelDouble::PixelDouble()
    : red(0), green(0), blue(0)
{
}


PixelDouble::PixelDouble(const double r, const double g, const double b)
    : red(r), green(g), blue(b)
{
}

PixelDouble::PixelDouble(const PixelDouble &other) = default;

PixelDouble::PixelDouble(const Pixel &other)
    : red(other.red), green(other.green), blue(other.blue)
{
}

void PixelDouble::set_all_zero()
{
    red = 0;
    green = 0;
    blue = 0;
}

Image3x8::Image3x8(const Image3x8 &other)
    : c_height(other.height()), c_width(other.width()), c_offset(), m_colors(new Pixel[other.height() * other.width()])
{
    init(other);
}

Image3x8::Image3x8(const BmpMetaData &meta_data)
    : c_height(meta_data.height()), c_width(meta_data.width()), c_offset(meta_data.offset())
{
    std::ifstream ifs;
    ifs.open(meta_data.path(), std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "File could not be opened" << '\n';
        return;
    }
    const uint8_t padding_amount = ((4 - (c_width * 3) % 4) % 4);
    ifs.ignore(meta_data.offset());

    m_colors = new Pixel[c_height * c_width];
    for (int32_t row = 0; row < c_height; ++row) {
        for (int32_t col = 0; col < c_width; ++col) {
            uint8_t color[3];
            ifs.read(reinterpret_cast<char *>(color), 3);
            (*this)[row][col].red = color[2];
            (*this)[row][col].green = color[1];
            (*this)[row][col].blue = color[0];
        }
        ifs.ignore(padding_amount);
    }
    if (meta_data.top_down())
        reflect_vertical();
    ifs.close();
    std::cout << "File read\n";
}

void Image3x8::black_out_part(const int32_t start_row,
                              const int32_t end_row,
                              const int32_t start_col,
                              const int32_t end_col)
{
    for (int32_t row = start_row; row < end_row; ++row)
        for (int32_t col = start_col; col < end_col; ++col) {
            (*this)[row][col].red = 0.0;
            (*this)[row][col].green = 0.0;
            (*this)[row][col].blue = 0.0;
        }
}

void Image3x8::gray_scale()
{
    for (size_t i = 0; i < size(); ++i) {
        double temp = m_colors[i].red;
        temp += m_colors[i].green * 0.7152;
        temp += m_colors[i].blue;
        temp = (temp + 0.5) / 3;
        clamp(temp, 0, 255);
        memset(reinterpret_cast<void *>(&m_colors[i]), static_cast<uint8_t>(temp), 3);
    }
}

void Image3x8::gray_scale_lum()
{
    for (size_t i = 0; i < size(); ++i) {
        double temp = m_colors[i].red * 0.2126;
        temp += m_colors[i].green * 0.7152;
        temp += m_colors[i].blue * 0.0722;
        temp = (temp + 0.5) / 3;
        clamp(temp, 0, 255);
        memset(reinterpret_cast<void *>(&m_colors[i]), static_cast<uint8_t>(temp), 3);
    }
}

void Image3x8::color_mask(const double red, const double green, const double blue)
{
    for (size_t i = 0; i < size(); ++i) {
        double temp = m_colors[i].red * red;
        clamp(temp, 0, 255);
        m_colors[i].red = static_cast<uint8_t>(temp);
        temp = m_colors[i].green * green;
        clamp(temp, 0, 255);
        m_colors[i].green = static_cast<uint8_t>(temp);
        temp = m_colors[i].blue * blue;
        clamp(temp, 0, 255);
        m_colors[i].blue = static_cast<uint8_t>(temp);
    }
}

void Image3x8::sepia()
{
    for (size_t i = 0; i < size(); ++i) {
        const Pixel temp(m_colors[i]);
        double temp_f = 0.393 * temp.red + 0.769 * temp.green + 0.189 * temp.blue;
        clamp(temp_f, 0, 255);
        m_colors[i].red = static_cast<uint8_t>(temp_f);
        temp_f = 0.349 * temp.red + 0.686 * temp.green + 0.168 * temp.blue;
        clamp(temp_f, 0, 255);
        m_colors[i].green = static_cast<uint8_t>(temp_f);
        temp_f = 0.272 * temp.red + 0.534 * temp.green + 0.131 * temp.blue;
        clamp(temp_f, 0, 255);
        m_colors[i].blue = static_cast<uint8_t>(temp_f);
    }
}

void Image3x8::reflect_horizontal()
{
    for (int32_t row = 0; row < c_height; ++row)
        for (int32_t col = 0; col <= c_width / 2; ++col) {
            const Pixel temp((*this)[row][col]);
            (*this)[row][col] = (*this)[row][c_width - col - 1];
            (*this)[row][c_width - col - 1] = temp;
        }
}

void Image3x8::reflect_vertical()
{
    for (int32_t row = 0; row < c_height / 2; ++row)
        for (int32_t col = 0; col < c_width; ++col) {
            const Pixel temp((*this)[row][col]);
            (*this)[row][col] = (*this)[c_height - row - 1][col];
            (*this)[c_height - row - 1][col] = temp;
        }
}

void Image3x8::blur()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = {1, 1, 1, 1, 1, 1, 1, 1, 1};
    for (int32_t row = 0; row < c_height; ++row)
        for (int32_t col = 0; col < c_width; ++col) {
            const uint8_t counter = kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0 / counter, color);
            color.set_all_zero();
        }
}

void Image3x8::ridge()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    for (int32_t row = 0; row < c_height; ++row)
        for (int32_t col = 0; col < c_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0, color);
            color.set_all_zero();
        }
}

void Image3x8::sharpen()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = {0, -1, 0, -1, 5, -1, 0, -1, 0};
    for (int32_t row = 0; row < c_height; ++row)
        for (int32_t col = 0; col < c_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0, color);
            color.set_all_zero();
        }
}

void Image3x8::emboss()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
    for (int32_t row = 0; row < c_height; ++row)
        for (int32_t col = 0; col < c_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0, color);
            color.set_all_zero();
        }
}

void Image3x8::eval_3x3_0(const int32_t row, const int32_t col, const double factor, const PixelDouble &color)
{
    double temp = color.red * factor;
    clamp(temp, 0, 255);
    (*this)[row][col].red = static_cast<uint8_t>(temp);
    temp = color.green * factor;
    clamp(temp, 0, 255);
    (*this)[row][col].green = static_cast<uint8_t>(temp);
    temp = color.blue * factor;
    clamp(temp, 0, 255);
    (*this)[row][col].blue = static_cast<uint8_t>(temp);
}

void Image3x8::edges()
{
    const Image3x8 copy(*this);
    PixelDouble color_x;
    PixelDouble color_y;
    const std::vector<int8_t> kernel_x = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
    const std::vector<int8_t> kernel_y = {-1, -2, -1, 0, 0, 0, 1, 2, 1};
    for (int32_t row = 0; row < c_height; ++row)
        for (int32_t col = 0; col < c_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel_x, color_x);
            kernel_3x3_0(row, col, copy, kernel_y, color_y);
            evaluate_edges(row, col, color_x, color_y);
            color_x.set_all_zero();
            color_y.set_all_zero();
        }
}

void Image3x8::evaluate_edges(const int32_t row,
                              const int32_t col,
                              const PixelDouble &color_x,
                              const PixelDouble &color_y)
{
    const Pixel color = edges_color_eval(color_x, color_y);
    constexpr double limit = 100;
    if (limit <= std::abs((*this)[row][col].red - color.red))
        (*this)[row][col].red = color.red;
    if (limit <= std::abs((*this)[row][col].green - color.green))
        (*this)[row][col].green = color.green;
    if (limit <= std::abs((*this)[row][col].blue - color.blue))
        (*this)[row][col].blue = color.blue;
}


void Image3x8::write(const char *path)
{
    std::ofstream ofs;
    ofs.open(path, std::ios::out | std::ios::binary);
    if (!ofs.is_open()) {
        std::cout << "File cannot be opened";
        return;
    }
    const uint8_t padding_amount = (4 - (c_width * 3) % 4) % 4;
    const size_t file_size = s_file_header_size + s_default_information_header_size
                             + c_height * (c_width * 3 + padding_amount);
    uint8_t file_header[s_file_header_size];
    write_file_header(file_size, file_header);
    uint8_t information_header[s_default_information_header_size];
    write_information_header(information_header);

    ofs.write(reinterpret_cast<char *>(file_header), s_file_header_size);
    ofs.write(reinterpret_cast<char *>(information_header), s_default_information_header_size);
    uint8_t bmpPad[3] = {0, 0, 0};
    for (int32_t row = 0; row < c_height; ++row) {
        for (int32_t col = 0; col < c_width; ++col) {
            const uint8_t r = (*this)[row][col].red;
            const uint8_t g = (*this)[row][col].green;
            const uint8_t b = (*this)[row][col].blue;

            uint8_t color[3] = {b, g, r};
            ofs.write(reinterpret_cast<char *>(color), 3);
        }
        ofs.write(reinterpret_cast<char *>(bmpPad), padding_amount);
    }
    ofs.close();
    std::cout << "file created\n";
}

void Image3x8::init(const Image3x8 &other)
{
    memcpy(reinterpret_cast<void *>(m_colors), reinterpret_cast<void *>(other.m_colors), other.size() * sizeof(Pixel));
}

uint8_t Image3x8::kernel_3x3_0(const int32_t row_img,
                               const int32_t col_img,
                               const Image3x8 &copy,
                               const std::vector<int8_t> &kernel,
                               PixelDouble &color) const
{
    uint8_t index = -1;
    uint8_t counter = 0;
    for (int32_t row = row_img - 1; row <= row_img + 1; ++row) {
        for (int32_t col = col_img - 1; col <= col_img + 1; ++col) {
            ++index;
            if (row <= -1 || c_height <= row)
                continue;
            if (col <= -1 || c_width <= col)
                continue;
            ++counter;
            color.red += copy[row][col].red * kernel[index];
            color.green += copy[row][col].green * kernel[index];
            color.blue += copy[row][col].blue * kernel[index];
        }
    }
    return counter;
}

Pixel edges_color_eval(const PixelDouble &color_x, const PixelDouble &color_y)
{
    double red = std::sqrt(color_x.red * color_x.red + color_y.red * color_y.red);
    double green = std::sqrt(color_x.green * color_x.green + color_y.green * color_y.green);
    double blue = std::sqrt(color_x.blue * color_x.blue + color_y.blue * color_y.blue);
    clamp(red, 0, 255);
    clamp(green, 0, 255);
    clamp(blue, 0, 255);
    return {static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue)};
}

void Image3x8::write_file_header(const size_t file_size, uint8_t *file_header)
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
    file_header[10] = s_file_header_size + s_default_information_header_size;
    file_header[11] = 0;
    file_header[12] = 0;
    file_header[13] = 0;
}

void Image3x8::write_information_header(uint8_t *information_header) const
{
    // Header size
    information_header[0] = s_default_information_header_size;
    information_header[1] = 0;
    information_header[2] = 0;
    information_header[3] = 0;
    // Image width
    information_header[4] = c_width;
    information_header[5] = c_width >> 8;
    information_header[6] = c_width >> 16;
    information_header[7] = c_width >> 24;
    // Image height
    information_header[8] = c_height;
    information_header[9] = c_height >> 8;
    information_header[10] = c_height >> 16;
    information_header[11] = c_height >> 24;
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
