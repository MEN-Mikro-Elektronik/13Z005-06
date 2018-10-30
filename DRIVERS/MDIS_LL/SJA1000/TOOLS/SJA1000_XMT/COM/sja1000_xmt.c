/****************************************************************************
 ************                                                    ************
 ************                    SJA1000_XMT                      ************
 ************                                                    ************
 ****************************************************************************
 *  
 *       Author: kp
 *        $Date: 2004/10/06 14:51:07 $
 *    $Revision: 1.4 $
 *
 *  Description: Transmit process for SJA1000 tests
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, sja1000_api
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
 
#include <stdio.h>
#include <string.h>
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
#define	TX_OBJ	1

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static u_int32 G_sigCnt;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void 	usage(void);
static int32 	PrintError(char *info, int nr);
static void 	SigHandler(u_int32 sigCode);


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
	printf("Usage: sja1000_xmt [<opts>] <device> [<opts>]\n");
	printf("Function: Transmit process for SJA1000 test\n");
	printf("Options:\n");
	printf("    device       device name                  [none]\n");
#if	1
	printf("    -f=<num>     transmit object (1..2)       [1]\n");
#endif
	printf("    -i=<id>      identifier for obj           [1]\n");
#if	1
	printf("    -e           use extended identifier      [std]\n");
#endif
	printf("    -l=<len>     number of bytes per message  [8]\n");
	printf("    -n=<num>     number of messages to send   [1]\n");
	printf("    -d=<num>     sleep <num> ms between msgs  [0]\n");
	printf("    -q=<ent>     number of xmt queue entries  [100]\n");
#if	0
	printf("    -m=<mode>    transmit mode (0/1)          [1]\n");	
	printf("                 0=send msg after receiving RTR\n");	
	printf("                 1=send msg immediately\n");	
#endif
	printf("    -m=<mode>    transmit mode (1/3)          [1]\n");	
	printf("                 0=send msg after receiving RTR\n");	
	printf("                 1=send msg immediately\n");	
	printf("                 3=send rtr msg immediately\n");

	printf("    -s           don't install signals\n");	
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
	int32	path=0, ret=1, n, i, error, mode, nosigs;
	u_int32 extid, nr, qentries, delay, entries, nil, test;
	u_int32 id=1, id_tmp, datlen, nmsg;
	char	*device,*str,*errstr,buf[40];
	u_int8 data[8];

	G_sigCnt = 0;

	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("f=i=etl=n=d=q=m=s?", buf))) { /* check args */
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
	nr       = ((str = UTL_TSTOPT("f=")) ? atoi(str) : 1);
	datlen   = ((str = UTL_TSTOPT("l=")) ? atoi(str) : 8);
	nmsg     = ((str = UTL_TSTOPT("n=")) ? atoi(str) : 1);
	qentries = ((str = UTL_TSTOPT("q=")) ? atoi(str) : 100);
	mode     = ((str = UTL_TSTOPT("m=")) ? atoi(str) : 1);
	delay    = ((str = UTL_TSTOPT("d=")) ? atoi(str) : 0);

	if( (str = UTL_TSTOPT("i=")))
		sscanf( str, "%lx", &id );

	extid    = (UTL_TSTOPT("e") ? 1 : 0);
	nosigs   = (UTL_TSTOPT("s") ? 1 : 0);
	test	 = (UTL_TSTOPT("t") ? 1 : 0);

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = sja1000_init(device)) < 0)
		return PrintError("open",0);

	/*--------------------+
    |  config             |
    +--------------------*/

	/*--- config object ---*/
	if( sja1000_config_msg( path, nr, CPL_DIR_XMT, extid,
							id, 0, qentries ) < 0){
		PrintError("config message",0);
		goto abort;
	}
		
	/*--- install signals ---*/
	if( (error = UOS_SigInit( SigHandler )) ){
		printf("Can't initialize signal handling error %d", error);
		goto abort;
	}

	if( (error = UOS_SigInstall( UOS_SIG_USR1 )) ){
		printf("Can't install signal error=%d", error);
		goto abort;
	}

	if( !nosigs ){
		if( sja1000_set_xmtsig( path, nr, UOS_SIG_USR1 ) < 0){
			PrintError("install signal for msg queue", nr);
			goto abort;
		}
	}

	/*--------------------------+
	|  Wait for messages  	    |
	+--------------------------*/
	printf("Sending messages\n");

	/*--- build data frame ---*/

	if (test == 0)
	{
		data[0] = 0x00;
		for(i=1; i<8; i++ )
			data[i] = 0xa0 + i;
	}
	else
		for(i=0; i<8; i++ )
			data[i] = 0xa0 + i;

	for( n=0; n<nmsg; n++ ){

		if (test == 0) {
			data[0] = n;
			id_tmp = id;
		}
		else
			id_tmp = id+n;

		if( sja1000_write_msg( path, nr, 0, mode, id_tmp, datlen, data ) < 0 ){
			if( UOS_ErrnoGet() == ERR_OSS_SIG_OCCURED )
				goto abort;

			PrintError("send message",nr);
			n--;
		}
		if( delay != 0 )
			UOS_Delay( delay );
	}

	if( sja1000_queue_status( path, nr, &entries, &nil ) < 0 )
		PrintError( "get queue status", 0 );

	printf("Messages sent. Signals received: %d, "
		   "free xmit queue entries: %d\n", G_sigCnt, entries );

	/*--------------------+
    |  cleanup            |
    +--------------------*/
abort:
	if( !nosigs ){
		if( sja1000_clr_xmtsig( path, nr ) < 0)
			PrintError("remove signal for queue",nr);
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
	G_sigCnt++;
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
