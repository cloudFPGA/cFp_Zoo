

#define FRAME_HEIGHT 512
#define FRAME_WIDTH 512
#define FRAME_INTERVAL (1000/30)
#define PACK_SIZE 1024 // This is our custom MTU. We must use a multiple of 8 (Bytes per transaction)! 1450 4086 udp pack size; note that OSX limits < 8100 bytes
#define BUF_LEN 65540 // Larger than maximum UDP packet size

#define CEIL(a, b)     (((a) + (b-1)) / (b))
#define TOT_TRANSFERS CEIL(FRAME_HEIGHT*FRAME_WIDTH, PACK_SIZE)

//#define INPUT_FROM_CAMERA
