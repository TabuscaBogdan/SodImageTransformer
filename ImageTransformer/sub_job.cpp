//
// Created by Cosmin on 15/12/2019.
//

#include <memory>

#include "mpi.h"

#include "operations.h"
#include "sub_job.h"
#include "job.h"
#include "utils.h"

using namespace std;

#define BYTE_MPI_DATA_TYPE MPI_BYTE

void MasterSubJob::SendInput(int workerId) {
    Header header(Dims, ImgJob.Op);

    const int headerSizeBytes = sizeof(header);
    const int payloadSizeBytes = Dims.Input.ElementsCount() * RgbMatrix::BytesPerElem();
    const int msgSizeBytes = headerSizeBytes + payloadSizeBytes;

    auto buf = make_unique<unsigned char[]>(msgSizeBytes);

    unsigned char* headerPtr = buf.get();
    unsigned char* payloadPtr = buf.get() + headerSizeBytes;

    memcpy(headerPtr, (void*)&header, headerSizeBytes);
    ImgJob.Image.SubImageToBuffer(payloadPtr, Dims.Input);

    MPI_Send(buf.get(), msgSizeBytes, BYTE_MPI_DATA_TYPE, workerId, 0 /*tag*/, MPI_COMM_WORLD);
}

void MasterSubJob::RecvOutput(int workerId) {
    const int outputSizeBytes = Dims.Output.ElementsCount() * sizeof(Output(0, 0));

    Output.Resize(Dims.Output);
    MPI_Recv(Output.DataPtr(), outputSizeBytes, BYTE_MPI_DATA_TYPE, workerId, 0 /*tag*/, MPI_COMM_WORLD, nullptr /*status*/);

    printf("Master received output of size %d bytes from worker %d.\n", outputSizeBytes, workerId);
}

void MasterSubJob::ExecuteLocal() {
    Execute(Output, ImgJob.Image, ImgJob.Op, Dims.Output);
    cout << "Master executed sub job locally, output size = " << Output.Dim().ToString() << '\n';
}

void SlaveSubJob::RecvInput(int masterId) {
    MPI_Status status;
    MPI_Probe(masterId, 0 /*tag*/, MPI_COMM_WORLD, &status);

    int msgSizeBytes;
    MPI_Get_count(&status, BYTE_MPI_DATA_TYPE, &msgSizeBytes);

    auto buf = make_unique<unsigned char[]>(msgSizeBytes);

    MPI_Recv(buf.get(), msgSizeBytes, BYTE_MPI_DATA_TYPE, masterId, 0 /*tag*/, MPI_COMM_WORLD, &status);

    const int headerSizeBytes = sizeof(Hdr);
    const int payloadSizeBytes = msgSizeBytes - headerSizeBytes;
    const unsigned char* headerPtr = buf.get();
    const unsigned char* payloadPtr = buf.get() + headerSizeBytes;

    memcpy((void*)&Hdr, headerPtr, headerSizeBytes);

    assert(payloadSizeBytes == Hdr.Dims.Input.ElementsCount() * RgbMatrix::BytesPerElem());

    Input.FromBuffer(payloadPtr, Hdr.Dims.Input);

#ifndef NDEBUG
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    printf("Slave %d received input [(%d, %d), (%d, %d)]\n", id,
            Input.MinRow(),
            Input.MinCol(),
            Input.MaxRow(),
            Input.MaxCol());
#endif
}

void SlaveSubJob::SendOutput(int masterId) {
    const int outputSizeBytes = Output.Dim().ElementsCount() * sizeof(Output(0, 0));

    MPI_Send(
            Output.DataPtr(),
            outputSizeBytes,
            MPI_BYTE, masterId,
            0 /*tag*/,
            MPI_COMM_WORLD);

#ifndef NDEBUG
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    printf("Slave %d sent output of size %d bytes.\n", id, outputSizeBytes);
#endif
}

void SlaveSubJob::ExecuteLocal() {
    Execute(Output, Input, Hdr.Op, Hdr.Dims.Output);
}
