/****************************************************************************
 ************                                                    ************
 ************                    SJA1000_DORTR                    ************
 ************                                                    ************
 ****************************************************************************
 *  
 *       Author: kp
 *        $Date: 2004/10/06 14:51:11 $
 *    $Revision: 1.5 $
 *
 *  Description: process for SJA1000 RTR tests
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, sja1000_api
 *     Switches: -
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: sja1000_dortr.c,v $
 * Revision 1.5  2004/10/06 14:51:11  dpfeuffer
 * minor modifications for MDIS4/2004 conformity
 *
 * Revision 1.4  2004/03/04 15:48:18  dpfeuffer
 * adapted to changed sja1000_rtr()
 *
 * Revision 1.3  2001/01/25 11:42:42  kp
 * removed main() prototype (problems on VxW compile)
 *
 * Revision 1.2  1999/08/18 10:08:12  ww
 * correced sja1000_rtr call
 *
 * Revision 1.1  1999/08/03 10:55:51  ww
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
#include <MEN/sja1000_api.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static u_int32 G_rxSigCnt;
static u_int32 G_txSigCnt;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void 	usage(void);
static int32 	PrintError(char *info, int nr);
static void 	SigHandler( u_int32 sigCode );

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
	printf("Usage: sja1000_dortr [<opts>] <device> [<opts>]\n");
	printf("Function: Send RTR, wait for message receiption\n");
	printf("Options:\n");
	printf("    device       device name                 [none]\n");
	printf("    -f=<num>     msg object number (1....)   [1]\n");
	printf("    -i=<id>      identifier for obj          [1]\n");
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
	int32	path=0, ret=1, n, i, error;
	u_int32 mask=0xffffffff, extid;
	u_int32 nr, id, datlen, tx, rx;
	char	*device,*str,*errstr,buf[40];
	u_int8 data[8];

	G_rxSigCnt = 0;
	G_txSigCnt = 0;
	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("f=i=?", buf))) {	/* check args */
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
	for (device=NULL, n=1; n<argc; n++)	{
		if (*argv[n] != '-') {
			device = argv[n];
			break;
		}
	}
	if (!device) {
		usage();
		return(1);
	}

	nr = ((str = UTL_TSTOPT("f=")) ? atoi(str) : 1);

	tx = nr;
	rx = nr+1;

	if( (str = UTL_TSTOPT("i=")))
		sscanf( str, "%lx", &id );
	extid    = 0;

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = sja1000_init(device)) < 0)
		return PrintError("open",0);

	/*--------------------+
    |  config             |
    +--------------------*/

	/*--- config object ---*/
	if( sja1000_config_msg(path, tx, CPL_DIR_XMT, extid, id, 0, 10 ) < 0){
		PrintError("config message",0);
		goto abort;
	}
	/*--- config object ---*/
	if( sja1000_config_msg(path, rx, CPL_DIR_RCV, extid, id, mask, 10 ) < 0){
		PrintError("config message",0);
		goto abort;
	}
		
	/*--- install signals ---*/
	if( (error = UOS_SigInit( SigHandler )) ){
		printf("Can't initialize signal handling error %d", error);
		goto abort;
	}

	if( sja1000_set_xmtsig( path, tx, UOS_SIG_USR1 ) < 0){
		PrintError("install xmt signal", tx);
		goto abort;
	}
	if( sja1000_set_rcvsig( path, rx, UOS_SIG_USR2 ) < 0){
		PrintError("install rcv signal for msg queue", rx);
		goto abort;
	}

	if( (error = UOS_SigInstall( UOS_SIG_USR1 )) ){
		printf("Can't install signal error=%d", error);
		goto abort;
	}
	if( (error = UOS_SigInstall( UOS_SIG_USR2 )) ){
		printf("Can't install signal error=%d", error);
		goto abort;
	}
	/*--------------------------+
	|  Wait for messages  	    |
	+--------------------------*/
	printf("Send RTR, wait for message...\n");
#if	0
	if( sja1000_write_msg( path, tx, 0, 3, id, 0, "" ) < 0 ){
		PrintError("send rtr", tx);
		goto abort;
	}
#endif
	if( sja1000_rtr( path, tx, id, 0 ) < 0 ){
		PrintError("send rtr", tx);
		goto abort;
	}

	if( sja1000_read_msg( path, rx, 2000, &id, &datlen, data ) < 0 ){
		PrintError("read from msg queue", rx);
		goto abort;
	}

	printf("Rx: ID=0x%08x LEN=%d DAT=", id, datlen);
	for( i=0; i<datlen; i++ )
		printf("%02x ", data[i]);
	printf("\n");

	printf("Rx Signals: %d, Tx Signals=%d\n", G_rxSigCnt, G_txSigCnt );
	ret = 0;
	/*--------------------+
    |  cleanup            |
    +--------------------*/
abort:

	if( sja1000_clr_xmtsig( path, tx ) < 0)
		PrintError("remove signal for queue",tx);

	if( sja1000_clr_rcvsig( path, rx ) < 0)
		PrintError("remove signal for queue",rx);

	UOS_SigExit();

	if ( sja1000_term(path) < 0)
		PrintError("close",0);

	return(ret);
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
	switch( sigCode ){
		case UOS_SIG_USR1:
			G_txSigCnt++;
			break;
		case UOS_SIG_USR2:
			G_rxSigCnt++;
			break;
	}
}	

/********************************* PrintError ********************************
 *
 *  Description: Print MDIS error message
 *			   
 *---------------------------------------------------------------------------
 *  Input......: info	info string
 *				 nr		related message obj
 *  Output.....: Returns: always 1
 *  Globals....: -
 ****************************************************************************/

static int32 PrintError(char *info, int nr)
{
	printf("*** can't %s (msg obj %d): %s\n", info, nr, 
		   sja1000_errmsg(UOS_ErrnoGet()));
	return 1;
}

