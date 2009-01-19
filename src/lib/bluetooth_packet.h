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
#ifndef INCLUDED_BLUETOOTH_PACKET_H
#define INCLUDED_BLUETOOTH_PACKET_H

#include <stdint.h>
#include <boost/enable_shared_from_this.hpp>

class bluetooth_packet;
typedef boost::shared_ptr<bluetooth_packet> bluetooth_packet_sptr;

/*!
 * \brief Return a shared_ptr to a new instance of bluetooth_packet.
 */
bluetooth_packet_sptr bluetooth_make_packet(char *stream, int length);

class bluetooth_packet
{
private:
	/* allow bluetooth_make_packet to access the private constructor. */
	friend bluetooth_packet_sptr bluetooth_make_packet(char *stream, int length);

	/* constructor */
	bluetooth_packet(char *stream, int length);

	/* maximum number of symbols */
	static const int MAX_SYMBOLS = 3125;

	/* index into whitening data array */
	static const uint8_t INDICES[64];

	/* whitening data */
	static const uint8_t WHITENING_DATA[127];

	/* lookup table for preamble hamming distance */
	static const uint8_t PREAMBLE_DISTANCE[32];

	/* lookup table for trailer hamming distance */
	static const uint8_t TRAILER_DISTANCE[2048];

	/* the raw symbol stream, one bit per char */
	char d_symbols[MAX_SYMBOLS];

	/* lower address part found in access code */
	uint32_t d_LAP;

	/* number of symbols */
	int d_length;

	/* type-specific CRC checks */
	//FIXME probably ought to use d_symbols, d_length
	int fhs(char *stream, int clock, uint8_t UAP, int size);
	int DM(char *stream, int clock, uint8_t UAP, int header_bytes, int size);
	int DH(char *stream, int clock, uint8_t UAP, int header_bytes, int size);
	int EV(char *stream, int clock, uint8_t UAP, int type, int size);

public:
	/* search a symbol stream to find a packet, return index */
	static int sniff_ac(char *stream, int stream_length);

	/* Error correction coding for Access Code */
	static uint8_t *lfsr(uint8_t *data, int length, int k, uint8_t *g);

	/* Reverse the bits in a byte */
	static uint8_t reverse(char byte);

	/* Generate Access Code from an LAP */
	static uint8_t *acgen(int LAP);

	/* Convert from normal bytes to one-LSB-per-byte format */
	static void convert_to_grformat(uint8_t input, uint8_t *output);

	/* Decode 1/3 rate FEC, three like symbols in a row */
	static char *unfec13(char *stream, char *output, int length);

	/* Decode 2/3 rate FEC, a (15,10) shortened Hamming code */
	static char *unfec23(char *input, int length);

	/* When passed 10 bits of data this returns a pointer to a 5 bit hamming code */
	static char *fec23gen(char *data);

	/* Create an Access Code from LAP and check it against stream */
	static bool check_ac(char *stream, int LAP);

	/* Convert some number of bits of an air order array to a host order integer */
	static uint8_t air_to_host8(char *air_order, int bits);
	static uint16_t air_to_host16(char *air_order, int bits);
	static uint32_t air_to_host32(char *air_order, int bits);
	// hmmm, maybe these should have pointer output so they can be overloaded

	/* Convert some number of bits in a host order integer to an air order array */
	static void host_to_air(uint8_t host_order, char *air_order, int bits);

	/* Remove the whitening from an air order array */
	static void unwhiten(char* input, char* output, int clock, int length, int skip);

	/* Create the 16bit CRC for packet payloads - input air order stream */
	static uint16_t crcgen(char *payload, int length, int UAP);

	/* extract UAP by reversing the HEC computation */
	static int UAP_from_hec(uint8_t *packet);

	/* check if the packet's CRC is correct */
	int crc_check(int type, int clock, uint8_t UAP);

	/* return the packet's LAP */
	uint32_t get_LAP();

	/* destructor */
	~bluetooth_packet();
};

#endif /* INCLUDED_BLUETOOTH_PACKET_H */