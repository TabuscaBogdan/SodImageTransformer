//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_SUB_JOB_H
#define SODIMAGETRANSFORMER_SUB_JOB_H


#include <utility>
#include <algorithm>
#include <variant>
#include "RgbMatrix.h"

/**
 * Example: for a blur operation of ray 2, a possible job dims:
 *   Input:     (2, 2), (8, 9)
 *   ToProcess: (4, 4), (6, 7)
 *   Output:    (4, 4), (6, 7)
 *
 *   The blur operation only processes a part of the matrix, but needs access
 *   to more than that, hence the additional range in the input.
 */
struct JobDims {
    SubImageDim Input;
    SubImageDim ToProcess;
    SubImageDim Output;

    // TODO: Why do we need ToProcess field? It's the same as Output currently, it would only be relevant
    //       if we needed custom shapes to process, in which case we actually need this for both input and output.

    JobDims() = default;
    JobDims(const SubImageDim& in, const SubImageDim& toProcess, const SubImageDim& out)
            : Input(in), ToProcess(toProcess), Output(out) {}
    JobDims(const JobDims&) = default;
    JobDims& operator = (const JobDims&) = default;

    inline std::string ToString() const {
        std::ostringstream ss;
        ss << "["
           << "Input=" << Input.ToString()
           << ", ToProcess=" << ToProcess.ToString()
           << ", Output=" << Output.ToString()
           << "]";
        return ss.str();
    }
};

struct BlurParams {
    int R;
    int Iters;

public:

    BlurParams() : BlurParams(0, 1) {}
    explicit BlurParams(int r, int iters) : R(r), Iters(iters) {}
    BlurParams(const BlurParams&) = default;
    BlurParams& operator=(const BlurParams&) = default;

public:

    inline std::string ToString() const {
        return std::string() + "[" + "R=" + std::to_string(R) + ";" + "Iters=" + std::to_string(Iters) + "]";
    }
};

struct SepiaParams {
    inline std::string ToString() const {
        return "[]";
    }
};

struct Operation {
    enum Type {
        BLUR,
        SEPIA,
        UNKNOWN,
    };

public:
    Operation() : OpType(UNKNOWN), OpParams() {};
    Operation(const Operation&) = default;
    Operation& operator = (const Operation&) = default;

    template <typename PARAMS>
    explicit Operation(const PARAMS& params) {
        Type type = UNKNOWN;

        if (std::is_same_v<PARAMS, BlurParams>) {
            type = BLUR;
        } else if (std::is_same_v<PARAMS, SepiaParams>) {
            type = SEPIA;
        } else {
            assert(false);
            type = UNKNOWN;
        }

        OpType = type;
        OpParams = params;
    }

public:
    Type OpType;
    std::variant<BlurParams, SepiaParams> OpParams;

public:
    inline std::string ToString() const {
        std::ostringstream ss;
        switch (OpType) {
            case BLUR:
                ss << "[Type=BLUR; Params=" << std::get<BlurParams>(OpParams).ToString() << "]";
                break;
            case SEPIA:
                ss << "[Type=SEPIA; Params=" << std::get<SepiaParams>(OpParams).ToString() << "]";
                break;
            default:
                ss << "[Type=UNKNOWN]";
        }
        return ss.str();
    }
};

class Job;

struct MasterSubJob {
    JobDims      Dims;
    const Job&   ImgJob;
    RgbSubMatrix Output;

    MasterSubJob(const JobDims& dims, const Job& job) : Dims(dims), ImgJob(job), Output() {}
    MasterSubJob(const MasterSubJob&) = default;
    MasterSubJob& operator = (const MasterSubJob&) = delete;

    void SendInput(int workerId);
    void RecvOutput(int workerId);
    void ExecuteLocal();
};

struct Header {
    JobDims   Dims;
    Operation Op;

    Header() = default;
    Header(const JobDims& dims, const Operation& op) : Dims(dims), Op(op) {}
    Header(const Header&) = default;
    Header& operator=(const Header&) = default;

    inline std::string ToString() const {
        std::ostringstream ss;
        ss << "[" << "Dims=" << Dims.ToString() << "; " << "Op=" << Op.ToString() << "]";
        return ss.str();
    }
};

struct SlaveSubJob {
    Header       Hdr;
    RgbSubMatrix Input;
    RgbSubMatrix Output;

    void RecvInput(int masterId);
    void SendOutput(int masterId);
    void ExecuteLocal();
};


#endif //SODIMAGETRANSFORMER_SUB_JOB_H
