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
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: init.c,v $
 * Revision 1.9  2004/10/06 14:50:26  dpfeuffer
 * minor modifications for MDIS4/2004 conformity
 *
 * Revision 1.8  2003/09/23 10:46:08  ww
 * clear tx/rx error counters
 *
 * Revision 1.7  2001/04/20 16:21:42  ww
 * included RCS_ID switch
 *
 * Revision 1.6  2001/01/25 11:28:54  ww
 * changed type of error variable from error_code to int32
 *
 * Revision 1.5  2001/01/23 14:59:39  ww
 * included more debug outputs
 *
 * Revision 1.4  1999/08/20 10:45:50  ww
 * changed mode in function _SJA1000_set_acccode from 3 to 1
 *
 * Revision 1.3  1999/08/18 10:04:24  ww
 * set rtr bit in code and mask
 *
 * Revision 1.2  1999/08/17 10:17:21  ww
 * added dual filer flag
 *
 * Revision 1.1  1999/08/03 10:55:38  ww
 * Initial Revision
 *---------------------------------------------------------------------------
 * (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

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
	_READ_D8((MACCESS)((u_int32)ma&~0xff),ModeControlReg);
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
