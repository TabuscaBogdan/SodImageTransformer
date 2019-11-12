//
// Created by Cosmin on 11/11/2019.
//

#ifndef SODIMAGETRANSFORMER_RGBMATRIX_H
#define SODIMAGETRANSFORMER_RGBMATRIX_H
#pragma once

#include <vector>
#include <cstring>
#include <cassert>

#include "bitmap.h"

class RgbMatrix {
public:
    RgbMatrix() : RgbMatrix(0, 0) {}
    RgbMatrix(int rows, int cols) {
        Resize(rows, cols);
    }
    explicit RgbMatrix(const bitmap& bm) {
        Resize(bm.getHeight(), bm.getWidth());

        for (int row = 0; row < _rows; ++row) {
            for (int col = 0; col < _cols; ++col) {
                rgb32* pixel = bm.getPixel(row, col);
                (*this)(row, col) = *pixel;
            }
        }
    }
    RgbMatrix(const RgbMatrix&) = default;
    RgbMatrix& operator= (const RgbMatrix&) = default;

    void ToBitmap(bitmap* bm) const {
        assert(bm->getWidth() == _rows);
        assert(bm->getHeight() == _cols);

        for (int row = 0; row < _rows; ++row) {
            for (int col = 0; col < _cols; ++col) {
                auto pixel = (*this)(row, col);
                bm->setPixel(&pixel, row, col);
            }
        }
    }

    int Rows() const {
        return _rows;
    }

    int Cols() const {
        return _cols;
    }

    void Resize(int rows, int cols) {
        _rows = rows;
        _cols = cols;
        _data.resize(rows * cols);
    }

    void Zero() {
        rgb32 zeroValue = rgb32();
        std::fill(_data.begin(), _data.end(), zeroValue);
    }

public:
    const rgb32& operator()(int i, int j) const {
        assert(0 <= i && i < _rows);
        assert(0 <= j && j < _cols);

        auto index = i * _cols + j;
        assert(0 <= index && index < _data.size());

        return _data[index];
}

    rgb32& operator()(int i, int j) {
        assert(0 <= i && i < _rows);
        assert(0 <= j && j < _cols);

        auto index = i * _cols + j;
        assert(0 <= index && index < _data.size());

        return _data[index];
    }

private:
    std::vector<rgb32> _data;
    int _rows;
    int _cols;
};


#endif //SODIMAGETRANSFORMER_RGBMATRIX_H
