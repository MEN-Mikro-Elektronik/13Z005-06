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
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: config.c,v $
 * Revision 1.5  2004/10/06 14:50:23  dpfeuffer
 * minor modifications for MDIS4/2004 conformity
 *
 * Revision 1.4  2001/04/20 16:21:40  ww
 * included RCS_ID switch
 *
 * Revision 1.3  1999/08/18 10:04:22  ww
 * removed unused code
 *
 * Revision 1.2  1999/08/17 10:15:31  ww
 * removed setting of global acceptance maske
 *
 * Revision 1.1  1999/08/03 10:55:37  ww
 * Initial Revision
 *---------------------------------------------------------------------------
 * (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany 
 ****************************************************************************/

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
