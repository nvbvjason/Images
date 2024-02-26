#pragma once

#ifndef READ_PNG_H
#define READ_PNG_H

#include <cstdint>
#include <ostream>
#include <vector>

struct Chunk {
    uint32_t length;
    std::string name;
    std::vector<uint8_t> data;
    uint8_t crc[4];
    Chunk();
};

struct IHDR {
    int32_t width;
    int32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;

    IHDR();
    explicit IHDR(const Chunk& chunk);
};

class ReadPNG {
    IHDR m_header;
    //int32_t m_offset;
    static const uint8_t s_signature[8];
    const char* c_path;
    std::vector<Chunk> m_chunks;
    std::vector<uint8_t> m_data;
public:
    explicit ReadPNG(const char *path);

    // ACCESSORS
    [[nodiscard]] const char* path() const {return c_path;}
    [[nodiscard]] int32_t width() const {return m_header.width;}
    [[nodiscard]] int32_t height() const {return  m_header.height;}
private:
    static bool read_signature(std::ifstream& ifs);
};

static int32_t get_int32_t_from_data(const std::vector<uint8_t>& data, int32_t start);
static uint32_t get_uint32_t_from_data(const uint8_t *data);
std::ostream& operator<<(std::ostream& os, const Chunk& chunk);
std::ostream& operator<<(std::ostream& os, const ReadPNG& read_png);
std::ostream& operator<<(std::ostream& os, const IHDR& header);
bool read(const char *path, std::ifstream &ifs, Chunk& chunk);
std::string get_chunk_name(const uint8_t *na);

#endif //READ_PNG_H