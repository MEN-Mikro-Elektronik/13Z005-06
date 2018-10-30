/*********************  P r o g r a m  -  M o d u l e ***********************
 *  
 *         Name: config.c
 *      Project: SJA1000 driver
 *
 *       Author: ww  
 *        $Date: 2004/10/06 14:50:23 $
 *    $Revision: 1.5 $
 *
 *  Description: Config a CAN message object
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

/******************************* _SJA1000_config_msg ***************************
 *
 *  Description: Config a CAN message 1... object
 *			   
 *---------------------------------------------------------------------------
 *  Input......: llHdl		low level driver handle
 *               nr			message object number (1....)
 *               dir		direction (0=receive, 1=transmit, 2=disable)
 *               extended	message ID type (0=standard, 1=extended)
 *               id         local acceptance code
 *               mask		local acceptance mask
 *  Output.....: return     0 or error code
 *				 			CPL_ERR_BADMSGNUM  illegal message object number
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_config_msg( LL_HANDLE *llHdl,
						   u_int32 nr,
						   u_int32 dir,
						   u_int32 extended,
						   u_int32 code,
						   u_int32 mask)
{
	return(0);
}
