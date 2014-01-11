#include <p18f4620.h>
#include <stddef.h>


typedef          char         int8_t;
typedef unsigned char        uint8_t;
typedef          int         int16_t;
typedef unsigned int        uint16_t;
typedef          short long  int24_t;	// Microchip specific
typedef unsigned short long uint24_t;	// Microchip specific
typedef          long        int32_t;
typedef unsigned long       uint32_t;

typedef unsigned char bool;
typedef unsigned char byte;

typedef union {
	uint32_t u32;
	struct {
		uint8_t b0;
		uint8_t b1;
		uint8_t b2;
		uint8_t b3;
	};
} uint32_u;

typedef union {
	uint24_t u24;
	struct {
		uint8_t b0;
		uint8_t b1;
		uint8_t b2;
	};
} uint24_u;

typedef union {
	uint16_t u16;
	struct {
		uint8_t b0;
		uint8_t b1;
	};
} uint16_u;


#include "..\Common\uavx_legacy.h"
#include "..\Common\leds.h"
#include "..\Common\ringbuffer.h"
#include "..\Common\crc.h"

#define Delay_ms(duration) Delay10KTCYx(duration)
#define LAT(p,b) LAT##p##bits.LAT##p##b

// 1 Cycle = 100 ns
#define Delay10Cycles() Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop()
#define Delay25Cycles() Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop()
#define Delay2us5() Delay25Cycles()
#define Delay50Cycles() Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop();Nop()
#define Delay5us() Delay50Cycles()

#define EnableInterrupts INTCONbits.GIEH=1
#define DisableInterrupts INTCONbits.GIEH=0

#define ESC 0x1B
#define STX 0x01
#define ETB 0x17

#define Limit(i,l,u) (((i) < l) ? l : (((i) > u) ? u : (i)))
#define Decay1(i) 			(((i) < 0) ? (i+1) : (((i) > 0) ? (i-1) : 0))
#define Max(i,j) 			((i<j) ? j : i)
#define Min(i,j) 			((i<j) ? i : j )
