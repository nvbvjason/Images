#include "ReadPNG.h"

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>

const uint8_t ReadPNG::s_signature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

IHDR::IHDR()
    : width(0), height(0), bit_depth(0), color_type(0), compression_method(0)
    , filter_method(0), interlace_method(0) { }

IHDR::IHDR(const Chunk& chunk)
{
    width = get_uint32_t_from_data(chunk.data, 0);
    height = get_uint32_t_from_data(chunk.data, 4);
    bit_depth = chunk.data[8];
    color_type = chunk.data[9];
    compression_method = chunk.data[10];
    filter_method = chunk.data[11];
    interlace_method = chunk.data[12];
}

Chunk::Chunk() : length(0), name{}, crc{} { }

bool Chunk::check_crc(const uint32_t calculated_crc) const
{
    uint32_t crc_file = (crc[0] << 24) + (crc[1] << 16) + (crc[2] << 8) + crc[3];
    return crc_file == calculated_crc;
}

void Chunk::create_crc_table()
{
    for (int32_t n = 0; n < 256; n++) {
        auto c = static_cast<uint32_t>(n);
        for (int32_t k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
}

unsigned long Chunk::update_crc(const uint32_t crc, const std::vector<uint8_t>& buffer)
{
    unsigned long c = crc;
    for (const uint8_t buf : buffer)
        c = crc_table[(c ^ buf) & 0xff] ^ (c >> 8);
    return c;
}

unsigned long Chunk::get_crc(const std::vector<uint8_t>& buffer)
{
    return update_crc(0xffffffffL, buffer) ^ 0xffffffffL;
}

ReadPNG::ReadPNG(const char* path)
{
    std::ifstream ifs;
    ifs.open(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "File could not be opened " << path << '\n';
        throw std::exception();
    }
    this->c_path = path;
    if (!read_signature(ifs))
        throw std::exception();
    Chunk::create_crc_table();
    std::vector<uint8_t> data;
    Chunk chunk;
    read_chunk(ifs, chunk);
    m_chunks.push_back(chunk);
    if (chunk.name != "IHDR" || chunk.length != 13) {
        std::cout << "Faulty starting chunk";
        throw std::exception();
    }
    m_header = IHDR(chunk);
    while (chunk.name != "IEND") {
        read_chunk(ifs, chunk);
        m_chunks.push_back(chunk);
    }
    std::vector<uint8_t> compressed_data = get_idat_data();
    std::cout << "done";
}

bool ReadPNG::read_signature(std::ifstream& ifs)
{
    constexpr uint8_t length_signature = 8;
    uint8_t signature[length_signature];
    ifs.read(reinterpret_cast<char*>(signature), length_signature);
    for (int32_t i = 0; i < length_signature; ++i)
        if (signature[i] != s_signature[i]) {
            std::cout << "Faulty signature at byte " << i + 1 << '\n';
            return false;
        }
    return true;
}

bool ReadPNG::read_chunk(std::ifstream& ifs, Chunk& chunk)
{
    std::vector<uint8_t> temp(4);
    ifs.read(reinterpret_cast<char*>(&temp[0]), 4);
    chunk.length = get_uint32_t_from_data(temp, 0);
    std::vector<uint8_t> buffer(chunk.length + 4);
    ifs.read(reinterpret_cast<char*>(&buffer[0]), chunk.length + 4);
    ifs.read(reinterpret_cast<char*>(&chunk.crc[0]), 4);
    if (!chunk.check_crc(Chunk::get_crc(buffer))) {
        std::cout << "Faulty crc code";
        throw std::exception();
    }
    for (int32_t i = 0; i < 4; ++i)
        temp[i] = buffer[i];
    chunk.name = get_chunk_name(temp);
    chunk.data.resize(chunk.length);
    for (int32_t i = 0; i < chunk.length; ++i)
        chunk.data[i] = buffer[i + 4];
    return true;
}

int32_t ReadPNG::get_idat_data_length()
{
    int32_t length = 0;
    for (const Chunk& chunk : m_chunks)
        if (chunk.name == "IDAT")
            length += chunk.length;
    return length;
}

std::vector<uint8_t> ReadPNG::get_idat_data()
{
    std::vector<uint8_t> result(get_idat_data_length());
    int32_t index = 0;
    for (const Chunk& chunk : m_chunks)
        if (chunk.name == "IDAT")
            for (const uint8_t data : chunk.data) {
                result[index] = data;
                ++index;
            }
    return result;
}

std::vector<uint8_t> ReadPNG::deflate(const std::vector<uint8_t>& compressed_data) const
{
    std::vector<uint8_t> picture(m_header.height * m_header.width * 3 + m_header.height);

    return picture;
}

static uint32_t get_uint32_t_from_data(const std::vector<uint8_t>& data, int32_t posistion)
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < 4; ++i)
        result += data[i + posistion] * std::pow(256, 3 - i);
    return result;
}

std::ostream& operator<<(std::ostream& os, const Chunk& chunk)
{
    os << "Length " << chunk.length << '\n';
    os << "Name " << chunk.name << '\n';
    if (chunk.name == "IDHR") {
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ReadPNG& read_png)
{
    os << "File name " << read_png.path() << '\n';
    os << "Heigth " << read_png.height() << '\n';
    os << "Width " << read_png.width() << '\n';
    return os;
}

std::ostream& operator<<(std::ostream& os, const IHDR& header)
{
    os << "Height " << header.height << '\n';
    os << "Width " << header.width << '\n';
    os << "Bit depth " << header.bit_depth << '\n';
    os << "Color type " << header.color_type << '\n';
    os << "Compression method " << header.compression_method << '\n';
    os << "Filter method " << header.filter_method << '\n';
    os << "Interlace method " << header.interlace_method << '\n';
    return os;
}

std::string get_chunk_name(const std::vector<uint8_t>& data)
{
    std::string result;
    for (uint8_t i = 0; i < 4; ++i) {
        if (!isalpha(data[i]))
            throw std::exception();
    }
    result += static_cast<char>(data[0]);
    result += static_cast<char>(data[1]);
    result += static_cast<char>(data[2]);
    result += static_cast<char>(data[3]);
    return result;
}