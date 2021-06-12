#pragma once

template <class T>
class dataBuffer {
public:

	std::atomic<UINT32> produceCount;
	std::atomic<UINT32> consumeCount;

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
		//std::cout<< "CONSUME COUNT: " << consumeCount << "\n";
		return 1;
	}

	UCHAR putPacket(T* pPacket) {
		if (produceCount - consumeCount == bufferSize) {
			return 0;
		}
		*(buffer + produceCount % bufferSize) = *pPacket;
		produceCount++;
		//std::cout << "PRODUCE COUNT: " << produceCount << "\n";
		return 1;
	}
};

