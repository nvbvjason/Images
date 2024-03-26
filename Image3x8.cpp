#include "Image3x8.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <cassert>
#include <numbers>

Pixel::Pixel()
    : red(0), green(0), blue(0)
{
}

Pixel::Pixel(const uint8_t r, const uint8_t g, const uint8_t b)
    : red(r), green(g), blue(b)
{
}

Pixel::Pixel(const Pixel& other) = default;

Pixel::Pixel(const PixelDouble& other)
{
    double red_ = other.red;
    clamp(red_, 0, 255);
    red_ += 0.5;
    red = static_cast<uint8_t>(red_);

    double green_ = other.green;
    clamp(green_, 0, 255);
    green_ += 0.5;
    green = static_cast<uint8_t>(green_);

    double blue_ = other.blue;
    clamp(blue_, 0, 255);
    blue_ += 0.5;
    blue = static_cast<uint8_t>(blue_);
}

PixelDouble::PixelDouble()
    : red(0), green(0), blue(0)
{
}

PixelDouble::PixelDouble(const double r, const double g, const double b)
    : red(r), green(g), blue(b)
{
}

PixelDouble::PixelDouble(const PixelDouble& other) = default;
PixelDouble::PixelDouble(const Pixel& other)
    : red(other.red), green(other.green), blue(other.blue)
{
}

void PixelDouble::set_all_zero()
{
    red = 0;
    green = 0;
    blue = 0;
}

Image3x8::Image3x8()
    : m_height(0), m_width(0), m_offset(s_file_header_size + s_default_information_header_size), m_pixels(new Pixel[1])
{
}

Image3x8::Image3x8(const std::vector<uint8_t>& data, const int32_t height, const int32_t width)
    : m_height(height), m_width(width), m_offset(s_file_header_size + s_default_information_header_size),
    m_pixels(new Pixel[height * width])
{
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            (*this)[row][col].red = data[(row * m_width + col) * 3 + 0];
            (*this)[row][col].green = data[(row * m_width + col) * 3 + 1];
            (*this)[row][col].blue = data[(row * m_width + col) * 3 + 2];
        }
}

Image3x8::Image3x8(const Image3x8& other)
    : m_height(other.m_height), m_width(other.m_width), m_offset(other.m_offset),
    m_pixels(new Pixel[other.m_height * other.m_width])

{
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col)
            (*this)[row][col] = other[row][col];
}

Image3x8::Image3x8(Image3x8&& other) noexcept
    : m_height(other.m_height), m_width(other.m_width), m_offset(other.m_offset),
    m_pixels(other.m_pixels)
{
    other.m_height = 0;
    other.m_width = 0;
    other.m_offset = 0;
    other.m_pixels = nullptr;
}

Image3x8::Image3x8(const int32_t height, const int32_t width)
    : m_height(height), m_width(width),
    m_offset(s_file_header_size + s_default_information_header_size),
    m_pixels(new Pixel[height * width])
{
}

Image3x8& Image3x8::operator=(const Image3x8& other)
{
    if (this == &other)
        return *this;
    m_height = other.m_height;
    m_width = other.m_width;
    delete[] m_pixels;
    m_pixels = new Pixel[m_height * m_width];
    for (int32_t row = 0; row < other.m_height; ++row)
        for (int32_t col = 0; col < other.m_width; ++col)
            (*this)[row][col] = other[row][col];
    return *this;
}

Image3x8& Image3x8::operator=(Image3x8&& other) noexcept
{
    this->m_height = other.m_height;
    this->m_width = other.m_width;
    this->m_pixels = other.m_pixels;
    this->m_offset = other.m_offset;
    other.m_height = 0;
    other.m_width = 0;
    other.m_offset = 0;
    other.m_pixels = nullptr;
    return *this;
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

void Image3x8::grey_scale()
{
    for (size_t i = 0; i < size(); ++i) {
        double temp = m_pixels[i].red;
        temp += m_pixels[i].green * 0.7152;
        temp += m_pixels[i].blue;
        temp = (temp + 0.5) / 3;
        clamp(temp, 0, 255);
        memset(reinterpret_cast<void*>(&m_pixels[i]), static_cast<uint8_t>(temp), 3);
    }
}

void Image3x8::grey_scale_lum()
{
    for (size_t i = 0; i < size(); ++i) {
        double temp = m_pixels[i].red * 0.2126;
        temp += m_pixels[i].green * 0.7152;
        temp += m_pixels[i].blue * 0.0722;
        temp = (temp + 0.5) / 3;
        clamp(temp, 0, 255);
        memset(reinterpret_cast<void*>(&m_pixels[i]), static_cast<uint8_t>(temp), 3);
    }
}

void Image3x8::color_mask(const double red, const double green, const double blue)
{
    for (size_t i = 0; i < size(); ++i) {
        double temp = m_pixels[i].red * red;
        clamp(temp, 0, 255);
        m_pixels[i].red = static_cast<uint8_t>(temp);
        temp = m_pixels[i].green * green;
        clamp(temp, 0, 255);
        m_pixels[i].green = static_cast<uint8_t>(temp);
        temp = m_pixels[i].blue * blue;
        clamp(temp, 0, 255);
        m_pixels[i].blue = static_cast<uint8_t>(temp);
    }
}

std::vector<Image3x8> Image3x8::interlace() const
{
    std::vector<Image3x8> result(7);
    Image3x8 one((m_height + 7) / 8, (m_width + 7) / 8);
    Image3x8 two((m_height + 7) / 8, (m_width + 7) / 8);
    Image3x8 three((m_height + 3) / 8, (m_width + 3) / 4);
    Image3x8 four((m_height + 3) / 4, (m_width + 1) / 4);
    Image3x8 five((m_height + 1) / 4, (m_width + 1) / 2);
    Image3x8 six((m_height + 1) / 2, m_width / 2);
    Image3x8 seven(m_height / 2, m_width);
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            if (row % 2 == 1)
                seven[row / 2][col] = (*this)[row][col];
            else if (row % 2 == 0 && col % 2 == 1)
                six[row / 2][col / 2] = (*this)[row][col];
            else if (row % 4 == 2 && col % 2 == 0)
                five[row / 4][col / 2] = (*this)[row][col];
            else if (row % 4 == 0 && col % 4 == 2)
                four[row / 4][col / 4] = (*this)[row][col];
            else if (row % 8 == 4 && col % 4 == 0)
                three[row / 8][col / 4] = (*this)[row][col];
            else if (row % 8 == 0 && col % 8 == 4)
                two[row / 8][col / 8] = (*this)[row][col];
            else if (row % 8 == 0 && col % 8 == 0)
                one[row / 8][col / 8] = (*this)[row][col];
        }
    result[0] = one;
    result[1] = two;
    result[2] = three;
    result[3] = four;
    result[4] = five;
    result[5] = six;
    result[6] = seven;
    return result;
}

std::vector<uint8_t> Image3x8::get_data() const
{
    std::vector<uint8_t> result(m_height * m_width * 3);
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            result[(row * m_width + col) * 3 + 0] = (*this)[row][col].red;
            result[(row * m_width + col) * 3 + 1] = (*this)[row][col].green;
            result[(row * m_width + col) * 3 + 2] = (*this)[row][col].blue;
        }
    return result;
}

void Image3x8::sepia()
{
    for (size_t i = 0; i < size(); ++i) {
        const Pixel temp(m_pixels[i]);
        double temp_f = 0.393 * temp.red + 0.769 * temp.green + 0.189 * temp.blue;
        clamp(temp_f, 0, 255);
        m_pixels[i].red = static_cast<uint8_t>(temp_f);
        temp_f = 0.349 * temp.red + 0.686 * temp.green + 0.168 * temp.blue;
        clamp(temp_f, 0, 255);
        m_pixels[i].green = static_cast<uint8_t>(temp_f);
        temp_f = 0.272 * temp.red + 0.534 * temp.green + 0.131 * temp.blue;
        clamp(temp_f, 0, 255);
        m_pixels[i].blue = static_cast<uint8_t>(temp_f);
    }
}

void Image3x8::reflect_horizontal()
{
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col <= m_width / 2; ++col) {
            const Pixel temp((*this)[row][col]);
            (*this)[row][col] = (*this)[row][m_width - col - 1];
            (*this)[row][m_width - col - 1] = temp;
        }
}

void Image3x8::reflect_vertical()
{
    for (int32_t row = 0; row < m_height / 2; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            const Pixel temp((*this)[row][col]);
            (*this)[row][col] = (*this)[m_height - row - 1][col];
            (*this)[m_height - row - 1][col] = temp;
        }
}

void Image3x8::blur()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            const uint8_t counter = kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0 / counter, color);
            color.set_all_zero();
        }
}

void Image3x8::gaussian_blur(const double std_deviation)
{
    const std::vector<double> kernel = make_gauss_kernel(std_deviation);
    const Image3x8 copy = gaussian_copy(get_kernel_distance(kernel));
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col)
            (*this)[row][col] = gaussian_kernel(row, col, copy, kernel);
}

void Image3x8::ridge()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = { 0, -1, 0, -1, 4, -1, 0, -1, 0 };
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0, color);
            color.set_all_zero();
        }
}

void Image3x8::sharpen()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = { 0, -1, 0, -1, 5, -1, 0, -1, 0 };
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0, color);
            color.set_all_zero();
        }
}

void Image3x8::emboss()
{
    const Image3x8 copy(*this);
    PixelDouble color;
    const std::vector<int8_t> kernel = { -2, -1, 0, -1, 1, 1, 0, 1, 2 };
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel, color);
            eval_3x3_0(row, col, 1.0, color);
            color.set_all_zero();
        }
}

void Image3x8::eval_3x3_0(const int32_t row, const int32_t col, const double factor, const PixelDouble& color)

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
    const std::vector<int8_t> kernel_x = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
    const std::vector<int8_t> kernel_y = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col) {
            kernel_3x3_0(row, col, copy, kernel_x, color_x);
            kernel_3x3_0(row, col, copy, kernel_y, color_y);
            evaluate_edges(row, col, color_x, color_y);
            color_x.set_all_zero();
            color_y.set_all_zero();
        }
}

void Image3x8::evaluate_edges(const int32_t row,
    const int32_t col,
    const PixelDouble& color_x,
    const PixelDouble& color_y)
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

Image3x8 Image3x8::gaussian_copy(const int32_t distance)
{
    Image3x8 result(m_height + distance * 2, m_width + distance * 2);
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < m_width; ++col)
            result[row + distance][col + distance] = (*this)[row][col];
    for (int32_t row = 0; row < m_height; ++row)
        for (int32_t col = 0; col < distance; ++col) {
            result[row + distance][col] = (*this)[row][0];
            result[row + distance][col + m_width + distance] = (*this)[row][m_width - 1];
        }
    for (int32_t row = 0; row < distance; ++row) {
        for (int32_t col = 0; col < m_width; ++col) {
            result[row][col + distance] = (*this)[0][col];
            result[row + m_height + distance][col + distance] = (*this)[m_height - 1][col];
        }
    }
    for (int32_t row = 0; row < distance; ++row) {
        for (int32_t col = 0; col < distance; ++col) {
            result[row][col] = (*this)[0][0];
            result[row + m_height + distance][col] = (*this)[m_height - 1][0];
            result[row][col + m_width + distance] = (*this)[0][m_width - 1];
            result[row + m_height + distance][col + m_width + distance] = (*this)[m_height - 1][m_width - 1];
        }
    }
    return result;
}

Pixel Image3x8::gaussian_kernel(int32_t row_img,
                                int32_t col_img,
                                const Image3x8& copy,
                                const std::vector<double>& kernel)
{
    PixelDouble temp;
    int32_t index_kernel = 0;
    const int32_t kernel_distance = get_kernel_distance(kernel);
    row_img += kernel_distance;
    col_img += kernel_distance;
    for (int32_t row = row_img - kernel_distance; row <= row_img + kernel_distance; ++row)
        for (int32_t col = col_img - kernel_distance; col <= col_img + kernel_distance; ++col) {
            temp.red += kernel[index_kernel] * copy[row][col].red;
            temp.green += kernel[index_kernel] * copy[row][col].green;
            temp.blue += kernel[index_kernel] * copy[row][col].blue;
            ++index_kernel;
        }
    return static_cast<Pixel>(temp);
}

uint8_t Image3x8::kernel_3x3_0(const int32_t row_img,
    const int32_t col_img,
    const Image3x8& copy,
    const std::vector<int8_t>& kernel,
    PixelDouble& color) const
{
    uint8_t index = 0;
    uint8_t counter = 0;
    for (int32_t row = row_img - 1; row <= row_img + 1; ++row) {
        for (int32_t col = col_img - 1; col <= col_img + 1; ++col) {
            if (row <= -1 || m_height <= row)
                continue;
            if (col <= -1 || m_width <= col)
                continue;
            ++counter;
            color.red += copy[row][col].red * kernel[index];
            color.green += copy[row][col].green * kernel[index];
            color.blue += copy[row][col].blue * kernel[index];
            ++index;
        }
    }
    return counter;
}

Pixel edges_color_eval(const PixelDouble& color_x, const PixelDouble& color_y)
{
    double red = std::sqrt(color_x.red * color_x.red + color_y.red * color_y.red);
    double green = std::sqrt(color_x.green * color_x.green + color_y.green * color_y.green);
    double blue = std::sqrt(color_x.blue * color_x.blue + color_y.blue * color_y.blue);
    clamp(red, 0, 255);
    clamp(green, 0, 255);
    clamp(blue, 0, 255);
    return { static_cast<uint8_t>(red), static_cast<uint8_t>(green), static_cast<uint8_t>(blue) };
}

static Pixel get_pixel(const std::vector<Image3x8>& images,
    const std::vector<std::vector<int8_t> >& kernel,
    const int32_t height,
    const int32_t width)
{
    const int8_t image = kernel[height % 8][width % 8];
    if (image == 1)
        return images[image - 1][height / 8][width / 8];
    if (image == 2)
        return images[image - 1][height / 8][width / 8];
    if (image == 3)
        return images[image - 1][height / 8][width / 4];
    if (image == 4)
        return images[image - 1][height / 4][width / 4];
    if (image == 5)
        return images[image - 1][height / 4][width / 2];
    if (image == 6)
        return images[image - 1][height / 2][width / 2];
    if (image == 7)
        return images[image - 1][height / 2][width];
    throw std::exception();
}


static std::vector<std::vector<int8_t> > make_interlace_kernel(const int32_t size)
{
    std::vector<std::vector<int8_t> > result(8, std::vector<int8_t>(8));
    if (1 <= size) {
        for (std::vector<int8_t>& row : result)
            for (int8_t& cell : row)
                cell = 1;
    }
    if (2 <= size) {
        for (std::vector<int8_t>& row : result)
            for (int32_t col = 4; col < 8; ++col)
                row[col] = 2;
    }
    if (3 <= size) {
        for (int32_t row = 4; row < 8; ++row)
            for (int32_t col = 0; col < 8; ++col)
                result[row][col] = 3;
    }
    if (4 <= size) {
        for (int32_t row = 0; row < 8; ++row)
            for (int32_t col = 0; col < 8; ++col)
                if (col == 2 || col == 3 || col == 6 || col == 7)
                    result[row][col] = 4;
    }
    if (5 <= size) {
        for (int32_t row = 0; row < 8; ++row)
            for (int32_t col = 0; col < 8; ++col)
                if (row == 2 || row == 3 || row == 6 || row == 7)
                    result[row][col] = 5;
    }
    if (6 <= size) {
        for (int32_t row = 0; row < 8; ++row)
            for (int32_t col = 0; col < 8; ++col)
                if (col % 2 == 1)
                    result[row][col] = 6;
    }
    if (7 <= size) {
        for (int32_t row = 0; row < 8; ++row)
            for (int32_t col = 0; col < 8; ++col)
                if (row % 2 == 1)
                    result[row][col] = 7;
    }
    return result;
}

Image3x8 interlace(const std::vector<Image3x8>& images, const int32_t height, const int32_t width)
{
    if (images.empty() || 8 <= images.size())
        throw std::exception();
    const std::vector<std::vector<int8_t> > interlace_kernel = make_interlace_kernel(images.size());
    assert(images[0].width() == (width + 7) / 8);
    assert(images[0].height() == (height + 7) / 8);
    Image3x8 result(height, width);
    for (int32_t row = 0; row < height; ++row)
        for (int32_t col = 0; col < width; ++col)
            result[row][col] = get_pixel(images, interlace_kernel, row, col);
    return result;
}

size_t index_(const int32_t row, const int32_t col, const int32_t width)
{
    return (row * width + col) * 3 + (row + 1);
}

std::vector<uint8_t> filter(const Image3x8& image, const uint8_t fitler_type)
{
    assert(fitler_type <= 5 && "out of range filter_type");
    const int64_t length_scanline = image.width() * 3 + 1;
    std::vector<uint8_t> result((image.height() * image.width()) * 3 + image.height());
    for (int32_t row = 0; row < image.height(); ++row) {
        result[length_scanline * row] = fitler_type;
        switch (fitler_type) {
        case 0:
            for (int32_t col = 0; col < image.width(); ++col) {
                const size_t index = index_(row, col, image.width());
                const Pixel x = image[row][col];
                result[index + 0] = x.red;
                result[index + 1] = x.green;
                result[index + 2] = x.blue;
            }
            break;
        case 1:
            for (int32_t col = 0; col < image.width(); ++col) {
                const size_t index = index_(row, col, image.width());
                const Pixel x = image[row][col];
                Pixel a;
                if (col != 0)
                    a = image[row][col - 1];
                result[index + 0] = filter_one_two(x.red, a.red);
                result[index + 1] = filter_one_two(x.green, a.green);
                result[index + 2] = filter_one_two(x.blue, a.blue);
            }
            break;
        case 2:
            for (int32_t col = 0; col < image.width(); ++col) {
                const size_t index = index_(row, col, image.width());
                const Pixel x = image[row][col];
                Pixel b;
                if (row != 0)
                    b = image[row - 1][col];
                result[index + 0] = filter_one_two(x.red, b.red);
                result[index + 1] = filter_one_two(x.green, b.green);
                result[index + 2] = filter_one_two(x.blue, b.blue);
            }
            break;
        case 3:
            for (int32_t col = 0; col < image.width(); ++col) {
                const size_t index = index_(row, col, image.width());
                Pixel a, b;
                const Pixel x = image[row][col];
                if (col != 0)
                    a = image[row][col - 1];
                if (row != 0)
                    b = image[row - 1][col];
                result[index + 0] = filter_three(x.red, a.red, b.red);
                result[index + 1] = filter_three(x.green, a.green, b.green);
                result[index + 2] = filter_three(x.blue, a.blue, b.blue);
            }
            break;
        case 4:
            for (int32_t col = 0; col < image.width(); ++col) {
                const size_t index = index_(row, col, image.width());
                Pixel a, b, c;
                const Pixel x = image[row][col];
                if (row != 0)
                    b = image[row - 1][col];
                if (col != 0)
                    a = image[row][col - 1];
                if (row != 0 && col != 0)
                    c = image[row - 1][col - 1];
                result[index + 0] = (x.red - peath(a.red, b.red, c.red)) % 256;
                result[index + 1] = (x.green - peath(a.green, b.green, c.green)) % 256;
                result[index + 2] = (x.blue - peath(a.blue, b.blue, c.blue)) % 256;
            }
            break;
        default:
            throw std::exception();
        }
    }
    return result;
}

uint8_t filter_one_two(const uint8_t x, const uint8_t ab)
{
    return (x - ab) % 256;
}

static uint8_t filter_three(const uint8_t x, const uint8_t a, const uint8_t b)
{
    return (x - (a + b) / 2) % 256;
}

static uint8_t peath(const uint8_t a, const uint8_t b, const uint8_t c)
{
    const int32_t p = a + b + c;
    int32_t pa = std::abs(p - a);
    int32_t pb = std::abs(p - b);
    int32_t pc = std::abs(p - c);
    if (pa <= pb && pa <= pc)
        return a;
    if (pb <= pc)
        return b;
    return c;
}

Image3x8 defilter(const std::vector<uint8_t>& data, const int32_t height, const int32_t width)
{
    Image3x8 result(height, width);
    result[0][0].red = data[1 + 0];
    result[0][0].green = data[1 + 1];
    result[0][0].blue = data[1 + 2];
    for (int32_t row = 0; row < height; ++row) {
        const uint32_t filter_type = data[row * width * 3 + row];
        assert(filter_type <= 5 && "out of range filter_type");
        switch (filter_type) {
        case 0:
            for (int32_t col = 0; col < width; ++col) {
                const size_t index_x = index_(row, col, width);
                result[row][col].red = data[index_x + 0];
                result[row][col].green = data[index_x + 1];
                result[row][col].blue = data[index_x + 2];
            }
            break;
        case 1:
            for (int32_t col = 0; col < width; ++col) {
                const size_t index_x = index_(row, col, width);
                Pixel a;
                if (col != 0)
                    a = result[row][col - 1];
                result[row][col].red = defilter_one_two(data[index_x + 0], a.red);
                result[row][col].green = defilter_one_two(data[index_x + 1], a.green);
                result[row][col].blue = defilter_one_two(data[index_x + 2], a.blue);
            }
            break;
        case 2:
            for (int32_t col = 0; col < width; ++col) {
                const size_t index_x = index_(row, col, width);
                Pixel b;
                if (row != 0)
                    b = result[row - 1][col];
                result[row][col].red = defilter_one_two(data[index_x + 0], b.red);
                result[row][col].green = defilter_one_two(data[index_x + 1], b.green);
                result[row][col].blue = defilter_one_two(data[index_x + 2], b.blue);
            }
            break;
        case 3:
            for (int32_t col = 0; col < width; ++col) {
                const size_t index_x = index_(row, col, width);
                Pixel a, b;
                if (col != 0)
                    a = result[row][col - 1];
                if (row != 0)
                    b = result[row - 1][col];
                result[row][col].red = defilter_three(data[index_x + 0], a.red, b.red);
                result[row][col].green = defilter_three(data[index_x + 1], a.green, b.green);
                result[row][col].blue = defilter_three(data[index_x + 2], a.blue, b.blue);
            }
            break;
        case 4:
            for (int32_t col = 0; col < width; ++col) {
                const size_t index_x = index_(row, col, width);
                Pixel a, b, c;
                if (col != 0)
                    a = result[row][col - 1];
                if (row != 0)
                    b = result[row - 1][col];
                if (row != 0 && col != 0)
                    c = result[row - 1][col - 1];
                result[row][col].red = (data[index_x + 0] + peath(a.red, b.red, c.red)) % 256;
                result[row][col].green = (data[index_x + 1] + peath(a.green, b.green, c.green)) % 256;
                result[row][col].blue = (data[index_x + 2] + peath(a.blue, b.blue, c.blue)) % 256;
            }
            break;
        default:
            throw std::exception();
        }
    }
    return result;
}

static uint8_t defilter_one_two(const uint8_t x, const uint8_t ab)
{
    return (x + ab) % 256;
}

static uint8_t defilter_three(const uint8_t x, const uint8_t a, const uint8_t b)
{
    return (x + (a + b) / 2) % 256;
}

std::vector<double> make_gauss_kernel(const double std_deviation)
{
    const int32_t distance = get_distance(std_deviation);
    std::vector<double> result((distance * 2 + 1) * (distance * 2 + 1));
    for (int32_t x = -distance; x <= distance; ++x)
        for (int32_t y = -distance; y <= distance; ++y)
            result[(x + distance) + (y + distance) * (distance * 2 + 1)] = G(x, y, std_deviation);
    return result;
}

int32_t get_kernel_distance(const std::vector<double>& kernel)
{
    return std::sqrt(kernel.size()) / 2;
}

int32_t get_distance(const double std_deviation)
{
    double sum = 0;
    int32_t distance = 0;
    while (sum <= 0.99) {
        sum = 0;
        for (int32_t x = -distance; x <= distance; ++x)
            for (int32_t y = -distance; y <= distance; ++y)
                sum += G(x, y, std_deviation);
        ++distance;
    }
    return distance;
}

double G(const int32_t x, const int32_t y, const double std_deviatiion)
{
    const double constant = 2.0 * std_deviatiion * std_deviatiion;
    const double high = -(x * x + y * y);
    const double bla = high / constant;
    return std::exp(bla) / constant / std::numbers::pi;
}