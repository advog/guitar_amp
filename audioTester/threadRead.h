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
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
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

    UINT32 FUCK;

    HANDLE readyHandle = NULL;

    HRESULT setDevice(IMMDevice* p) {
        pDevice = p;

        return 0;
    }

    HRESULT initializeEngine() {
        HRESULT hr;

        //activate the device
        hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);

        //get format from device
        hr = pAudioClient->GetMixFormat(&pFormat);
        pFormatExtensible = (WAVEFORMATEXTENSIBLE*)pFormat;

        //set format info
        samplesPerSecond = pFormat->nSamplesPerSec;
        bytesPerSample = pFormat->wBitsPerSample;
        numChannels = pFormat->nChannels;

        //initialize the audio engine //must set the periodicity
        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, hnsRequestedDuration, pFormat, NULL);
        checkR(hr);
        //get the buffer size
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);

        //get render client
        hr = pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&pCaptureClient);

        return hr;
    }

    HRESULT setBuffer(dataBuffer<float*>* p) {
        outputBuffer = p;
        return 0;
    }

    HRESULT initializeBufferHandle() {
        HRESULT hr;

        //handle indicates when to call getBuffer
        readyHandle = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);
        hr = pAudioClient->SetEventHandle(readyHandle);

        return hr;
    }

    WAVEFORMATEXTENSIBLE* getDefaultFormatExtensible() {
        return pFormatExtensible;
    }

    WAVEFORMATEX* getDefaultFormat() {
        return pFormat;
    }

    HRESULT setOutputPacketLength(UINT32 l) {
        packetLength = l;
        bytesPerPacket = l * sizeof(float);
        framesPerPacket = l / numChannels;
        return 0;
    }

    HRESULT start() {
        HRESULT hr;

        std::cout << "CHANNELS IN: " << numChannels << "\n";

        //buffer pointer
        BYTE* pData;

        //packet pointer
        float* pPacket;

        //packet return result
        UCHAR recievedPacket;

        hr = pAudioClient->Start();  // Start playing.

        UINT32 leftoverSize = 0;

        while (/**halt == 0*/1) {

            BYTE* pData = (BYTE*)malloc(sizeof(float) * packetLength);


            hr = pCaptureClient->GetNextPacketSize(&FUCK);
            std::cout << "FRAMES IN NEXT BUFFER: " << FUCK << "\n";

            //create N many packet sized arrays to copy
            //data where n is enough to completely fill with buffer + leftovers

            DWORD waitResult = WaitForSingleObject(readyHandle, INFINITE);

            hr = pCaptureClient->GetBuffer(&pData, &framesPerPacket, &flags, NULL, NULL);


            
            //copy the data into the new packets
            //copy the rest into leftovers
            //update leftovers length

            float* castedPData = (float*)pData;
            while (!outputBuffer->putPacket(&castedPData)) {
                Sleep(10);
            }

            hr = pCaptureClient->ReleaseBuffer(framesPerPacket);
            
            
            

            
        }
        return 0;
    }

};