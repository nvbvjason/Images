#include "BmpMetaData.h"

#include <fstream>
#include <iostream>
#include <ostream>

BmpMetaData::BmpMetaData(const char *path)
    :m_path(path)
{
    std::ifstream ifs;
    ifs.open(path, std::ios::in | std::ios::binary);
    if (!ifs.is_open()) {
        std::cout << "File could not be opened\n";
        return;
    }
    uint8_t file_header[s_file_header_size];
    ifs.read(reinterpret_cast<char *>(file_header), s_file_header_size);
    if (file_header[0] != 'B' || file_header[1] != 'M') {
        std::cout << "The specified path is not a bitmap image\n";
        ifs.close();
        return;
    }
    m_file_header.header_field = file_header[0] + (file_header[1] << 8);
    m_file_header.size = file_header[2] + (file_header[3] << 8)
                         + (file_header[4] << 16) + (file_header[5] << 24);
    m_file_header.idk_1 = 0;
    m_file_header.idk_2 = 0;
    m_file_header.offset = file_header[10] + (file_header[11] << 8) +
                           +(file_header[12] << 16) + (file_header[13] << 24);
    if (m_file_header.offset < 54) {
        std::cout << "Cannot read this header file format.\n";
        ifs.close();
        return;
    }
    uint8_t file_info[s_file_info_size];
    ifs.read(reinterpret_cast<char *>(file_info), s_file_info_size);
    m_file_info.size_info_header = file_info[0] + (file_info[1] << 8) + (file_info[2] << 16) + (file_info[3] << 24);
    m_file_info.width = file_info[4] + (file_info[5] << 8) + (file_info[6] << 16) + (file_info[7] << 24);
    m_file_info.height = file_info[8] + (file_info[9] << 8) + (file_info[10] << 16) + (file_info[11] << 24);
    m_is_top_down = false;
    if (m_file_info.height < 0) {
        m_is_top_down = true;
        m_file_info.height = -m_file_info.height;
    }
}

std::ostream &operator<<(std::ostream &os, const BmpMetaData &meta_data)
{
    os << "Size " << meta_data.m_file_header.size << '\n';
    os << "offset " << meta_data.m_file_header.offset << '\n';
    os << "Size info header " << meta_data.m_file_info.size_info_header << '\n';
    os << "Width " << meta_data.m_file_info.width << '\n';
    os << "Height " << meta_data.m_file_info.height << '\n';
    return os;
}