#pragma once

#include <Windows.h>

#include "dataBuffer.h"

HRESULT loadSine(BYTE* buffer, UINT32 nFrames, UINT32 samplesPerSecond, USHORT channels, UINT32 freq, double* theta) {

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
    return 0;
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

    HRESULT setBuffers(dataBuffer <float*>* pIn, dataBuffer<float*>* pOut) {
        outputBuffer = pOut;
        inputBuffer = pIn;
        return 0;
    }

    HRESULT setInputPacketLength(UINT32 l, UCHAR c) {
        numChannelsIn = c;
        packetLengthIn = l;
        bytesPerPacketIn = l * sizeof(float);
        framesPerPacketIn = l / numChannelsIn;
        return 0;
    }

    HRESULT setOutputPacketLength(UINT32 l, UCHAR c) {
        numChannelsOut = c;
        packetLengthOut = l;
        bytesPerPacketOut = l * sizeof(float);
        framesPerPacketOut = l / numChannelsOut;
        return 0;
    }
	
	HRESULT start() {

        //just move packets here
        while (1) {
            float* pInData = (float*)malloc(bytesPerPacketIn);
            //float* pOutData = (float*)malloc(bytesPerPacketOut);
            while (!inputBuffer->getPacket(&pInData)) {
                Sleep(10);
            }

            //for (size_t i = 0; i < ; i++)
            //{
            //
            //}

            while (!outputBuffer->putPacket(&pInData)) {
                Sleep(10);
            }
            Sleep(10);
        }
        
        return 0;
	}

};