/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: misc.c
 *      Project: SJA1000 driver
 *
 *       Author: ww
 *        $Date: 2004/10/06 14:50:28 $
 *    $Revision: 1.7 $
 *
 *  Description: Misc functions
 *
 *     Required: -
 *     Switches: -
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: misc.c,v $
 * Revision 1.7  2004/10/06 14:50:28  dpfeuffer
 * minor modifications for MDIS4/2004 conformity
 *
 * Revision 1.6  2003/09/23 10:49:49  ww
 * changed busoff handling and removed _SJA1000_rtr() function.
 *
 * Revision 1.5  2001/04/20 16:21:44  ww
 * included RCS_ID switch
 *
 * Revision 1.4  2001/01/23 14:59:41  ww
 * included debug output
 *
 * Revision 1.3  2000/02/10 12:25:19  ww
 * changed _SJA1000_enable() from void to int32
 *
 * Revision 1.2  1999/08/17 10:09:28  ww
 * new function added
 *
 * Revision 1.1  1999/08/03 10:55:39  ww
 * Initial Revision
 *---------------------------------------------------------------------------
 * (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#include "sja1000_int.h"

/**************************** _SJA1000_clear_busoff ***************************
 *
 *  Description: Reset controller from busoff state
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level driver handle
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_clear_busoff(LL_HANDLE *llHdl)
{
	DBGWRT_2((DBH, "  _SJA1000_clear_busoff\n"));
	/*--------------------------+
    |  clear init               |
    +--------------------------*/
	_SJA1000_reset_mode( llHdl );

	_WRITE_D8(llHdl->ma,CommandReg,AT_Bit);			/* ed2, abort transmission */
	_WRITE_D8(llHdl->ma,TxErrCountReg,16);			/* clear tx error counter */

	_SJA1000_operating_mode( llHdl );

	if ( _READ_D8(llHdl->ma,StatusReg) &  BS_Bit)	/* ed2 */
		return CPL_RECOVER_BUSOFF;					/* ed2 */
	else											/* ed2 */
		return 0;
}

/******************************* _SJA1000_transmit_abort ********************
 *
 *  Description:  Abort a transmission which has not been completed
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level handle
 *               nr			message object number (1..14)
 *  Output.....: return     0 or error code
 *							CPL_ERR_BADMSGNUM    illegal message object number
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_transmit_abort( LL_HANDLE *llHdl, u_int32 nr )
{
	DBGWRT_2((DBH, "  _SJA1000_transmit_abort nr=%d\n", nr));
#if	0
	if( (nr==0) || (nr >= NUM_MSG_OBJ) )
		return CPL_ERR_BADMSGNUM;
#endif
	/*------------------------------+
    |  clear send request           |
    +------------------------------*/
	_WRITE_D8(llHdl->ma,CommandReg,AT_Bit);

	llHdl->xmtHalt = TRUE;
	return 0;
}

/**************************** _SJA1000_irq_enable ***************************
 *
 *  Description: Enable/Disable interrupts
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level driver handle
 *               val			irq (0=disable, 1=enable)
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/

void _SJA1000_irq_enable( LL_HANDLE *llHdl,u_int32 val)
{
	MACCESS ma = llHdl->ma;

	DBGWRT_2((DBH, "  _SJA1000_irq_enable val=%d\n", val));
	DBGWRT_2((DBH, "  irq mask=0x%x\n", llHdl->mask));

	if (val) {
		if (llHdl->irqFlag == 0) {
			_WRITE_D8(ma,InterruptEnReg,llHdl->mask);
			llHdl->irqFlag = 1;
		}
	}
	else {
		if (llHdl->irqFlag == 1) {
			_WRITE_D8(ma,InterruptEnReg,0);
			llHdl->irqFlag = 0;
		}
	}
}

/********************************** _SJA1000_chech_reset_mode ***************
 *
 *  Description:  test if the controller is in the reset mode
 *
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl		low level handle
 *  Output.....:  ---
 *  Globals....:  ---
 ****************************************************************************/

int32 _SJA1000_check_reset_mode( LL_HANDLE *llHdl )
{
	MACCESS ma = llHdl->ma;
	u_int8	mode;

	DBGWRT_2((DBH, "  _SJA1000_check_reset_mode\n"));

	mode = _READ_D8(ma,ModeControlReg);

	DBGWRT_2((DBH, "  mode=0x%02x\n",mode));

	if ((mode & RM_Bit) == 0)
		return CPL_ERR_BADMODE;
	else
		return 0;
}

/********************************** _SJA1000_reset_mode *********************
 *
 *  Description:  put the controller into the reset mode
 *
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl		low level handle
 *  Output.....:  ---
 *  Globals....:  ---
 ****************************************************************************/

int32 _SJA1000_reset_mode( LL_HANDLE *llHdl )
{
	MACCESS ma = llHdl->ma;
	u_int32	cnt	= 1000;
	u_int8	mode;

	DBGWRT_2((DBH, "  _SJA1000_reset_mode\n"));

	while(cnt--)
	{
		mode = _READ_D8(ma,ModeControlReg);

		DBGWRT_2((DBH, "  mode=0x%02x\n",mode));

		if ((mode & RM_Bit) != 0)
			return 0;

		_SETMASK_D8(ma,ModeControlReg,RM_Bit);
	}
	return CPL_ERR_BADMODE;
}

/********************************** _SJA1000_operating_mode *****************
 *
 *  Description:  put the controller into normal mode
 *
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl		low level handle
 *  Output.....:  ---
 *  Globals....:  ---
 ****************************************************************************/

int32 _SJA1000_operating_mode( LL_HANDLE *llHdl )
{
	MACCESS ma 	= llHdl->ma;
	u_int32	cnt	= 1000;
	u_int8	mode;

	DBGWRT_2((DBH, "  _SJA1000_operating_mode\n"));

	while(cnt--)
	{
		mode = _READ_D8(ma,ModeControlReg);

		DBGWRT_2((DBH, "  mode=0x%02x\n",mode));

		if ((mode & RM_Bit) == 0)
			return 0;

		_CLRMASK_D8(ma,ModeControlReg,RM_Bit);
	}
	return CPL_ERR_BADMODE;
}

/**************************** _SJA1000_enable *******************************
 *
 *  Description: Enable/Disable controller from bus activity
 *
 *               While bus activity is disabled, no messages may
 *               transmitted or received.
 *               Transmitter line is then in recessive state.
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level driver handle
 *               enable		enable/disable bus activity (0=disable, 1=enable)
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_enable(LL_HANDLE *llHdl, u_int32 enable)
{
	int32 	error = ERR_SUCCESS;

	DBGWRT_2((DBH, "  _SJA1000_enable %d\n", enable));

	if (enable)
		error = _SJA1000_operating_mode( llHdl );
	else
		error = _SJA1000_reset_mode( llHdl );

	return (error);
}

#ifdef	NOTUSED
/******************************* _SJA1000_rtr *******************************
 *
 *  Description:  Start RTR frame for receive object
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level handle
 *               nr			message object number (1)
 *  Output.....: return     0 or error code
 *							CPL_ERR_BADMSGNUM    illegal message object number
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_rtr( LL_HANDLE *llHdl, u_int32 nr )
{
	DBGWRT_2((DBH, "  _SJA1000_rtr nr=%d\n", nr));

	if( (nr==0) || (nr >= NUM_MSG_OBJ) )
		return CPL_ERR_BADMSGNUM;

	return 0;
}
#endif
