/*! \file net.c \brief Network support library. */
//*****************************************************************************
//
// File Name	: 'net.c'
// Title		: Network support library
// Author		: Pascal Stang
// Created		: 8/30/2004
// Revised		: 7/3/2005
// Version		: 0.1
// Target MCU	: Atmel AVR series
// Editor Tabs	: 4
//
//*****************************************************************************

#include <inttypes.h>
#include "global.h"
#include "rprintf.h"

#include "net.h"

unsigned long IpMyAddress;

uint16_t htons(uint16_t val)
{
	return (val<<8) | (val>>8);
}

uint32_t htonl(uint32_t val)
{
	return (htons(val>>16) | (uint32_t)htons(val&0x0000FFFF)<<16);
}


uint16_t netChecksum(void *data, uint16_t len)
{
    register uint32_t sum = 0;

    for (;;) {
        if (len < 2)
            break;
		//sum += *((uint16_t *)data)++;
		sum += *((uint16_t *)data);
		data+=2;
        len -= 2;
    }
    if (len)
        sum += *(uint8_t *) data;

    while ((len = (uint16_t) (sum >> 16)) != 0)
        sum = (uint16_t) sum + len;

    return (uint16_t) sum ^ 0xFFFF;
}

void netPrintEthAddr(struct netEthAddr* ethaddr)
{
	rprintfu08(ethaddr->addr[0]);
	rprintfChar(':');
	rprintfu08(ethaddr->addr[1]);
	rprintfChar(':');
	rprintfu08(ethaddr->addr[2]);
	rprintfChar(':');
	rprintfu08(ethaddr->addr[3]);
	rprintfChar(':');
	rprintfu08(ethaddr->addr[4]);
	rprintfChar(':');
	rprintfu08(ethaddr->addr[5]);
}

void netPrintIPAddr(uint32_t ipaddr)
{
	rprintf("%d.%d.%d.%d",
		((unsigned char*)&ipaddr)[3],
		((unsigned char*)&ipaddr)[2],
		((unsigned char*)&ipaddr)[1],
		((unsigned char*)&ipaddr)[0]);
}

void netPrintEthHeader(struct netEthHeader* eth_hdr)
{
	rprintfProgStrM("Eth Packet Type: 0x");
	rprintfu16(eth_hdr->type);

	rprintfProgStrM(" SRC:");
	rprintfu08(eth_hdr->src.addr[0]);
	rprintfu08(eth_hdr->src.addr[1]);
	rprintfu08(eth_hdr->src.addr[2]);
	rprintfu08(eth_hdr->src.addr[3]);
	rprintfu08(eth_hdr->src.addr[4]);
	rprintfu08(eth_hdr->src.addr[5]);

	rprintfProgStrM("->DST:");
	rprintfu08(eth_hdr->dest.addr[0]);
	rprintfu08(eth_hdr->dest.addr[1]);
	rprintfu08(eth_hdr->dest.addr[2]);
	rprintfu08(eth_hdr->dest.addr[3]);
	rprintfu08(eth_hdr->dest.addr[4]);
	rprintfu08(eth_hdr->dest.addr[5]);
	rprintfCRLF();
}
