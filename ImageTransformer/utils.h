//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_UTILS_H
#define SODIMAGETRANSFORMER_UTILS_H

/**
 *  CeilDiv(6, 2) = 3
 *  CeilDiv(6, 4) = 2
 *  CeilDiv(6, 100) = 1
 */
inline int CeilDiv(int numerator, int denominator) {
    return (numerator + (denominator - 1)) / denominator;
}

#define UNUSED(v) (void)(v);


// Used for simple formatting of paths.
static std::ostringstream _ss;
#define SSTR(x) (_ss.str(""), _ss.clear(), _ss << x, _ss.str())

#endif //SODIMAGETRANSFORMER_UTILS_H
