/***********************  I n c l u d e  -  F i l e  ************************
 *  
 *         Name: sja1000.h
 *
 *       Author: ww
 *        $Date: 2002/07/24 14:32:44 $
 *    $Revision: 2.5 $
 * 
 *  Description: CAN-Bus controller SJA1000 register layout
 *                      
 *     Switches: BasicCanMode
 *
 *---------------------------------------------------------------------------
 * Copyright (c) 1997-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/
/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#ifndef BASIC_CAN_MODE
#	define	PeliCANMode
#endif

/* address and bit definitions for the Mode & Control Register */
#define ModeControlReg	0
#define RM_RR_Bit 		0x01	/* reset mode (request) bit */
#define	RM_Bit			0x01

#if defined (PeliCANMode)
#define	LOM_Bit			0x02 	/* listen only mode bit */
#define STM_Bit 		0x04 	/* self test mode bit */
#define AFM_Bit 		0x08 	/* acceptance filter mode bit */
#define SM_Bit 			0x10 	/* enter sleep mode bit */
#endif

/* address and bit definitions for the Command Register */
#define CommandReg 		1
#define TR_Bit 			0x01 	/* transmission request bit */
#define AT_Bit 			0x02 	/* abort transmission bit */
#define RRB_Bit 		0x04 	/* release receive buffer bit */
#define CDO_Bit 		0x08 	/* clear data overrun bit */
#if defined (PeliCANMode)
#define SRR_Bit 		0x10 	/* self reception request bit */

#else /* BasicCAN mode */
#define GTS_Bit 		0x10 	/* goto sleep bit (BasicCAN mode) */
#endif

/* address and bit definitions for the Status Register */
#define StatusReg 		2
#define RBS_Bit 		0x01 	/* receive buffer status bit */
#define DOS_Bit 		0x02 	/* data overrun status bit */
#define TBS_Bit 		0x04 	/* transmit buffer status bit */
#define TCS_Bit		 	0x08 	/* transmission complete status bit */
#define RS_Bit 			0x10 	/* receive status bit */
#define TS_Bit 			0x20 	/* transmit status bit */
#define ES_Bit 			0x40 	/* error status bit */
#define BS_Bit 			0x80 	/* bus status bit */

/* address and bit definitions for the Interrupt Register */
#define InterruptReg 	3
#define RI_Bit 			0x01 	/* receive interrupt bit */
#define TI_Bit 			0x02 	/* transmit interrupt bit */
#define EI_Bit 			0x04 	/* error warning interrupt bit */
#define DOI_Bit 		0x08 	/* data overrun interrupt bit */
#define WUI_Bit 		0x10 	/* wake-up interrupt bit */
#if defined (PeliCANMode)
#define EPI_Bit 		0x20 	/* error passive interrupt bit */
#define ALI_Bit 		0x40 	/* arbitration lost interrupt bit */
#define BEI_Bit 		0x80 	/* bus error interrupt bit */
#endif

/* address and bit definitions for the 
   Interrupt Enable & Control Register
*/
#if defined (PeliCANMode)
#define InterruptEnReg 	4		/* PeliCAN mode */
#define RIE_Bit			0x01 	/* receive interrupt enable bit */
#define TIE_Bit 		0x02 	/* transmit interrupt enable bit */
#define EIE_Bit 		0x04 	/* error warning interrupt enable bit */
#define DOIE_Bit 		0x08 	/* data overrun interrupt enable bit */
#define WUIE_Bit 		0x10 	/* wake-up interrupt enable bit */
#define EPIE_Bit 		0x20 	/* error passive interrupt enable bit */
#define ALIE_Bit 		0x40 	/* arbitration lost interr. enable bit*/
#define BEIE_Bit 		0x80 	/* bus error interrupt enable bit */

#else /* BasicCAN mode */
#define InterruptEnReg	0 		/* Control Register */
#define RIE_Bit 		0x02 	/* Receive Interrupt enable bit */
#define TIE_Bit 		0x04 	/* Transmit Interrupt enable bit */
#define EIE_Bit 		0x08 	/* Error Interrupt enable bit */
#define DOIE_Bit 		0x10 	/* Overrun Interrupt enable bit */
#endif

#if defined (PeliCANMode)
#define	Reserved1		5
#endif

/* address and bit definitions for the Bus Timing Registers */
#define BusTiming0Reg	6
#define BusTiming1Reg	7
#define SAM_Bit 		0x80 	/* sample mode bit
								1 == the bus is sampled 3 times
								0 == the bus is sampled once */

/* address and bit definitions for the Output Control Register */
#define OutControlReg 	8
/* OCMODE1, OCMODE0 */
#define BiPhaseMode		0x00 	/* bi-phase output mode */
#define NormalMode 		0x02 	/* normal output mode */
#define ClkOutMode 		0x03 	/* clock output mode */

/* output pin configuration for TX1 */
#define OCPOL1_Bit 		0x20 	/* output polarity control bit */
#define Tx1Float 		0x00 	/* configured as float */
#define Tx1PullDn 		0x40 	/* configured as pull-down */
#define Tx1PullUp 		0x80 	/* configured as pull-up */
#define Tx1PshPull 		0xC0 	/* configured as push/pull */

/* output pin configuration for TX0 */
#define OCPOL0_Bit 		0x04 	/* output polarity control bit */
#define Tx0Float 		0x00 	/* configured as float */
#define Tx0PullDn 		0x08 	/* configured as pull-down */
#define Tx0PullUp 		0x10 	/* configured as pull-up */
#define Tx0PshPull 		0x18 	/* configured as push/pull */

#if defined (PeliCANMode)
#define TestReg 		9
#define	Reserved2		10
#endif

/* address definitions of Other Registers */
#if defined (PeliCANMode)
#define ArbLostCapReg 	11
#define ErrCodeCapReg 	12
#define ErrWarnLimitReg 13
#define RxErrCountReg 	14
#define TxErrCountReg 	15
#define RxMsgCountReg 	29
#define RxBufStartAdr 	30
#endif

/* address definitions of Acceptance Code & Mask Registers */
#if defined (PeliCANMode)
#define AcceptCode0Reg 	16
#define AcceptCode1Reg 	17
#define AcceptCode2Reg 	18
#define AcceptCode3Reg 	19
#define AcceptMask0Reg 	20
#define AcceptMask1Reg 	21
#define AcceptMask2Reg 	22
#define AcceptMask3Reg 	23
#else /* BasicCAN mode */
#define AcceptCodeReg 	4
#define AcceptMaskReg 	5
#endif
/* address definitions of the Rx-Buffer */
#if defined (PeliCANMode)
#define RxFrameInfo 	16
#define RxBuffer1 		17
#define RxBuffer2 		18
#define RxBuffer3 		19
#define RxBuffer4 		20
#define RxBuffer5 		21
#define RxBuffer6 		22
#define RxBuffer7 		23
#define RxBuffer8 		24
#define RxBuffer9 		25
#define RxBuffer10 		26
#define RxBuffer11 		27
#define RxBuffer12 		28
#else /* BasicCAN mode */
#define RxBuffer1 		20
#define RxBuffer2 		21
#define RxBuffer3 		22
#define RxBuffer4 		23
#define RxBuffer5 		24
#define RxBuffer6 		25
#define RxBuffer7 		26
#define RxBuffer8 		27
#define RxBuffer9 		28
#define RxBuffer10 		29
#endif
/* address definitions of the Tx-Buffer */
#if defined (PeliCANMode)
/* write only addresses */
#define TxFrameInfo 	16
#define	FF_Bit			0x80
#define	RTR_Bit			0x40
#define	DLC_Mask	 	0x0f
#define	DLC0			0x01
#define	DLC1			0x02
#define	DLC2			0x04
#define	DLC3			0x08
#define TxBuffer1 		17
#define TxBuffer2 		18
#define TxBuffer3 		19
#define TxBuffer4 		20
#define TxBuffer5 		21
#define TxBuffer6 		22
#define TxBuffer7 		23
#define TxBuffer8 		24
#define TxBuffer9 		25
#define TxBuffer10		26
#define TxBuffer11 		27
#define TxBuffer12 		28
/* read only addresses */
#define TxFrameInfoRd 	96
#define TxBufferRd1 	97
#define TxBufferRd2 	98
#define TxBufferRd3 	99
#define TxBufferRd4 	100
#define TxBufferRd5 	101
#define TxBufferRd6 	102
#define TxBufferRd7 	103
#define TxBufferRd8 	104
#define TxBufferRd9 	105
#define TxBufferRd10 	106
#define TxBufferRd11 	107
#define TxBufferRd12 	108
#else /* BasicCAN mode */
#define TxBuffer1 		10
#define TxBuffer2 		11
#define TxBuffer3 		12
#define TxBuffer4 		13
#define TxBuffer5 		14
#define TxBuffer6 		15
#define TxBuffer7 		16
#define TxBuffer8 		17
#define TxBuffer9 		18
#define TxBuffer10 		19
#endif

/* address and bit definitions for the Clock Divider Register */
#define ClockDivideReg 	31
#define DivBy1 			0x07 	/* CLKOUT = oscillator frequency */
#define DivBy2 			0x00 	/* CLKOUT = 1/2 oscillator frequency */
#define ClkOff_Bit 		0x08 	/* clock off bit, control of the CLK OUT pin */
#define RXINTEN_Bit 	0x20 	/* pin TX1 used for receive interrupt */
#define CBP_Bit 		0x40 	/* CAN comparator bypass control bit */
#define CANMode_Bit 	0x80 	/* CAN mode definition bit */

#define	BW8
/* bus width 8 */
#ifdef	BW8
#define	BW			u_int8
#define	PAD(_x_)	_x_	
#else
#define	BW			u_int16
#define	PAD(_x_)
#endif

typedef struct {

	BW	PAD(pad00);		BW	control;			/* rw, operating and reset mode */
	BW	PAD(pad01);		BW	command;			/* -w, operating and reset mode */
	BW	PAD(pad02);		BW	status;				/* r-, operating and reset mode */
	BW	PAD(pad03);		BW	interrupt;			/* r-, operating and reset mode */
	BW	PAD(pad04);		BW	acceptance_code;	/* rw, reset mode */
	BW	PAD(pad05);		BW	acceptance_mask;	/* rw, reset mode */
	BW	PAD(pad06);		BW	bus_timing_0;		/* rw, reset mode */
	BW	PAD(pad07);		BW  bus_timing_1;		/* rw, reset mode */
	BW	PAD(pad08);		BW	output_control;		/* rw, reset mode */
	BW	PAD(pad09);		BW	test;

	BW	PAD(pad10);		BW	tx_identifier_h;	/* rw, operating mode */
	BW	PAD(pad11);		BW	tx_identifier_l;	/* rw, operating mode */
	BW	PAD(pad12);		BW	tx_data_1;			/* rw, operating mode */
	BW	PAD(pad13);		BW	tx_data_2;			/* rw, operating mode */
	BW	PAD(pad14);		BW	tx_data_3;			/* rw, operating mode */
	BW	PAD(pad15);		BW	tx_data_4;			/* rw, operating mode */
	BW	PAD(pad16);		BW	tx_data_5;			/* rw, operating mode */
	BW	PAD(pad17);		BW	tx_data_6;			/* rw, operating mode */
	BW	PAD(pad18);		BW	tx_data_7;			/* rw, operating mode */
	BW	PAD(pad19);		BW	tx_data_8;			/* rw, operating mode */
	
	BW	PAD(pad20);		BW	rx_identifier_h;	/* rw, operating and reset mode */
	BW	PAD(pad21);		BW	rx_identifier_l;	/* rw, operating and reset mode */
	BW	PAD(pad22);		BW	rx_data_1;			/* rw, operating and reset mode */
	BW	PAD(pad23);		BW	rx_data_2;			/* rw, operating and reset mode */
	BW	PAD(pad24);		BW	rx_data_3;			/* rw, operating and reset mode */
	BW	PAD(pad25);		BW	rx_data_4;			/* rw, operating and reset mode */
	BW	PAD(pad26);		BW	rx_data_5;			/* rw, operating and reset mode */
	BW	PAD(pad27);		BW	rx_data_6;			/* rw, operating and reset mode */
	BW	PAD(pad28);		BW	rx_data_7;			/* rw, operating and reset mode */
	BW	PAD(pad29);		BW	rx_data_8;			/* rw, operating and reset mode */
	BW	PAD(pad30);		BW	rsrvd;
	BW	PAD(pad31);		BW	clock_divider;		/* rw, operating and reset mode */
	
} BasicCAN;


typedef struct {

	BW	PAD(pad00);		BW	control;			/* rw, operating and reset mode */
	BW	PAD(pad01);		BW	command;			/* -w, operating and reset mode */
	BW	PAD(pad02);		BW	status;				/* r-, operating and reset mode */
	BW	PAD(pad03);		BW	interrupt;			/* r-, operating and reset mode */
	BW	PAD(pad04);		BW	interrupt_enable;	/* rw, operating and reset mode */
	BW	PAD(pad05);		BW	reserved_1;			/* r-, reset mode */
	BW	PAD(pad06);		BW	bus_timing_0;		/* rw, reset mode */
	BW	PAD(pad07);		BW  bus_timing_1;		/* rw, reset mode */
	BW	PAD(pad08);		BW	output_control;		/* rw, reset mode */
	BW	PAD(pad09);		BW	test;

	BW	PAD(pad10);		BW	reserved_2;					/* r-, operating mode */
	BW	PAD(pad11);		BW	arbitration_lost_capture;	/* rw, operating mode */
	BW	PAD(pad12);		BW	error_code_capture;			/* rw, operating mode */
	BW	PAD(pad13);		BW	error_warning_limit;		/* rw, operating mode */
	BW	PAD(pad14);		BW	rx_error_counter;			/* rw, operating mode */
	BW	PAD(pad15);		BW	tx_error_counter;			/* rw, operating mode */
	BW	PAD(pad16);		BW	rx_fram_information;		/* rw, operating mode */
	BW	PAD(pad17);		BW	rx_identifier_1;		/* rw, operating mode */
	BW	PAD(pad18);		BW	rx_idendifier_2;		/* rw, operating mode */
	BW	PAD(pad19);		BW	rx_idendifier_3;		/* rw, operating mode */
	
	BW	PAD(pad20);		BW	rx_idendifier_4;		/* rw, operating and reset mode */
	BW	PAD(pad21);		BW	rx_data_1;				/* rw, operating and reset mode */
	BW	PAD(pad22);		BW	rx_data_2;				/* rw, operating and reset mode */
	BW	PAD(pad23);		BW	rx_data_3;				/* rw, operating and reset mode */
	BW	PAD(pad24);		BW	rx_data_4;				/* rw, operating and reset mode */
	BW	PAD(pad25);		BW	rx_data_5;				/* rw, operating and reset mode */
	BW	PAD(pad26);		BW	rx_data_6;				/* rw, operating and reset mode */
	BW	PAD(pad27);		BW	rx_data_7;				/* rw, operating and reset mode */
	BW	PAD(pad28);		BW	rx_data_8;				/* rw, operating and reset mode */
	BW	PAD(pad29);		BW	rx_message_counter;		/* rw, operating and reset mode */
	BW	PAD(pad30);		BW	rx_buffer_start_address;
	BW	PAD(pad31);		BW	clock_divider;			/* rw, operating and reset mode */

} PeliCAN;
