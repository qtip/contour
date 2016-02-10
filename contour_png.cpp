#include "contour.h"
#include "PNGImage.h"

#include <fstream>

int main(int argc, char *argv[]) {
    using namespace std;
    if (argc != 4) {
        cerr<<"usage: "<<argv[0]<<" FILE STEP OFFSET\n";
        return 1;
    }
    float step = atof(argv[2]);
    float offset = atof(argv[3]);
    cerr<<step<<", "<<offset<<"\n";
    ifstream png_stream(argv[1], ios::binary);
    PNGImage png(png_stream);

    cout<<"<svg height=\""<<png.height<<"\" width=\""<<png.width<<"\" xmlns=\"http://www.w3.org/2000/svg\">";
    cout<<"<style type=\"text/css\"><![CDATA[\n"
                        "   rect {\n"
                        "       fill: #fefdd2;\n"
                        "   }\n"
                        "   path {\n"
                        "       stroke: rgb(255,179,128);\n"
                        "       stroke-width: 2.0;\n"
                        "       fill: none;\n"
                        "   }\n"
                        "   ]]></style>\n";
    cout<<"<rect height=\""<<png.height<<"\" width=\""<<png.width<<"\" />";

    auto result = generate(static_cast<uint8_t*>(png), png.width, png.height, step, offset);

    for (auto &pair : result) {
        pair.second.write_out(cout);
        cout<<"\n";
    }
    cout<<"</svg>\n";

    return 0;
}
