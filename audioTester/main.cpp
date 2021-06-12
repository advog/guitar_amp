#include <Windows.h>
#include <iostream>
#include <Audioclient.h>
#include <Audiopolicy.h>
#include <mmdeviceapi.h>
#include <math.h>
#include <Mmreg.h>
#include <combaseapi.h>
#include <vector>
#include <thread>
#include <synchapi.h>
#include <system_error>
#include <WinSock.h>
#include <assert.h>
#include <avrt.h>

#include "dataBuffer.h"
#include "threadMaster.h"

// REFERENCE_TIME time units per second and per millisecond
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000
#define M_PI 3.14159265358979323846



const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);


//pepper rep pep

/*
int main() {

    HRESULT hr;
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC / 40;
    REFERENCE_TIME hnsActualDuration;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    IAudioRenderClient* pRenderClient = NULL;
    WAVEFORMATEX* pwfx = NULL;
    UINT32 bufferFrameCount;
    UINT32 numFramesAvailable;
    UINT32 numFramesPadding;
    BYTE* pData;
    DWORD flags = 0;
    HANDLE audioReadyHandle = NULL;
    //UINT32 packetSize = 600;

    double t = 0;
    double* theta = &t;

    hr = CoInitialize(NULL);

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

    hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);

    hr = pAudioClient->GetMixFormat(&pwfx);

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, hnsRequestedDuration, 0, pwfx, NULL);

    // Get the actual size of the allocated buffer.
    hr = pAudioClient->GetBufferSize(&bufferFrameCount);

    hr = pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient);

    // Calculate the actual duration of the allocated buffer.
    hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

    audioReadyHandle = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE);

    hr = pAudioClient->SetEventHandle(audioReadyHandle);

    hr = pAudioClient->Start();  // Start playing.

    UINT32 flipper = 0;

    // Each loop fills about half of the shared buffer.
    while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
     
        //float* data = getPacket();
        
        DWORD waitResult = WaitForSingleObject(audioReadyHandle, INFINITE);

        if (flipper % 2 == 0) {
            // See how much buffer space is available.
            hr = pAudioClient->GetCurrentPadding(&numFramesPadding);

            numFramesAvailable = bufferFrameCount - numFramesPadding;

            // Grab all the available space in the shared buffer.
            hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);

            // Get next 1/2-second of data from the audio source.
            std::cout << "FRAMES: " << numFramesAvailable << "\n";

            hr = loadSine(pData, numFramesAvailable, pwfx->nSamplesPerSec, pwfx->nChannels, 440, theta, numFramesAvailable);

            hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);

            flipper++;
        }

    }

    // Wait for last data in buffer to play before stopping.
    Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

    hr = pAudioClient->Stop();  // Stop playing.

    return 0;
}
*/

/*
if (FAILED(hr)) {
        std::cout << std::system_category().message(hr) << "\n";
    }
*/

/*
void threadWrite(int start, dataBuffer<float*>* q) {
    for (size_t i = 0; i < 5; i++)
    {
        float* pVal = (float*)malloc(sizeof(float));
        *pVal = start*10 + i;
        q->putPacket(&pVal);
    }
}

void threadRead(dataBuffer<float*>* q) {
    for (size_t i = 0; i < 5; i++)
    {
        float* pVal = (float*)malloc(sizeof(float));
        UCHAR Result = q->getPacket(&pVal);
        if (Result) {
            std::cout << *pVal << "\n";
        }
    }
}

int main() {
    dataBuffer<float*>* q = new dataBuffer<float*>(10);

    std::vector<std::thread*>* pVec = new std::vector<std::thread*>();

    for (size_t i = 0; i < 10; i++)
    {
        pVec->push_back(new std::thread(threadWrite, i, q));
        pVec->push_back(new std::thread(threadRead, q));
        Sleep(10);
    }
    
    for (size_t i = 0; i < 20; i++)
    {
        pVec->at(i)->join();
    }

    return 0;
}
*/

int main() {
    HRESULT hr;

    IMMDeviceEnumerator* pEnumerator;
    threadMaster* pMaster = new threadMaster();
    
    //create device enumerator
    hr = CoInitialize(NULL);

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);

    //give enumerator to master thread
    pMaster->setEnumerator(pEnumerator);

    //set the buffer sizes on the master thread
    pMaster->setBufferSettings(2, 2, 1024, 1024);

    //initialize the output endpoint
    pMaster->initializeOutputDevice();

    //initialize the input endpoint
    pMaster->initializeInputDevice();

    //initialize the data buffers
    pMaster->initializeDataBuffers();

    //initialize classes
    //order is render, read, process becaues process needs the formats of the other devices
    pMaster->initializeThreadRenderer();
    pMaster->initializeThreadRead();
    pMaster->initializeThreadProcessor();

    //start
    pMaster->start();

    return hr;
}