#pragma once

#ifndef READ_PNG_H
#define READ_PNG_H

#include <cstdint>
#include <ostream>
#include <vector>

static unsigned long crc_table[256];

struct Chunk {
    uint64_t length;
    std::string name;
    std::vector<uint8_t> data;
    uint8_t crc[4];
    Chunk();
    bool check_crc(uint32_t calculated_crc) const;
    static unsigned long update_crc(uint32_t crc, const std::vector<uint8_t>& buffer);
    static unsigned long get_crc(const std::vector<uint8_t>& buffer);
    static void create_crc_table();
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
    static const uint8_t s_signature[8];
    const char* c_path;
    std::vector<Chunk> m_chunks;
public:
    explicit ReadPNG(const char* path);

    // ACCESSORS
    [[nodiscard]] const char* path() const { return c_path; }
    [[nodiscard]] int32_t width() const { return m_header.width; }
    [[nodiscard]] int32_t height() const { return  m_header.height; }
private:
    static bool read_signature(std::ifstream& ifs);
    std::vector<uint8_t> get_idat_data();
    int32_t get_idat_data_length();
    static bool read_chunk(std::ifstream& ifs, Chunk& chunk);
    [[nodiscard]] std::vector<uint8_t> deflate(const std::vector<uint8_t>& compressed_data) const;
};

static uint32_t get_uint32_t_from_data(const std::vector<uint8_t>& data, int32_t posistion);
std::ostream& operator<<(std::ostream& os, const Chunk& chunk);
std::ostream& operator<<(std::ostream& os, const ReadPNG& read_png);
std::ostream& operator<<(std::ostream& os, const IHDR& header);
std::string get_chunk_name(const std::vector<uint8_t>& data);

#endif //READ_PNG_H