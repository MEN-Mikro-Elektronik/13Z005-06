/****************************************************************************
 ************                                                    ************
 ************                   SJA1000_SIMP                        ************
 ************                                                    ************
 ****************************************************************************
 *
 *       Author: ww
 *        $Date: 2004/10/06 14:51:20 $
 *    $Revision: 1.9 $
 *
 *  Description: Simple example program for the SJA1000 driver
 *
 * This example configures the CAN controller with one message object and
 * one transmitter object. After configure the example will bee send 10 messages.
 *
 *     Required: MDIS user interface library, sja1000_api
 *     Switches: NO_MAIN_FUNC	(for systems with one namespace)
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: sja1000_simp.c,v $
 * Revision 1.9  2004/10/06 14:51:20  dpfeuffer
 * minor modifications for MDIS4/2004 conformity
 *
 * Revision 1.8  2004/03/23 12:02:36  ww
 * included new error code
 *
 * Revision 1.7  2004/03/04 15:48:20  dpfeuffer
 * changed type of error variable from error_code to int32
 *
 * Revision 1.6  2002/07/24 15:02:46  ww
 * added output of error string additional to error number
 *
 * Revision 1.5  2001/08/29 09:33:43  Schmidt
 * last change was buggy :-(
 *
 * Revision 1.4  2001/08/14 08:54:26  Schmidt
 * switch OS9 for strings.h added
 *
 * Revision 1.3  2001/04/20 16:21:49  ww
 * added string include file
 *
 * Revision 1.2  2001/01/23 15:06:45  ww
 * changed queue_status call
 *
 * Revision 1.1  1999/08/03 10:55:53  ww
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 1998 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/mdis_err.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/sja1000_api.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#define	BITRATE	2			/* 500kbit */
#define	SPL		0			/* sample mode (0=fast, 1=slow) */
#define	DATA	"01234567"
#define	WQUEUE	6

/*--------------------------------------+
|   TYPDEFS                             |
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
static int32 _sja1000_simp(char *device, int format, int verbose);
static void PrintError(char *info);
static void PrintDriverError(int32 error);


/********************************* main *************************************
 *
 *  Description: MAIN entry (not used in systems with one namespace)
 *
 *---------------------------------------------------------------------------
 *  Input......: argc, argv		command line arguments/counter
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
int main(int argc, char *argv[])
{
	char	*errstr,buf[40];
	int		format;
	int		verbose;

	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: sja1000_simp <device>\n");
		printf("Function: SJA1000 example: configure and send message\n");
		printf("Fix: Bitrate:500 (-b=2), Frame: 29 Bit (-e)\n");
		printf("Options:\n");
		printf("    device       device name\n");
		printf("\n");
		return(1);
	}

	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("ev?", buf))) {	/* check args */
		printf("*** %s\n", errstr);
		return(1);
	}
	format 	= (UTL_TSTOPT("e") ? 1 : 0);
	verbose	= (UTL_TSTOPT("v") ? 1 : 0);

    return _sja1000_simp(argv[1], format, verbose);
}


/******************************* sja1000_simp **********************************
 *
 *  Description: Example (directly called in systems with one namespace)
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: device    device name
 *  Output.....: return    always 0
 *  Globals....: -
 ****************************************************************************/
static int32 _sja1000_simp(char *device, int format, int verbose)
{
	int32 path=0, i;
	u_int32 entries, nil, errcode, nr, errcounter;
	u_int8 *msgData = (u_int8 *)DATA;

	if( device==NULL)
		return 0;

	if( verbose ){
		if( (format == 0) )	printf("BasicMode\n");
		else				printf("PelicanMode\n");
	}

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = sja1000_init(device)) < 0){
		PrintError("open");
	    return 0;
	}

	/*--------------------+
    |  config             |
    +--------------------*/

	/*--- set bitrate 1MBit/s ---*/
	if( sja1000_set_bitrate( path, BITRATE, SPL ) < 0 ){
		PrintError("set bitrate");
		goto abort;
	}

	/*--- set standard acceptance mask, exact ---*/
	if( sja1000_set_accmask( path, format, 0xffffffff ) < 0 ){
		PrintError("set acceptance mask");
		goto abort;
	}

	/*--- config error object, 20 queue entries---*/
	if( sja1000_config_msg( path, 0, 0, format, 0, 0, 20 ) < 0){
		PrintError("config error queue");
		goto abort;
	}

	/*--- config transmit message object, 5 queue entries---*/
	if( sja1000_config_msg( path, 1, CPL_DIR_XMT, format, 0, 0, WQUEUE ) < 0){
		PrintError("config xmt object");
		goto abort;
	}

	/*--- enable bus ---*/
	if( sja1000_enable( path, TRUE ) < 0 ){
		PrintError("enable bus activity");
		goto abort;
	}

	/*--------------------+
	|  Send messages      |
	+--------------------*/

	/*
	 * Send 10 messages, wait max. 1 second for queue space or timeout
	 * send object with ID=0x333, datalength=8, data="01234567"
	 * send object immediately (don't wait for RTR)
	 */
	errcounter = 0;

	printf("send %d messages:\n",10);
	for( i=0; i<10; i++ ){
		if( sja1000_write_msg( path, 1, 1000, 1, i, 8, msgData ) < 0 ) {
			printf("%02d: ",i);
			PrintError("send message");
			errcounter++;
		}
	}
#if	0
	/* all messages sent ? */
	i = 1;
	while (i)
	{
		/*--- check error queue for messages ---*/
		if( sja1000_queue_status( path, 1, &entries, &nil ) < 0 ){
			PrintError("read the write queue status");
			goto abort;
		}
		if (entries == WQUEUE)
			break;

		sleep(1);
		i--;
	}

	/*--- disable bus ---*/
	if( sja1000_enable( path, FALSE ) < 0 ){
		PrintError("disable bus activity");
		goto abort;
	}
#endif
	/*--- check error queue for messages ---*/
	if( sja1000_queue_status( path, 0, &entries, &nil ) < 0 ){
		PrintError("read the error queue status");
		goto abort;
	}
	if (errcounter == 0)
	{
		printf("*** all messages sent!\n");
	}
	printf("\n");

	printf("read messages from error queue:\n");
	if (entries == 0)
		printf("*** error queue empty!\n");
	else {
		/*--- read messages from error queue ---*/
		for( i=0; i<entries; i++ ){
			if( sja1000_read_error( path, &errcode, &nr ) < 0 ){
				PrintError("read from error queue");
				goto abort;
			}
			printf("*** Error: errcode=%d message object=%d, ", errcode, nr );
			PrintDriverError(errcode);
		}
	}

abort:
	/*--------------------+
    |  cleanup            |
    +--------------------*/
	if ( sja1000_term(path) < 0)
		PrintError("close");

	return(0);
}

/********************************* PrintError ********************************
 *
 *  Description: Print MDIS error message
 *
 *---------------------------------------------------------------------------
 *  Input......: info	info string
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/

static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}

static void PrintDriverError( int32 error )
{
	switch (error)
	{
		case CPL_ERR_BADSPEED:		printf("bitrate not supported\n");			break;
		case CPL_ERR_BADINTERFACE:	printf("interface not supported\n");		break;
		case CPL_ERR_NOMESSAGE:		printf("no message in receive buffer\n");	break;
		case CPL_ERR_RESETACTIVE:	printf("controller reset in progress\n");	break;
		case CPL_ERR_BADCLOCK:		printf("illegal system clock\n");			break;
		case CPL_ERR_BADTMDETAILS:	printf("illegal timing details\n");			break;
		case CPL_ERR_BADMSGNUM:		printf("illegal message object nr.\n");		break;
		case CPL_ERR_BADDIR:		printf("illegal message direction\n");		break;
		case CPL_ERR_QFULL:			printf("transmit queue full\n");			break;
		case CPL_ERR_SIGBUSY:		printf("signal already installed\n");		break;
		case CPL_ERR_BADPARAMETER:	printf("bad parameter\n");					break;
		case CPL_ERR_TXQFULL:		printf("transmit queue full\n");			break;
		case CPL_ERR_RXQFULL:		printf("receive queue full\n");				break;
		case CPL_ERR_RXQNRDY:		printf("receive queue not ready\n");		break;
		case CPL_ERR_BADMODE:		printf("illegal mode\n");					break;
        case CPL_ERR_NOOBJECT:      printf("message object not defined\n");    		break;
        case CPL_ERR_BADPARAM:  	printf("illegal parameter\n");		        break;
        case CPL_ERR_BADBUFSIZE:	printf("illegal buffer size\n");		    break;

		case CPL_BUSOFF_SET:		printf("controller bus off\n"); 			break;
		case CPL_BUSOFF_CLR:		printf("controller bus on\n"); 				break;
		case CPL_WARN_SET:			printf("controller warning on\n"); 			break;
		case CPL_WARN_CLR:			printf("controller warning off\n"); 		break;
		case CPL_MSGLOST:			printf("controller's msg buf overwr.\n"); 	break;
		case CPL_QOVERRUN:			printf("object queue overrun\n"); 			break;
		case CPL_BUS_ERROR:			printf("bus error intterrupt\n"); 			break;
		case CPL_ARBITRATION_LOST:	printf("arbitration lost interrupt\n"); 	break;
		case CPL_ERROR_PASSIVE:		printf("error passive interrupt\n"); 		break;
		case CPL_WAKE_UP:			printf("wake up interrupt\n"); 				break;
		case CPL_DATA_OVERRUN:		printf("data overrun interrupt\n"); 		break;
		case CPL_ERROR_WARNING:		printf("error warning interrupt\n");		break;
		case CPL_TBUFFER_LOCKED:	printf("transmit buffer locked\n"); 		break;
		case CPL_RQUEUE_NOTREADY:	printf("receive queue notready\n"); 		break;
		case CPL_RTR_FRAME:			printf("rtr frame received\n"); 			break;
		case CPL_PASSIVE_STATE:		printf("controller has reached the passive status\n"); 		break;
		case CPL_ACTIVE_STATE:		printf("controller has enter the active status again\n"); 	break;

		default:
			printf("unsupported error code\n");
			break;
	}
}


