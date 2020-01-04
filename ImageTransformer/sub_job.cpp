//
// Created by Cosmin on 15/12/2019.
//

#include <memory>

#include "sub_job.h"
#include "job.h"
#include "mpi.h"
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
    UNUSED(workerId);
    printf("RecvOutput not implemented\n");
}

void MasterSubJob::ExecuteLocal() {
    printf("ExecuteLocal not implemented\n");
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

    // Debug
    int id;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    printf("Slave %d received input [(%d, %d), (%d, %d)]\n", id,
            Input.Dim().MinRow(),
            Input.Dim().MinCol(),
            Input.Dim().MaxRow(),
            Input.Dim().MaxCol());
}

void SlaveSubJob::SendOutput(int masterId) {
    UNUSED(masterId);
    printf("SendOutput not implemented\n");
}

void SlaveSubJob::ExecuteLocal() {
    printf("SlaveSubJob::ExecuteLocal not implemented\n");
}
