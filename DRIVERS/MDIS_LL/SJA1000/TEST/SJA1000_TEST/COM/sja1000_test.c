/****************************************************************************
 ************                                                    ************
 ************                    SJA1000_TEST                    ************
 ************                                                    ************
 ****************************************************************************
 *  
 *       Author: kp
 *        $Date: 2004/10/06 14:51:25 $
 *    $Revision: 1.4 $
 *
 *  Description: Automatic driver test for SJA1000 driver
 *
 *  Requires two CAN interfaces connected to the local CPU (e.g. B5)
 *
 *  Functions under test:
 *	  - Signal handling for error, rcv and xmt (set_rcvsig, set_xmtsig)
 *	  - message receiption (read_msg, queue_status)	 
 *	  - message transmission (write_msg) 	 
 *	  - messages with standard and extended identifiers	 
 *	  - all bitrates	 
 *	  - queue overrun handling	 
 *	  - bus off handling & recovery	 
 *	  - RTR frame handling	 
 *	  - Xmt/Rcv calls speed test	 
 *
 *     Required: libraries: mdis_api, usr_oss, usr_utl, sja1000_api
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
#define PTD(path) ((path)==G_path1 ? 1:2)

#define GLOB_ACCMASK1 	0xffffffff
#define GLOB_ACCMASK2 	0xfffffffe
#define GLOB_ACCCODE1 	0x00000000
#define GLOB_ACCCODE2 	0x00000000
#define GLOB_ACCMASK	1	/* maybe wrong */

#define MASK15			0xfffff3ff

#define SPEEDTEST_BUFS	1000

/*
 some MDIS implementations allows only two signals
 but more signals may work ;-)
*/
#ifndef UOS_SIG_USR3
#	define UOS_SIG_USR3 (UOS_SIG_USR2 + 1)
#	if (UOS_SIG_USR3 > UOS_SIG_MAX)
#		error "*** UOS_SIG_USR3 not supported for this MDIS implementation!"
#	endif	
#endif

#ifndef UOS_SIG_USR4
#	define UOS_SIG_USR4 (UOS_SIG_USR3 + 1)
#	if (UOS_SIG_USR4 > UOS_SIG_MAX)
#		error "*** UOS_SIG_USR4 not supported for this MDIS implementation!"
#	endif	
#endif

/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static u_int32 G_sigErr1=0, G_sigErr2=0, G_sigRcv=0, G_sigXmt=0;
static int32 G_path1, G_path2;

/*--- id table for rcv objects ---*/
static u_int32 G_stdMsgId[] = {
	0,2,4,6,8,0xa,0xc,0xe,0x10,0x12,0x14,0x16,0x100,0x200,0x400
};
static u_int32 G_extMsgId[] = {
	0,2,4,6,8,0xa,0xc,0xe,0x10,0x12,0x14,0x16,0x100,0x15555555,0x1aaaaaaa
};

static u_int32 G_matchId[] = {
	2,2,2,1,1,1,1,1,1,1,1,1,2,2,2
};

/*--- id table for xmt messages ---*/
static u_int32 G_stdSndId[] = {
	0,1,2,3,4,5,6,8,0xa,0xc,0xe,0x10,0x12,0x14,0x16,0xff,0x100,0x101,0x200,
	0x201, 0x400, 0xc01,0x402,0x555
};
static u_int32 G_extSndId[] = {
	0,1,2,3,4,5,6,8,0xa,0xc,0xe,0x10,0x12,0x14,0x16,0xff,0x100,0x101,
	0x15555554,0x15555555,0x15555556, 0x1aaaaaaa, 0x1aaaaaab
};

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/

static void usage(void);
static int32 PrintError(char *info, u_int32 deviceNr, u_int32 msgNr );
static void SigHandler( u_int32 sigCode );
static int Config( u_int32 extIds );
static void ShowError( int32 path);
static void GenMsgData( u_int32 id, u_int8 seq, u_int8 *data );
static int CheckMsgData( u_int32 id, u_int8 datlen, u_int8 *data,
						 u_int32 deviceNr, u_int32 nr);
static int IdTest( u_int32 extIds );
static int RateTest(void);
static int QoverTest(void);
static int BusOffTest(void);
static int RtrTest(void);
static int SpeedTest(void);
static int CheckErrorQ(void);

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
	printf("Usage: sja1000_test [<opts>] <device1> <device2> [<opts>]\n");
	printf("Function: Automatic test for SJA1000 driver\n");
	printf("Options:\n");
	printf("    device1      first device name           [none]\n");
	printf("    device2      second device name          [none]\n");
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
int main(argc,argv)
int  argc;
char *argv[];
{
	int32	n, error, ret=1;
	u_int32	nr;
	char	*device1=NULL,*device2=NULL,*errstr,buf[40];

	/*--- init globals ---*/
	G_sigErr1 = G_sigErr2 = G_sigRcv = G_sigXmt = 0;
	
	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("?", buf))) {	/* check args */
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
	for (n=1; n<argc; n++)
		if (*argv[n] != '-') {
			if( device1 == NULL )
				device1 = argv[n];
			else {
				device2 = argv[n];
				break;
			}
		}

	if (device1==NULL || device2==NULL) {
		usage();
		return(1);
	}

	/*--------------------+
    |  open paths         |
    +--------------------*/
	if ((G_path1 = sja1000_init(device1)) < 0)
		return PrintError("open",1,0);

	if ((G_path2 = sja1000_init(device2)) < 0)
		return PrintError("open",2,0);


	/*--- show driver revision string ---*/
	{
		M_SG_BLOCK blk;

		if( M_getstat( G_path1, M_MK_REV_SIZE, &blk.size ) < 0 )
			return PrintError("get revision size",1,0);

		blk.data = (void *)malloc(blk.size);

		if( M_getstat( G_path1, M_MK_BLK_REV_ID, (int32 *)&blk ) < 0 )
			return PrintError("get revision id string",1,0);

		printf("=========== REVISION STRING ===========\n%s\n", blk.data );
		free(blk.data);
	}

	/*--- setup for signal handling ---*/
	if( (error = UOS_SigInit( SigHandler )) ){
		printf("Can't initialize signal handling error %d", error);
		goto abort;
	}

	if( (error = UOS_SigInstall( UOS_SIG_USR1 )) ){
		printf("Can't install signal UOS_SIG_USR1 err=%d", error);
		goto abort;
	}
	if( (error = UOS_SigInstall( UOS_SIG_USR2 )) ){
		printf("Can't install signal UOS_SIG_USR2 err=%d", error);
		goto abort;
	}
	if( (error = UOS_SigInstall( UOS_SIG_USR3 )) ){
		printf("Can't install signal UOS_SIG_USR3 err=%d", error);
		goto abort;
	}
	if( (error = UOS_SigInstall( UOS_SIG_USR4 )) ){
		printf("Can't install signal UOS_SIG_USR4 err=%d", error);
		goto abort;
	}

	/*--------------------+
	|  Do tests           |
	+--------------------*/
	if( Config( FALSE ) < 0 )					/* setup parameters */
		goto abort;

	/*--- install signals ---*/
	if( sja1000_set_rcvsig( G_path1, 0, UOS_SIG_USR1 ) < 0){
		PrintError("install signal for error queue",1,0);
		goto abort;
	}
	if( sja1000_set_xmtsig( G_path1, 1, UOS_SIG_USR2 ) < 0){
		PrintError("install signal for xmt msg queue",1,0);
		goto abort;
	}
	if( sja1000_set_rcvsig( G_path2, 0, UOS_SIG_USR3 ) < 0){
		PrintError("install signal for error queue",2,0);
		goto abort;
	}
	for( nr=1; nr<=15; nr++ ){
		if( sja1000_set_rcvsig( G_path2, nr, UOS_SIG_USR4 ) < 0){
			PrintError("install signal for rcv msg queue",2,nr);
			goto abort;
		}
	}

	printf("Configuration ok.\n");


	/*--- Standard identifier test ---*/
	printf("Test Standard Identifiers and masking...\n");
	if( IdTest( FALSE ) < 0 )
		goto abort;

	if( Config( TRUE ) < 0 )	/* config for extended IDs */
		goto abort;

	printf("Test Extended Identifiers and masking...\n");
	if( IdTest( TRUE ) < 0 )
		goto abort;

	printf("Testing all Bitrates...\n");
	if( RateTest() < 0 )
		goto abort;

	if( Config( FALSE ) < 0 )	/* setup for std ids */
		goto abort;

	printf("Testing queue overruns...\n");
	if( QoverTest() < 0 )
		goto abort;
	
	printf("Testing bus off handling...\n");
	if( BusOffTest() < 0 )
		goto abort;
	
	if( Config( FALSE ) < 0 )	/* setup for std ids */
		goto abort;

	printf("Testing RTR frame handling...\n");
	if( RtrTest() < 0 )
		goto abort;
	
	printf("Speed test...\n");
	if( SpeedTest() < 0 )
		goto abort;
	

	/*--------------------+
    |  cleanup            |
    +--------------------*/
	ret = 0;
abort:


	printf("Cleanup...\n");
	/*--- deinstall signals ---*/
	if( sja1000_clr_rcvsig( G_path1, 0 ) < 0)
		PrintError("deinstall signal for error queue",1,0);

#if 0
	if( sja1000_clr_xmtsig( G_path1, 1 ) < 0)
		PrintError("deinstall signal for xmt msg queue",1,0);
#endif
	if( sja1000_clr_rcvsig( G_path2, 0 ) < 0)
		PrintError("deinstall signal for error queue",2,0);

	for( nr=2; nr<=15; nr++ ){
		if( sja1000_clr_rcvsig( G_path2, nr ) < 0)
			PrintError("deinstall signal for rcv msg queue",2,nr);
	}

	UOS_SigExit();

	if ( sja1000_term(G_path1) < 0)
		PrintError("close",1,0);

	if ( sja1000_term(G_path2) < 0)
		PrintError("close",2,0);

	return(ret);
}

static void GenMsgData( u_int32 id, u_int8 seq, u_int8 *data )
{
	data[0] = id >> 24;
	data[1] = id >> 16;
	data[2] = id >> 8;
	data[3] = id;
	data[4] = seq;
	data[5] = 0xaa;
	data[6] = 0xbb;
	data[7] = 0xcc;
}

static int CheckMsgData( u_int32 id, u_int8 datlen, u_int8 *data,
						 u_int32 deviceNr, u_int32 nr)
{
	u_int8 refBuf[8];
	int32 i, ret=0;

	GenMsgData( id, 0, refBuf );

	for(i=0; i<datlen; i++){
		if( i!=4 && data[i] != refBuf[i] ){
			printf("*** Dev%d (Msg Nr %d): bad data received:\n", deviceNr, 
				   nr );
			printf("    Id=0x%x off=%d is=0x%02x should be=0x%02x\n",
				   id, i, data[i], refBuf[i] );
			ret = -1;
		}
	}
	return ret;
}
			
static int IdTest( u_int32 extIds )
{
	u_int32 i, seq=0, nr, totMsg=0;
	u_int8 sndBuf[8], rcvBuf[8];
	u_int32 *sndMsg = extIds ? G_extSndId : G_stdSndId;
	u_int32 *msgId = extIds ? G_extMsgId : G_stdMsgId;
	u_int32 nSnds = (extIds ? sizeof(G_extSndId) : 
					 sizeof(G_stdSndId))/sizeof(u_int32);
	u_int32 id, datlen, mask, entries, direction;
		
	/*
	 * Send messages using device 1
	 */

	for( i=0; i<nSnds; i++ ){
		GenMsgData( sndMsg[i], seq++, sndBuf );

		if( sja1000_write_msg( G_path1, 1, 1000, 1, sndMsg[i], 
							  5+(i%4), sndBuf) < 0 ){
			PrintError("send msg",1,1);
			goto abort;
		}
		if( CheckErrorQ() < 0 )
			goto abort;
	}

	/*
	 * Check received messages on device 2
	 */
	for( nr=1; nr<=15; nr++ ){

		if( sja1000_queue_status( G_path2, nr, &entries, &direction ) < 0){
			PrintError("get msg queue status",2,nr);
			goto abort;
		}	

		if( entries != G_matchId[nr-1] ){
			printf("*** Dev%d (Msg Nr %d): bad number of messages in queue\n",
				   2, nr );
			printf("    in queue: %d, expected %d\n", entries,G_matchId[nr-1]);
			goto abort;
		}

		/*--- read message ---*/
		if( sja1000_read_msg( G_path2, nr, 1000, &id, &datlen, rcvBuf ) < 0){
			PrintError("read message",2,nr);
			goto abort;
		}

		/*--- check for correct ID ---*/
		mask = GLOB_ACCMASK;
		if( nr==15 ) mask &= MASK15;

		if( (id&mask) != (msgId[nr-1]&mask) ){
			printf("*** Dev%d (Msg Nr %d): Bad ID 0x%x received "
				   "(msgId[]=%x)\n",
				   2, nr, id, msgId[nr-1] );
			goto abort;
		}

		/*--- check data ---*/
		if( CheckMsgData( id, datlen, rcvBuf, 2, nr ) < 0 )
			goto abort;

		totMsg += entries;

		if( CheckErrorQ() < 0 )
			goto abort;
	}

	/*--- check number of signals ---*/
	if( totMsg != G_sigRcv ){
		printf("*** Dev%d (Msg Nr %d): bad number of rcv signals received\n",
			   2, 0 );
		printf("    received: %d, expected %d\n", G_sigRcv, totMsg);
		goto abort;
	}
	if( nSnds != G_sigXmt ){
		printf("*** Dev%d (Msg Nr %d): bad number of xmt signals received\n",
			   1, 1 );
		printf("    received: %d, expected %d\n", G_sigXmt, nSnds);
		goto abort;
	}
	G_sigRcv = G_sigXmt = 0;
		
	return 0;
 abort:	
	return -1;
}

static int RateTest(void)
{
	u_int32 rate;
	u_int32 nSnds = 19, i;
	u_int8 sndBuf[8], rcvBuf[8];
	u_int32 id, datlen;

	for( rate=0; rate<9; rate++ ){
		printf(" rate=%d\n", rate );

		/*--- disable bus ---*/
		if( sja1000_enable( G_path1, FALSE ) < 0 ){
			PrintError("disable bus activity",1,0);
			goto abort;
		}
		if( sja1000_enable( G_path2, FALSE ) < 0 ){
			PrintError("disable bus activity",2,0);
			goto abort;
		}

		/*--- configure Bitrate ---*/
		if( sja1000_set_bitrate( G_path1, rate, 0 ) < 0 ){
			PrintError("set bitrate",1,0);
			goto abort;
		}
		if( sja1000_set_bitrate( G_path2, rate, 0 ) < 0 ){
			PrintError("set bitrate",2,0);
			goto abort;
		}
	
		/*--- enable bus ---*/
		if( sja1000_enable( G_path1, TRUE ) < 0 ){
			PrintError("enable bus activity",1,0);
			goto abort;
		}
		if( sja1000_enable( G_path2, TRUE ) < 0 ){
			PrintError("enable bus activity",2,0);
			goto abort;
		}
		/*
		 * Send messages using device 1 (id=0)
		 */

		for( i=0; i<nSnds; i++ ){
			GenMsgData( 0, 0, sndBuf );

			if( sja1000_write_msg( G_path1, 1, 1000, 1, 0, 
								  8, sndBuf) < 0 ){
				PrintError("send msg",1,1);
				goto abort;
			}
			if( CheckErrorQ() < 0 )
				goto abort;
		}

		/*
		 * Check received messages on device 2 (obj 1)
		 */
		for( i=0; i<nSnds; i++ ){
			
			/*--- read message ---*/
			if( sja1000_read_msg( G_path2, 1, 1000, &id, &datlen, rcvBuf ) < 0){
				PrintError("read message",2,1);
				goto abort;
			}
			if( CheckErrorQ() < 0 )
				goto abort;
		}
	}
	return 0;
 abort:	
	return -1;
}		

static int QoverTest(void)
{
	u_int32 nSnds = 100, i;
	u_int8 sndBuf[8];
	u_int32 errcode, nr, entries, direction;

	G_sigErr2 = 0;
	/*
	 * Send messages using device 1 (id=0)
	 */

	for( i=0; i<nSnds; i++ ){
		GenMsgData( 0, 0, sndBuf );

		if( sja1000_write_msg( G_path1, 1, 1000, 1, 0, 
							  8, sndBuf) < 0 ){
			PrintError("send msg",1,1);
			goto abort;
		}
	}

	if( G_sigErr1 ){
		G_sigErr1 = 0;
		ShowError( G_path1 );
		goto abort;
	}
	if( !G_sigErr2 ){
		printf("*** Dev%d: no error signal received!\n",
			   2);
		goto abort;
	}

	G_sigErr2 = 0;

	if( sja1000_queue_status( G_path2, 0, &entries, &direction ) < 0){
		PrintError("get error queue status",2,0);
		goto abort;
	}

	if( entries != 1 ){
		printf("*** Dev%d: bad number of entries in error queue!\n",
			   2);		
		goto abort;
	}

	if( sja1000_read_error( G_path2, &errcode, &nr ) < 0 ){
		PrintError("read from error queue",2,0);
		goto abort;
	}

	if( errcode != CPL_QOVERRUN ){
		printf("*** Dev%d: bad errcode %d!\n", 2, errcode);		
		goto abort;
	}

	if( sja1000_queue_clear( G_path2, 1, 0 ) < 0 ){
		PrintError("clear receive queue",2,0);
		goto abort;
	}

	

	return 0;
 abort:	
	return -1;
}		

static int BusOffTest(void)
{
	u_int32 nSnds = 2, i;
	u_int8 sndBuf[8];
	u_int32 errcode, nr, entries, expErrcode, direction;

	G_sigErr1 = 0;
	G_sigErr2 = 0;

	printf(" force bus off\n");
	/*--- disable bus ---*/
	if( sja1000_enable( G_path1, FALSE ) < 0 ){
		PrintError("disable bus activity",1,0);
		goto abort;
	}
	if( sja1000_enable( G_path2, FALSE ) < 0 ){
		PrintError("disable bus activity",2,0);
		goto abort;
	}

	/*--- configure DIFFERENT bitrates to force bus errors ---*/
	if( sja1000_set_bitrate( G_path1, 0, 0 ) < 0 ){
		PrintError("set bitrate",1,0);
		goto abort;
	}
	if( sja1000_set_bitrate( G_path2, 3, 0 ) < 0 ){
		PrintError("set bitrate",2,0);
		goto abort;
	}
	
	/*--- enable bus ---*/
	if( sja1000_enable( G_path1, TRUE ) < 0 ){
		PrintError("enable bus activity",1,0);
		goto abort;
	}
	if( sja1000_enable( G_path2, TRUE ) < 0 ){
		PrintError("enable bus activity",2,0);
		goto abort;
	}

	/*
	 * Send messages using device 1 (id=0)
	 */

	for( i=0; i<nSnds; i++ ){
		GenMsgData( 0, 0, sndBuf );

		if( sja1000_write_msg( G_path1, 1, 1000, 1, 0, 
							  8, sndBuf) < 0 ){
			PrintError("send msg",1,1);
			goto abort;
		}
	}

	UOS_Delay( 1000 );			/* wait for bus off */
	
	/*
	 * Only device 1 reports warning/bus off. I'm not sure why
	 */
	if( G_sigErr2 ){
		G_sigErr2 = 0;
		ShowError( G_path2 );
		goto abort;
	}
	if( !G_sigErr1 ){
		printf("*** Dev%d: no error signal received!\n",
			   1);
		goto abort;
	}

	G_sigErr1 = 0;

	if( sja1000_queue_status( G_path1, 0, &entries, &direction ) < 0){
		PrintError("get error queue status",1,0);
		goto abort;
	}

	if( entries != 2 ){
		printf("*** Dev%d: bad number of entries in error queue (%d)!\n",
			   1, entries);		
		goto abort;
	}

	for(i=0; i<entries; i++){
		if( sja1000_read_error( G_path1, &errcode, &nr ) < 0 ){
			PrintError("read from error queue",1,0);
			goto abort;
		}

		expErrcode = i==0 ? CPL_WARN_SET : CPL_BUSOFF_SET;

		if( errcode != expErrcode ){
			printf("*** Dev%d: bad errcode %d! (expected)\n", 2, 
				   errcode, expErrcode);		
			goto abort;
		}
	}

	printf(" recover bus off\n");
	/*--- clear xmt queue, abort any transmissions ---*/
	if( sja1000_queue_clear( G_path1, 1, TRUE ) < 0 ){
		PrintError("clear xmt queue",1,0);
		goto abort;
	}

	/*--- configure good bitrate ---*/
	if( sja1000_set_bitrate( G_path2, 0, 0 ) < 0 ){
		PrintError("set bitrate",2,0);
		goto abort;
	}

	/*--- try to recover from busoff ---*/
	if( sja1000_clear_busoff( G_path1 ) < 0 ){
		PrintError("clear busoff",1,0);
		goto abort;
	}
	
	UOS_Delay( 1000 );			/* wait for bus off released */
	
	if( G_sigErr2 ){
		G_sigErr2 = 0;
		ShowError( G_path2 );
		goto abort;
	}
	if( !G_sigErr1 ){
		printf("*** Dev%d: no error signal received! (B)\n",
			   1);
		goto abort;
	}

	G_sigErr1 = 0;

	if( sja1000_queue_status( G_path1, 0, &entries, &direction ) < 0){
		PrintError("get error queue status (B)",1,0);
		goto abort;
	}

	if( entries != 2 ){
		printf("*** Dev%d: bad number of entries in error queue (%d) (B)!\n",
			   1, entries);		
		goto abort;
	}

	for(i=0; i<entries; i++){
		if( sja1000_read_error( G_path1, &errcode, &nr ) < 0 ){
			PrintError("read from error queue (B)",1,0);
			goto abort;
		}

		expErrcode = i==0 ? CPL_BUSOFF_CLR : CPL_WARN_CLR;

		if( errcode != expErrcode ){
			printf("*** Dev%d: bad errcode %d! (expected) (B)\n", 2, 
				   errcode, expErrcode);		
			goto abort;
		}
	}

	return 0;
 abort:	
	return -1;
}		

static int RtrTest(void)
{
	u_int8 sndBuf[8], rcvBuf[8];
	u_int32 id, datlen;
	int32 ret;

	G_sigErr2 = 0;

	/*
	 * Send RTR frame to get obj with ID 0, device 1 shouldn't respond now
	 */
	if( sja1000_rtr( G_path2, 1, 0, 0 ) < 0 ){
		PrintError("send rtr",2,1);
		goto abort;
	}

	ret = sja1000_read_msg( G_path2, 1, 300, &id, &datlen, rcvBuf );

	if( ret == 0 ){
		printf("*** device2 shouldn't respond to RTR now\n");
		goto abort;
	}

	if( CheckErrorQ() ) goto abort;
	/*
	 * Send message using device 1 (id=0, wait for RTR)
	 */

	GenMsgData( 0, 0, sndBuf );

	if( sja1000_write_msg( G_path1, 1, 1000, 0, 0, 
						  8, sndBuf) < 0 ){
		PrintError("send msg",1,1);
		goto abort;
	}

	/*
	 * Send RTR frame to get obj with ID 0, device 1 SHOULD respond now
	 */
	if( sja1000_rtr( G_path2, 1, 0, 0 ) < 0 ){
		PrintError("send rtr (B)",2,1);
		goto abort;
	}

	if( sja1000_read_msg( G_path2, 1, 300, &id, &datlen, rcvBuf ) < 0 ){
		PrintError("receive msg",2,1);
		goto abort;
	}

	if( CheckMsgData( 0, datlen, rcvBuf, 2, 1 ) < 0 )
		goto abort;

	if( CheckErrorQ() ) goto abort;

	return 0;
 abort:	
	return -1;
}		
		
static int SpeedTest(void)
{
	u_int8 sndBuf[8], rcvBuf[8];
	u_int32 id, datlen, startTime, endTime, i, pass;

	G_sigErr1 = 0;
	G_sigErr2 = 0;

	/*--- configure message object for device1 (sender) ---*/
	if( sja1000_config_msg( G_path1, 1, CPL_DIR_XMT, 0, 0, 0, 
						   SPEEDTEST_BUFS ) < 0){
		PrintError("config msg queue",1,1);
		goto abort;
	}
	
	/*--- configure message object for device2 (receiver) ---*/
	if( sja1000_config_msg( G_path2, 1, CPL_DIR_RCV, 0, 
						   0, 0, SPEEDTEST_BUFS ) < 0){
		PrintError("config msg queue",2,1);
		goto abort;
	}
	
	for( pass=0; pass<2; pass++ ){
		printf(" Speed with signals %sabled...\n", pass ? "dis" : "en");

		startTime = UOS_MsecTimerGet();

		/*--- Transmit objects (0 bytes data length) ---*/
		for(i=0; i<SPEEDTEST_BUFS; i++ ){
			if( sja1000_write_msg( G_path1, 1, 0, 1, 0, 
								  0, sndBuf) < 0 ){
				PrintError("send msg",1,1);
				goto abort;
			}
		}
		if( CheckErrorQ() ) goto abort;


		endTime = UOS_MsecTimerGet();
		printf("  Xmt Speed: %d us/call\n", (endTime-startTime)*1000 / 
			   SPEEDTEST_BUFS );

		UOS_Delay(100);

		/*--- Receive Objects ---*/
		startTime = UOS_MsecTimerGet();

		for(i=0; i<SPEEDTEST_BUFS; i++ ){
			if( sja1000_read_msg( G_path2, 1, -1, &id, &datlen, rcvBuf ) < 0) {
				PrintError("receive msg",2,1);
				goto abort;
			}
		}
		if( CheckErrorQ() ) goto abort;
		
		endTime = UOS_MsecTimerGet();
		printf("  Rcv Speed: %d us/call\n", (endTime-startTime)*1000 / 
			   SPEEDTEST_BUFS );

		if( pass==1 ) break;

		/*--- now disable signals for xmt/rcv ---*/
		if( sja1000_clr_rcvsig( G_path2, 1 ) < 0){
			PrintError("deinstall signal for rcv msg queue",2,1);
			goto abort;
		}
		if( sja1000_clr_xmtsig( G_path1, 1 ) < 0){
			PrintError("deinstall signal for xmt msg queue",1,1);
			goto abort;
		}
	}

	return 0;
 abort:	
	return -1;
}		

		
static int CheckErrorQ(void)
{
	int ret = 0;

	UOS_SigMask();

	if( G_sigErr1 ){
		G_sigErr1 = 0;
		ShowError( G_path1 );
		ret = -1;
	}
	if( G_sigErr2 ){
		G_sigErr2 = 0;
		ShowError( G_path2 );
		ret = -1;
	}
	UOS_SigUnMask();
	return ret;
}


static void ShowError( int32 path)
{
	u_int32 errcode, nr, entries, n, direction;
	char *str;

	if( sja1000_queue_status( path, 0, &entries, &direction ) < 0){
		PrintError("get error queue status",PTD(path),0);
		goto abort;
	}

	/*--- Read error messages ---*/
	for( n=0; n<entries; n++ ){
		if( sja1000_read_error( path, &errcode, &nr ) < 0 ){
			PrintError("read from error queue",PTD(path),0);
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
		case CPL_MSGLOST:	
			str = "Message lost (driver too slow)";
			break;
		case CPL_QOVERRUN:	
			str = "Message queue overrun (application too slow)";
			break;	
		default:
			str = "UNKNOWN";
			break;
		}
		if( nr == 0 )
			printf("Dev%d: Global error event: %s\n", PTD(path), str );
		else
			printf("Dev%d: Message object %d error: %s\n", PTD(path),nr, str );

	}
 abort:
	return;
}

static int Config( u_int32 extIds )
{
	u_int32 nr;
	u_int32 *msgId = extIds ? G_extMsgId : G_stdMsgId;

	/*--- disable bus ---*/
	if( sja1000_enable( G_path1, FALSE ) < 0 ){
		PrintError("disable bus activity",1,0);
		goto abort;
	}
	if( sja1000_enable( G_path2, FALSE ) < 0 ){
		PrintError("disable bus activity",2,0);
		goto abort;
	}

	/*--- configure 1MB/s ---*/
	if( sja1000_set_bitrate( G_path1, 0, 0 ) < 0 ){
		PrintError("set bitrate",1,0);
		goto abort;
	}
	if( sja1000_set_bitrate( G_path2, 0, 0 ) < 0 ){
		PrintError("set bitrate",2,0);
		goto abort;
	}

	/*--- set acceptance mask ---*/
	if( sja1000_set_accmask( G_path1, extIds, GLOB_ACCMASK1 ) < 0 ){
		PrintError("acceptance mask",1,0);
		goto abort;
	}

	if( sja1000_set_accmask( G_path2, extIds, GLOB_ACCMASK2 ) < 0 ){
		PrintError("acceptance mask",2,0);
		goto abort;
	}

	/*--- config error object ---*/
	if( sja1000_config_msg( G_path1, 0, 0, 0, 0, 0, 20 ) < 0){
		PrintError("config error queue",1,0);
		goto abort;
	}
	if( sja1000_config_msg( G_path2, 0, 0, 0, 0, 0, 20 ) < 0){
		PrintError("config error queue",2,0);
		goto abort;
	}
		
	/*--- enable bus ---*/
	if( sja1000_enable( G_path1, TRUE ) < 0 ){
		PrintError("enable bus activity",1,0);
		goto abort;
	}
	if( sja1000_enable( G_path2, TRUE ) < 0 ){
		PrintError("enable bus activity",2,0);
		goto abort;
	}
	
	/*--- configure message objects for device1 (sender) ---*/
	if( sja1000_config_msg( G_path1, 1, CPL_DIR_XMT, extIds, 0, 0, 20 ) < 0){
		PrintError("config msg queue",1,1);
		goto abort;
	}
	
	/*--- configure message objects for device2 (receiver) ---*/
	for( nr=1; nr<=15; nr++ ){
		if( sja1000_config_msg( G_path2, nr, CPL_DIR_RCV, extIds, 
							   msgId[nr-1], MASK15, 20 ) < 0){
			PrintError("config msg queue",2,nr);
			goto abort;
		}
	}

	return 0;

abort:
	return -1;
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
	switch(sigCode){
	case UOS_SIG_USR1:
		G_sigErr1++;
		break;
	case UOS_SIG_USR2:
		G_sigXmt++;
		break;
	case UOS_SIG_USR3:
		G_sigErr2++;
		break;
	case UOS_SIG_USR4:
		G_sigRcv++;
		break;
	}
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

static int32 PrintError(char *info, u_int32 deviceNr, u_int32 msgNr )
{
	printf("*** Dev%d (Msg Nr %d): can't %s: %s\n", 
		   deviceNr, msgNr, info, sja1000_errmsg(UOS_ErrnoGet()));
	return 1;
}
