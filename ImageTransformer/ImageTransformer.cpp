#include <iostream>
#include "bitmap.h"
#include "RgbMatrix.h"

void MakeSepia(RgbMatrix &m) {
    for (int i = 0; i < m.Rows(); ++i) {
        for (int j = 0; j < m.Cols(); ++j) {
            float r = (m(i, j).r * 0.393f) + (m(i, j).g * 0.769f) + (m(i, j).b * 0.189f);
            float g = (m(i, j).r * 0.349f) + (m(i, j).g * 0.686f) + (m(i, j).b * 0.168f);
            float b = (m(i, j).r * 0.272f) + (m(i, j).g * 0.534f) + (m(i, j).b * 0.131f);

            m(i, j).r = uint8_t(std::min(r, 255.0f));
            m(i, j).g = uint8_t(std::min(g, 255.0f));
            m(i, j).b = uint8_t(std::min(b, 255.0f));
        }
    }
}

int main() {
    bitmap bmp{"../data/sunflower.bmp"};

    RgbMatrix m{bmp};
    MakeSepia(m);

    m.ToBitmap(&bmp);

    bmp.save("../data/sunflower_sepia.bmp");
}
