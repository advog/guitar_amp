#pragma once

#include <Windows.h>
#include <iostream>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <mmdeviceapi.h>
#include <system_error>

#include "dataBuffer.h"

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000
#define M_PI 3.14159265358979323846

void checkR(HRESULT h) {
    if (FAILED(h)) {
        std::cout << std::system_category().message(h);
    }
}

class threadRead {
public:

    //input queue
    dataBuffer<float*>* outputBuffer;

    //utility
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_MILLISEC * 0.5;
    DWORD flags = 0;

    //wasapi pointers
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioCaptureClient* pCaptureClient = NULL;

    //format pointers
    WAVEFORMATEXTENSIBLE* pFormatExtensible = NULL;
    WAVEFORMATEX* pFormat = NULL;

    //format info
    UINT32 samplesPerSecond;
    UINT32 bytesPerSample;
    UINT32 numChannels;

    //buffer info
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;

    //packet info
    UINT32 packetLength;
    UINT32 framesPerPacket;
    UINT32 bytesPerPacket;


    HANDLE readyHandle = NULL;

    void setDevice(IMMDevice* p) {
        pDevice = p;
    }

    void initializeEngine() {
        HRESULT hr;

        //activate the device
        hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
        checkR(hr);
        //get format from device
        hr = pAudioClient->GetMixFormat(&pFormat);
        pFormatExtensible = (WAVEFORMATEXTENSIBLE*)pFormat;
        checkR(hr);
        //set format info
        samplesPerSecond = pFormat->nSamplesPerSec;
        bytesPerSample = pFormat->wBitsPerSample;
        numChannels = pFormat->nChannels;

        //initialize the audio engine //must set the periodicity
        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, hnsRequestedDuration, pFormat, NULL);
        checkR(hr);
        //get the buffer size
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);
        checkR(hr);
        //get render client
        hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);

        checkR(hr);
    }

    void setBuffer(dataBuffer<float*>* p) {
        outputBuffer = p;
    }

    void initializeBufferHandle() {
        HRESULT hr;

        //handle indicates when to call getBuffer
        readyHandle = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
        hr = pAudioClient->SetEventHandle(readyHandle);

        checkR(hr);
    }

    WAVEFORMATEXTENSIBLE* getDefaultFormatExtensible() {
        return pFormatExtensible;
    }

    WAVEFORMATEX* getDefaultFormat() {
        return pFormat;
    }

    void setOutputPacketLength(UINT32 l) {
        packetLength = l;
        bytesPerPacket = l * sizeof(float);
        framesPerPacket = l / numChannels;
    }

    void start() {
        HRESULT hr;

        //buffer pointer
        BYTE* pData = (BYTE*)malloc(sizeof(float) * packetLength * 4);

        //packet pointer
        float* pPacket;

        //packet return result
        UCHAR recievedPacket;

        //in bytes
        UINT32 leftoverSize = 0;
        
        float* leftovers = (float*)malloc(sizeof(float) * 2048);
        UINT32 numPackets = 0;

        UINT32 framesInNextBuffer = 0;
        UINT32 framesInCurBuffer = 0;
        UINT32 bytesInCurBuffer = 0;

        hr = pAudioClient->Start();  // Start playing.

        
        while (/**halt == 0*/1) {

            

            
            //create N many packet sized arrays to copy
            //data where n is enough to completely fill with buffer + leftovers
            //leftover size in frames = leftoverSize/numchannels
            //leftoverSize / numChannels + 
            

            DWORD waitResult = WaitForSingleObject(readyHandle, INFINITE);
            
            //std::cout << "WAIT OVER";

            hr = pCaptureClient->GetBuffer(&pData, &framesInCurBuffer, &flags, NULL, NULL);

            numPackets = (framesInCurBuffer) / framesPerPacket;
            bytesInCurBuffer = framesInCurBuffer * sizeof(float) * numChannels;
            //std::cout << "NUMPACKETS: " << numPackets << "\n";

            float** newPackets = (float**)malloc(sizeof(float*) * numPackets);
            for (size_t i = 0; i < numPackets; i++)
            {
                *(newPackets + i) = (float*)malloc(bytesPerPacket);
            }


            //fill up the new packets
            //use leftovers first
            //copy the rest into leftovers
            //update leftovers length
            
            //all indexes are in bytes
            
            UINT32 bufferIndex = 0;
            
            memcpy(*(newPackets), leftovers, leftoverSize);
            memcpy(*(newPackets), pData, bytesPerPacket - leftoverSize);
            bufferIndex = bytesPerPacket - leftoverSize;

            UCHAR pcount = 1;
            while (bufferIndex + bytesPerPacket < bytesInCurBuffer)
            {
                memcpy(*(newPackets + pcount), pData + bufferIndex, bytesPerPacket);
                bufferIndex += bytesPerPacket;
            }

            memcpy(leftovers, pData + bufferIndex, bytesInCurBuffer - bufferIndex);
            leftoverSize = bytesInCurBuffer - bufferIndex;

            //frees pData

            //WHAT!
            //free(pData);
            hr = pCaptureClient->ReleaseBuffer(framesInCurBuffer);
            
            for (size_t i = 0; i < numPackets; i++)
            {
                while (!outputBuffer->putPacket(newPackets+i)) {}
                //free(*(newPackets + i));
            }
            
            //hr = pCaptureClient->GetNextPacketSize(&framesInNextBuffer);

            free(newPackets);

        }
    }
};