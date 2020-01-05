//
// Created by Cosmin on 15/12/2019.
//

#ifndef SODIMAGETRANSFORMER_JOB_H
#define SODIMAGETRANSFORMER_JOB_H

#include "RgbMatrix.h"
#include "sub_job.h"

struct Job {
    const RgbMatrix& Image;
    const Operation& Op;
    RgbMatrix        Output;

    Job(const RgbMatrix& img, const Operation& op) : Image(img), Op(op) {}

    std::vector<MasterSubJob> ComputeJobSplits(int workersCount);
    void JoinSubJobOutputs(const std::vector<MasterSubJob>& jobs);
    MasterSubJob ComputeJobDims(const SubImageDim& toProcess, int lastRow, int lastCol) const;

};

#endif //SODIMAGETRANSFORMER_JOB_H
