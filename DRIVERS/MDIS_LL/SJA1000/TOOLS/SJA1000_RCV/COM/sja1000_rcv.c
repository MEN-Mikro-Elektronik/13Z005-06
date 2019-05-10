/****************************************************************************
 ************                                                    ************
 ************                    SJA1000_RCV                      ************
 ************                                                    ************
 ****************************************************************************
 *  
 *       Author: kp
 *        $Date: 2004/10/06 14:51:02 $
 *    $Revision: 1.4 $
 *
 *  Description: Receive process for SJA1000 tests
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, SJA1000_api
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
 
#include <stdio.h>
#include <string.h>
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
#ifdef OS9
# define USE_DIFFERENT_SIGNALS
#endif

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static u_int32 G_sigCnt[16];

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static 	void usage(void);
static 	int32 PrintError(char *info, int nr);
static 	void SigHandler(u_int32 sigCode);


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
	printf("Usage: SJA1000_rcv [<opts>] <device> [<opts>]\n");
	printf("Function: Receive process for SJA1000 test\n");
	printf("Options:\n");
	printf("    device       device name                 [none]\n");
	printf("    -f=<num>     first rcv object (1....)    [1]\n");
	printf("    -l=<num>     last  rcv object (1....)    [1]\n");
	printf("    -i=<id>      identifier for first obj    [1]\n");
	printf("                 other obj IDs count up from there\n");
#if	0
	printf("    -e           use extended identifier     [std]\n");
#endif
	printf("    -q=<ent>     number of rcv queue entries [100]\n");
	printf("    -m=<mask>    acceptance mask for obj     [ffffffff]\n");	
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
int main(int argc, char *argv[])
{
	int32	path=0, ret=1, n, i, error;
	u_int32 mask=0xffffffff, extid, firstId=1, qentries, sigCode;
	u_int32 firstNr, lastNr, nr, id, datlen, entries, nil;
	char	*device,*str,*errstr,buf[40];
	u_int8 data[8];

	for(i=0; i<16; i++ ) G_sigCnt[i] = 0;

	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("f=l=i=eq=m=?", buf))) {	/* check args */
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

	firstNr  = ((str = UTL_TSTOPT("f=")) ? atoi(str) : 1);
	lastNr   = ((str = UTL_TSTOPT("l=")) ? atoi(str) : 1);
	qentries = ((str = UTL_TSTOPT("q=")) ? atoi(str) : 100);

	if( (str = UTL_TSTOPT("i=")))
		sscanf( str, "%lx", &firstId );

	if( (str = UTL_TSTOPT("m=")))
		sscanf( str, "%lx", &mask );

	extid    = (UTL_TSTOPT("e") ? 1 : 0);

	if (lastNr < firstNr)
		lastNr = firstNr;

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = sja1000_init(device)) < 0)
		return PrintError("open",0);

	/*--------------------+
    |  config             |
    +--------------------*/

	/*--- config objects ---*/
	id = firstId;
	for( nr=firstNr; nr<=lastNr; nr++ ){
		if( sja1000_config_msg( path, nr, CPL_DIR_RCV, extid, 
							   id++, mask, qentries ) < 0){
			PrintError("config message",0);
			goto abort;
		}
	}
		
	/*--- install signals ---*/
	if( (error = UOS_SigInit( SigHandler )) ){
		printf("Can't initialize signal handling error %d", error);
		goto abort;
	}

	n = 0;
	for( nr=firstNr; nr<=lastNr; nr++ ){

		if( sja1000_set_rcvsig( path, nr, UOS_SIG_USR1+n ) < 0){
			PrintError("install signal for msg queue", nr);
			goto abort;
		}
	    if( (error = UOS_SigInstall( UOS_SIG_USR1+n )) ){
			printf("Can't install signal error=%d", error);
			goto abort;
		}
        #ifdef USE_DIFFERENT_SIGNALS
		    n++;
		#endif
	}

	/*--------------------------+
	|  Wait for messages  	    |
	+--------------------------*/
	printf("Waiting for messages\n");

	while(TRUE){

		UOS_SigUnMask();
#if	0
		/*--- wait for signals (sleep forever) ---*/
		if( UOS_SigWait( 0, &sigCode ) == ERR_UOS_ABORTED ){
			printf("...ABORTED...\n");
			goto abort;
		}
		UOS_SigMask();
#else
		i = 0;
		for( nr=firstNr; nr<=lastNr; nr++ ){
			n = nr-firstNr;		/* index in G_sigCnt */

			i += G_sigCnt[n];
		}
		if (i == 0) {
			/*--- wait for signals (sleep forever) ---*/
			if( UOS_SigWait( 0, &sigCode ) == ERR_UOS_ABORTED ){
				printf("...ABORTED...\n");
				goto abort;
			}
			UOS_SigMask();
		}
#endif
		printf("sig!\n");
		for( nr=firstNr; nr<=lastNr; nr++ ){
			n = nr-firstNr;		/* index in G_sigCnt */

			if( G_sigCnt[n] ){
				/*--- check how many entries are available in msg queue ---*/
				if( sja1000_queue_status( path, nr, &entries, &nil ) < 0){
					PrintError("get queue status",nr);
					goto abort;
				}
		
				printf("obj %d: Signals received: %d, queue entries: %d\n", 
					   nr, G_sigCnt[n], entries );

				G_sigCnt[n] = 0;			/* reset signal counter */

				/*--- Read messages ---*/
				for( n=0; n<entries; n++ ){
					if( sja1000_read_msg( path, nr, -1, &id, &datlen, 
										 data ) < 0 ){
						PrintError("read from msg queue", nr);
						goto abort;
					}
					printf("Rx: ID=0x%08x LEN=%d DAT=", id, datlen);
					for( i=0; i<datlen; i++ )
						printf("%02x ", data[i]);
					printf("\n");
				}
			}
		}
	}
	/*--------------------+
    |  cleanup            |
    +--------------------*/
abort:
	for( nr=firstNr; nr<=lastNr; nr++ )
	{
		if ( sja1000_clr_rcvsig( path, nr ) < 0)
			PrintError("remove signal for queue",nr);
#if	1
		if ( sja1000_config_msg( path, nr, CPL_DIR_DIS, 0, 0, 0, 0 ) < 0)
			PrintError("config message",0);
#endif
	}
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
	/*
	 * NOTE: This code relies on subsequent signal codes starting at
	 * UOS_SIG_USR1 !!!
	 */
	if( sigCode >= UOS_SIG_USR1 ){
		G_sigCnt[sigCode - UOS_SIG_USR1]++;
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
