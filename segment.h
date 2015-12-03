#include <stdbool.h>

typedef struct segment{
	int seq;
	int len;
	int fileSize;
	char* data;
} segment, *segment_t;



segment_t makeSegment(int seq, int len, int fileSize, char* data);	//allocates memory for segment and constructs it
	//FIXME 1. NEED TO IMPLEMENT

char* segToChar(segment_t seg);	//takes in seg, returns char representation
	//FIXME 2. NEED TO IMPLEMENT

charToSeg(char* c, segment_t seg_t);	//takes in char* and returns a segment_t
	//FIXME 3. NEED TO IMPLEMENT
