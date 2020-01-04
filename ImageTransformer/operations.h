//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_OPERATIONS_H
#define SODIMAGETRANSFORMER_OPERATIONS_H

#include "RgbMatrix.h"


int Clamp(int val, int min, int max);
void MakeSepia(RgbMatrix &m);

void MakeBlur(RgbSubMatrix& dst, const RgbSubMatrix& src, int radius = 2);

void MakeSwirl(RgbMatrix& dst, const RgbMatrix& src, double factor);

#endif //SODIMAGETRANSFORMER_OPERATIONS_H
