/****************************************************************************
 ************                                                    ************
 ************                    SJA1000_MAIN                     ************
 ************                                                    ************
 ****************************************************************************
 *  
 *       Author: kp
 *        $Date: 2004/10/06 14:50:58 $
 *    $Revision: 1.5 $
 *
 *  Description: Main process for SJA1000 tests
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, sja1000_api
 *     Switches: -
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: sja1000_main.c,v $
 * Revision 1.5  2004/10/06 14:50:58  dpfeuffer
 * minor modifications for MDIS4/2004 conformity
 *
 * Revision 1.4  2001/01/25 11:42:36  kp
 * removed main() prototype (problems on VxW compile)
 *
 * Revision 1.3  2001/01/23 15:06:39  ww
 * changed queue_status call and included ClearBusOff routine
 *
 * Revision 1.2  1999/08/18 10:08:10  ww
 * added acceptance code
 *
 * Revision 1.1  1999/08/03 10:55:44  ww
 * Initial Revision
 *
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany 
 ****************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <MEN/men_typs.h>
#include <MEN/usr_oss.h>
#include <MEN/usr_utl.h>
#include <MEN/mdis_api.h>
#include <MEN/mdis_err.h>
#include <MEN/usr_err.h>
#include <MEN/sja1000_api.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static u_int32 G_sigCnt;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void usage(void);
static int32 PrintError(char *info);
static void	SigHandler( u_int32 sigCode );
static void ClearBusOff(int path);


/********************************* usage ************************************
 *
 *  Description: Print program usage
 *			   
 *---------------------------------------------------------------------------
 *  Input......: -
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
static void usage(void)
{
	printf("Usage: sja1000_main [<opts>] <device> [<opts>]\n");
	printf("Function: Main process for SJA1000 test\n");
	printf("Options:\n");
	printf("    device       device name                  [none]\n");
	printf("    -b=<code>    bitrate code (0..8)          [0]\n");
	printf("                  0=1MBit 1=800kbit 2=500kbit 3=250kbit 4=125kbit\n");
	printf("                  5=100kbit 6=50kbit 7=20kbit 8=10kbit\n");
#if	1
	printf("    -c=<code>    acceptance code in hex       [00000001]\n");	
	printf("    -m=<mask>    acceptance mask in hex       [ffffffff]\n");	
#endif
	printf("    -s           slow bit sampling mode       [fast]\n");
#if	1
	printf("    -e           set extended acceptance mask [std]\n");
#endif
	printf("    -q=<ent>     number of err queue entries  [100]\n");
	printf("\n");
	printf("(c) 1999 by MEN mikro elektronik GmbH\n\n");
}

/********************************* main *************************************
 *
 *  Description: Program main function
 *			   
 *---------------------------------------------------------------------------
 *  Input......: argc,argv	argument counter, data ..
 *  Output.....: return	    success (0) or error (1)
 *  Globals....: -
 ****************************************************************************/

int main(int argc,char *argv[])
{
	int32	path=0, ret=1, n, error;
	u_int32 accmask=0xffffffff, acccode=0x00000001;
	u_int32 bitrate, extmask, spl, errqent;
	u_int32 errcode, nr, sigCode, entries, nil;
	u_int32	count;
	
	char	*device,*str,*errstr,buf[40];
	int		busoff;

	G_sigCnt = 0;
	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("b=c=m=esq=?", buf))) {	/* check args */
		printf("*** %s\n", errstr);
		return(1);
	}

	if (UTL_TSTOPT("?")) {						/* help requested ? */
		usage();
		return(1);
	}

	/*--------------------+
    |  get arguments      |
    +--------------------*/
	for (device=NULL, n=1; n<argc; n++)
		if (*argv[n] != '-') {
			device = argv[n];
			break;
		}

	if (!device) {
		usage();
		return(1);
	}

	bitrate  = ((str = UTL_TSTOPT("b=")) ? atoi(str) : 0);
	errqent  = ((str = UTL_TSTOPT("q=")) ? atoi(str) : 100);

	if( (str = UTL_TSTOPT("c=")))	sscanf( str, "%lx", &acccode );
	if( (str = UTL_TSTOPT("m=")))	sscanf( str, "%lx", &accmask );

	extmask  = (UTL_TSTOPT("e") ? 1 : 0);
	spl      = (UTL_TSTOPT("s") ? 1 : 0);

	extmask = 1;

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = sja1000_init(device)) < 0) {
		char	buf[80];
		sprintf(buf,"open %s",device);
		return PrintError(buf);
	}
	/*--------------------+
    |  config             |
    +--------------------*/
	if( sja1000_set_bitrate( path, bitrate, spl ) < 0 ){
		PrintError("set bitrate");
		goto abort;
	}
#if	1
	if( sja1000_set_accmask( path, extmask, accmask ) < 0 ){
		PrintError("acceptance mask");
		goto abort;
	}
	if( sja1000_set_acccode( path, extmask, acccode ) < 0 ){
		PrintError("acceptance code");
		goto abort;
	}
#endif
	/*--- config error object ---*/
	if( sja1000_config_msg( path, 0, 0, 0, 0, 0, errqent ) < 0){
		PrintError("config error queue");
		goto abort;
	}
#if	1		
	/*--- enable bus ---*/
	if( sja1000_enable( path, TRUE ) < 0 ){
		PrintError("enable bus activity");
		goto abort;
	}
#endif
	/*--- install signal ---*/
	if( (error = UOS_SigInit( SigHandler )) ){
		printf("Can't initialize signal handling error %d", error);
		goto abort;
	}

	if( (error = UOS_SigInstall( UOS_SIG_USR1 )) ){
		printf("Can't install signal err=%d", error);
		goto abort;
	}

	if( sja1000_set_rcvsig( path, 0, UOS_SIG_USR1 ) < 0){
		PrintError("install signal for error queue");
		goto abort;
	}

	/*--------------------------+
	|  Wait for error messages  |
	+--------------------------*/
	printf("Configuration ok. Waiting for error messages...\n");

	busoff = 0;
	count = 0;
	
	while(TRUE){

		UOS_SigUnMask();
#if	1
		if (busoff){
			busoff = 0;
			ClearBusOff(path);
		}
#endif
		/*--- wait for signals (sleep forever) ---*/
		if( UOS_SigWait( 0, &sigCode ) == ERR_UOS_ABORTED ){
			printf("...ABORTED...\n");
			goto abort;
		}

		UOS_SigMask();

		/*--- check how many entries are available in error queue ---*/
		if( sja1000_queue_status( path, 0, &entries, &nil ) < 0){
			PrintError("get queue status");
			goto abort;
		}
		
		printf("Signals received: %d, error queue entries: %d\n", G_sigCnt,
			   entries);

		G_sigCnt = 0;			/* reset signal counter */

		/*--- Read error messages ---*/
		for( n=0; n<entries; n++ ){
			if( sja1000_read_error( path, &errcode, &nr ) < 0 ){
				PrintError("read from error queue");
				goto abort;
			}
			switch( errcode ){
				case CPL_BUSOFF_SET:	
					str = "Controller now in bus off state";
					break;
				case CPL_BUSOFF_CLR:	
					str = "Controller left bus off state";
					break;
				case CPL_WARN_SET:	
					str = "Controller reached warning level";
					break;
				case CPL_WARN_CLR:	
					str = "Controller left warning level";
					break;
				case CPL_PASSIVE_STATE:
					str = "Controller reached passive state";
					break;
				case CPL_ACTIVE_STATE:
					str = "Controller enter active state";
					break;

				case CPL_MSGLOST:	
					str = "Message lost (driver too slow)";
					break;
				case CPL_QOVERRUN:	
					str = "Message queue overrun (application too slow)";
					break;	

				case CPL_TBUFFER_LOCKED:
					str = "Transmitter buffer locked";
					break;
				case CPL_RQUEUE_NOTREADY:
					str = "Receive queue not ready";
					break;
				case CPL_RTR_FRAME:
					str = "Rtr frame received";
					break;

				/* interrupt messages */
				case CPL_BUS_ERROR:
					str = "Bus error interrupt";
					break;
				case CPL_ARBITRATION_LOST:
					str = "Arbitration lost interrupt";
					break;
				case CPL_ERROR_PASSIVE:
					str = "Error passive interrupt";
					break;
				case CPL_WAKE_UP:
					str = "Wake up interrupt";
					break;
				case CPL_DATA_OVERRUN:
					str = "Data overrun interrupt";
					break;
				case CPL_ERROR_WARNING:
					str = "Error warning interrupt";
					break;
				
				default:
					str = "UNKNOWN";
					break;
			}
			printf("%04d ",count++);
			
			if( nr == 0 )
				printf("Global error event: %s\n", str );
			else
				printf("Message object %d error: %s\n", nr, str );

			if( errcode == CPL_BUSOFF_SET ){				
				printf("*** Now trying to clear bus off state\n");
				busoff = 1;
			}
		}
	}
	/*--------------------+
    |  cleanup            |
    +--------------------*/
abort:
	if( sja1000_clr_rcvsig( path, 0 ) < 0){
		PrintError("remove signal for error queue");
		goto abort;
	}

	UOS_SigExit();

	if ( sja1000_term(path) < 0)
		PrintError("close");

	return(ret);
}

/********************************** ? ***************************************
 *
 *  Description:  
 *                         
 *
 *---------------------------------------------------------------------------
 *  Input......:  
 *  Output.....:  
 *  Globals....:  ---
 ****************************************************************************/
static void ClearBusOff(int path)
{
	int		nr;
	u_int32	entries, direction;
			
	sja1000_clear_busoff( path );
					
	for (nr=1; nr<6; nr++)
	{
		if( sja1000_queue_status( path, nr, &entries, &direction ) < 0)
		{
			if (UOS_ErrnoGet() == 0x1f0d)
				break;
				PrintError("get queue status");
				break;
		}
		if( direction == CPL_DIR_XMT )
		{			
			/* 1: abort pending transmission */
			if (sja1000_queue_clear(path,nr,1) < 0)
				PrintError("clear queue");
					
			printf("*** Clear tx queue %d\n",nr);
			break;
		}
	}
}

/********************************* SigHandler *******************************
 *
 *  Description: Signal Handler routine
 *			   
 *			   
 *---------------------------------------------------------------------------
 *  Input......: sigCode		signal number received
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
static void SigHandler( u_int32 sigCode )
{
	G_sigCnt++;
}	

/********************************* PrintError ********************************
 *
 *  Description: Print MDIS error message
 *			   
 *---------------------------------------------------------------------------
 *  Input......: info	info string
 *  Output.....: Returns: always 1
 *  Globals....: -
 ****************************************************************************/

static int32 PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, sja1000_errmsg(UOS_ErrnoGet()));
	return 1;
}
