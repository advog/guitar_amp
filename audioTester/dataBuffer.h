#pragma once
#include <iostream>
#include <chrono>
#include <ctime>

template <class T>
class dataBuffer {
public:


	std::atomic<UINT32> produceCount;
	std::atomic<UINT32> consumeCount;

	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::chrono::time_point<std::chrono::high_resolution_clock> stop;


	T* buffer;

	UINT32 bufferSize;

	dataBuffer(UINT32 sizeB) {
		produceCount = 0;
		consumeCount = 0;
		bufferSize = sizeB;
		buffer = (T*)malloc(sizeof(T) * bufferSize);
	}

	UCHAR getPacket(T* pPacket) {
		if (produceCount - consumeCount == 0) {
			return 0;
		}
		*pPacket = *(buffer + consumeCount % bufferSize);
		consumeCount++;
		
		if (consumeCount % 1000 == 1) { 
			stop = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> diff = stop - start;
			std::cout << "dataBuffer latency: " << diff.count() << "\n";
		}
		
		return 1;
	}

	UCHAR putPacket(T* pPacket) {
		if (produceCount - consumeCount == bufferSize) {
			return 0;
		}
		*(buffer + produceCount % bufferSize) = *pPacket;
		produceCount++;

		if (produceCount % 1000 == 1) { start = std::chrono::high_resolution_clock::now(); }

		return 1;
	}
};

