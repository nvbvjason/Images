#pragma once

#ifndef BMP_METADATA_H
#define BMP_METADATA_H

#include <cstdint>
#include <ostream>

struct FileHeader {
    uint16_t header_field;
    uint32_t size;
    uint16_t idk_1;
    uint16_t idk_2;
    uint32_t offset;
};

struct InfoHeader {
    int32_t size_info_header;
    int32_t width;
    int32_t height;
};

class BmpMetaData {
    static constexpr uint8_t s_file_header_size = 14;
    static constexpr uint8_t s_file_info_size = 40;
    InfoHeader m_file_info;
    const char *m_path;
    bool m_is_top_down;
    FileHeader m_file_header{};

public:
    explicit BmpMetaData(const char *path);

    [[nodiscard]] bool top_down() const { return m_is_top_down; }
    [[nodiscard]] const char *path() const { return m_path; }
    [[nodiscard]] uint32_t offset() const { return m_file_header.offset; }
    [[nodiscard]] int32_t size_header() const { return m_file_info.size_info_header; }
    [[nodiscard]] int32_t width() const { return m_file_info.width; }
    [[nodiscard]] int32_t height() const { return m_file_info.height; }
};

std::ostream &operator<<(std::ostream &os, const BmpMetaData &meta_data);

#endif //BMP_METADATA_H
