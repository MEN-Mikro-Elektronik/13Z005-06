/****************************************************************************
 ************                                                    ************
 ************                    SJA1000_CNT                     ************
 ************                                                    ************
 ****************************************************************************
 *
 *       Author: ww
 *        $Date: 2004/10/06 14:51:16 $
 *    $Revision: 1.2 $
 *
 *  Description: Example how can dump SJA1000 interrupt counters
 *                  and configuration data
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
/*--------------------------------------+
|   GLOBALS                             |
+--------------------------------------*/
static int32	g_verbose;

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/

static void 	usage(void);
static int32 	PrintError(char *info, int nr);


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
	printf("Usage: sja1000_cnt [<opts>] <device> [<opts>]\n");
	printf("Function: Dump SJA1000 interrupt counters and configuration data\n");
	printf("Options:\n");
	printf("    device       device name                 [none]\n");
	printf("    -v           verbose\n");
	printf("\n");
	printf("(c) 1999,2004 by MEN mikro elektronik GmbH\n\n");
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
	int32	path=0,ret=1,n,i;
	char	*device,*errstr,buf[40];
	u_int32	data[16];
	u_int32 entries, nil;

	/*--------------------+
    |  check arguments    |
    +--------------------*/
	if ((errstr = UTL_ILLIOPT("=?v", buf))) {	/* check args */
		printf("*** %s\n", errstr);
		return(1);
	}
	if (UTL_TSTOPT("?")) {						/* help requested ? */
		usage();
		return(1);
	}
	g_verbose = 0;
	if (UTL_TSTOPT("v"))
		g_verbose++;

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

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = sja1000_init(device)) < 0)
		return PrintError("open",0);

	if( sja1000_read_counter( path, 0, (u_int32 *)data, sizeof(data) ) < 0){
		PrintError("read_register",0);
		goto abort;
	}
	printf("Interrupt Counters:\n");
	printf("irqCount 			= %d\n",data[0]);
	printf("irqBusError			= %d\n",data[1]);
	printf("irqArbitrationLost 	= %d\n",data[2]);
	printf("irqErrorPassive 	= %d\n",data[3]);
	printf("irqWakeUp 			= %d\n",data[4]);
	printf("irqDataOverrun 		= %d\n",data[5]);
	printf("irqErrorWarning 	= %d\n",data[6]);
	printf("irqTransmit 		= %d\n",data[7]);
	printf("irqReceive 			= %d\n",data[8]);
	printf("\n");

    if( sja1000_read_configuration( path, 0, (u_int32 *)data, sizeof(data) ) < 0){
    	PrintError("read_register",0);
		goto abort;
    }
    printf("ModeControlReg      = %d\n",  data[ 0]);
    printf("dbgLevel            = 0x%x\n",data[ 1]);
    printf("inClk               = %d\n",  data[ 2]);
    printf("canClk              = %d\n",  data[ 3]);
    printf("irqFlag             = %d\n",  data[ 4]);
    printf("physIntf            = %d\n",  data[ 5]);
    printf("cbpFlag             = %d\n",  data[ 6]);
    printf("dualFilterMode      = %d\n",  data[ 7]);
    printf("mask                = 0x%x\n",data[ 8]);
    printf("acccode             = 0x%x\n",data[ 9]);
    printf("accmask             = 0x%x\n",data[10]);
    printf("bitrate             = %d\n",  data[11]);
    printf("spl                 = %d\n",  data[12]);
    printf("tseg1               = %d\n",  data[13]);
    printf("tseg2               = %d\n",  data[14]);
    printf("extended            = %d\n",  data[15]);

    if (g_verbose) {

        printf("\nCheck Message Objects:\n");

	    for (i=0; i<3; i++)
	    {
	    	if( sja1000_queue_status( path, i, &entries, &nil ) < 0 )
	    		PrintError( "get queue status", 0 );

	    	printf("Queue %d,  entries: %d\n", i, entries );
	    }
    }
	/*--------------------+
    |  cleanup            |
    +--------------------*/
abort:
	if ( sja1000_term(path) < 0)
		PrintError("close",0);

	return(ret);
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
	printf("*** can't %s: %s\n", info, sja1000_errmsg(UOS_ErrnoGet()));
	return 1;
}
