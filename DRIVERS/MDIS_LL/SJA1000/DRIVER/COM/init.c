/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: init.c
 *      Project: SJA1000 driver
 *
 *       Author: ww
 *        $Date: 2004/10/06 14:50:26 $
 *    $Revision: 1.9 $
 *
 *  Description: Init/terminate controller
 *
 *     Required: -
 *     Switches: -
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany
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

#include "sja1000_int.h"

/******************************* _SJA1000_init ********************************
 *
 *  Description: Init and reset controller
 *
 *               - disable rcv/xmt
 *               - check for controller ready
 *               - reset all registers
 *		         - config controller clock
 *
 *               (Bus activity is left disabled by this function)
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level driver handle
 *               sysclock	system clock [Hz]
 *  Output.....: canclock	resulting controller clock [Hz]
 *               return     0 or error code
 *							CPL_ERR_RESETACTIVE  controller in reset state
 *							CPL_ERR_BADCLOCK     illegal sysclock
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_init(LL_HANDLE *llHdl, u_int32 sysclock, u_int32 *canclock)
{
	MACCESS ma 	= llHdl->ma;

	u_int8 	val;
	int32	error;

	DBGWRT_1((DBH, "->_SJA1000_init sysclock=%d\n",sysclock));

	/*---------------------+
    | reset controller     |
    +---------------------*/
    /* read control register from first module. This read will allowed the
     *  access to the upper address array (80-ff) for the M51 module
     */
	_READ_D8((MACCESS)((U_INT32_OR_64)ma&~0xff),ModeControlReg);
#if	0
	_WRITE_D8(ma,ModeControlReg,RM_Bit);			/* enter reset */
#endif
	if( (error = _SJA1000_reset_mode(llHdl) ))
		goto exit;

	if (llHdl->cbpFlag)	val = CANMode_Bit|CBP_Bit;	/* peliCAN + bypass */
	else				val = CANMode_Bit;			/* peliCAN */

	_WRITE_D8(ma,ClockDivideReg,val);				/* enter PeliCAN Mode */
	_WRITE_D8(ma,InterruptEnReg,0);					/* disable interrupts */

	_WRITE_D8(ma,RxErrCountReg,0);					/* ed2 */
	_WRITE_D8(ma,TxErrCountReg,0);					/* ed2 */

	if (llHdl->dualFilterMode == 0) {
		_SETMASK_D8(ma,ModeControlReg,AFM_Bit);		/* enter Single Filter Mode */
	}
#ifdef	DBG
	if (val & CANMode_Bit) {
		DBGWRT_2((DBH, "  peliCan mode\n"));
	}
	if (val & CBP_Bit) {
		DBGWRT_2((DBH, "  bypass\n"));
	}
	if (llHdl->dualFilterMode) {
		DBGWRT_2((DBH, "  dual filter mode\n"));
	}
	else {
		DBGWRT_2((DBH, "  single filter mode\n"));
	}
#endif
	/* accept all message */
	_SJA1000_set_acccode(llHdl,1,0x00000000);	/* clear code */
	_SJA1000_set_accmask(llHdl,3,0xffffffff);	/* set mask */

	/*---------------------+
    | load interrupts mask |
    +---------------------*/
	DBGWRT_1((DBH, "  irq mask=0x%x\n",llHdl->mask));
    if (llHdl->mask == 0)
    {
		llHdl->mask =
#if	0
			BEIE_Bit| 	/* 7 bus error interrupt enable bit */
			ALIE_Bit| 	/* 6 arbitration lost interr. enable bit*/
			EPIE_Bit| 	/* 5 error passive interrupt enable bit */
			WUIE_Bit| 	/* 4 wake-up interrupt enable bit */
#endif
			DOIE_Bit| 	/* 3 data overrun interrupt enable bit */
			EIE_Bit |	/* 2 error warning interrupt enable bit */
			TIE_Bit | 	/* 1 transmit interrupt enable bit */
			RIE_Bit;	/* 0 receive interrupt enable bit */

		DBGWRT_1((DBH, "  irq mask=0x%x\n",llHdl->mask));
	}
	DBGWRT_1((DBH, "  irq standartmask=0x%x\n",llHdl->mask));
 	*canclock = sysclock;
#if	0
	DBGWRT_2((DBH, "  can clock=%d\n",sysclock));
#endif
exit:
	DBGWRT_1((DBH, "<-_SJA1000_init (%d)\n",error));
	return error;
}

/******************************* _SJA1000_term ********************************
 *
 *  Description: De-init controller
 *
 *               - switch off transmitter (recessive)
 *               - disable irqs
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level driver handle
 *  Output.....: return     0 or error code
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_term(LL_HANDLE *llHdl)
{
	MACCESS ma = llHdl->ma;

	/*--------------------------+
    |  switch off rcv/xmt       |
    +--------------------------*/

	/*--------------------------+
    |  disable irqs             |
    +--------------------------*/
	_WRITE_D8(ma,InterruptEnReg,0);

	return(0);
}
