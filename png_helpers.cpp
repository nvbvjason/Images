#include "png_helpers.h"

#include <cassert>

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