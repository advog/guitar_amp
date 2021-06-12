#pragma once

#include <Windows.h>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <mmdeviceapi.h>

#include "dataBuffer.h"

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000
#define M_PI 3.14159265358979323846

class threadRender{
public:
	
    //input queue
    dataBuffer<float*>* inputBuffer;

    //utility
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    DWORD flags = 0;

    //wasapi pointers
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
   
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

        //initialize the audio engine
        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, hnsRequestedDuration, pFormat, NULL);

        //get the buffer size
        hr = pAudioClient->GetBufferSize(&bufferFrameCount);

        //get render client
        hr = pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient);

        //set format info
        samplesPerSecond = pFormat->nSamplesPerSec;
        bytesPerSample = pFormat->wBitsPerSample;
        numChannels = pFormat->nChannels;

        return hr;
    }

    HRESULT setBuffer(dataBuffer<float*>* p) {
        inputBuffer = p;
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

    HRESULT setInputPacketLength(UINT32 l) {
        packetLength = l;
        bytesPerPacket = l * sizeof(float);
        framesPerPacket = l / numChannels;
        return 0;
    }

    HRESULT start() {
        HRESULT hr;
        
        //buffer pointer
        BYTE* pData;

        //packet pointer
        float* pPacket;

        //packet return result
        UCHAR recievedPacket;

        hr = pAudioClient->Start();  // Start playing.

        while (/**halt == 0*/1) {

            //std::cout << "FREE FRAMES IN BUFFER: " << numFramesAvailable << "\n";
            //std::cout << "QUEUESIZE: " << input->size() << "\n";

            DWORD waitResult = WaitForSingleObject(readyHandle, INFINITE);

            while(!inputBuffer->getPacket(&pPacket)){}
               
            hr = pRenderClient->GetBuffer(framesPerPacket, &pData);

            memcpy(pData, pPacket, bytesPerPacket);

            hr = pRenderClient->ReleaseBuffer(framesPerPacket, flags);
            

        }
        return 0;
    }
};