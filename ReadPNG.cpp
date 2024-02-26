#include "ReadPNG.h"

#include <cassert>
#include <fstream>
#include <iostream>

const uint8_t ReadPNG::s_signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};

bool read(std::ifstream &ifs, Chunk &chunk)
{
    uint8_t temp[4];
    ifs.read(reinterpret_cast<char *>(temp), 4);
    chunk.length = get_uint32_t_from_data(temp);
    ifs.read(reinterpret_cast<char *>(temp), 4);
    chunk.name = get_chunk_name(temp);
    chunk.data.resize(chunk.length);
    ifs.read(reinterpret_cast<char *>(&chunk.data[0]), chunk.length);
    ifs.read(reinterpret_cast<char *>(chunk.crc), 4);
    return true;
}

IHDR::IHDR()
    : width(0), height(0), bit_depth(0), color_type(0), compression_method(0), filter_method(0), interlace_method(0)
{
}

IHDR::IHDR(const Chunk &chunk)
{
    width = get_int32_t_from_data(chunk.data, 0);
    height = get_int32_t_from_data(chunk.data, 4);
    bit_depth = chunk.data[8];
    color_type = chunk.data[9];
    compression_method = chunk.data[10];
    filter_method = chunk.data[11];
    interlace_method = chunk.data[12];
}

ReadPNG::ReadPNG(const char *path)
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
    Chunk chunk;
    read(ifs, chunk);
    m_chunks.push_back(chunk);
    if (chunk.name == "IDHR" || chunk.length != 13) {
        std::cout << "Faulty starting chunk";
        throw std::exception();
    }
    m_header = IHDR(chunk);
    if (m_header.bit_depth != 8 ||
        m_header.color_type != 6 ||
        m_header.compression_method != 0 ||
        m_header.filter_method != 0 ||
        m_header.interlace_method != 0) {
        std::cout << "unsupported png type";
        throw std::exception();
    }
    while (chunk.name != "IEND") {
        read(ifs, chunk);
        m_chunks.push_back(chunk);
    }

    for (const Chunk& chunk_temp : m_chunks)
        if (chunk_temp.name == "IDAT")
            m_data.insert(m_data.end(), chunk_temp.data.begin(), chunk_temp.data.end());
}

bool ReadPNG::read_signature(std::ifstream &ifs)
{
    constexpr uint8_t length_signature = 8;
    uint8_t signature[length_signature];
    ifs.read(reinterpret_cast<char *>(signature), length_signature);
    for (int32_t i = 0; i < length_signature; ++i)
        if (signature[i] != s_signature[i]) {
            std::cout << "Faulty signature at byte " << i + 1 << '\n';
            return false;
        }
    return true;
}

Chunk::Chunk(): length(0), name{}, crc{} { }

int32_t get_int32_t_from_data(const std::vector<uint8_t> &data, const int32_t start)
{
    assert(start + 4 < data.size() && "faulty length for uint32_t read");
    uint8_t temp[4];
    for (int32_t i = 0; i < 3; ++i)
        temp[i] = data[i + start];
    return temp[3] + (temp[2] << 8) + (temp[1] << 16) + (temp[0] << 24);
}

static uint32_t get_uint32_t_from_data(const uint8_t *data)
{
    return data[3] + (data[2] << 8) + (data[1] << 16) + (data[0] << 24);
}

std::ostream &operator<<(std::ostream &os, const Chunk &chunk)
{
    os << "Length " << chunk.length << '\n';
    os << "Name " << chunk.name << '\n';
    if (chunk.name == "IDHR") {
    }
    return os;
}

std::ostream &operator<<(std::ostream &os, const ReadPNG &read_png)
{
    os << "File name " << read_png.path() << '\n';
    os << "Heigth " << read_png.height() << '\n';
    os << "Width " << read_png.width() << '\n';
    return os;
}

std::ostream & operator<<(std::ostream &os, const IHDR &header)
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

std::string get_chunk_name(const uint8_t *na)
{
    std::string result;
    for (uint8_t i = 0; i < 4; ++i) {
        if (!isalpha(na[i]))
            throw std::exception();
    }
    result += static_cast<char>(na[0]);
    result += static_cast<char>(na[1]);
    result += static_cast<char>(na[2]);
    result += static_cast<char>(na[3]);
    return result;
}