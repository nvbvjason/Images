#pragma once

#ifndef IMAGE_H
#define IMAGE_H

#include "BmpMetaData.h"

#include <cstdint>
#include <cstdio>
#include <vector>

struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Pixel();

    Pixel(uint8_t r, uint8_t g, uint8_t b);

    Pixel(const Pixel &other);
};

struct PixelDouble {
    double red;
    double green;
    double blue;

    PixelDouble();
    PixelDouble(double r, double g, double b);
    PixelDouble(const PixelDouble &other);
    explicit PixelDouble(const Pixel &other);
    void set_all_zero();
};

class Image3x8 {
    int32_t m_height;
    int32_t m_width;
    uint32_t m_offset;
    Pixel *m_colors;
    static constexpr uint8_t s_file_header_size = 14;
    static constexpr uint8_t s_default_information_header_size = 40;

public:
    // CREATORS
    Image3x8(const Image3x8 &other);
    explicit Image3x8(const char *path);
    ~Image3x8() { delete[] m_colors; }

    // MANIPULATORS
    void black_out_part(int32_t start_row, int32_t end_row, int32_t start_col, int32_t end_col);
    Pixel *operator[](const int32_t row) { return m_colors + m_width * row; }
    void gray_scale();
    void gray_scale_lum();
    void sepia();
    void reflect_horizontal();
    void reflect_vertical();
    void blur();
    void ridge();
    void sharpen();
    void emboss();
    void edges();
    void color_mask(double red, double green, double blue);

    // ACCESSORS
    [[nodiscard]] int32_t height() const { return m_height; }
    [[nodiscard]] int32_t width() const { return m_width; }
    [[nodiscard]] uint32_t offset() const { return m_offset; }
    [[nodiscard]] size_t size() const { return m_height * m_width; }
    [[nodiscard]] const Pixel *operator[](const int32_t row) const { return m_colors + m_width * row; }

    void write(const char *path);

private:
    void init(const Image3x8 &other);
    uint8_t kernel_3x3_0(int32_t row_img,
                         int32_t col_img,
                         const Image3x8 &copy,
                         const std::vector<int8_t> &kernel,
                         PixelDouble &color) const;
    void eval_3x3_0(int32_t row, int32_t col, double factor, const PixelDouble &color);
    void evaluate_edges(int32_t row, int32_t col, const PixelDouble &color_x, const PixelDouble &color_y);
    static inline void edges_pixel_helper(int32_t row, int32_t col, const Image3x8 &copy, PixelDouble &color,
                                          double factor);
    static void write_file_header(size_t file_size, uint8_t *file_header);
    void write_information_header(uint8_t *information_header) const;
};

static Pixel edges_color_eval(const PixelDouble &color_x, const PixelDouble &color_y);

void Image3x8::edges_pixel_helper(const int32_t row, const int32_t col, const Image3x8 &copy, PixelDouble &color,
                                  const double factor)
{
    color.red += factor * copy[row][col].red;
    color.green += factor * copy[row][col].green;
    color.blue += factor * copy[row][col].blue;
}


static void clamp(double &num, const int32_t low, const int32_t up)
{
    if (num < low)
        num = low;
    if (up < num)
        num = up;
}

#endif //IMAGE_H
