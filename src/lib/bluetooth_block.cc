/* -*- c++ -*- */
/*
 * Copyright 2004 Free Software Foundation, Inc.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

/*
 * config.h is generated by configure.  It contains the results
 * of probing for features, options etc.  It should be the first
 * file included in your .cc file.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <bluetooth_block.h>
#include <gr_io_signature.h>

//private constructor
bluetooth_block::bluetooth_block ()
  : gr_sync_block ("bluetooth block",
	      gr_make_io_signature (IN, IN, sizeof (char)),
	      gr_make_io_signature (OUT, OUT, OUT))
{
	d_payload_size = 0;
	d_packet_type = -1;
	d_stream_length = 0;
	d_consumed = 0;
}

//This is all imported from packet_LAP.c
//It has been converted to C++

/* Error correction coding for Access Code */
uint8_t *bluetooth_block::codeword(uint8_t *data, int length, int k)
/*
 * Compute redundacy cw[], the coefficients of b(x). The redundancy
 * polynomial b(x) is the remainder after dividing x^(length-k)*data(x)
 * by the generator polynomial g(x).
 */
{
	int    i, j;
	uint8_t *g, *cw, feedback;

	g = (uint8_t *) malloc(35);
	g[0] = 1;
	g[1] = 0;
	g[2] = 0;
	g[3] = 1;
	g[4] = 0;
	g[5] = 1;
	g[6] = 0;
	g[7] = 1;
	g[8] = 1;
	g[9] = 0;
	g[10] = 1;
	g[11] = 1;
	g[12] = 1;
	g[13] = 1;
	g[14] = 0;
	g[15] = 0;
	g[16] = 1;
	g[17] = 0;
	g[18] = 0;
	g[19] = 0;
	g[20] = 1;
	g[21] = 1;
	g[22] = 1;
	g[23] = 0;
	g[24] = 1;
	g[25] = 0;
	g[26] = 1;
	g[27] = 0;
	g[28] = 0;
	g[29] = 0;
	g[30] = 0;
	g[31] = 1;
	g[32] = 1;
	g[33] = 0;
	g[34] = 1;

	cw = (uint8_t *) malloc(34);

	for (i = 0; i < length - k; i++)
		cw[i] = 0;
	for (i = k - 1; i >= 0; i--) {
		feedback = data[i] ^ cw[length - k - 1];
		if (feedback != 0) {
			for (j = length - k - 1; j > 0; j--)
				if (g[j] != 0)
					cw[j] = cw[j - 1] ^ feedback;
				else
					cw[j] = cw[j - 1];
			cw[0] = g[0] && feedback;
		} else {
			for (j = length - k - 1; j > 0; j--)
				cw[j] = cw[j - 1];
			cw[0] = 0;
		}
	}
	free(g);
	return cw;
}

/* Reverse the bits in a byte */
uint8_t bluetooth_block::reverse(char byte)
{
	return (byte & 0x80) >> 7 | (byte & 0x40) >> 5 | (byte & 0x20) >> 3 | (byte & 0x10) >> 1 | (byte & 0x08) << 1 | (byte & 0x04) << 3 | (byte & 0x02) << 5 | (byte & 0x01) << 7;
}

/* Generate Access Code from an LAP */
uint8_t *bluetooth_block::acgen(int LAP)
{
	/* Endianness - Assume LAP is MSB first, rest done LSB first */
	uint8_t *retval, *pn, count, *cw, *data;
	retval = (uint8_t *) malloc(9);
	pn = (uint8_t *) malloc(9);
	data = (uint8_t *) malloc(30);

	LAP = reverse((LAP & 0xff0000)>>16) | (reverse((LAP & 0x00ff00)>>8)<<8) | (reverse(LAP & 0x0000ff)<<16);

	retval[4] = (LAP & 0xc00000)>>22;
	retval[5] = (LAP & 0x3fc000)>>14;
	retval[6] = (LAP & 0x003fc0)>>6;
	retval[7] = (LAP & 0x00003f)<<2;

	/* Trailer */
	if(LAP & 0x1)
	{	retval[7] |= 0x03;
		retval[8] = 0x2a;
	} else
		retval[8] = 0xd5;

	pn[0] = 0x03;
	pn[1] = 0xF2;
	pn[2] = 0xA3;
	pn[3] = 0x3D;
	pn[4] = 0xD6;
	pn[5] = 0x9B;
	pn[6] = 0x12;
	pn[7] = 0x1C;
	pn[8] = 0x10;

	for(count = 4; count < 9; count++)
		retval[count] ^= pn[count];

	/* Codeword */
	//g(d) = 0x585713DA9
	data[0] = (retval[4] & 0x02) >> 1;
	data[1] = (retval[4] & 0x01);
	data[2] = (retval[5] & 0x80) >> 7;
	data[3] = (retval[5] & 0x40) >> 6;
	data[4] = (retval[5] & 0x20) >> 5;
	data[5] = (retval[5] & 0x10) >> 4;
	data[6] = (retval[5] & 0x08) >> 3;
	data[7] = (retval[5] & 0x04) >> 2;
	data[8] = (retval[5] & 0x02) >> 1;
	data[9] = (retval[5] & 0x01);
	data[10] = (retval[6] & 0x80) >> 7;
	data[11] = (retval[6] & 0x40) >> 6;
	data[12] = (retval[6] & 0x20) >> 5;
	data[13] = (retval[6] & 0x10) >> 4;
	data[14] = (retval[6] & 0x08) >> 3;
	data[15] = (retval[6] & 0x04) >> 2;
	data[16] = (retval[6] & 0x02) >> 1;
	data[17] = (retval[6] & 0x01);
	data[18] = (retval[7] & 0x80) >> 7;
	data[19] = (retval[7] & 0x40) >> 6;
	data[20] = (retval[7] & 0x20) >> 5;
	data[21] = (retval[7] & 0x10) >> 4;
	data[22] = (retval[7] & 0x08) >> 3;
	data[23] = (retval[7] & 0x04) >> 2;
	data[24] = (retval[7] & 0x02) >> 1;
	data[25] = (retval[7] & 0x01);
	data[26] = (retval[8] & 0x80) >> 7;
	data[27] = (retval[8] & 0x40) >> 6;
	data[28] = (retval[8] & 0x20) >> 5;
	data[29] = (retval[8] & 0x10) >> 4;

	cw = codeword(data, 64, 30);
	free(data);

	retval[0] = cw[0] << 3 | cw[1] << 2 | cw[2] << 1 | cw[3];
	retval[1] = cw[4] << 7 | cw[5] << 6 | cw[6] << 5 | cw[7] << 4 | cw[8] << 3 | cw[9] << 2 | cw[10] << 1 | cw[11];
	retval[2] = cw[12] << 7 | cw[13] << 6 | cw[14] << 5 | cw[15] << 4 | cw[16] << 3 | cw[17] << 2 | cw[18] << 1 | cw[19];
	retval[3] = cw[20] << 7 | cw[21] << 6 | cw[22] << 5 | cw[23] << 4 | cw[24] << 3 | cw[25] << 2 | cw[26] << 1 | cw[27];
	retval[4] = cw[28] << 7 | cw[29] << 6 | cw[30] << 5 | cw[31] << 4 | cw[32] << 3 | cw[33] << 2 | (retval[4] & 0x3);;
	free(cw);

	for(count = 0; count < 9; count++)
		retval[count] ^= pn[count];
	free(pn);

	/* Preamble */
	if(retval[0] & 0x08)
		retval[0] |= 0xa0;
	else
		retval[0] |= 0x50;

	return retval;
}

/* Convert from normal bytes to one-LSB-per-byte format */
void bluetooth_block::convert_to_grformat(uint8_t input, uint8_t *output)
{
	int count;
	for(count = 0; count < 8; count++)
	{
		output[count] = (input & 0x80) >> 7;
		input <<= 1;
	}
}

/* Decode 1/3 rate FEC, three like symbols in a row */
char *bluetooth_block::unfec13(char *stream, char *output, int length)
{
    int count, a, b, c;

    for(count = 0; count < length; count++)
    {
        a = 3*count;
        b = a + 1;
        c = a + 2;
        output[count] = ((stream[a] & stream[b]) | (stream[b] & stream[c]) | (stream[c] & stream[a]));
    }
    return stream;
}

/* Decode 2/3 rate FEC, a (15,10) shortened Hamming code */
void bluetooth_block::unfec23(char *stream, char *output, int length)
{
	/* stream points to the stream of data
	 * length is length in bits of the data
	 * before it was encoded with fec2/3 */
	int count, pointer;
	pointer = -5;

	//FIXME do error correction instead of ignoring the check digits
	for(count = 0; count < length; count++)
	{
		if((count % 10) == 0)
			pointer += 5;

		output[count] = stream[pointer++];
	}
}

/* When passed 10 bits of data this returns a pointer to a 5 bit hamming code */
char *bluetooth_block::fec23gen(char *data)
{
	char* codeword;
	uint8_t reg, counter;
	char bit;

	codeword = malloc(5);
	if(NULL==codeword)
		return codeword;

	for(counter=0;counter<10;counter++) {
		bit = (reg ^ data[counter]) & 1;

		reg = (reg >> 1) | bit<<4;

		reg ^= bit;

		reg ^= bit<<2;
	}

	for(counter=0;counter<5;counter++) {
		codeword[counter] = reg & 1;
		reg >>= 1;
	}

	return codeword;
}

/* Create an Access Code from LAP and check it against stream */
bool bluetooth_block::check_ac(char *stream, int LAP)
{
	int count, aclength;
	uint8_t *ac, *grdata;
	aclength = 72;

	/* Generate AC */
	ac = acgen(LAP);

	/* Check AC */
	/* Convert it to grformat, 1 bit per byte, in the LSB */
	grdata = (uint8_t *) malloc(aclength);

	for(count = 0; count < 9; count++)
		convert_to_grformat(ac[count], &grdata[count*8]);
	free(ac);

	for(count = 0; count < aclength; count++)
	{
		if(grdata[count] != stream[count])
		{
			//FIXME do error correction instead of giving up on the first wrong bit
			free(grdata);
			return false;
		}
	}

	free(grdata);
	return true;
}

void bluetooth_block::print_out()
{
	printf("LAP:%06x UAP:%02x\nType: ", d_LAP, d_UAP);
	switch(d_packet_type)
	{
		case 0:printf("NULL Slots:1"); break;
		case 1:printf("DV Slots:1"); break;
		case 2:printf("DH1 Slots:1"); break;
		case 3:printf("EV4 Slots:3"); break;
		case 4:printf("FHS Slots:1"); break;
		case 5:printf("DM3 Slots:3"); break;
		case 6:printf("HV2 Slots:1"); break;
		case 7:printf("DM5 Slots:5"); break;
		case 8:printf("POLL Slots:1"); break;
		case 9:printf("AUX1 Slots:1"); break;
		case 10:printf("HV1 Slots:1"); break;
		case 11:printf("EV5 Slots:3"); break;
		case 12:printf("DM1 Slots:1"); break;
		case 13:printf("DH3 Slots:3"); break;
		case 14:printf("HV3/EV3 Slots:1"); break;
		case 15:printf("DH5 Slots:5"); break;
	}
	printf(" Length:%d\n\n", d_payload_size);
}

/* Convert some number of bits of an air order array to a host order integer */
uint8_t bluetooth_block::air_to_host8(char *air_order, int bits)
{
	int i;
	uint8_t host_order = 0;
	for (i = 0; i < bits; i++)
		host_order |= (air_order[i] << i);
	return host_order;
}
uint16_t bluetooth_block::air_to_host16(char *air_order, int bits)
{
	int i;
	uint16_t host_order = 0;
	for (i = 0; i < bits; i++)
		host_order |= (air_order[i] << i);
	return host_order;
}
uint32_t bluetooth_block::air_to_host32(char *air_order, int bits)
{
	int i;
	uint32_t host_order = 0;
	for (i = 0; i < bits; i++)
		host_order |= (air_order[i] << i);
	return host_order;
}

/* Convert some number of bits in a host order integer to an air order array */
void bluetooth_block::host_to_air(uint8_t host_order, char *air_order, int bits)
{
    int i;
    for (i = 0; i < bits; i++)
        air_order[i] = (host_order >> i) & 0x01;
}

/* Remove the whitening from an air order array */
void bluetooth_block::unwhiten(char* input, char* output, int clock, int length, int skip)
{
	int count, index;
	index = d_indices[clock & 0x3f];
	index += skip;
	index %= 127;

	for(count = 0; count < length; count++)
	{
		output[count] = input[count] ^ d_whitening_data[index];
		index += 1;
		index %= 127;
	}
}

/* Pointer to start of packet, length of packet in bits, UAP */
uint16_t bluetooth_block::crcgen(char *packet, int length, int UAP)
{
	char byte;
	uint16_t reg, count;

	reg = UAP & 0xff;
	for(count = 0; count < length; count++)
	{
		byte = packet[count];

		reg = (reg << 1) | (((reg & 0x8000)>>15) ^ (byte & 0x01));

		/*Bit 5*/
		reg ^= ((reg & 0x0001)<<5);

		/*Bit 12*/
		reg ^= ((reg & 0x0001)<<12);
	}
	return reg;
}
