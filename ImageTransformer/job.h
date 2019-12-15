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

    Job(const RgbMatrix& img, const Operation& op) : Image(img), Op(op) {}

    std::vector<MasterSubJob> ComputeJobSplits(int workersCount);
    void JoinResults(const std::vector<MasterSubJob>& jobs);
    JobDims ComputeJobDims(const SubImageDim& toProcess, int lastRow, int lastCol) const;

};

#endif //SODIMAGETRANSFORMER_JOB_H