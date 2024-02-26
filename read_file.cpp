#include "read_file.h"

#include <cstdint>
#include <fstream>
#include <iostream>

void show_first_54_bytes(const char *path)
{
    const uint8_t len = 54;
    std::ifstream ifs;
    ifs.open(path);
    if (!ifs.is_open()) {
        std::cout << "File could not be opened\n";
        return;
    }
    uint8_t head[len];
    ifs.read(reinterpret_cast<char*>(head), len);
    for (int32_t row = 0; row < 6; ++row) {
        for (int32_t col = 0; col < 10; ++col) {
            if (row == 5 && col == 4)
                break;
            const uint8_t temp = head[row * 10 + col];
            std::cout << std::hex << temp / 16 << temp % 16 << '\t';
        }
        std::cout << '\n';
    }
    std::cout << std::dec << '\n';
    for (int32_t row = 0; row < 6; ++row) {
        for (int32_t col = 0; col < 10; ++col) {
            if (row == 5 && col == 4)
                break;
            uint8_t temp = head[row * 10 + col];
            std::cout << temp * 1 <<'\t';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    for (int32_t row = 0; row < 6; ++row) {
        for (int32_t col = 0; col < 10; ++col) {
            if (row == 5 && col == 4)
                break;
            const uint8_t temp = head[row * 10 + col];
            std::cout << temp <<'\t';
        }
        std::cout << '\n';
    }
}
