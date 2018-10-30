/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: accmask.c
 *      Project: SJA1000 i/o library
 *
 *       Author: ww
 *        $Date: 2004/10/06 14:50:17 $
 *    $Revision: 1.7 $
 *
 *  Description: Set global acceptance mask
 *
 *     Required: -
 *     Switches: S_RESET
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
#define S_RESETno

/**************************** _SJA1000_set_accmask ****************************
 *
 *  Description: Set global acceptance mask for standard or extended ID's
 *
 *               S_RESET: (when setting extended acceptance code,
 *                         bus activity is temporarily disabled)
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level driver handle
 *               extended		message ID type
 *								0=standard, rtr bit clear
 *								1=extended, rtr bit clear
 *								2=standard, rtr bit set
 *								3=extended, rtr bit set
 *               mask		    global acceptance mask
 *                              (0=must match, 1=don't care)
 *  Output.....: return     	0
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_set_accmask( LL_HANDLE *llHdl,
							u_int32 extended,
							u_int32 acc_mask )
{
	MACCESS 	ma = llHdl->ma;

	DBGWRT_2((DBH, "  _SJA1000_set_accmask: ext=%d acc_mask=0x%08x\n",
			  extended, acc_mask ));

#ifdef	DBG
		if (extended&0x1) {
			DBGWRT_2((DBH, "  extended mode (29Bit)\n"));
		}
		else {
			DBGWRT_2((DBH, "  standart mode (11Bit)\n"));
		}
		if (extended&0x2) {
			DBGWRT_2((DBH, "  set rtr bit\n"));
		}
#endif
	llHdl->extended = extended;
	llHdl->accmask 	= acc_mask;

	/*--------------------------+
    |  config mask              |
    +--------------------------*/
#ifdef  S_RESET
	_SJA1000_reset_mode(llHdl);
#else
	if (_SJA1000_check_reset_mode(llHdl))
		return CPL_ERR_BADMODE;
#endif
	if (extended&0x1) {
		/*--------------------------+
		|  extended mask (29-bit)   |
		+--------------------------*/
		if (llHdl->dualFilterMode == 0) {
			/* single filter */
			acc_mask <<= 3;						/* adjust mask */
			acc_mask  |= ((extended&0x2)<<1);	/* set rtr bit */
			acc_mask  |= 0x3;	                /* compatible reason */

			_WRITE_D8(ma,AcceptMask0Reg,(acc_mask >> 24) & 0xff);	/*>>21*/
			_WRITE_D8(ma,AcceptMask1Reg,(acc_mask >> 16) & 0xff);	/*>>13*/
			_WRITE_D8(ma,AcceptMask2Reg,(acc_mask >>  8) & 0xff);	/*>> 5*/
			_WRITE_D8(ma,AcceptMask3Reg,(acc_mask      ) & 0xff);	/*<< 3*/
		}
		else {
			/* dual filter */

			/* filter 1 */
			_WRITE_D8(ma,AcceptMask0Reg,(acc_mask >>  8) & 0xff);
			_WRITE_D8(ma,AcceptMask1Reg,(acc_mask      ) & 0xff);

			/* filter 2 */
			_WRITE_D8(ma,AcceptMask2Reg,(acc_mask >> 24) & 0xff);
			_WRITE_D8(ma,AcceptMask3Reg,(acc_mask >> 16) & 0xff);
		}
	}
	else {
		/*--------------------------+
		|  standard mask (11-bit)   |
		+--------------------------*/
		if (llHdl->dualFilterMode == 0) {
			/* single filter */

			acc_mask <<= 5;						/* adjust mask */
			acc_mask  |= ((extended&0x2)<<3);	/* set rtr bit */
			acc_mask  |= 0xf;	                /* compatible reason */

			_WRITE_D8(ma,AcceptMask0Reg,(acc_mask >>  8) & 0xff);
			_WRITE_D8(ma,AcceptMask1Reg,(acc_mask      ) & 0xff);
			_WRITE_D8(ma,AcceptMask2Reg,(0xff          ));
			_WRITE_D8(ma,AcceptMask3Reg,(0xff          ));
		}
		else {
			/* dual filter */
			acc_mask <<= 5;						/* adjust mask */
			acc_mask  |= ((extended&0x2)<<3);	/* set rtr bit */
			acc_mask  |= ((extended&0x2)<<19);	/* set rtr bit */

			/* filter 1 */
			_WRITE_D8(ma,AcceptMask0Reg,(acc_mask >>  8) & 0xff);
			_WRITE_D8(ma,AcceptMask1Reg,((acc_mask     ) & 0xff) | 0x0f);

			/* filter 2 */
			_WRITE_D8(ma,AcceptMask2Reg,(acc_mask >> 24) & 0xff);
			_WRITE_D8(ma,AcceptMask3Reg,((acc_mask>> 16) & 0xff) | 0x0f);
		}
	}
#ifdef  S_RESET
	_SJA1000_operating_mode(llHdl);
#endif
	return 0;
}
