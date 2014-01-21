#include <stdio.h>
#include "udpcam.h"

#ifdef WIN32
    #include <winsock.h>
#else
    #include "uip.h"
	
    #define HIWORD(x)		((uint16_t) ((x) >> 16))
    #define LOWORD(x)		((uint16_t) (x))
#endif

//#define _DEBUG      1

enum cam_cmd
{
    NO_OP                   = 0,
    CAMERA_RESET            = 2,
    SERIAL_NUMBER           = 4,
    GET_REVISION            = 5,

    FPA_PARAMS                  =   170,
    
    READ_MEMORY                 =   210,
    WRITE_MEMORY                =   211,
    ERASE_FLASH_BLOCK           =   212,
    GET_PROGRAM_ADDRESS         =   213,
    GET_NUC_ADDRESS             =   214,
};

enum nuc_type
{
    NUC_CAPTURE_BUFFER      = 11,
    DRAM_VIDEO_BUFFER       = 12,
    DRAM_ONCHIP_BUFFER      = 13,
    DRAM_GAIN_BUFFER        = 15,
    DRAM_OFFSET_BUFFER      = 16,
};

#define UDP_MAX         1500
#define MEMORY_SIZE     16*1024*1024    /* 16 Megabyte of memory */

#define NUM_FPA_COLS    640             /* Simulate VGA size FPA */
#define NUM_FPA_ROWS    512
#define NUM_FPA_PIXELS  (NUM_FPA_COLS * NUM_FPA_ROWS)

#ifdef _WIN32
    #define DRAM_BASE       0x01000000
#else
    #define DRAM_BASE       0x22010000
#endif

long getNUCSize(short type)
{
    switch (type)
    {
    case DRAM_VIDEO_BUFFER:
    case DRAM_ONCHIP_BUFFER:
        return NUM_FPA_PIXELS;
    case DRAM_GAIN_BUFFER:
    case DRAM_OFFSET_BUFFER:
    case NUC_CAPTURE_BUFFER:
        return NUM_FPA_PIXELS * sizeof(short);
    default:
        ;
    }
    return 0;
}

long getNUCAddress(short buf, short type)
{
    long addr = 0L;

    switch (type)
    {
    case DRAM_VIDEO_BUFFER:
        addr = DRAM_BASE;
        break;
    case DRAM_ONCHIP_BUFFER:
        addr = DRAM_BASE + NUM_FPA_PIXELS;
		break;
    case DRAM_GAIN_BUFFER:
        addr = DRAM_BASE + NUM_FPA_PIXELS * 2;
		break;
    case DRAM_OFFSET_BUFFER:
        addr = DRAM_BASE + NUM_FPA_PIXELS * 4;
		break;
    case NUC_CAPTURE_BUFFER:
        addr = DRAM_BASE + NUM_FPA_PIXELS * (6 + buf * 2);
    default:
        ;
    }
    return addr;
}

#ifdef _WIN32
unsigned char g_ddr_mem[MEMORY_SIZE];
#endif

typedef int (*FUNC_PTR)(void);

short CamCommand(short func, short count, short data[])
{
    long addr, value;

#ifdef _DEBUG
    printf("CamCommand(%d,%d,%04x)\n", func, count, ntohs(data[0]));
#endif
    switch (func)
    {
    case NO_OP:
        break;
    case CAMERA_RESET:
		if (count == 0)
		{
			((FUNC_PTR) 0)();
		}
		count = CAM_BYTE_COUNT_ERROR;
		break;
    case SERIAL_NUMBER:
        if (count == 0)
        {
            long serno = 12345678;
            data[0] = htons(HIWORD(serno));
            data[1] = htons(LOWORD(serno));
            count = sizeof(long);
        }
        else
        {
            count = CAM_BYTE_COUNT_ERROR;
        }
        break;
    case GET_REVISION:
        if (count == 0)
        {
            const long sw_ver = 0x01020304;
            const long fw_ver = 0x05060708;
            data[0] = htons(HIWORD(sw_ver));
            data[1] = htons(LOWORD(sw_ver));
            data[2] = htons(HIWORD(fw_ver));
            data[3] = htons(LOWORD(fw_ver));
            count = sizeof(long) * 2;
        }
        else
        {
            count = CAM_BYTE_COUNT_ERROR;
        }
        break;
    case FPA_PARAMS:
        if (count == 0)
        {
            const short fpaType = 0x1002;
            data[0] = htons(fpaType);
            count = sizeof(short);
        }
        else
        {
            count = CAM_BYTE_COUNT_ERROR;
        }
        break;
    case READ_MEMORY:
        if (count == 6)
        {
            addr = (ntohs(data[0]) << 16) | ntohs(data[1]);
            count = ntohs(data[2]);
#ifdef _WIN32
            if (addr < 0 || addr + count >= MEMORY_SIZE ||
                count < 0 || count >= UDP_MAX)
            {
                return CAM_RANGE_ERROR;
            }
            memcpy(data, &g_ddr_mem[addr], count);
#else

#ifdef _DEBUG
			printf("READ_MEMORY(%8x,%d)\n", addr, count);
#endif
            memcpy(data, (void*) addr, count);
#endif
        }
        else
        {
            count = CAM_BYTE_COUNT_ERROR;
        }
        break;
    case WRITE_MEMORY:
        if (count >= 6)
        {
            addr = (ntohs(data[0]) << 16) | ntohs(data[1]);
            count = count - sizeof(long);
#ifdef _WIN32
            if (addr < 0 || addr + count >= MEMORY_SIZE)
            {
                return CAM_RANGE_ERROR;
            }
            memcpy(&g_ddr_mem[addr], data + 2, count);
#else
#ifdef _DEBUG
			printf("WRITE_MEMORY(%8x,%d)\n", addr, count);
#endif
            memcpy((void*) addr, data + 2, count);
#endif
            count = 0;
        }
        else
        {
            count = CAM_BYTE_COUNT_ERROR;
        }
        break;
    case GET_NUC_ADDRESS:
        if (count == 4) 
        {
            addr = getNUCAddress(ntohs(data[0]), ntohs(data[1]));
			value = getNUCSize(ntohs(data[1]));
			if (addr != (long) NULL)
			{
				//fprintf( stderr, "GET_NUC_ADDRESS:  Type: %d  Address: %x  Size: %x:%d\n", data[1], addressValue, tempValue, tempValue);
				data[0] = htons(HIWORD(addr));
				data[1] = htons(LOWORD(addr));
				data[2] = htons(HIWORD(value));
				data[3] = htons(LOWORD(value));
				count = 8;
			}
			else
			{
				count = CAM_RANGE_ERROR;
			}
        }
        break;
    default:
        count = CAM_UNDEFINED_FUNCTION_ERROR;
    }
    return count;
}
