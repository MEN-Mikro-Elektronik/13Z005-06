/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: sja1000_api.h
 *
 *       Author: ww
 *
 *  Description: SJA1000_API API header file
 *
 *     Switches: _LL_DRV_	when compiling low level driver
 *
 *---------------------------------------------------------------------------
 * Copyright (c) 1999-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/
/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _SJA1000_API_H
#define _SJA1000_API_H

#ifdef __cplusplus
	extern "C" {
#endif

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#if	0	/* obsoleted */
#define	EX_OBJ	0
#define	TX_OBJ	1
#define	RX_OBJ	2
#endif

/*--- object direction ---*/
#define CPL_DIR_RCV				0			/* receive */
#define CPL_DIR_XMT				1			/* transmit */
#define CPL_DIR_DIS				2			/* disable */

/*--- physical interface ---*/
#define CPL_USERINTERFACE		0			/* user defined */
#define CPL_RS485				1			/* RS 485 */
#define CPL_ISOHIGHSPEED		2			/* ISO High-Speed */
#define CPL_PASSIVEINTERFACE	3			/* passive */

/*----- canbus error codes returned by sja1000_read_error() ---*/
#define CPL_BUSOFF_SET			1			/* controller bus off */
#define CPL_BUSOFF_CLR			2			/* controller bus on */
#define CPL_WARN_SET			3			/* controller warning on */
#define CPL_WARN_CLR			4			/* controller warning off */
#define CPL_MSGLOST				5			/* controller's msg buf overwr. */
#define CPL_QOVERRUN			6			/* object queue overrun */

#define	CPL_BUS_ERROR			7			/* bus error intterrupt */
#define	CPL_ARBITRATION_LOST	8			/* arbitration lost interrupt */
#define	CPL_ERROR_PASSIVE		9			/* error passive interrupt */
#define	CPL_WAKE_UP				10			/* wake up interrupt */
#define	CPL_DATA_OVERRUN		11			/* data overrun interrupt */
#define	CPL_ERROR_WARNING		12			/* error warning interrupt */

#define	CPL_TBUFFER_LOCKED		13			/* transmit buffer locked */
#define	CPL_RQUEUE_NOTREADY		14			/* receive queue notready */

#define	CPL_RTR_FRAME			15			/* rtr frame received */

#define CPL_PASSIVE_STATE		16			/* controller has reached the passive status */
#define CPL_ACTIVE_STATE		17			/* controller has enter the active status again */
#define	CPL_RECOVER_BUSOFF		18			/* can't release busoff state */

/*----- API Error codes -----------------------------------------------------*/
#define CPL_ERR_BADSPEED        (ERR_DEV+1) /* bitrate not supported */
#define CPL_ERR_BADINTERFACE    (ERR_DEV+2)	/* interface not supported */
#define CPL_ERR_NOMESSAGE       (ERR_DEV+8)	/* no message in receive buffer */
#define	CPL_ERR_RESETACTIVE		(ERR_DEV+10)/* controller reset in progress */
#define	CPL_ERR_BADCLOCK		(ERR_DEV+11)/* illegal system clock */
#define	CPL_ERR_BADTMDETAILS	(ERR_DEV+12)/* illegal timing details */
#define	CPL_ERR_BADMSGNUM		(ERR_DEV+13)/* illegal message object nr. */
#define	CPL_ERR_BADDIR			(ERR_DEV+14)/* illegal message direction */
#define	CPL_ERR_QFULL			(ERR_DEV+15)/* transmit queue full */
#define CPL_ERR_SIGBUSY			(ERR_DEV+16)/* signal already installed */
#define	CPL_ERR_BADPARAMETER	(ERR_DEV+17)/* bad parameter */
#define	CPL_ERR_TXQFULL			(ERR_DEV+18)/* transmit queue full */
#define	CPL_ERR_RXQFULL			(ERR_DEV+19)/* receive queue full */
#define	CPL_ERR_RXQNRDY			(ERR_DEV+20)/* receive queue not ready */
#define	CPL_ERR_BADMODE			(ERR_DEV+21)/* illegal mode */
#define	CPL_ERR_NOOBJECT        (ERR_DEV+22)/* object not defined */
#define	CPL_ERR_BADPARAM		(ERR_DEV+23)/* illegal parameter */
#define	CPL_ERR_BADBUFSIZE		(ERR_DEV+24)/* illegal buffer size */

/*--------------------------------------+
|   TYPEDEFS                            |
+--------------------------------------*/
/*--------------------------------------+
|   EXTERNALS                           |
+--------------------------------------*/
/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
/*
 * DON'T CHANGE THE ORDER OF THIS DECLARATIONS
 * ADD NEW FUNCTIONS ONLY BEHIND THE LAST ONE
 * (REQUIREMENT FOR WINDOWS API DLL)
 */
#ifndef _LL_DRV_
int32 __MAPILIB sja1000_init(
char *device);
int32 __MAPILIB sja1000_term(
int32 path);
int32 __MAPILIB sja1000_set_accmask(
int32 path,
u_int32 extended,
u_int32 acc_mask);
int32 __MAPILIB sja1000_set_acccode(
int32 path,
u_int32 extended,
u_int32 acc_code);
int32 __MAPILIB sja1000_set_accfilter(
int32 path,
u_int32 extended,
u_int32 acc_code,
u_int32 acc_mask);
int32 __MAPILIB sja1000_config_msg(
int32 path,
u_int32 nr,
u_int32 dir,
u_int32 extended,
u_int32 id,
u_int32 mask,
u_int32 qentries);
int32 __MAPILIB sja1000_set_bitrate(
int32 path,
u_int32 bitrate,
u_int32 spl);
int32 __MAPILIB sja1000_enable(
int32 path,
u_int32 enable);
int32 __MAPILIB sja1000_read_msg(
int32 path,
u_int32 nr,
int32 timeout,
u_int32 *idP,
u_int32 *datlenP,
u_int8 *data);
int32 __MAPILIB sja1000_write_msg(
int32 path,
u_int32 nr,
int32 timeout,
u_int32 mode,
u_int32 id,
u_int32 datlen,
u_int8 *data);
int32 __MAPILIB sja1000_read_error(
int32 path,
u_int32 *errcodeP,
u_int32 *nrP);
int32 __MAPILIB sja1000_read_error_timeout(
int32 path,
u_int32 *errcodeP,
u_int32 *nrP,
int32 timeout);
int32 __MAPILIB sja1000_set_rcvsig(
int32 path,
u_int32 nr,
int32 signal);
int32 __MAPILIB sja1000_set_xmtsig(
int32 path,
u_int32 nr,
int32 signal);
int32 __MAPILIB sja1000_clr_rcvsig(
int32 path,
u_int32 nr);
int32 __MAPILIB sja1000_clr_xmtsig(
int32 path,
u_int32 nr);
int32 __MAPILIB sja1000_rtr(
int32 path,
u_int32 nr,
u_int32 id,
u_int32	dl);
int32 __MAPILIB sja1000_queue_clear(
int32 path,
u_int32 nr,
u_int32 txabort);
int32 __MAPILIB sja1000_clear_busoff(
int32 path);
char *__MAPILIB sja1000_errmsg(
int32 error);
int32 __MAPILIB sja1000_read_register(
int32 path,
u_int32	select,
u_int32 *bufP,
u_int32 idx);
int32 __MAPILIB sja1000_read_counter(
int32 path,
u_int32	select,
u_int32 *bufP,
u_int32 size);
int32 __MAPILIB sja1000_read_configuration(
int32 path,
u_int32	select,
u_int32 *bufP,
u_int32 size);
int32 __MAPILIB sja1000_read_eccr(
int32 path,
u_int32 *bufP);
int32 __MAPILIB sja1000_read_alcr(
int32 path,
u_int32 *bufP);
int32 __MAPILIB sja1000_queue_status(
int32 path,
u_int32 nr,
u_int32 *entriesP,
u_int32 *directionP);
int32 __MAPILIB sja1000_write_register(
int32 path,
u_int32	select,
u_int32 val,
u_int32 idx);
int32 __MAPILIB sja1000_read_edition_number(
int32 path,
u_int32 *editionnumberP);
int32 __MAPILIB sja1000_abort_transmission(
int32 path);
#endif /* ? _LL_DRV_ */

#ifdef __cplusplus
	}
#endif

#endif	/* _SJA1000_API_H */
