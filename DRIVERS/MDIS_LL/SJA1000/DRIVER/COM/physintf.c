/*********************  P r o g r a m  -  M o d u l e ***********************
 *  
 *         Name: physintf.c
 *      Project: SJA1000 driver
 *
 *       Author: ww 
 *        $Date: 2004/10/06 14:50:14 $
 *    $Revision: 1.4 $
 *
 *  Description: Misc functions
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

/**************************** _SJA1000_set_physintf ***************************
 *
 *  Description: Config physical interface
 *		         (Bus activity should be disabled when calling this function)
 *   
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level driver handle
 *               phys_intf  physical interface type
 *                          CPL_USERINTERFACE		(not supported)
 *                          CPL_RS485				(not supported)
 *                          CPL_ISOHIGHSPEED
 *                          CPL_PASSIVEINTERFACE	(not supported)
 *               user_cfg	user configuration (written to BUS_CFG register)
 *                          (if CPL_USERINTERFACE)
 *  Output.....: return     0 or error code
 *                          CPL_ERR_BADINTERFACE   interface not supported 
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_set_physintf( LL_HANDLE *llHdl, 
							 u_int32 phys_intf, 
							 u_int32 user_cfg )
{
	MACCESS ma 		= llHdl->ma;
	int32	error	= 0;

	DBGWRT_2((DBH, "  _SJA1000_set_physintf phys_intf=%x user_cfg=%x\n",
			  phys_intf,user_cfg));

	/*------------------------+
    | check config reg access |
    +------------------------*/
	if (_SJA1000_check_reset_mode(llHdl))
		return CPL_ERR_BADMODE;

    /*----------------------------------------+
    | Output driver configuration:            |
    | Input is Tx  (dominant/recessive)       |
    | Output is Tx0 and Tx1 (high/low/float)  |
    +----------------------------------------*/
    switch(phys_intf)
	{
        /*------------------------+
        | User defined interface  |
        | Tx :   dom(0) rec(1)    |
        | Tx0:   ?      ?         |
        | Tx1:   ?      ?         |
        +------------------------*/
        case CPL_USERINTERFACE: 
			_WRITE_D8(ma,OutControlReg,user_cfg);
            break;
        /*------------------------+
        | RS485 interface         |
        | Tx :   dom(0) rec(1)    |
        | Tx0:   HIGH   LOW       |
        | Tx1:   float  float     |
        +------------------------*/
        case CPL_RS485:
			_WRITE_D8(ma,OutControlReg,
					  NormalMode|
					  OCPOL0_Bit|
					  Tx0PshPull);
			break;
        /*------------------------+
        | ISO high speed interface|
        | Tx :   dom(0) rec(1)    |
        | Tx0:   LOW    HIGH      |
        | Tx1:   float  float     |
        +------------------------*/
        case CPL_ISOHIGHSPEED:
			_WRITE_D8(ma,OutControlReg,
					  NormalMode|
					  Tx0PshPull);					  
            break;
        /*------------------------+
        | Passive interface       |
        | Tx :   dom(0) rec(1)    |
        | Tx0:   HIGH   float     |
        | Tx1:   HIGH   float     |
        +------------------------*/
        case CPL_PASSIVEINTERFACE:
			_WRITE_D8(ma,OutControlReg,
					  NormalMode|
					  Tx0PullDn|
					  OCPOL1_Bit|
					  Tx1PullUp);					  
			break;
        /*------------------------+
        | Unknown interface       |
        +------------------------*/
        default:
			error = CPL_ERR_BADINTERFACE;
			break;
    }
	return error;
}		
