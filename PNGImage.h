#ifndef PNG_H_
#define PNG_H_

#include <string>
#include <iostream>
#include <cstdint>
#include <vector>

class PNGImage {
public:
    int width, height, bpp;
    std::vector<uint8_t> pixels, palette;

    PNGImage(std::istream &input);
    virtual ~PNGImage();
    operator uint8_t*();
    operator uint32_t*();
};

#endif /* PNG_H_ */
