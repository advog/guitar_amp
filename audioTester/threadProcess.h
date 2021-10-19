#pragma once

#include <Windows.h>

#include "dataBuffer.h"

void loadSine(BYTE* buffer, UINT32 nFrames, UINT32 samplesPerSecond, USHORT channels, UINT32 freq, double* theta) {

    float* castedBuffer = (float*)(buffer);
    DWORD location = 0;
    double increment = (freq * M_PI * 2) / samplesPerSecond;

    for (size_t i = 0; i < nFrames; i++) {
        for (size_t j = 0; j < channels; j++) {
            *(castedBuffer + location) = sin(*theta);
            location++;
        }
        *theta += increment;
    }
}

class threadProcess {
public:

    //dataBuffers
    dataBuffer<float*>* outputBuffer;
    dataBuffer<float*>* inputBuffer;

    //necissary WASAPI buffer info
    UINT32 numChannelsOut;
    UINT32 numChannelsIn;

    //packet out info
    UINT32 packetLengthOut;
    UINT32 framesPerPacketOut;
    UINT32 bytesPerPacketOut;
    
    //packet in info
    UINT32 packetLengthIn;
    UINT32 framesPerPacketIn;
    UINT32 bytesPerPacketIn;

    void setBuffers(dataBuffer <float*>* pIn, dataBuffer<float*>* pOut) {
        outputBuffer = pOut;
        inputBuffer = pIn;
    }

    void setInputPacketLength(UINT32 l, UCHAR c) {
        numChannelsIn = c;
        packetLengthIn = l;
        bytesPerPacketIn = l * sizeof(float);
        framesPerPacketIn = l / numChannelsIn;
    }

    void setOutputPacketLength(UINT32 l, UCHAR c) {
        numChannelsOut = c;
        packetLengthOut = l;
        bytesPerPacketOut = l * sizeof(float);
        framesPerPacketOut = l / numChannelsOut;
    }
	
	void start() {

        //just move packets here
        //placeholder for signal processing
        while (1) {
            float* pInData;
            //= (float*)malloc(bytesPerPacketIn);
            //float* pOutData = (float*)malloc(bytesPerPacketOut);
            while (!inputBuffer->getPacket(&pInData)) {}
            while (!outputBuffer->putPacket(&pInData)) {}
        }
	}
};