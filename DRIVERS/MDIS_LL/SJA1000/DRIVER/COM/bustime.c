/*********************  P r o g r a m  -  M o d u l e ***********************
 *  
 *         Name: bustime.c
 *      Project: SJA1000 driver
 *
 *       Author: ww
 *
 *  Description: Config bus timing
 *
 *     Required: - 
 *     Switches: - 
 *
 *---------------------------------------------------------------------------
 * Copyright (c) 1999-2019, MEN Mikro Elektronik GmbH
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

/*------------------+
|  defines          |
+------------------*/
#define SJW		2			/* default SJW */


/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static int32 _SJA1000_calc_bustime(
register u_int32 canclock,
register u_int32 bitrate,
u_int32 *calc_brp,
u_int32 *calc_tseg );

static int32 _SJA1000_set_bustime(
LL_HANDLE *llHdl,
u_int32 sjw,
u_int32 brp,
u_int32 spl,
u_int32 tseg1,
u_int32 tseg2);


/**************************** _SJA1000_set_bitrate ****************************
 *
 *  Description: Calculate and config bus timing for given bitrate
 *		         (Bus activity should be disabled when calling this function)
 *			   
 *               This function subsequently calls _SJA1000_set_bustime() with
 *               predefined parameters.
 *
 *               - SJW is set to 2
 *               - TSEG1:TSEG2 ratio is 2:1
 *               - sample point is at circa 66% of bittime
 *   
 *               Supports all bitrates recommended by CiA Standard 102 V2.0 
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level driver handle
 *  Input......: canclock		controller clock [Hz]
 *                              (as returned from _SJA1000_init) 
 *               bitrate		bus bitrate specifier
 *                              0 = 1   Mbit/s
 *                              1 = 800 kbit/s
 *                              2 = 500 kbit/s
 *                              3 = 250 kbit/s
 *                              4 = 125 kbit/s
 *                              5 = 100 kbit/s
 *                              6 = 50  kbit/s
 *                              7 = 20  kbit/s
 *                              8 = 10  kbit/s
 *               spl			sample mode (0=fast, 1=slow)
 *  Output.....: return     	0 or error code
 *                              CPL_ERR_BADSPEED       bitrate not supported
 *                              CPL_ERR_BADTMDETAILS   (should never occure)
 *  Globals....: -
 ****************************************************************************/

int32 _SJA1000_set_bitrate( LL_HANDLE *llHdl,
							u_int32	canclock,
							u_int32	bitrate,
							u_int32	spl )
{
    u_int32 brp, tseg, tseg1, tseg2, error=0;

	DBGWRT_2((DBH, "  _SJA1000_set_bitrate: canclk=%d bitrate=%d spl=%d\n",
			canclock, bitrate, spl));  
#if	0
	switch(bitrate) 
	{
		case 0:	brp=1;  tseg1=4;   tseg2=3;  break;	/* 1 Mbit/s   (tseg=7) */
		case 1:	brp=1;  tseg1=6;   tseg2=3;  break;	/* 800 kbit/s (tseg=9) */
		case 2:	brp=1;  tseg1=10;  tseg2=5;  break;	/* 500 kbit/s (tseg=15) */
		case 3:	brp=2;  tseg1=10;  tseg2=5;  break;	/* 250 kbit/s (tseg=15) */
		case 4:	brp=4;  tseg1=10;  tseg2=5;  break;	/* 125 kbit/s (tseg=15) */
		case 5:	brp=4;  tseg1=12;  tseg2=7;  break;	/* 100 kbit/s (tseg=19) */
		case 6:	brp=8;  tseg1=12;  tseg2=7;  break;	/* 50 kbit/s  (tseg=19) */
		case 7:	brp=16; tseg1=16;  tseg2=8;  break;	/* 20 kbit/s  (tseg=24) */
		case 8:	brp=32; tseg1=16;  tseg2=8;  break;	/* 10 kbit/s  (tseg=24) */
		default: return(CPL_ERR_BADSPEED);
	}
#endif
	switch(bitrate) 
	{
		case 0:	bitrate = 1000000;	tseg2=3;	break;
		case 1:	bitrate =  800000;	tseg2=3;	break;
		case 2:	bitrate =  500000;	tseg2=5;	break;
		case 3:	bitrate =  250000;	tseg2=5;	break;
		case 4:	bitrate =  125000;	tseg2=5;	break;
		case 5:	bitrate =  100000;	tseg2=7;	break;
		case 6:	bitrate =   50000;	tseg2=7;	break;
		case 7:	bitrate =   20000;	tseg2=8;	break;
		case 8:	bitrate =   10000;	tseg2=8;	break;

		default: return(CPL_ERR_BADSPEED);
	}
	bitrate = _SJA1000_calc_bustime( canclock,bitrate,&brp,&tseg );

	DBGWRT_2((DBH, "  bitrate=%d\n",bitrate));

	if ((tseg2 = tseg/3) < 3)
		tseg2 = 3;

	tseg1 = tseg - tseg2;

	error = _SJA1000_set_bustime(llHdl,SJW,brp,spl,tseg1,tseg2);

	llHdl->bitrate 	= bitrate;
	llHdl->spl		= spl;
	llHdl->tseg1	= tseg1;
	llHdl->tseg2	= tseg2;
	
	DBGWRT_2((DBH, "  error=0x%x\n",error));

	return error;
}		

/**************************** _SJA1000_set_bustime ****************************
 *
 *  Description: Config bus timing registers by value (BIT_TIME0, BIT_TIME1)
 *		         (Bus activity should be disabled when calling this function)
 *			   
 *               Defined parameters are written to the BIT_TIME0/1 registers 
 *   
 *               NOTES:
 *		         - tseg1 must be >= sjw
 *		         - tseg2 must be >= sjw
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level driver handle
 *				 sjw			synch jump width (1..4)
 *               spl			sample mode (0=fast, 1=slow)
 *               tseg1			time segment 1 (3..16)	[clocks]
 *               tseg2			time segment 2 (2..8)	[clocks]
 *  Output.....: return     	0 or error code
 *                              CPL_ERR_BADTMDETAILS   illegal timing details
 *  Globals....: -
 ****************************************************************************/

static int32 _SJA1000_set_bustime( LL_HANDLE *llHdl,
							u_int32 sjw,
							u_int32 brp,
							u_int32 spl,
							u_int32 tseg1,
							u_int32 tseg2 )
{
	MACCESS ma = llHdl->ma;

	DBGWRT_2((DBH, "  _SJA1000_set_bustime: sjw=%d brp=%d spl=%d tseg1=%d tseg2=%d\n", sjw, brp, spl, tseg1, tseg2));  

	if ((sjw > 4) || (sjw > tseg2) || (sjw > tseg1))		/* check SJW */
		return(CPL_ERR_BADTMDETAILS);

	if (!IN_RANGE(tseg1,3,16) || !IN_RANGE(tseg2,2,8))		/* check TSEG1/2 */
		return(CPL_ERR_BADTMDETAILS);

 	/*------------------------+
    | check config reg access |
    +------------------------*/
	if (_SJA1000_check_reset_mode(llHdl))
		return CPL_ERR_BADMODE;

	/*------------------------+
    | write bus timing        |
    +------------------------*/
	_WRITE_D8(ma,BusTiming0Reg,((sjw-1)<<6) | (brp-1));
    _WRITE_D8(ma,BusTiming1Reg,(spl<<7) | ((tseg2-1)<<4) | (tseg1-1));

	return 0;
}		

/***************************** _SJA1000_calc_bustime **************************
 *
 *  Description: Calculate BRP and TSEG values (in clocks) for given bitrate
 *
 *               Tries to find out the best combination of BRP and TSEG:
 *			   
 *                   bitrate = canclock / (BRP * (TSEG+1))
 *			   
 *			     (BRP=1..64)
 *			     (TSEG=TSEG1+TSEG2=5..24)
 *
 *---------------------------------------------------------------------------
 *  Input......: canclock		controller clock [Hz]
 *                              (as returned from _SJA1000_init) 
 *               bitrate		bus bitrate [bit/s]
 *  Output.....: calc_brp		best matching BRP
 *               calc_tseg		best matching TSEG1+TSEG2 sum [clocks]
 *               return			resulting bitrate [bit/s]
 *  Globals....: -
 ****************************************************************************/

static int32 _SJA1000_calc_bustime( register u_int32 canclock,
							 register u_int32 bitrate,
							 u_int32 *calc_brp,
							 u_int32 *calc_tseg )
{
	register u_int32 brp,best_brp=0,tseg,best_tseg=0;
	register u_int32 rate,best_rate=0,diff,best_diff;

	best_diff = canclock / 16;			/* max. diff */

	for (brp=1; brp<=64; brp++) {
		for (tseg=5; tseg<=24; tseg++) {
			rate = canclock / 2 / (brp * (tseg+1));
			diff = (bitrate > rate ? bitrate-rate : rate-bitrate);

			if (diff < best_diff) {		/* better match ? */
				best_brp = brp;			/* store params */
				best_tseg = tseg;
				best_rate = rate;
				best_diff = diff;
			}

			if (diff == 0) 				/* exact match ? */
				goto alldone;
		}
	}

alldone:	
	*calc_brp  = best_brp;			/* return best matching params */
	*calc_tseg = best_tseg;
	return(best_rate);
}
