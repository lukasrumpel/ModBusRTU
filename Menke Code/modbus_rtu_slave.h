/*
*	Author:				Niklas Menke - https://www.niklas-menke.de/ (GE)
*	Description:		This library provide some functions for the Modbus RTU protocol. The FC03 (Read Holding Registers) and FC16 (Preset Multiple Registers) are supported.
*						Also there are functions to read and write the registers and to provide the break between two telegrams.
*						This library is designed for the usi-uart library ( https://www.niklas-menke.de/ (GE)).
*
*	File:				modbus_rtu_slave.h
*	Description:		Functions declaration
*	Microcontroller:	Attiny861A@8MHz; No other microcontrollers were tested
*	Date (created):		Mar 11, 2021
*
*	Version (Hardware):	-
*	Version (Software):	1.0.0
*	Date (updated):		-
*	Change log:			-
*	ToDo:				- (Ideas or Problems? --> https://www.niklas-menke.de/kontakt/ (GE))
*
*
*	WARNING!:	This code was only test with the Attiny861A controller and a CPU frequency of 8MHz. Support is not guaranteed for other conditions!
*				To get a CPU frequency of 8MHz, you have to clear the clock divider fuse (LOW.CKDIV8).
*/


#ifndef MODBUS_RTU_H_
#define MODBUS_RTU_H_


#include <avr/io.h>
#include <avr/interrupt.h>
#include "usi_uart.h"

uint16_t rtu_read(const uint8_t regID);						// Read a register
void rtu_write(const uint8_t regID, const uint16_t data);	// Write a register

void rtu_setAddress(void);										// Get the address from the coding switches
void rtu_break(float duration);									// Break for a specific symbol duration
uint16_t rtu_crc(const uint8_t *data, const uint8_t length);	// Calculate CRC-16/Modbus checksum

void rtu_response(void);	// Transmit a response

#endif /* MODBUS_RTU_H_ */