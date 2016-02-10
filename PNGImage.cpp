#include "PNGImage.h"
#include <png.h>
#include <cstdio>
using std::FILE;
using std::fopen;
using std::fclose;
#include <string>
using std::string;
#include <memory>
using std::unique_ptr;
#include <sstream>
using std::stringstream;
#include <stdexcept>
using std::runtime_error;

static void read_data_istream(png_structp png_ptr, png_bytep data, png_size_t length) {
    png_voidp io_ptr = png_get_io_ptr(png_ptr);
    std::istream *stream = static_cast<std::istream*>(io_ptr);
    stream->read(reinterpret_cast<char *>(data), length);
}

PNGImage::PNGImage(std::istream &input) {
    int ctype;

    // Represents the png itself. Gets passed to pretty much every libpng function.
    // A pointer since it will be allocated by the library code.
    png_structp png = NULL;

    // Holds information about the png such as dimensions, etc.
    // A pointer since it will be allocated by the library code.
    png_infop info = NULL;

    // For some reason libpng wants another one referred to as "end info".
    // A pointer since it will be allocated by the library code.
    png_infop endinfo = NULL;

    // Storage for the first eight bytes of the file with which to check the png signature.
    png_byte sig[8];

    // Read the first eight bytes of the file to check its info.
    input.read(reinterpret_cast<char *>(sig), 8);

    // Check to see if it's a good png.
    // int png_sig_cmp (png_bytep sig, png_size_t start, png_size_t num_to_check);
    if (png_sig_cmp(sig, 0, 8)) {
        // Not a good png, the signature is messed up or it's not really a png.
        stringstream what;
        what<<"Bad PNG";
        throw runtime_error(what.str());
    }

    // Initialize the png struct for reading from files. This allocates memory.
    // png_structp png_create_read_struct (png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn);
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png) {
        // For some reason the png_struct wasn't created.
        stringstream what;
        what<<"Couldn't create png_struct";
        throw runtime_error(what.str());
    }

    // Initialize the info struct. This allocates memory.
    // png_infop png_create_info_struct (png_structp png_ptr);
    info = png_create_info_struct(png);

    if (!info) {
        // For some reason the info_struct wasn't created.
        stringstream what;
        what<<"Couldn't create info_struct";
        throw runtime_error(what.str());
    }

    // Initialize the info struct again? The manual does it in the example code
    // and doesn't really give any reasons.
    // png_infop png_create_info_struct (png_structp png_ptr);
    endinfo = png_create_info_struct(png);

    if (!endinfo) {
        // For some reason the (end) info_struct wasn't created.
        stringstream what;
        what<<"Couldn't create (end) info_struct";
        throw runtime_error(what.str());
    }

    // Handle interal png errors.
    // int setjmp(jmp_buf env ); from C standard
    // jmp_buf png_jmpbuf(png_structp png);
    if (setjmp(png_jmpbuf(png))) {
        // Using ancient crappy C stuff, this code below will run if libpng has an error.
        // Essentially it's like the catch part of a try-catch.
        stringstream what;
        what<<"Internal libpng error";
        throw runtime_error(what.str());
    }

    // Set the read function to use the istream
    png_set_read_fn(png, static_cast<png_voidp>(&input), read_data_istream);

    // Tell the library that I already read eight bytes from the file and that
    // the pointer has moved.
    // void png_set_sig_bytes (png_structp png_ptr, int num_bytes);
    png_set_sig_bytes(png, 8);

    // Actually reads all of the file.
    // void png_read_png (png_structp png_ptr, png_infop info_ptr, int transforms, png_voidp params);
    png_read_png(png, info, 0, NULL);

    // Pull information from the png.
    //png_uint_32 png_get_image_width (png_structp png_ptr, png_infop info_ptr);
    //png_uint_32 png_get_image_height (png_structp png_ptr, png_infop info_ptr);
    //png_byte png_get_bit_depth (png_structp png_ptr, png_infop info_ptr);
    //png_byte png_get_color_type (png_structp png_ptr, png_infop info_ptr);
    //types can be:
    //PNG_COLOR_TYPE_GRAY, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB, PNG_COLOR_TYPE_RGB_ALPHA , or PNG_COLOR_TYPE_GRAY_ALPHA
    this->width = (int) png_get_image_width(png, info);
    this->height = (int) png_get_image_height(png, info);
    this->bpp = (int) png_get_bit_depth(png, info);
    ctype = (int) png_get_color_type(png, info);
    if (ctype == PNG_COLOR_TYPE_PALETTE) {
        // Don't handle images with a color palette.
        stringstream what;
        what<<"Can't handle images with a color palette. Change the color mode of the image to RGB.";
        throw runtime_error(what.str());
    }
    if (ctype == PNG_COLOR_TYPE_RGB)
        this->bpp *= 3;
    else if (ctype == PNG_COLOR_TYPE_RGB_ALPHA)
        this->bpp *= 4;

    // Allocate memory for the pixel data.
    this->pixels.reserve(png_get_rowbytes(png, info) * this->height);
    // Find the pointer to the pixel rows.
    // png_bytepp png_get_rows (png_structp png_ptr, png_infop info_ptr);
    uint8_t** tempi = png_get_rows(png, info);

    unsigned int row_size_bytes = png_get_rowbytes(png, info);
    // Copy the pixels into our own array.
    for (int y = 0; y < this->height; y++) {
        this->pixels.insert(pixels.end(), tempi[y], tempi[y]+row_size_bytes);
    }

    // Clean up.
    png_destroy_read_struct(&png, &info, &endinfo);

}

PNGImage::~PNGImage() {
}

PNGImage::operator uint8_t*() {
    return this->pixels.data();
}

PNGImage::operator uint32_t*() {
    return (uint32_t*)this->pixels.data();
}


