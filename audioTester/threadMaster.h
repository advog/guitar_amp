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



void startRender(threadRender* pR) {
	pR->start();
}

void startProcess(threadProcess* pP) {
	pP->start();
}

void startRead(threadRead* pR) {
	pR->start();
}

void FUDGE() {
	std::cout << "FUDGE\n";
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

	void setEnumerator(IMMDeviceEnumerator* p) {
		pEnumerator = p;
	}

	void initializeOutputDevice() {
		pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pOutDevice);
	}

	void initializeInputDevice() {
		pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pInDevice);
	}

	void setBufferSettings(UINT32 prl, UINT32 rpl, UINT32 rppl, UINT32 prpl) {
		prPacketLength = prpl;
		rpPacketLength = rppl;
		prBufferSize = prl;
		rpBufferSize = rpl;
	}

	void initializeDataBuffers() {
		proc_to_render = new dataBuffer<float*>(prBufferSize);
		reader_to_proc = new dataBuffer<float*>(rpBufferSize);
	}

	void initializeThreadRenderer() {
		pRender = new threadRender();

		pRender->setDevice(pOutDevice);

		pRender->initializeEngine();

		pRender->setInputPacketLength(prPacketLength);

		pRender->setBuffer(proc_to_render);

		pRender->initializeBufferHandle();

		outFormat = pRender->getDefaultFormatExtensible();
	}

	void initializeThreadRead() {
		pRead = new threadRead();

		pRead->setDevice(pInDevice);

		pRead->initializeEngine();

		pRead->setOutputPacketLength(rpPacketLength);

		pRead->setBuffer(reader_to_proc);

		pRead->initializeBufferHandle();

		inFormat = pRead->getDefaultFormatExtensible();
	}
	
	void initializeThreadProcessor() {
		pProcess = new threadProcess();

		pProcess->setBuffers(reader_to_proc, proc_to_render);

		pProcess->setOutputPacketLength(prPacketLength, prBufferSize);

		pProcess->setInputPacketLength(rpPacketLength, rpBufferSize);
	}

	void start() {

		std::thread* captureThread = new std::thread(startRead, pRead);
		//Sleep(10);
		std::thread* processThread = new std::thread(startProcess, pProcess);
		//Sleep(10);
		std::thread* renderThread = new std::thread(startRender, pRender);
		

		renderThread->join();
		processThread->join();
		captureThread->join();
	}
};