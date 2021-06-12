#pragma once

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
#include "threadRender.h"
#include "threadProcess.h"
#include "threadRead.h"



HRESULT startRender(threadRender* pR) {
	pR->start();
	return 0;
}

HRESULT startProcess(threadProcess* pP) {
	pP->start();
	return 0;
}

HRESULT startRead(threadRead* pR) {
	pR->start();
	return 0;
}

HRESULT FUCK() {
	std::cout << "FUCK\n";
	return 0;
}

class threadMaster {
public:
	
	threadProcess* pProcess;
	threadRender* pRender;
	threadRead* pRead;
	UINT32 prPacketLength;
	UINT32 rpPacketLength;

	dataBuffer<float*>* proc_to_render;
	dataBuffer<float*>* reader_to_proc;

	UINT32 rpBufferSize;
	UINT32 prBufferSize;

	IMMDeviceEnumerator* pEnumerator;
	IMMDevice* pOutDevice;
	IMMDevice* pInDevice;

	WAVEFORMATEXTENSIBLE* outFormat;
	WAVEFORMATEXTENSIBLE* inFormat;

	HRESULT setEnumerator(IMMDeviceEnumerator* p) {
		pEnumerator = p;
		return 0;
	}

	HRESULT initializeOutputDevice() {
		pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pOutDevice);
		return 0;
	}

	HRESULT initializeInputDevice() {
		pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pInDevice);
		return 0;
	}

	HRESULT setBufferSettings(UINT32 prl, UINT32 rpl, UINT32 rppl, UINT32 prpl) {
		prPacketLength = prpl;
		rpPacketLength = rppl;
		prBufferSize = prl;
		rpBufferSize = rpl;
		return 0;
	}

	HRESULT initializeDataBuffers() {
		proc_to_render = new dataBuffer<float*>(prBufferSize);
		reader_to_proc = new dataBuffer<float*>(rpBufferSize);
		return 0;
	}

	HRESULT initializeThreadRenderer() {
		pRender = new threadRender();

		pRender->setDevice(pOutDevice);

		pRender->initializeEngine();

		pRender->setInputPacketLength(prPacketLength);

		pRender->setBuffer(proc_to_render);

		pRender->initializeBufferHandle();

		outFormat = pRender->getDefaultFormatExtensible();
		return 0;
	}

	HRESULT initializeThreadProcessor() {
		pProcess = new threadProcess();

		pProcess->setBuffers(reader_to_proc, proc_to_render);

		pProcess->setOutputPacketLength(1024, 2);

		pProcess->setInputPacketLength(1024, 2);
		
		return 0;
	}

	HRESULT initializeThreadRead() {
		pRead = new threadRead();

		pRead->setDevice(pInDevice);

		pRead->initializeEngine();

		pRead->setOutputPacketLength(rpPacketLength);

		pRead->setBuffer(reader_to_proc);

		pRead->initializeBufferHandle();

		inFormat = pRead->getDefaultFormatExtensible();

		return 0;
	}

	HRESULT start() {
		
		std::thread* captureThread = new std::thread(startRead, pRead);
		//Sleep(10);
		std::thread* processThread = new std::thread(startProcess, pProcess);
		//Sleep(10);
		std::thread* renderThread = new std::thread(startRender, pRender);
		

		renderThread->join();
		processThread->join();
		captureThread->join();
		return 0;
	}
};