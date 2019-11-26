/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: sja1000_drv.h
 *
 *       Author: ww
 *
 *  Description: Header file for SJA1000 driver
 *               - SJA1000 specific status codes
 *               - SJA1000 function prototypes
 *
 *     Switches: _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 *				 B5
 *---------------------------------------------------------------------------
 * Copyright 1998-2019, MEN Mikro Elektronik GmbH
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

#ifndef _SJA1000_DRV_H
#define _SJA1000_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/*--- function to perform ---*/
enum SJA1000_API_FUNC {
	F_SET_ACCMASK,
	F_SET_ACCCODE,
	F_CONFIG_MSG,
	F_SET_BITRATE,
	F_ENABLE,
	F_READ_MSG,
	F_WRITE_MSG,
	F_READ_ERROR,
	F_SET_RCVSIG,
	F_SET_XMTSIG,
	F_SET_ERRSIG,
	F_CLR_RCVSIG,
	F_CLR_XMTSIG,
	F_CLR_ERRSIG,
	F_RTR,
	F_QUEUE_STATUS,
	F_QUEUE_CLEAR,
	F_CLEAR_BUSOFF,
	F_READ_REGISTER,
	F_READ_COUNTER,
	F_READ_CONFIGURATION,
	F_READ_ALCR,
	F_READ_ECCR,
	F_WRITE_REGISTER,	/* edition 2 */
	F_READ_EDITION,
	F_ABORT,
};

/*--- parameter block to pass via SJA1000_BLK_CALL ---*/
typedef struct {

	/*--- function to perform ---*/
	enum SJA1000_API_FUNC func;

	/*--- function parameters ---*/
	union {
		struct {
			u_int32 extended;
			u_int32 acc_mask;
		} set_accmask;

		struct {
			u_int32 extended;
			u_int32 acc_code;
		} set_acccode;

		struct {
			u_int32 nr;
			u_int32 dir;
			u_int32 extended;
			u_int32 id;
			u_int32 mask;
			u_int32 qentries;
		} config_msg;

		struct {
			u_int32 bitrate;
			u_int32 spl;
		} set_bitrate;

		struct {
			u_int32 enable;
		} enable;

		struct {
			u_int32 nr;
			int32	timeout;
			u_int32 id;
			u_int32 datlen;
			u_int8  data[8];
		} read_msg;

		struct {
			int32 path;
			u_int32 nr;
			int32 	timeout;
			u_int32 mode;
			u_int32 id;
			u_int32 datlen;
			u_int8  data[8];
		} write_msg;

		struct {
			u_int32 errcode;
			u_int32 nr;
			int32   timeout;
		} read_error;

		struct {
			u_int32 nr;
			int32 	signal;
		} set_rcvsig;

		struct {
			u_int32 nr;
			int32 	signal;
		} set_xmtsig;

		struct {
			u_int32 nr;
		} clr_rcvsig;

		struct {
			u_int32 nr;
		} clr_xmtsig;

		struct {
			u_int32 nr;
		} rtr;

		struct {
			u_int32 nr;
			u_int32 entries;
			u_int32 direction;
		} queue_status;

		struct {
			u_int32 nr;
			u_int32 txabort;
		} queue_clear;

		struct {
			u_int32	select;
			u_int32 *bufP;
			u_int32 idx;
		} read_register;

		struct {
			u_int32	select;
			u_int32 *bufP;
			u_int32 size;
		} read_counter;

		struct {
			u_int32	select;
			u_int32 *bufP;
			u_int32 size;
		} read_configuration;

		struct {
			u_int32	select;
			u_int32	val;
			u_int32 idx;
		} write_register;

		struct {
			u_int32 edition;
		} read_edition;

	} p;
} SJA1000_CALL_PB;

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* SJA1000 specific status codes (BLK) */   /* S,G: S=setstat, G=getstat */
#define	SJA1000_BLK_CALL M_DEV_BLK_OF+0x00  /*   G: generic API call */

#ifndef  SJA1000_VARIANT
# define SJA1000_VARIANT SJA1000
#endif

# define _SJA1000_GLOBNAME(var,name) var##_##name
#ifndef _ONE_NAMESPACE_PER_DRIVER_
# define SJA1000_GLOBNAME(var,name) _SJA1000_GLOBNAME(var,name)
#else
# define SJA1000_GLOBNAME(var,name) _SJA1000_GLOBNAME(SJA1000,name)
#endif

#define  __SJA1000_GetEntry	  SJA1000_GLOBNAME(SJA1000_VARIANT,GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __SJA1000_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

#ifdef __cplusplus
      }
#endif

#endif /* _SJA1000_DRV_H */
