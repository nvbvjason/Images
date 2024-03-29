#pragma once

#ifndef IMAGE_H
#define IMAGE_H

#include <cstdint>
#include <cstdio>
#include <vector>

struct PixelDouble;

struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Pixel();
    Pixel(uint8_t r, uint8_t g, uint8_t b);
    Pixel(const Pixel& other);
    explicit Pixel(const PixelDouble& other);
};

struct PixelDouble {
    double red;
    double green;
    double blue;

    PixelDouble();
    PixelDouble(double r, double g, double b);
    PixelDouble(const PixelDouble& other);
    explicit PixelDouble(const Pixel& other);
    void set_all_zero();
};

class Image3x8 {
    int32_t m_height;
    int32_t m_width;
    uint32_t m_offset;
    Pixel* m_pixels;
    static constexpr uint8_t s_file_header_size = 14;
    static constexpr uint8_t s_default_information_header_size = 40;

public:
    // CREATORS
    Image3x8();
    Image3x8(const std::vector<uint8_t>& data, int32_t height, int32_t width);
    Image3x8(const Image3x8& other);
    Image3x8(Image3x8&& other) noexcept;
    Image3x8(int32_t height, int32_t width);
    ~Image3x8() { delete[] m_pixels; }

    // MANIPULATORS
    Image3x8& operator=(const Image3x8& other);
    Image3x8& operator=(Image3x8&& other) noexcept;
    void black_out_part(int32_t start_row, int32_t end_row, int32_t start_col, int32_t end_col);
    Pixel* operator[](const int32_t row) { return m_pixels + m_width * row; }
    void grey_scale();
    void grey_scale_lum();
    void sepia();
    void reflect_horizontal();
    void reflect_vertical();
    void blur();
    void gaussian_blur(double std_deviation);
    void ridge();
    void sharpen();
    void emboss();
    void edges();
    void color_mask(double red, double green, double blue);

    // ACCESSORS
    [[nodiscard]] std::vector<Image3x8> interlace() const;
    [[nodiscard]] std::vector<uint8_t> get_data() const;
    [[nodiscard]] int32_t height() const { return m_height; }
    [[nodiscard]] int32_t width() const { return m_width; }
    [[nodiscard]] uint32_t offset() const { return m_offset; }
    [[nodiscard]] size_t size() const { return m_height * m_width; }
    [[nodiscard]] const Pixel* operator[](const int32_t row) const { return m_pixels + m_width * row; }
private:
    uint8_t kernel_3x3_0(int32_t row_img,
        int32_t col_img,
        const Image3x8& copy,
        const std::vector<int8_t>& kernel,
        PixelDouble& color) const;
    Image3x8 gaussian_copy(int32_t distance);
    static Pixel gaussian_kernel(int32_t row_img,
        int32_t col_img,
        const Image3x8& copy,
        const std::vector<double>& kernel);
    void eval_3x3_0(int32_t row, int32_t col, double factor, const PixelDouble& color);
    void evaluate_edges(int32_t row, int32_t col, const PixelDouble& color_x, const PixelDouble& color_y);
    static inline void edges_pixel_helper(int32_t row, int32_t col, const Image3x8& copy, PixelDouble& color,
        double factor);
};

static Pixel edges_color_eval(const PixelDouble& color_x, const PixelDouble& color_y);
std::vector<double> make_gauss_kernel(double std_deviation);
int32_t get_kernel_distance(const std::vector<double>& kernel);
int32_t get_distance(double std_deviation);
double G(int32_t x, int32_t y, double std_deviatiion);
static Pixel get_pixel(const std::vector<Image3x8>& images,
    const std::vector<std::vector<int8_t> >& kernel,
    int32_t height,
    int32_t width);

void Image3x8::edges_pixel_helper(const int32_t row, const int32_t col, const Image3x8& copy, PixelDouble& color,
    const double factor)
{
    color.red += factor * copy[row][col].red;
    color.green += factor * copy[row][col].green;
    color.blue += factor * copy[row][col].blue;
}

static void clamp(double& num, const int32_t low, const int32_t up)
{
    if (num < low)
        num = low;
    if (up < num)
        num = up;
}


#endif //IMAGE_H