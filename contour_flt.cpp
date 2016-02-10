#include "contour.h"
#include "PNGImage.h"

#include <fstream>
#include <iostream>

struct HDR {
    int ncols;
    int nrows;
    float xllcorner;
    float yllcorner;
    float cellsize;
    int NODATA_value;
    int NODATA;
    HDR() =default;
    HDR(std::istream &stream) {
        using namespace::std;
        string key;
        while(!stream.eof()) {
            stream>>key;
            if (key == "ncols") {
                stream>>ncols;
            } else if (key == "nrows") {
                stream>>nrows;
            } else if (key == "xllcorner") {
                stream>>xllcorner;
            } else if (key == "yllcorner") {
                stream>>yllcorner;
            } else if (key == "cellsize") {
                stream>>cellsize;
            } else if (key == "NODATA_value") {
                stream>>NODATA_value;
            } else if (key == "NODATA") {
                stream>>NODATA;
            } else if (key == "byteorder") {
                stream>>key;
                if (key != "LSBFIRST") {
                    throw std::runtime_error("LSBFIRST is the only supported byteorder");
                }
            }
        }
    }
};


int main(int argc, char *argv[]) {
    std::cerr<<"Not yet implemented\n";
    return 0;
}
