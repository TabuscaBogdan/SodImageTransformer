//
// Created by Cosmin on 11/11/2019.
//

#include "RgbMatrix.h"

void RgbMatrix::SubImageToBuffer(unsigned char* buf, const SubImageDim& dim) const {
    for (int row = dim.MinRow(); row <= dim.MaxRow(); ++row) {
        int rowPartSize = dim.NumCols() * BytesPerElem();
        const rgb32& first = (*this)(row, dim.MinCol());

        memcpy(buf, &first, rowPartSize);
        buf += rowPartSize;
    }
}

void RgbMatrix::FromBuffer(const unsigned char* buf) {
    memcpy(&_data[0], buf, sizeof(_data[0]) * Rows() * Cols());
}
