#pragma once

#include <Windows.h>

class packetFormater {

	UINT32 inSizeBytes;
	UINT32 outSizeBytes;
	UCHAR mode;
	
	UINT32 leftoverSize; //not size of array, size of relevant data starting from Index
	UINT32 leftoverIndex;
	BYTE* leftovers;
	
	// que;

	packetFormater(UINT32 si, UINT32 so) {
		inSizeBytes = si;
		outSizeBytes = so;
		
		if (inSizeBytes > outSizeBytes) {
			mode = 0;
			leftovers = (BYTE*)malloc(inSizeBytes);
		}
		if (inSizeBytes < outSizeBytes) {
			mode = 1;
			leftovers = (BYTE*)malloc(outSizeBytes);
		}
		else {mode = 2;}
		
		leftoverIndex = 0;
		leftoverSize = 0;

		
	}

};
