//
// Created by Cosmin on 04/01/2020.
//

#ifndef SODIMAGETRANSFORMER_SUB_IMAGE_DIM_H
#define SODIMAGETRANSFORMER_SUB_IMAGE_DIM_H

#include <sstream>

struct SubImageDim {
    std::pair<int, int> Min;
    std::pair<int, int> Max;

    SubImageDim() = default;
    SubImageDim(int minRow, int minCol, int maxRow, int maxCol) : Min(minRow, minCol), Max(maxRow, maxCol) {}
    SubImageDim(const SubImageDim&) = default;
    SubImageDim& operator = (const SubImageDim&) = default;

    inline int MinRow()  const { return Min.first; }
    inline int MinCol()  const { return Min.second; }
    inline int MaxRow()  const { return Max.first; }
    inline int MaxCol()  const { return Max.second; }
    inline int NumRows() const { return MaxRow() - MinRow() + 1; }
    inline int NumCols() const { return MaxCol() - MinCol() + 1; }

    inline void Expand(int delta, int lastRow, int lastCol) {
        Min.first  = std::max(Min.first  - delta, 0);
        Min.second = std::max(Min.second - delta, 0);
        Max.first  = std::min(Max.first  + delta, lastRow);
        Max.second = std::min(Max.second + delta, lastCol);
    }

    inline int ElementsCount() const {
        return NumRows() * NumCols();
    }

    inline std::string ToString() const {
        std::ostringstream ss;
        ss << "[(" << Min.first << "," << Min.second << "), (" << Max.first << "," << Max.second << ")]";
        return ss.str();
    }
};

#endif //SODIMAGETRANSFORMER_SUB_IMAGE_DIM_H
