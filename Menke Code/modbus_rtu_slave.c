/*
*	Author:				Niklas Menke - https://www.niklas-menke.de/ (GE)
*	Description:		This library provide some functions for the Modbus RTU protocol. The FC03 (Read Holding Registers) and FC16 (Preset Multiple Registers) are supported.
*						Also there are functions to read and write the registers and to provide the break between two telegrams.
*						This library is designed for the usi-uart library ( https://www.niklas-menke.de/ (GE)).
*
*	File:				modbus_rtu_slave.c
*	Description:		Functions definition
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

#include "modbus_rtu_slave.h"

static uint16_t registers[2];	// Storage for the registers
static uint8_t deviceAddress;	// Modbus RTU address for this device



// ----- Read a register BEGIN -----
uint16_t rtu_read(const uint8_t regID) {
	if(regID < 2) return registers[regID];
	else return 0;
}
// ----- Read a register END -----



// ----- Write a register BEGIN -----
void rtu_write(const uint8_t regID, const uint16_t data) {
	if(regID < 2) registers[regID] = data;
}
// ----- Write a register END -----


	
// ----- Get the address from the coding switches BEGIN -----
void rtu_setAddress(void) {
	// Set pins as input
	DDRA &= 0xc2;
	DDRB &= 0x97;
	
	// Enable pullups
	PORTA |= 0x3d;
	PORTB |= 0x68;
	
	deviceAddress = ~(((PINA&0x0c)<<4) | ((PINA&0x01)<<5) | ((PINB&0x08)<<1) | ((PINA&0x20)>>2) | ((PINB&0x20)>>3) | ((PINA&0x10)>>3) | ((PINB&0x40)>>6));	// Get each bit of address in correct order and toogle bits
}
// ----- Get the address from the coding switches END -----



// ----- Break for a specific symbol duration BEGIN -----
void rtu_break(float duration) {
	uint32_t counter = 0, setpoint = F_CPU/8/uart_getBaudrate()*11*duration;	// F_CPU/BAUDRATE = Duration of one bit ; 8 = Duration of one iteration of the while loop ; 11 = Number of bits (8 data bit, 1 start bit, 2 stop bits) ; duration = Number of symbols
	while(counter < setpoint) counter++;
}
// ----- Break for a specific symbol duration END -----



// ----- Calculate CRC-16/Modbus checksum BEGIN -----
uint16_t rtu_crc(const uint8_t *data, const uint8_t length) {
	uint16_t crc = ~0;
	for(uint8_t i = 0; i < length; i++) {
		crc ^= data[i];
		for(uint8_t b = 8; b > 0; b--) {
			if((crc & 1) != 0) crc = (crc>>1)^0xA001;
			else crc >>= 1;
		}
	}
	return crc;
}
// ----- Calculate CRC-16/Modbus checksum END -----



// ----- Transmit a response BEGIN -----
void rtu_response(void) {
	uart_stop();	// UART is not available to receive bytes
	if(uart_readByte() != deviceAddress || uart_available() < 5) uart_delete();	// The device address is incorrect (Message is not for this device) or the minimal length of a request is not reached --> Ignore request
	else {
		// Get all bytes from the buffer
		uint8_t request_length = uart_available();
		uint8_t request[request_length];
		for(uint8_t i = 0; i < request_length; i++) request[i] = uart_getByte();
		
		if(rtu_crc(request, request_length)) uart_delete();	// CRC-Checksum is incorrect --> Ignore request
		else {
			if(request[1] == 0x03) {	// FC03: Read holding registers
				if(request[2] || request[4] || (request[3] > 1) || (request[3] + request[5] > 2)) {	// EXCEPTION: Illegal data address: Only register 0x00 and 0x01 is available
					uint8_t response[5] = {request[0], 0x83, 0x02};	// Response array: Device address, Function code with MSB=1 and exception code 0x02 (Illegal Data address)
						
					// Calculate CRC-Checksum of the response and add it to the response array
					uint16_t crc = rtu_crc(response,3);
					response[3] = crc;
					response[4] = crc>>8;
					
					uart_transmitArray(response,5); // Transmit response
					rtu_break(3.5);					// Break after a telegram
				} // Illegal data address END
				else {
					uint8_t responseLength = 2*request[5]+5;	// Number of bytes of the response
					uint8_t response[responseLength];			// Response array: Device address, function code, number of data bytes
					response[0] = request[0];					// Device address
					response[1] = 0x03;							// Function code
					response[2] = 2*request[5];					// Number of data bytes
					
					// Add content of the requested registers to the response array
					uint8_t responseID = 3;
					for(uint8_t i = request[3]; i < request[3] + request[5]; i++) {
						response[responseID++] = (rtu_read(i)>>8);
						response[responseID++] = rtu_read(i);
					}
					
					// Calculate CRC-Checksum of the response and add it to the response array
					uint16_t crc = rtu_crc(response,responseLength-2);
					response[responseLength-2] = crc;
					response[responseLength-1] = crc>>8;
					
					uart_transmitArray(response,responseLength);	// Transmit response
					rtu_break(3.5);									// Break after a telegram
				} // Legal data address END
			} // FC03 END
			else if(request[1] == 0x08) {	// FC08: Diagnostics
				if(request[2] || request[3]) {	// EXCEPTION: Illegal sub function code: Only sub function 0x00 (Return query data) is available
					uint8_t response[5] = {request[0], 0x88, 0x01};	// Response array: Device address, function code with MSB=1 and exception code 0x01 (Illegal function code)
						
					// Calculate CRC-Checksum of the response and add it to the response array
					uint16_t crc = rtu_crc(response,3);
					response[3] = crc;
					response[4] = crc>>8;
					
					uart_transmitArray(response,5); // Transmit response
					rtu_break(3.5);					// Break after a telegram
				} // Illegal function code END
				else {
					for(uint8_t i = 0; i < request_length; i++) uart_transmit(request[i]);	// Return the request
					rtu_break(3.5);															// Break after a telegram
				} // Legal function code END
			} // FC08 END
			else if(request[1] == 0x10) {	// FC16: Preset Multiple Registers
				if(request[2] || request[4] || (request[3] > 1) || (request[3] + request[5] > 2) || (2*request[5] != request[6])) {	// EXCEPTION: Illegal data address: Only register 0x00 and 0x01 is available. Or number of registers is incompatible to number of bytes
					uint8_t response[5] = {request[0], 0x90, 0x02};	// Response array: Device address, Function code with MSB=1 and exception code 0x02 (Illegal Data address)
					
					// Calculate CRC-Checksum of the response and add it to the response array
					uint16_t crc = rtu_crc(response,3);
					response[3] = crc;
					response[4] = crc>>8;
					
					uart_transmitArray(response,5); // Transmit response
					rtu_break(3.5);					// Break after a telegram
				} // Illegal function code END
				else {
					for(uint8_t i = request[3], j = 7; i < request[3] + request[5]; i++, j+=2) rtu_write(i, request[j]<<8 | request[j+1]);	// Write received data into registers
					
					uint8_t response[8] = {request[0], 0x10, 0x00, request[3], 0x00, request[5]};	// Response Array: Device address, function code, data address of first register and number of written registers
						
					// Calculate CRC-Checksum of the response and add it to the response array
					uint16_t crc = rtu_crc(response,6);
					response[6] = crc;
					response[7] = crc>>8;
					
					uart_transmitArray(response,8); // Transmit response
					rtu_break(3.5);					// Break after a telegram
				}
			} // FC16 END
			else {	// EXCEPTION: Function code is not supported
				uint8_t response[5] = {request[0], request[1]|0x80, 0x01};	// Response array: Device address, Function code with MSB=1 and exception code 0x01 (Illegal function code)
				
				// Calculate CRC-Checksum of the response and add it to the response array
				uint16_t crc = rtu_crc(response,3);
				response[3] = crc;
				response[4] = crc>>8;
				
				uart_transmitArray(response,5); // Transmit response
				rtu_break(3.5);					// Break after a telegram
			} // Illegal function code END
		} // Correct CRC-Checksum END
	} // Correct device address and length END
	uart_listen(); // Listen for a new telegram
}
// ----- Transmit a response END -----