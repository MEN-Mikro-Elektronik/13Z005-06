/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: sja1000_drv.c
 *      Project: SJA1000 chip driver (MDIS V4.x)
 *
 *       Author: ww
 *        $Date: 2004/10/06 14:50:10 $
 *    $Revision: 1.13 $
 *
 *  Description: Low level driver for SJA1000 CAN controller chips
 *
 *				 This driver provides a direct layer 2 access to the
 *				 CAN controller. All features of the SJA1000 are supported
 *
 *				 All driver functions are provided through a generic block
 *				 getstat call (SJA1000_BLK_CALL).
 *
 *				 The user library sja1000_api should be used to access
 *				 this driver
 *
 *				 This driver will not work without interrupts!
 *
 *				 Only one channel is used for all message objects.
 *
 *				 The driver uses NON LOCKING mode to allow multiple processes
 *				 to wait for messages objects simultanously.
 *
 *				 Supports buffer queues for each of the xx message objects
 *				 plus one virtual "error object".
 *
 *				 See documentation for further details.
 *
 *     Required: oss.l, desc.l
 *     Switches: _ONE_NAMESPACE_PER_DRIVER_
 *				 M51	use special controller access macros for MEN M51 Module
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

#include "sja1000_int.h"

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
#define	VERBOSEno
#define	LOCAL_MASK
#define	SHno			/* test for schlafhorst */
#define	V2no			/* ed2. 09.04.2003 */

/* lear test */
#if	0
#define	DDBG(_x_,_y_)	DBGWRT_2((DBH,"*p* %s: %02d\n",_x_, _y_));
#else
#define	DDBG(_x_,_y_)	;
#endif

#if	0
#define	XMIT	(TR_Bit|AT_Bit)
#else
#define	XMIT	(TR_Bit)
#endif

/* defines to decouple the meaning of SJA1000 API blocking/nonblocking from the oss.h semaphore-defines which are inverse */
#define SJA1000_API_NONBLOCK       -1
#define SJA1000_API_BLOCK           0


#define CHECK_OBJECT_AVAILABLE     {\
    if (obj->q.first == NULL)\
        return CPL_ERR_NOOBJECT;\
}

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 SJA1000_Init(DESC_SPEC *descSpec,
						  OSS_HANDLE *osHdl,
						  MACCESS *ma,
						  OSS_SEM_HANDLE *devSemHdl,
						  OSS_IRQ_HANDLE *irqHdl,
						  LL_HANDLE **llHdlP);
static int32 SJA1000_Exit(LL_HANDLE **llHdlP );
static int32 SJA1000_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 SJA1000_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 SJA1000_SetStat(LL_HANDLE *llHdl,
							 int32 ch,
							 int32 code,
							 int32 value);
static int32 SJA1000_GetStat(LL_HANDLE *llHdl,
							 int32 ch,
							 int32 code,
							 int32 *valueP);
static int32 SJA1000_BlockRead(LL_HANDLE *llHdl,
							   int32 ch,
							   void *buf,
							   int32 size,
							   int32 *nbrRdBytesP);
static int32 SJA1000_BlockWrite(LL_HANDLE *llHdl,
								int32 ch,
								void *buf,
								int32 size,
								int32 *nbrWrBytesP);
static int32 SJA1000_Irq(LL_HANDLE *llHdl );
static int32 SJA1000_Info(int32 infoType, ... );

static char* Ident( void );
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);
static int32 ConfMsg( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb );
static int32 QueueClear( LL_HANDLE *llHdl, u_int32 nr, u_int32 txabort );
static int32 ReadMsg( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb );
static int32 ReadError( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb );
static int32 WriteMsg( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb );
static void  StartXmt( LL_HANDLE *llHdl, u_int32 nr );
#ifndef	LOCAL_MASK
static int32 InsertRcvMsg( LL_HANDLE *llHdl, u_int32 nr );
#else
static int32 InsertRcvMsg( LL_HANDLE *llHdl, u_int32 nr, u_int32 id );
#endif
static void  InsertErrMsg( LL_HANDLE *llHdl, u_int32 errcode, u_int32 nr );
static void  StatChange( LL_HANDLE *llHdl, u_int8 status );
static int32 SetSig( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb );
static int32 ClrSig( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb );
static int32 QueueStatus( LL_HANDLE *llHdl, u_int32 nr, u_int32 *entriesP, u_int32 *directionP );

static void SJA1000_IrqTx( LL_HANDLE *llHdl );
static void SJA1000_IrqRx( LL_HANDLE *llHdl );

static int32 ReadRegister(
LL_HANDLE *llHdl,
u_int32 select,
u_int32 *bufP,
u_int32 size );

static int32 ReadCounter(
LL_HANDLE *llHdl,
u_int32 select,
u_int32 *bufP,
u_int32 size );

static int32 ReadConfiguration(
LL_HANDLE *llHdl,
u_int32 select,
u_int32 *bufP,
u_int32 size );

static int32 WriteRegister(
LL_HANDLE *llHdl,
u_int32 select,
u_int32 val,
u_int32 idx );

/**************************** SJA1000_GetEntry *******************************
 *
 *  Description:  Initialize drivers jump table
 *
 *---------------------------------------------------------------------------
 *  Input......:  ---
 *  Output.....:  drvP  pointer to the initialized jump table structure
 *  Globals....:  ---
 ****************************************************************************/
#ifdef _ONE_NAMESPACE_PER_DRIVER_
    extern void LL_GetEntry( LL_ENTRY* drvP )
#else
    extern void __SJA1000_GetEntry( LL_ENTRY* drvP )
#endif
{
    drvP->init        = SJA1000_Init;
    drvP->exit        = SJA1000_Exit;
    drvP->read        = SJA1000_Read;
    drvP->write       = SJA1000_Write;
    drvP->blockRead   = SJA1000_BlockRead;
    drvP->blockWrite  = SJA1000_BlockWrite;
    drvP->setStat     = SJA1000_SetStat;
    drvP->getStat     = SJA1000_GetStat;
    drvP->irq         = SJA1000_Irq;
    drvP->info        = SJA1000_Info;
}

/******************************** SJA1000_Init *******************************
 *
 *  Description:  Allocate and return ll handle, initialize hardware
 *
 *                The function initializes all channels with the
 *                definitions made in the descriptor. The interrupt
 *                is disabled.
 *
 *                The following descriptor keys are used:
 *
 *                Deskriptor key        Default          Range
 *                --------------------  ---------------  -------------
 *                DEBUG_LEVEL_DESC      OSS_DBG_DEFAULT  see dbg.h/oss_os.h
 *                DEBUG_LEVEL           OSS_DBG_DEFAULT  see dbg.h/oss_os.h
 *                INCLOCK				-				 -
 *				  PHYSINTF				1				 see sja1000_api.h
 *				  PI_USERCFG	        0				 0x00..0xff
 *
 *				  INCLOCK defines the clock frequency to the CAN controller
 *				  (usually 16000000)
 *
 *				  PHYSINTF defines the CAN physical interface
 *
 *				  PI_USERCFG defines the user configuration when PHYSINTF
 *				  is CPL_USERINTERFACE (0)
 *---------------------------------------------------------------------------
 *  Input......:  descSpec   pointer to descriptor data
 *                osHdl      oss handle
 *                ma         hw access handle
 *                devSemHdl  device semaphore handle
 *                irqHdl     irq handle
 *  Output.....:  llHdlP     ptr to low level driver handle
 *                return     success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_Init( DESC_SPEC       *descP,
						   OSS_HANDLE      *osHdl,
						   MACCESS         *ma,
						   OSS_SEM_HANDLE  *devSemHdl,
						   OSS_IRQ_HANDLE  *irqHdl,
						   LL_HANDLE       **llHdlP )
{
    LL_HANDLE *llHdl = NULL;
    u_int32 gotsize;
    u_int32 value;
	u_int32 piUserCfg;
    int32 	error;
    int32	i;

    /*------------------------------+
    |  prepare the handle           |
    +------------------------------*/
	/* alloc */
    if ((*llHdlP = llHdl = (LL_HANDLE*)
		 OSS_MemGet(osHdl, sizeof(LL_HANDLE), &gotsize)) == NULL)
       return(ERR_OSS_MEM_ALLOC);

	/* clear */
    OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
    llHdl->memAlloc   	= gotsize;
    llHdl->osHdl      	= osHdl;
    llHdl->irqHdl     	= irqHdl;
	llHdl->devSemHdl  	= devSemHdl;
	llHdl->irqFlag		= 0;
	llHdl->xmtHalt 		= TRUE;
	llHdl->xmtRtr 		= TRUE;

	MACCESS_CLONE(*ma, llHdl->ma,0);

    /*------------------------------+
    |  init id function table       |
    +------------------------------*/
	/* drivers ident function */
	llHdl->idFuncTbl.idCall[0].identCall = Ident;
	/* libraries ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;

    /*------------------------------+
    |  prepare debugging            |
    +------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;	/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

    /*------------------------------+
    |  scan descriptor              |
    +------------------------------*/
	/* prepare access */
    if ((error = DESC_Init(descP, osHdl, &llHdl->descHdl)))
		return( Cleanup(llHdl,error) );

    /* DEBUG_LEVEL_DESC */
    if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&value, "DEBUG_LEVEL_DESC")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

    /* DEBUG_LEVEL */
    if ((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&llHdl->dbgLevel, "DEBUG_LEVEL")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    DBGWRT_1((DBH, "LL - SJA1000_Init ma=%x\n",llHdl->ma));
#if	0
	/* these code was called by MDIS/BBIS */
    /*------------------------------+
    | SJA1000 base address          |
    +------------------------------*/
	/* Sub Device Offset */
    if (error = DESC_GetUInt32(llHdl->descHdl, 0,
							   &llHdl->offset, "SUBDEVICE_OFFSET_0"))
		return( Cleanup(llHdl,error) );

	MACCESS_CLONE(*ma, llHdl->ma, llHdl->offset);

	DBGWRT_1((DBH, " ma=%x\n",*ma));
#endif

	/*---------------------------------+
    | SJA1000 configuration parameters |
    +---------------------------------*/

    /* INCLOCK (mandatory) */
    if ((error = DESC_GetUInt32(llHdl->descHdl, DFLT_CLOCK,
								&llHdl->inClk, "INCLOCK")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    /* PHYSINTF */
    if ((error = DESC_GetUInt32(llHdl->descHdl, CPL_ISOHIGHSPEED,
								&llHdl->physIntf, "PHYSINTF")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    /* PI_USERCFG (user config for physical interface) */
    if ((error = DESC_GetUInt32(llHdl->descHdl, 0,
								&piUserCfg, "PI_USERCFG")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    /* CBP comparator bypass function */
    if ((error = DESC_GetUInt32(llHdl->descHdl, 0,
								&llHdl->cbpFlag, "CBYPASSF")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    /* Dual Filter mode */
    if ((error = DESC_GetUInt32(llHdl->descHdl, 0,
								&llHdl->dualFilterMode, "DUAL_FILTER")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    /* Interrupt Mask */
    if ((error = DESC_GetUInt32(llHdl->descHdl, 0,
								&llHdl->mask, "INTERRUPT_MASK")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

	/*-----------------------+
	|  init message objects  |
	+-----------------------*/
	for( i=0; i<NUM_MSG_OBJ; i++ ) {

		llHdl->msgObj[i].nr 	= i;
		llHdl->msgObj[i].q.first= NULL;
		llHdl->msgObj[i].q.dir 	= CPL_DIR_DIS;
	}
    /*------------------------------+
    |  init hardware                |
    +------------------------------*/
	if( (error = _SJA1000_init( llHdl, llHdl->inClk, &llHdl->canClk ))){
		DBGWRT_ERR((DBH," *** SJA1000_Init: error %04x in _SJA1000_init\n",error));
		return( Cleanup(llHdl,error) );
	}

	/*--- set physical interface ---*/
	if( (error = _SJA1000_set_physintf( llHdl, llHdl->physIntf, piUserCfg ))){
		DBGWRT_ERR((DBH," *** SJA1000_Init: error %04x in _SJA1000_physintf\n",error));
		return( Cleanup(llHdl,error) );
	}

	DBGWRT_2((DBH,"  InClk=%d CanClk=%d\n", llHdl->inClk, llHdl->canClk ));

	return(ERR_SUCCESS);
}

/****************************** SJA1000_Exit *********************************
 *
 *  Description:  De-initialize hardware and cleanup memory
 *
 *                The function deinitializes the hardware
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdlP  	ptr to low level driver handle
 *  Output.....:  return    success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_Exit( LL_HANDLE    **llHdlP )
{
    LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;
	OSS_IRQ_STATE oldIrqState;

    DBGWRT_1((DBH, "LL - SJA1000_Exit\n"));

    /*------------------------------+
    |  de-init hardware             |
    +------------------------------*/
	oldIrqState = OSS_IrqMaskR( llHdl->osHdl, llHdl->irqHdl );
	error = _SJA1000_term( llHdl );
	OSS_IrqRestore( llHdl->osHdl, llHdl->irqHdl, oldIrqState );

    /*------------------------------+
    |  cleanup memory               |
    +------------------------------*/
	error = Cleanup(llHdl,error);

	return(error);
}

/****************************** SJA1000_Read *********************************
 *
 *  Description:  Not supported by this driver
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl    ll handle
 *                ch       current channel
 *  Output.....:  valueP   read value
 *                return   success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_Read( LL_HANDLE *llHdl,
						   int32 ch,
						   int32 *valueP )
{
    DBGWRT_1((DBH, "LL - SJA1000_Read: ch=%d\n",ch));

	return(ERR_LL_ILL_FUNC);
}

/****************************** SJA1000_Write ********************************
 *
 *  Description:  Not supported by this driver
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl    ll handle
 *                ch       current channel
 *                value    value to write
 *  Output.....:  return   success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_Write( LL_HANDLE *llHdl,
							int32 ch,
							int32 value )
{
    DBGWRT_1((DBH, "LL - SJA1000_Write: ch=%d\n",ch));

	return(ERR_LL_ILL_FUNC);
}

/****************************** SJA1000_SetStat ******************************
 *
 *  Description:  Set driver status
 *
 *                Following status codes are supported:
 *
 *                Code                 Description                Values
 *                -------------------  -------------------------  ----------
 *                M_LL_DEBUG_LEVEL     driver debug level         see dbg.h
 *                M_MK_IRQ_ENABLE      interrupt enable           0..1
 *                M_LL_IRQ_COUNT       interrupt counter          0..max
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl      ll handle
 *                code       status code
 *                ch         current channel
 *                value      data or
 *                           ptr to block data struct (M_SG_BLOCK)  (*)
 *                (*) = for block status codes
 *  Output.....:  return     success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_SetStat( LL_HANDLE *llHdl,
							  int32  code,
							  int32  ch,
							  int32  value )
{
	int32 error = ERR_SUCCESS;

    DBGWRT_1((DBH, "LL - SJA1000_SetStat: ch=%d code=0x%04x value=0x%x\n",
			  ch,code,value));

    switch(code) {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
			DBGWRT_1((DBH, "     DebugLevel\n"));
            llHdl->dbgLevel = value;
            break;
        /*--------------------------+
        |  enable interrupts        |
        +--------------------------*/
        case M_MK_IRQ_ENABLE:
			DBGWRT_1((DBH, "     IrqEnable\n"));
			/*--- enable/disable all interrupt sources ---*/
			_SJA1000_irq_enable( llHdl, value );
            break;
        /*--------------------------+
        |  set irq counter          |
        +--------------------------*/
        case M_MK_IRQ_COUNT:
			DBGWRT_1((DBH, "     IrqCount\n"));
            llHdl->irqCount = value;
            break;
        /*--------------------------+
        |  (unknown)                |
        +--------------------------*/
        default:
            error = ERR_LL_UNK_CODE;
    }
    DBGWRT_1((DBH, "     error=0x%x\n",error));
	return(error);
}

/****************************** SJA1000_GetStat ******************************
 *
 *  Description:  Get driver status
 *
 *                Following status codes are supported:
 *
 *                Code                 Description                Values
 *                -------------------  -------------------------  ----------
 *				  SJA1000_BLK_CALL	   generic API call           -
 *                M_LL_DEBUG_LEVEL     driver debug level         see dbg.h
 *                M_LL_CH_NUMBER       number of channels         1
 *                M_LL_CH_DIR          direction of curr chan     M_CH_INOUT
 *                M_LL_CH_LEN          length of curr chan [bits] 32 (dummy)
 *                M_LL_CH_TYP          description of curr chan   M_CH_BINARY
 *                M_LL_IRQ_COUNT       interrupt counter          0..max
 *                M_MK_BLK_REV_ID      ident function table ptr   -
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl      ll handle
 *                code       status code
 *                ch         current channel
 *                valueP     ptr to block data struct (M_SG_BLOCK)  (*)
 *                (*) = for block status codes
 *  Output.....:  valueP     data ptr or
 *                           ptr to block data struct (M_SG_BLOCK)  (*)
 *                return     success (0) or error code
 *                (*) = for block status codes
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_GetStat( LL_HANDLE *llHdl,
							  int32  code,
							  int32  ch,
							  int32  *valueP )
{
	M_SG_BLOCK 		*blk	= (M_SG_BLOCK*)valueP;
	SJA1000_CALL_PB *pb;
	int32 			error 	= ERR_SUCCESS;

    DBGWRT_1((DBH, "LL - SJA1000_GetStat: ch=%d code=0x%04x\n",
			  ch,code));

    switch(code){
		case SJA1000_BLK_CALL:
			/*--------------------------------------------------------+
			|  This is our generic API wrapper call                   |
			|  All API functions are executed through this interface  |
			+--------------------------------------------------------*/
			pb = (SJA1000_CALL_PB *)blk->data;

			switch(pb->func) {
			    /* sets */
				case F_SET_ACCMASK:
					DBGWRT_1((DBH, "     AccMask\n"));
					error = _SJA1000_set_accmask( llHdl,
												  pb->p.set_accmask.extended,
												  pb->p.set_accmask.acc_mask );
					break;

				case F_SET_ACCCODE:
					DBGWRT_1((DBH, "     AccCode\n"));
					error = _SJA1000_set_acccode( llHdl,
												  pb->p.set_acccode.extended,
												  pb->p.set_acccode.acc_code );
					break;

				case F_SET_BITRATE:
					DBGWRT_1((DBH, "     BitRate\n"));
					error = _SJA1000_set_bitrate( llHdl,
												  llHdl->canClk,
												  pb->p.set_bitrate.bitrate,
												  pb->p.set_bitrate.spl );
					break;

				case F_CONFIG_MSG:
					DBGWRT_1((DBH, "     ConfigMsg\n"));
					error = ConfMsg( llHdl, pb );
					break;

				case F_ENABLE:
					DBGWRT_1((DBH, "     Enable\n"));
					error = _SJA1000_enable( llHdl, pb->p.enable.enable );
					break;

				case F_WRITE_MSG:
					DBGWRT_1((DBH, "     WriteMsg\n"));
					error = WriteMsg( llHdl, pb );
					break;

				case F_SET_RCVSIG:
					DBGWRT_1((DBH, "     SetRcvSig\n"));
					error = SetSig( llHdl, pb );
					break;

				case F_CLR_RCVSIG:
					DBGWRT_1((DBH, "     ClrRcvSig\n"));
					error = ClrSig( llHdl, pb );
					break;

				case F_SET_XMTSIG:
					DBGWRT_1((DBH, "     SetXmtSig\n"));
					error = SetSig( llHdl, pb );
					break;

				case F_CLR_XMTSIG:
					DBGWRT_1((DBH, "     ClrXmtSig\n"));
					error = ClrSig( llHdl, pb );
				break;

				case F_ABORT:
					DBGWRT_1((DBH, "     AbortTransmission\n"));
					error =  _SJA1000_transmit_abort( llHdl, 0 );
					break;

                /* gets */
				case F_READ_MSG:
					DBGWRT_1((DBH, "     ReadMsg\n"));
					error = ReadMsg( llHdl, pb );
					break;

				case F_READ_ERROR:
					DBGWRT_1((DBH, "     ReadError\n"));
					error = ReadError( llHdl, pb );
					break;
#ifdef	NOTUSED
				case F_RTR:
					DBGWRT_1((DBH, "     Rtr\n"));
					error = _SJA1000_rtr( llHdl, pb->p.rtr.nr );
					break;
#endif
				case F_QUEUE_STATUS:
					DBGWRT_1((DBH, "     QueueStatus\n"));
					error = QueueStatus( llHdl,
										 pb->p.queue_status.nr,
 										 &pb->p.queue_status.entries,
										 &pb->p.queue_status.direction );
					break;

				case F_QUEUE_CLEAR:
					DBGWRT_1((DBH, "     QueueClear\n"));
					error = QueueClear( llHdl,
										pb->p.queue_clear.nr,
										pb->p.queue_clear.txabort );
					break;

				case F_CLEAR_BUSOFF:
					DBGWRT_1((DBH, "     ClearBusOff\n"));
					error = _SJA1000_clear_busoff( llHdl );
#if	0
					/* send next message if any there */
					SJA1000_IrqTx( llHdl );
#endif
					break;

                /* the following functions are for internal test purposes only */

				case F_WRITE_REGISTER:
					DBGWRT_1((DBH, "     WriteRegister\n"));
					error = WriteRegister( llHdl,
										  pb->p.write_register.select,
										  pb->p.write_register.val,
										  pb->p.write_register.idx);
					break;

				case F_READ_REGISTER:
					DBGWRT_1((DBH, "     ReadRegister\n"));
					error = ReadRegister( llHdl,
										  pb->p.read_register.select,
										  pb->p.read_register.bufP,
										  pb->p.read_register.idx);
					break;

				case F_READ_COUNTER:
					DBGWRT_1((DBH, "     ReadCounter\n"));
					error = ReadCounter( llHdl,
										  pb->p.read_counter.select,
										  pb->p.read_counter.bufP,
										  pb->p.read_counter.size);
					break;

				case F_READ_CONFIGURATION:
					DBGWRT_1((DBH, "     ReadConfiguration\n"));
					error = ReadConfiguration( llHdl,
										  pb->p.read_configuration.select,
										  pb->p.read_configuration.bufP,
										  pb->p.read_configuration.size);
					break;

				case F_READ_EDITION:
					DBGWRT_1((DBH, "     ReadEdition\n"));
					pb->p.read_edition.edition = EDITION;
					break;

				default:
					error = ERR_LL_ILL_PARAM;
			}
			break;
			/*--------------------------+
			|  debug level              |
			+--------------------------*/
		case M_LL_DEBUG_LEVEL:
			*valueP = llHdl->dbgLevel;
			break;
			/*--------------------------+
			|  nr of channels           |
			+--------------------------*/
		case M_LL_CH_NUMBER:
			*valueP = CH_NUMBER;
			break;
			/*--------------------------+
			|  channel direction        |
			+--------------------------*/
		case M_LL_CH_DIR:
			*valueP = M_CH_INOUT;
			break;
			/*--------------------------+
			|  channel length [bits]    |
			+--------------------------*/
		case M_LL_CH_LEN:
			*valueP = 32;
			break;
			/*--------------------------+
			|  channel type info        |
			+--------------------------*/
		case M_LL_CH_TYP:
			*valueP = M_CH_BINARY;
			break;
			/*--------------------------+
			|  irq counter              |
			+--------------------------*/
		case M_LL_IRQ_COUNT:
			*valueP = llHdl->irqCount;
			break;
			/*--------------------------+
			|   ident table pointer     |
			|   (treat as non-block!)   |
			+--------------------------*/
		case M_MK_BLK_REV_ID:
			*valueP = (int32)&llHdl->idFuncTbl;
			break;
			/*--------------------------+
			|  (unknown)                |
			+--------------------------*/
		default:
			error = ERR_LL_UNK_CODE;
    }
    DBGWRT_1((DBH, "     error=0x%x\n",error));
	return(error);
}

/******************************* SJA1000_BlockRead ***************************
 *
 *  Description:  Not supported by this driver
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl        ll handle
 *                ch           current channel
 *                buf          data buffer
 *                size         data buffer size
 *  Output.....:  nbrRdBytesP  number of read bytes
 *                return       success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_BlockRead( LL_HANDLE *llHdl,
								int32     ch,
								void      *buf,
								int32     size,
								int32     *nbrRdBytesP )
{
    DBGWRT_1((DBH, "LL - SJA1000_BlockRead: ch=%d, size=%d\n",ch,size));

	/* return nr of read bytes */
	*nbrRdBytesP = 0;

	return(ERR_LL_ILL_FUNC);
}

/****************************** SJA1000_BlockWrite ***************************
 *
 *  Description:  Not supported by this driver
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl        ll handle
 *                ch           current channel
 *                buf          data buffer
 *                size         data buffer size
 *  Output.....:  nbrWrBytesP  number of written bytes
 *                return       success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_BlockWrite( LL_HANDLE *llHdl,
								 int32     ch,
								 void      *buf,
								 int32     size,
								 int32     *nbrWrBytesP )
{
    DBGWRT_1((DBH, "LL - SJA1000_BlockWrite: ch=%d, size=%d\n",ch,size));

	/* return nr of written bytes */
	*nbrWrBytesP = 0;

	return(ERR_LL_ILL_FUNC);
}

/****************************** SJA1000_Irq **********************************
 *
 *  Description:  Interrupt service routine
 *
 *                The interrupt is triggered when a CAN object has been
 *				  received or transmitted or when the CAN controller's
 *				  state changes
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl    ll handle
 *  Output.....:  return   LL_IRQ_DEVICE	irq caused from device
 *                         LL_IRQ_DEV_NOT   irq not caused from device
 *                         LL_IRQ_UNKNOWN   unknown
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_Irq( LL_HANDLE *llHdl )
{
	MACCESS ma = llHdl->ma;
	u_int8 cause, status;
	int32 ret = LL_IRQ_DEV_NOT;

    IDBGWRT_1((DBH, ">>> SJA1000_Irq ma=0x%x\n",ma));
#if	0
	/* read interrupt register */
	cause = _READ_D8(ma,InterruptReg);

	IDBGWRT_3((DBH, "  cause  = 0x%02x\n",cause));

	if (cause){
#else
	/* read interrupt register */
	if ( (cause = _READ_D8(ma,InterruptReg)) ){

		IDBGWRT_3((DBH, "  cause  = 0x%02x\n",cause));
#endif
		status = _READ_D8(ma,StatusReg);
#ifdef	V2
		if (status & ES_Bit)								/* ed2 */
			llHdl->ecapture = _READ_D8(ma,ErrCodeCapReg);	/* ed2 */
#endif
#ifdef	DBG
	if (cause & BEI_Bit)	IDBGWRT_3((DBH, "   bus error\n"));
	if (cause & ALI_Bit)	IDBGWRT_3((DBH, "   arbitration lost\n"));
	if (cause & EPI_Bit)	IDBGWRT_3((DBH, "   error passive\n"));
	if (cause & WUI_Bit)	IDBGWRT_3((DBH, "   wakeup\n"));
	if (cause & DOI_Bit)	IDBGWRT_3((DBH, "   data overrun\n"));
	if (cause & EI_Bit)		IDBGWRT_3((DBH, "   error warning\n"));
	if (cause & TI_Bit)		IDBGWRT_3((DBH, "   transmit\n"));
	if (cause & RI_Bit)		IDBGWRT_3((DBH, "   receive\n"));
#endif
		IDBGWRT_3((DBH, "  status = 0x%02x\n",status));
#ifdef	DBG
	if (status & BS_Bit)	IDBGWRT_3((DBH, "   bus off\n"));
	if (status & ES_Bit)	IDBGWRT_3((DBH, "   error\n"));
	if (status & TS_Bit)	IDBGWRT_3((DBH, "   transmit\n"));
	if (status & RS_Bit)	IDBGWRT_3((DBH, "   receive\n"));
	if (status & TCS_Bit)	IDBGWRT_3((DBH, "   complete\n"));
	if (status & TBS_Bit)	IDBGWRT_3((DBH, "   released\n"));
	if (status & DOS_Bit)	IDBGWRT_3((DBH, "   overrun\n"));
	if (status & RBS_Bit)	IDBGWRT_3((DBH, "   full\n"));
#endif
		/*------------------+
        | receive interrupt |
        +------------------*/
		if( cause&RI_Bit ){

			/* test rtr bit */
			if( _READ_D8(ma,RxFrameInfo)& RTR_Bit )	{

				if (llHdl->xmtRtr == FALSE) {
					llHdl->xmtRtr = TRUE;

					/* send waiting message */
					_WRITE_D8(ma,CommandReg,XMIT);	/* ed2 */
				}
			}
			SJA1000_IrqRx( llHdl );

			/* release receive buffer, regardless to lost the message */
			_WRITE_D8(ma,CommandReg,RRB_Bit);

			llHdl->irqReceive++;
		}

		/*-------------------+
        | transmit interrupt |
        +-------------------*/
		if( cause&TI_Bit ){

			SJA1000_IrqTx( llHdl );

			llHdl->irqTransmit++;
		}

		/*------------------------------+
        | check other interrupt sources |
        +------------------------------*/

		/* error warning interrupt */
		if( cause&EI_Bit ){
			IDBGWRT_ERR((DBH,"*** SJA1000: error warning interrupt\n"));
#ifdef	VERBOSE
			InsertErrMsg( llHdl, CPL_ERROR_WARNING, 0 );
#endif
			StatChange( llHdl, status );	/* check bus or error status */

			llHdl->irqErrorWarning++;
		}

		/* data overrun interrupt */
		if( cause&DOI_Bit ){
			IDBGWRT_ERR((DBH,"*** SJA1000: data overrun interrupt\n"));

			_WRITE_D8(ma,CommandReg,CDO_Bit);
#ifdef	SH
			/* on this error use the queue number as Rx message counter */
			InsertErrMsg( llHdl, CPL_DATA_OVERRUN, _READ_D8(ma,RxMsgCountReg) );
#else
			InsertErrMsg( llHdl, CPL_MSGLOST, 0 );
#endif
			llHdl->irqDataOverrun++;
		}

		/* wake-up interrupt */
		if( cause&WUI_Bit ){
			IDBGWRT_ERR((DBH,"*** SJA1000: wake-up interrupt\n"));
#ifdef	VERBOSE
			InsertErrMsg( llHdl, CPL_WAKE_UP, 0 );
#endif
			llHdl->irqWakeUp++;
		}

		/* error passive interrupt */
		if (cause&EPI_Bit){
			IDBGWRT_ERR((DBH,"*** SJA1000: error passive interrupt\n"));
#ifdef	VERBOSE
			InsertErrMsg( llHdl, CPL_ERROR_PASSIVE, 0 );
#endif
			IDBGWRT_ERR((DBH," TxErrCount %d\n",_READ_D8(ma,TxErrCountReg)));

			if ((u_int8)_READ_D8(ma,TxErrCountReg) > 127)
				InsertErrMsg( llHdl, CPL_PASSIVE_STATE, 0 );
			else
				InsertErrMsg( llHdl, CPL_ACTIVE_STATE, 0 );

			llHdl->irqErrorPassive++;
		}

		/* arbitration interrupt */
		if (cause&ALI_Bit){
			IDBGWRT_ERR((DBH,"*** SJA1000: arbitration interrupt\n"));
#ifdef	VERBOSE
			InsertErrMsg( llHdl, CPL_ARBITRATION_LOST, 0 );
#if	0
			llHdl->acapture = _READ_D8(ma,ArbLostCapReg);
#endif
#endif
			llHdl->irqArbitrationLost++;
		}

		/* bus error interrupt */
		if (cause&BEI_Bit){
			IDBGWRT_ERR((DBH,"*** SJA1000: bus error interrupt\n"));
#ifdef	VERBOSE
			InsertErrMsg( llHdl, CPL_BUS_ERROR, 0 );
#if	0
			llHdl->ecapture = _READ_D8(ma,ErrCodeCapReg);
#endif
#endif
			llHdl->irqBusError++;
		}
		llHdl->irqCount++;
		ret = LL_IRQ_DEVICE;
	}
	else {
		IDBGWRT_ERR((DBH,"*** SJA1000: irq no intpnd\n"));
		ret = LL_IRQ_DEV_NOT;
	}
    IDBGWRT_1((DBH, "<<<\n"));
	return ret;
}

/********************************** SJA1000_IrqRx ***************************
 *
 *  Description:  receive interrupt
 *
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl    ll handle
 *  Output.....:  ---
 *  Globals....:  ---
 ****************************************************************************/
#ifndef	LOCAL_MASK
static void SJA1000_IrqRx( LL_HANDLE *llHdl )
{
	MSG_OBJ	*obj;
	int32 	nr;

	for (nr=1; nr<NUM_MSG_OBJ; nr++){
		obj = &llHdl->msgObj[nr];

		if (obj->q.dir != CPL_DIR_RCV)
			continue;

		IDBGWRT_3((DBH, "  Rx Irq object=%d\n",nr));

		/*--- put received message into message queue ---*/
#if	1
		InsertRcvMsg( llHdl, nr );
#else
		/* in case of receive queue full put the error message to error queue  */
		if (InsertRcvMsg( llHdl, nr )){
			if((obj->q.errSent & ERRSENT_MSGLOST) == 0){
#ifdef	VERBOSE
				InsertErrMsg( llHdl, CPL_MSGLOST, nr );
#endif
				obj->q.errSent |= ERRSENT_MSGLOST;
			}
		}
#endif
		/*--- send signal to user app ---*/
		if( obj->rcvSig ){
			IDBGWRT_3((DBH,"   SendSig\n"));
			OSS_SigSend( llHdl->osHdl, obj->rcvSig );
		}
		break;
	}
}
#else
static void SJA1000_IrqRx( LL_HANDLE *llHdl )
{
	MACCESS ma	= llHdl->ma;
	MSG_OBJ	*obj;
	int32 	nr;
	u_int32	id;
	u_int32	val;

	if( _READ_D8(ma,RxFrameInfo) & FF_Bit ){	/* read extended id */
		id = ((_READ_D8(ma,RxBuffer1) << 21) |
			  (_READ_D8(ma,RxBuffer2) << 13) |
			  (_READ_D8(ma,RxBuffer3) <<  5) |
			  (_READ_D8(ma,RxBuffer4) >>  3));
	}
	else {										/* read standard id */
		id = ((_READ_D8(ma,RxBuffer1) <<  3) |
			  (_READ_D8(ma,RxBuffer2) >>  5));
	}

	/* scan objects */
	for (nr=1; nr<NUM_MSG_OBJ; nr++){
		obj = &llHdl->msgObj[nr];

		if (obj->q.dir != CPL_DIR_RCV)
			continue;

		IDBGWRT_3((DBH, "  Rx Irq object=%d\n",nr));

		IDBGWRT_3((DBH, "  code=%x\n",obj->q.code));
		IDBGWRT_3((DBH, "  mask=%x\n",obj->q.mask));

		/* (not (id exor code)) or mask */
		val = ~(id^obj->q.code) | obj->q.mask;

		IDBGWRT_3((DBH, "  val =%x\n",val));

		if (~val != 0)
			continue;

		/*--- put received message into message queue ---*/
		InsertRcvMsg( llHdl, nr, id );

		/*--- send signal to user app ---*/
		if( obj->rcvSig ){
			IDBGWRT_3((DBH,"   SendSig\n"));
			OSS_SigSend( llHdl->osHdl, obj->rcvSig );
		}
	}
}
#endif

/********************************** SJA1000_IrqTx ***************************
 *
 *  Description:  transmitter interrupt
 *
 *
 *---------------------------------------------------------------------------
 *  Input......:  llHdl    ll handle
 *  Output.....:  ---
 *  Globals....:  ---
 ****************************************************************************/

static void SJA1000_IrqTx( LL_HANDLE *llHdl )
{
	MSG_OBJ *obj;
	int32 	nr;

	int	flag = 0;

	for (nr=1; nr< NUM_MSG_OBJ; nr++) {
		obj = &llHdl->msgObj[nr];

		if (obj->q.dir != CPL_DIR_XMT)
			continue;

		IDBGWRT_3((DBH, "  Tx Irq object=%d\n",nr));

		if (obj->q.filled > 0) {
			IDBGWRT_3((DBH, "   continue\n"));

			StartXmt( llHdl, nr );

			/*--- send signal to user app ---*/
			if( obj->xmtSig ) {
				IDBGWRT_3((DBH,"   SendSig\n"));
				OSS_SigSend( llHdl->osHdl, obj->xmtSig );
			}
			flag++;
			break;
		}
		else {
			if (obj->q.xmtHalt == FALSE) {
				obj->q.xmtHalt = TRUE;

				IDBGWRT_3((DBH, "   empty\n"));

				/*--- send signal to user app ---*/
				if( obj->xmtSig ) {
					IDBGWRT_3((DBH,"   SendSig\n"));
					OSS_SigSend( llHdl->osHdl, obj->xmtSig );
				}
			}
			else {
				IDBGWRT_3((DBH, "   nothing to do\n"));
				continue;
			}
		}
	}
	if (flag == 0)
		llHdl->xmtHalt = TRUE;
}


/****************************** SJA1000_Info *********************************
 *
 *  Description:  Get information about hardware and driver requirements.
 *
 *                Following info codes are supported:
 *
 *                Code                      Description
 *                ------------------------  -----------------------------
 *                LL_INFO_HW_CHARACTER      hardware characteristics
 *                LL_INFO_ADDRSPACE_COUNT   nr of required address spaces
 *                LL_INFO_ADDRSPACE         address space information
 *                LL_INFO_IRQ               interrupt required
 *                LL_INFO_LOCKMODE          process lock mode required
 *
 *                The LL_INFO_HW_CHARACTER code returns all address and
 *                data modes (OR'ed), which are supported from the
 *                hardware (MDIS_MAxx, MDIS_MDxx).
 *
 *                The LL_INFO_ADDRSPACE_COUNT code returns the number
 *                of address spaces used from the driver.
 *
 *                The LL_INFO_ADDRSPACE code returns information about one
 *                specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *                data mode represents the widest hardware access used from
 *                the driver.
 *
 *                The LL_INFO_IRQ code returns, if the driver supports an
 *                interrupt routine (TRUE or FALSE).
 *
 *                The LL_INFO_LOCKMODE code returns, which process locking
 *                mode is required from the driver (LL_LOCK_xxx).
 *
 *---------------------------------------------------------------------------
 *  Input......:  infoType	   info code
 *                ...          argument(s)
 *  Output.....:  return       success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 SJA1000_Info( int32 infoType, ... )
{
    int32   error = ERR_SUCCESS;
    va_list argptr;

    va_start(argptr, infoType );

    switch(infoType) {
		/*-------------------------------+
        |  hardware characteristics      |
        |  (all addr/data modes OR'ed)   |
        +-------------------------------*/
        case LL_INFO_HW_CHARACTER:
		{
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);

			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD08 | MDIS_MD16;
			break;
	    }
		/*-------------------------------+
        |  nr of required address spaces |
        |  (total spaces used)           |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE_COUNT:
		{
			u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

			*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
			break;
	    }
		/*-------------------------------+
        |  address space type            |
        |  (widest used data mode)       |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE:
		{
			u_int32 addrSpaceIndex = va_arg(argptr, u_int32);
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);
			u_int32 *addrSizeP = va_arg(argptr, u_int32*);

			if (addrSpaceIndex >= ADDRSPACE_COUNT)
				error = ERR_LL_ILL_PARAM;
			else {
				*addrModeP = MDIS_MA08;
				*dataModeP = MDIS_MD16;
				*addrSizeP = ADDRSPACE_SIZE;
			}

			break;
	    }
		/*-------------------------------+
        |   interrupt required           |
        +-------------------------------*/
        case LL_INFO_IRQ:
		{
			u_int32 *useIrqP = va_arg(argptr, u_int32*);

			*useIrqP = USE_IRQ;
			break;
	    }
		/*-------------------------------+
        |   process lock mode            |
        +-------------------------------*/
        case LL_INFO_LOCKMODE:
		{
			u_int32 *lockModeP = va_arg(argptr, u_int32*);

			*lockModeP = LL_LOCK_NONE; /* driver locks critical sections */
			break;
	    }
		/*-------------------------------+
        |   (unknown)                    |
        +-------------------------------*/
        default:
          error = ERR_LL_ILL_PARAM;
    }

    va_end(argptr);
    return(error);
}

/*******************************  Ident  *************************************
 *
 *  Description:  Return ident string
 *
 *---------------------------------------------------------------------------
 *  Input......:  -
 *  Output.....:  return  ptr to ident string
 *  Globals....:  -
 ****************************************************************************/

static char* Ident( void )		/* nodoc */
{
    return( "SJA1000 - SJA1000 low level driver: $Id: sja1000_drv.c,v 1.13 2004/10/06 14:50:10 dpfeuffer Exp $" );
}

/********************************* Cleanup ***********************************
 *
 *  Description: Close all handles, free memory and return error code
 *		         NOTE: The ll handle is invalid after calling this function
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl		ll handle
 *               retCode    return value
 *  Output.....: return	    retCode
 *  Globals....: -
 ****************************************************************************/

static int32 Cleanup( LL_HANDLE *llHdl, int32 retCode )
{
	u_int32 nr;
	/*------------------------------+
	|  Free message queues/sems     |
	+------------------------------*/
	for( nr=0; nr<NUM_MSG_OBJ; nr++ )
	{
		if( llHdl->msgObj[nr].rcvSig )
			OSS_SigRemove( llHdl->osHdl, &llHdl->msgObj[nr].rcvSig );
		if( llHdl->msgObj[nr].xmtSig )
			OSS_SigRemove( llHdl->osHdl, &llHdl->msgObj[nr].xmtSig );
		if( llHdl->msgObj[nr].q.sem )
			OSS_SemRemove( llHdl->osHdl, &llHdl->msgObj[nr].q.sem );

		if( llHdl->msgObj[nr].q.first )
		{
			OSS_MemFree( llHdl->osHdl, (int8 *)llHdl->msgObj[nr].q.first,
						 llHdl->msgObj[nr].q.memAlloc );
			llHdl->msgObj[nr].q.first = NULL;
		}
	}

    /*------------------------------+
    |  close handles                |
    +------------------------------*/
	/* clean up desc */
	if (llHdl->descHdl)
		DESC_Exit(&llHdl->descHdl);

	/* cleanup debug */
	DBGEXIT((&DBH));

    /*------------------------------+
    |  free memory                  |
    +------------------------------*/
    /* free my handle */
    OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

    /*------------------------------+
    |  return error code            |
    +------------------------------*/
	return(retCode);
}

/********************************* ConfMsg ***********************************
 *
 *  Description: Configure message object
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 pb				API parameter block
 *  Output.....: return       	success (0) or error code
 *  Globals....: -
 ****************************************************************************/

static int32 ConfMsg( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb )
{
	u_int32 	nr 			= pb->p.config_msg.nr;
	u_int32 	dir 		= pb->p.config_msg.dir;
	u_int32 	qentries 	= pb->p.config_msg.qentries;
	OSS_IRQ_STATE oldIrqState;

	MSG_OBJ 	*obj;
	MQUEUE_ENT 	*ent;

	int32 		error 		= 0;
	u_int32		i;

	DBGWRT_2((DBH,"  ConfMsg: nr=%d dir=%d ext=%d code=0x%x mask=0x%x "
			  "entries=%d\n", nr, dir, pb->p.config_msg.extended,
			  pb->p.config_msg.id, pb->p.config_msg.mask,
			  qentries));

	/* sanity checks */
	if( dir > CPL_DIR_DIS )
		return CPL_ERR_BADDIR;

	if( nr >= NUM_MSG_OBJ )
		return CPL_ERR_BADMSGNUM;

    obj = &llHdl->msgObj[nr];

	/*-------------+
	|  Init queue  |
	+-------------*/
	/*--- realloc memory for queue ---*/
	if( obj->q.first ){
		OSS_MemFree( llHdl->osHdl, (void *)obj->q.first, obj->q.memAlloc );
		obj->q.first = NULL;
	}
	if( dir != CPL_DIR_DIS ){
		/*--- allocate new queue entries ---*/

		if( (obj->q.first = (MQUEUE_ENT*)OSS_MemGet( llHdl->osHdl,
										qentries * sizeof(MQUEUE_ENT),
										&obj->q.memAlloc )) == NULL){
			DBGWRT_ERR((DBH,"*** SJA1000:ConfMsg: can't alloc queue mem\n"));
			return ERR_OSS_MEM_ALLOC;
		}

		/*--- init queue entries ---*/
		for( i=0, ent=obj->q.first; i<qentries; ent++, i++ )
			ent->next = ent+1;

		ent[-1].next = obj->q.first;

		/*--- init queue ---*/
		obj->q.totEntries = qentries;
		obj->q.filled	  = 0;
		obj->q.ready	  = FALSE;
		obj->q.dir		  = (u_int8)dir;
		obj->q.xmtHalt	  = TRUE;

		obj->q.extended	  = (u_int8)pb->p.config_msg.extended;
		obj->q.code		  = pb->p.config_msg.id;
		obj->q.mask		  = pb->p.config_msg.mask;

		if( (error = QueueClear( llHdl, nr, 0 )) )
			return error;

		/*------------------------------------+
		|  Configure hardware message object  |
		+------------------------------------*/
		oldIrqState = OSS_IrqMaskR( llHdl->osHdl, llHdl->irqHdl );

		if( nr ){

			/*--- config msg 1,2 ---*/
			_SJA1000_config_msg( llHdl, nr, dir,
								 pb->p.config_msg.extended,
								 pb->p.config_msg.id,
								 pb->p.config_msg.mask );
		}
		else {
			MACCESS	ma	= llHdl->ma;
			/*--- error object ---*/
			StatChange( llHdl, _READ_D8(ma,StatusReg)); 	/* read status */
		}
		OSS_IrqRestore( llHdl->osHdl, llHdl->irqHdl, oldIrqState );
	}
	else {
		/*--- disable object ---*/
		obj->q.totEntries = 0;
		obj->q.filled	  = 0;
		obj->q.ready	  = FALSE;
		obj->q.dir		  = (u_int8)dir;

		if( obj->q.sem != NULL ){
			obj->q.sem = 0;

			/*--- remove semaphore ---*/
			if( (error = OSS_SemRemove( llHdl->osHdl, &obj->q.sem)) ){
				DBGWRT_ERR((DBH,"*** SJA1000:ConfMsg: error %x removing sem\n",
							error));
				return error;
			}
		}
	}
	return 0;
}

/********************************* QueueClear ********************************
 *
 *  Description: Reset queue of message object
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 nr				message object number (0....)
 *				 txabort		flags if pending xmits shoule be aborted
 *  Output.....: return       	success (0) or error code
 *  Globals....: -
 ****************************************************************************/

static int32 QueueClear( LL_HANDLE *llHdl, u_int32 nr, u_int32 txabort )
{
	MSG_OBJ *obj;
	int32 error = 0;

	DBGWRT_2((DBH,"  QueueClear: nr=%d txabort=%d\n", nr, txabort ));

	if( nr >= NUM_MSG_OBJ )
		return CPL_ERR_BADMSGNUM;

	obj = &llHdl->msgObj[nr];

    CHECK_OBJECT_AVAILABLE;

	/*
	 * Since there is no semaphore reset call, we must delete and
	 * re-create the semaphore in order to clear it
	 */

    /* prevent interrupt routine from accessing semaphore */
	obj->q.ready = FALSE;

	if( obj->q.sem != NULL ){
		/*--- remove old semaphore ---*/
		if( (error = OSS_SemRemove( llHdl->osHdl, &obj->q.sem))){
			DBGWRT_ERR((DBH,"*** SJA1000:QeueuClear: error %x removing sem\n",
						error));
			return error;
		}
	}

	/*--- create new sem ---*/
	if( (error = OSS_SemCreate( llHdl->osHdl, OSS_SEM_COUNT,
								obj->q.dir==CPL_DIR_XMT ?
								 obj->q.totEntries : 0,
								&obj->q.sem ))){
		DBGWRT_ERR((DBH,"*** SJA1000:QeueuClear: error %x creating sem\n",
					error));
		return error;
	}

	if( txabort && obj->q.dir == CPL_DIR_XMT )
		_SJA1000_transmit_abort( llHdl, nr );

	/*--- init queue pointers ---*/
	obj->q.nxtIn 	= obj->q.first;
	obj->q.nxtOut 	= obj->q.first;
	obj->q.filled	= 0;
	obj->q.ready	= TRUE;
	obj->q.errSent  = 0;

	obj->q.xmtHalt	= TRUE;

	return 0;
}

/********************************* ReadMsg ***********************************
 *
 *  Description: Read message from queue
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 pb				API parameter block
 *  Output.....: return       	success (0) or error code
 *  Globals....: -
 ****************************************************************************/

static int32 ReadMsg( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb )
{
	MSG_OBJ     *obj;
	MQUEUE_ENT  *ent;
	u_int32     nr      = pb->p.read_msg.nr;
	int32       timeout = pb->p.read_msg.timeout;
	int32       error   = 0;
	OSS_IRQ_STATE oldIrqState;
	int32 oss_semtimeout = OSS_SEM_NOWAIT;

	DBGWRT_2((DBH,"  ReadMsg: nr=%d tout=%d\n", nr, timeout ));

	if( (nr==0) || (nr >= NUM_MSG_OBJ) )
		return CPL_ERR_BADMSGNUM;

	obj = &llHdl->msgObj[nr];

    CHECK_OBJECT_AVAILABLE;

	if( obj->q.dir != CPL_DIR_RCV )
		return CPL_ERR_BADDIR;

    if ( timeout < SJA1000_API_NONBLOCK ) {
    	return CPL_ERR_BADPARAM;
    }

    if ( timeout == SJA1000_API_NONBLOCK ) {
    	if( obj->q.filled == 0) {
    		return CPL_ERR_NOMESSAGE;
    	} else {
    		/* ok we have data, call OSS_SemWait in nonblocking mode */
    		oss_semtimeout = OSS_SEM_NOWAIT;
    	}
    } else if ( timeout == SJA1000_API_BLOCK ) {
    	/* call OSS_SemWait in blocking mode until data arrives */
    	oss_semtimeout = OSS_SEM_WAITFOREVER;
    } else {
    	/* any value > 0: wait that amount of ms */
    	oss_semtimeout = timeout;
    }

	/*--- Wait for message to come in ---*/
	if( llHdl->devSemHdl )
		OSS_SemSignal( llHdl->osHdl, llHdl->devSemHdl );

	if( (error = OSS_SemWait( llHdl->osHdl, obj->q.sem, oss_semtimeout )))
		return error;

	if( llHdl->devSemHdl ){
		if( (error = OSS_SemWait( llHdl->osHdl, llHdl->devSemHdl,
								  OSS_SEM_WAITFOREVER )))
			return error;
	}
	/*--- extract message ---*/
	ent = obj->q.nxtOut;

	pb->p.read_msg.id 		= ent->d.m.id;
	pb->p.read_msg.datlen	= ent->d.m.datlen;

	/*--- copy data ---*/
	*(u_int32 *)&pb->p.read_msg.data[0]	= *(u_int32 *)&ent->d.m.data[0];
	*(u_int32 *)&pb->p.read_msg.data[4]	= *(u_int32 *)&ent->d.m.data[4];

	DBGWRT_2((DBH,"   extract ent=0x%x id=0x%x datlen=%d\n", ent, ent->d.m.id,
			  ent->d.m.datlen));

	/*--- advance next out pointer ---*/
	obj->q.nxtOut = ent->next;

	/*--- adjust queue counter ---*/
	oldIrqState = OSS_IrqMaskR( llHdl->osHdl, llHdl->irqHdl );

	obj->q.filled--;
	obj->q.errSent = 0;

	OSS_IrqRestore( llHdl->osHdl, llHdl->irqHdl, oldIrqState );

	return 0;
}

/********************************* ReadError *********************************
 *
 *  Description: Read message from error queue
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 pb				API parameter block
 *  Output.....: return       	success (0) or error code
 *  Globals....: -
 ****************************************************************************/

static int32 ReadError( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb )
{
	MSG_OBJ 	*obj;
	MQUEUE_ENT 	*ent;
	int32       timeout = pb->p.read_error.timeout;
	int32 		error 	= 0;
	OSS_IRQ_STATE oldIrqState;

	DBGWRT_2((DBH,"->ReadError\n"));
	DBGWRT_2((DBH,"  timeout=%d\n",timeout));

    obj = &llHdl->msgObj[0];

    CHECK_OBJECT_AVAILABLE;

	/*--- in no wait mode, check if message present, if not return error ---*/
	if( timeout == -1 && obj->q.filled == 0)
		return CPL_ERR_NOMESSAGE;

	/*--- convert timeout parameter for SemWait ---*/
	if( timeout == 0 )
		timeout = -1;

	/*--- Wait for message to come in ---*/
	if( llHdl->devSemHdl )
		OSS_SemSignal( llHdl->osHdl, llHdl->devSemHdl );

	if( (error = OSS_SemWait( llHdl->osHdl, obj->q.sem, timeout )))
		return error;

	if( llHdl->devSemHdl ){
		if( (error = OSS_SemWait( llHdl->osHdl, llHdl->devSemHdl,
								  OSS_SEM_WAITFOREVER )))
			return error;
	}
	/*--- extract message ---*/
	ent = obj->q.nxtOut;

	pb->p.read_error.errcode = ent->d.e.errcode;
	pb->p.read_error.nr		 = ent->d.e.nr;

	DBGWRT_2((DBH,"   extract ent=0x%x errcode=%d nr=%d\n", ent,
			  ent->d.e.errcode, ent->d.e.nr));

	/*--- advance next out pointer ---*/
	obj->q.nxtOut = ent->next;

	/*--- adjust queue counter ---*/
	oldIrqState = OSS_IrqMaskR( llHdl->osHdl, llHdl->irqHdl );

	obj->q.filled--;

	OSS_IrqRestore( llHdl->osHdl, llHdl->irqHdl, oldIrqState );

	DBGWRT_2((DBH,"<-ReadError (%d)\n",0));
	return 0;
}

/********************************* WriteMsg **********************************
 *
 *  Description: Put message into queue
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 pb				API parameter block
 *  Output.....: return       	success (0) or error code
 *  Globals....: -
 ****************************************************************************/

static int32 WriteMsg( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb )
{
	u_int32 	nr 		= pb->p.write_msg.nr;
	int32 		timeout = pb->p.write_msg.timeout;
	OSS_IRQ_STATE oldIrqState;

	MSG_OBJ 	*obj;
	MQUEUE_ENT 	*ent;

	int32 		error 	= 0;

	DBGWRT_2((DBH,"  WriteMsg: nr=%d tout=%d\n", nr, timeout));

	if( (nr==0) || (nr >= NUM_MSG_OBJ) )
		return CPL_ERR_BADMSGNUM;
	DDBG("WriteMsg",1);

	obj 	= &llHdl->msgObj[nr];

    CHECK_OBJECT_AVAILABLE;

	if( obj->q.dir != CPL_DIR_XMT )
		return CPL_ERR_BADDIR;
	DDBG("WriteMsg",2);

	/*--- in no wait mode, check if space available, if not return error ---*/
	if( timeout == -1 && obj->q.filled == obj->q.totEntries )
		return CPL_ERR_QFULL;
	DDBG("WriteMsg",3);

	/*--- convert timeout parameter for SemWait ---*/
	if( timeout == 0 )
		timeout = -1;

	/*--- Wait for message to come in ---*/
	if( llHdl->devSemHdl )
		OSS_SemSignal( llHdl->osHdl, llHdl->devSemHdl );
	DDBG("WriteMsg",4);

	if( (error = OSS_SemWait( llHdl->osHdl, obj->q.sem, timeout )))
		return error;
	DDBG("WriteMsg",5);

	if( llHdl->devSemHdl ){
		if( (error = OSS_SemWait( llHdl->osHdl, llHdl->devSemHdl,
								  OSS_SEM_WAITFOREVER )))
			return error;
	}
	DDBG("WriteMsg",6);

	/*--- fill in message ---*/
	ent	= obj->q.nxtIn;

	ent->d.m.id 			= pb->p.write_msg.id;
	ent->d.m.datlen			= (u_int8)pb->p.write_msg.datlen;
	ent->d.m.mode			= (u_int8)pb->p.write_msg.mode;

	/*--- copy data ---*/
	*(u_int32 *)&ent->d.m.data[0] = *(u_int32 *)&pb->p.write_msg.data[0];
	*(u_int32 *)&ent->d.m.data[4] = *(u_int32 *)&pb->p.write_msg.data[4];

	DBGWRT_2((DBH,"   put ent=0x%x id=0x%x datlen=%d mode=%d\n", ent,
			  ent->d.m.id, ent->d.m.datlen,ent->d.m.mode ));
	DBGWRT_2((DBH,"   xmtHalt=%d\n", llHdl->xmtHalt ));

	/*--- advance next out pointer ---*/
	obj->q.nxtIn = ent->next;

	/*--- adjust queue counter ---*/
	oldIrqState = OSS_IrqMaskR( llHdl->osHdl, llHdl->irqHdl );

	obj->q.filled++;
#if	0
	/*--- start xmit when halted ---*/
	if( obj->q.xmtHalt )
		StartXmt( llHdl, nr );
#else
	/*--- start xmit when halted ---*/
	if( llHdl->xmtHalt == TRUE ) {
		StartXmt( llHdl, nr );
	}
#endif
	OSS_IrqRestore( llHdl->osHdl, llHdl->irqHdl, oldIrqState );

	return 0;
}

/********************************* StartXmt **********************************
 *
 *  Description: Get message from Xmit queue and start transmit
 *
 *	Should be called from interrupt or with interrupts masked
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 nr				message object number
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/

static void StartXmt( LL_HANDLE *llHdl, u_int32 nr )
{
	MACCESS 	ma 		= llHdl->ma;
	MSG_OBJ 	*obj 	= &llHdl->msgObj[nr];
	MQUEUE_ENT 	*ent 	= obj->q.nxtOut;
	u_int32 	id 		= ent->d.m.id;
	u_int32 	datalen = ent->d.m.datlen;
	u_int8 		*data 	= ent->d.m.data;
	u_int32		i,offset;
	u_int8		info;

	DBGWRT_2((DBH,"  StartXmt: nr=%d\n", nr));

	/*----------------------------+
    |  check if transmit buffer   |
    |  is ready (empty)           |
    +----------------------------*/
	if ((_READ_D8(ma,StatusReg) & TBS_Bit) == 0) {
		DBGWRT_ERR((DBH,"*** SJA1000:StartXmt: buffer not ready!\n"));
		InsertErrMsg( llHdl, CPL_TBUFFER_LOCKED, nr );
		return;
	}
	/*----------------------------+
    |  write id+data              |
    +----------------------------*/
	info = (u_int8)datalen;

	/* request frame */
	if (ent->d.m.mode&(1<<1))
		info |= RTR_Bit;			/* set rtr bit and data bit to zero */

	if (obj->q.extended)
		info |= FF_Bit;				/* set eff bit, extended frame format */

	DBGWRT_2((DBH,"   info=0x%02x\n",info));

	_WRITE_D8(ma,TxFrameInfo,info);					/* write frame info */
/*
  standard
  * 		|               |               |               |               |
  *			.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.0.9.8.7.6.5.4.3.2.1.0
  *		    |               |               |               |               |
  *x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.0.9.8.7.6.5.4.3.2.1.0.x.x.x.x.x
  *			      .x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.0.9.8.7.6.5.4.3
  *
  extended
  * 		|               |               |               |               |
  *			.x.x.x.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3.2.1.0
  *		    |               |               |               |               |
  *	   x.x.x.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3.2.1.0.x.x.x
  *	                   x.x.x.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3.2.1.0.9.8.7.6.5
  *	                                   x.x.x.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3
  *	                                                   x.x.x.8.7.6.5.4.3.2.1
*/
	if (obj->q.extended) {							/* write extended id */
		DBGWRT_2((DBH,"   extended frame\n"));

		_WRITE_D8(ma,TxBuffer1,(id >> 21) & 0xff);
		_WRITE_D8(ma,TxBuffer2,(id >> 13) & 0xff);
		_WRITE_D8(ma,TxBuffer3,(id >>  5) & 0xff);
		_WRITE_D8(ma,TxBuffer4,(id <<  3) & 0xf8);

		offset = TxBuffer5;
	}
	else {											/* write standard id */
		DBGWRT_2((DBH,"   standard frame\n"));

		_WRITE_D8(ma,TxBuffer1,(id >>  3) & 0xff);
		_WRITE_D8(ma,TxBuffer2,(id <<  5) & 0xe0);

		offset = TxBuffer3;
	}
	for (i=0; i<datalen; i++)
		_WRITE_D8(ma,TxBuffer(offset+i),*data++);	/* write data bytes */

	/*----------------------------+
    |  request to send data       |
    +----------------------------*/
	if (ent->d.m.mode&(1<<0)){
		DBGWRT_2((DBH,"   immediately\n"));
		_WRITE_D8(ma,CommandReg,XMIT);	/* ed2 */
	}
	else {
		llHdl->xmtRtr = FALSE;
	}
	obj->q.xmtHalt = FALSE;
	llHdl->xmtHalt = FALSE;

	/*--- advance queue ptr ---*/
	obj->q.filled--;
	obj->q.nxtOut = ent->next;

	OSS_SemSignal( llHdl->osHdl, obj->q.sem );
}

/********************************* InsertRcvMsg ******************************
 *
 *  Description: Get message and put it into receive queue
 *
 *	Should be called from interrupt only
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 nr				message object number
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/
#ifndef	LOCAL_MASK
static int32 InsertRcvMsg( LL_HANDLE *llHdl, u_int32 nr )
{
	MACCESS 	ma 		= llHdl->ma;
	MSG_OBJ 	*obj 	= &llHdl->msgObj[nr];
	MQUEUE_ENT 	*ent 	= obj->q.nxtIn;
	u_int32 	datlen;
	u_int32 	i,offset;
	u_int8 		*data;
	u_int8 		msg_cfg;

	if( obj->q.ready == FALSE ){
		IDBGWRT_ERR((DBH,"*** SJA1000:InsertRcvMsg: queue %d not rdy\n", nr ));
		return CPL_ERR_RXQNRDY;
	}

	if( obj->q.filled < obj->q.totEntries ){
		/*--- there is space in the buffer ---*/

		/*----------------------------+
		|  read id+data               |
		+----------------------------*/
		msg_cfg = _READ_D8(ma,RxFrameInfo);		/* read configuration */

		datlen = ent->d.m.datlen = msg_cfg & 0xf; 	/* read data len*/
		data = ent->d.m.data;
/*
  standard
  *	|               |               |               |         .2.1.0.x.x.x.x.x
  *	|	            |               |         .0.9.8.7.6.5.4.3      |
  *	|               |               |               |               |
  *	.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.0.9.8.7.6.5.4.3.2.1.0|
  *
  extended
  *	|               |               |               |     .4.3.2.1.0.x.x.x
  *	|               |               |     .2.1.0.9.8.7.6.5          |
  *	|               |     .0.9.8.7.6.5.4.3          |               |
  * |     .8.7.6.5.4.3.2.1          |               |               |
  *	|               |               |               |               |
  *	.x.x.x.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3.2.1.0.9.8.7.6.5.4.3.2.1.0|
*/
		if( msg_cfg&FF_Bit ){						/* read extended id */
			ent->d.m.id = ((_READ_D8(ma,RxBuffer1) << 21) |
						   (_READ_D8(ma,RxBuffer2) << 13) |
						   (_READ_D8(ma,RxBuffer3) <<  5) |
						   (_READ_D8(ma,RxBuffer4) >>  3));

			offset = RxBuffer5;
		}
		else {										/* read standard id */
			ent->d.m.id = ((_READ_D8(ma,RxBuffer1) <<  3) |
						   (_READ_D8(ma,RxBuffer2) >>  5));
			offset = RxBuffer3;
		}
		if( msg_cfg&RTR_Bit ) {
			ent->d.m.id |= (u_int32)(1<<31);		/* mark flag */
		}

		/* read data bytes */
		for (i=0; i<datlen; i++)
			*data++ = _READ_D8(ma,RxBuffer(offset+i));

		IDBGWRT_2((DBH, "  InsertRcvMsg: nr=%d id=0x%x len=%d %s frame\n",
				   nr, ent->d.m.id, ent->d.m.datlen,
				   msg_cfg&FF_Bit?"extended":"standard" ));

		IDBGDMP_3((DBH, "Rx Data", (void *)ent->d.m.data, datlen, 1 ));

		/*-----------------------------------------+
		|  Update queue, semaphore, send signals   |
		+-----------------------------------------*/
		obj->q.filled++;
		obj->q.nxtIn = ent->next;

		OSS_SemSignal( llHdl->osHdl, obj->q.sem ); /* flag new entry */
		return 0;
	}
	else {
		/*--- no space in buffer ---*/
		if( ! (obj->q.errSent & ERRSENT_OVERRUN) ){
			InsertErrMsg( llHdl, CPL_QOVERRUN, nr );
			obj->q.errSent |= ERRSENT_OVERRUN;
		}
		IDBGWRT_ERR((DBH,"*** sja1000: rx irq %d buffer overrun\n", nr ));

		return CPL_ERR_RXQFULL;
	}
}
#else
static int32 InsertRcvMsg( LL_HANDLE *llHdl, u_int32 nr, u_int32 id )
{
	MACCESS 	ma 		= llHdl->ma;
	MSG_OBJ 	*obj 	= &llHdl->msgObj[nr];
	MQUEUE_ENT 	*ent 	= obj->q.nxtIn;
	u_int32 	i,offset;
	u_int8 		*data;
	u_int8 		msg_cfg;

	if( obj->q.ready == FALSE ){
		IDBGWRT_ERR((DBH,"*** SJA1000:InsertRcvMsg: queue %d not rdy\n", nr ));
		return CPL_ERR_RXQNRDY;
	}

	if( obj->q.filled < obj->q.totEntries ){
		/*--- there is space in the buffer ---*/

		/*----------------------------+
		|  read id+data               |
		+----------------------------*/
		msg_cfg = _READ_D8(ma,RxFrameInfo);			/* read configuration */

		if( msg_cfg&RTR_Bit )
			id |= (u_int32)(1<<31);					/* mark flag */

		ent->d.m.id 	= id;
		ent->d.m.datlen = msg_cfg & 0xf; 			/* read data len*/

		data 			= ent->d.m.data;

		if( msg_cfg&FF_Bit )						/* read extended id */
			offset = RxBuffer5;
		else 										/* read standard id */
			offset = RxBuffer3;

		/* read data bytes */
		for (i=0; i<ent->d.m.datlen; i++)
			*data++ = _READ_D8(ma,RxBuffer(offset+i));

		IDBGWRT_2((DBH, "  InsertRcvMsg: nr=%d id=0x%x len=%d %s frame\n",
				   nr, ent->d.m.id, ent->d.m.datlen,
				   msg_cfg&FF_Bit?"extended":"standard" ));

		IDBGDMP_3((DBH, "Rx Data", (void *)ent->d.m.data, ent->d.m.datlen, 1));

		/*-----------------------------------------+
		|  Update queue, semaphore, send signals   |
		+-----------------------------------------*/
		obj->q.filled++;
		obj->q.nxtIn = ent->next;

		OSS_SemSignal( llHdl->osHdl, obj->q.sem ); /* flag new entry */
		return 0;
	}
	else {
		/*--- no space in buffer ---*/
		if( ! (obj->q.errSent & ERRSENT_OVERRUN) ){
			InsertErrMsg( llHdl, CPL_QOVERRUN, nr );
			obj->q.errSent |= ERRSENT_OVERRUN;
		}
		IDBGWRT_ERR((DBH,"*** sja1000: rx irq %d buffer overrun\n", nr ));

		return CPL_ERR_RXQFULL;
	}
}
#endif

/********************************* InsertErrMsg ******************************
 *
 *  Description: Put message into error queue
 *
 *	Should be called from interrupt only
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 errcode		error code
 *				 nr				message object number
 *  Output.....: -
 *  Globals....: -
 ****************************************************************************/

static void InsertErrMsg( LL_HANDLE *llHdl, u_int32 errcode, u_int32 nr )
{
	MSG_OBJ *obj = &llHdl->msgObj[0];
	MQUEUE_ENT *ent = obj->q.nxtIn;

	if( obj->q.ready == FALSE ){
		IDBGWRT_ERR((DBH,"*** SJA1000:InsertErrMsg: No error queue\n"));
		return;
	}
	if( obj->q.filled < obj->q.totEntries ){
		/*--- there is space in the buffer ---*/

		ent->d.e.errcode 	= errcode;
		ent->d.e.nr			= nr;
	}
	else {
		/*--- no space in buffer ---*/
		IDBGWRT_ERR((DBH,"*** SJA1000: error queue overrun\n"));
		return;
	}

	/*-----------------------------------------+
	|  Update queue, semaphore, send signals   |
	+-----------------------------------------*/
	obj->q.filled++;
	obj->q.nxtIn = ent->next;

	OSS_SemSignal( llHdl->osHdl, obj->q.sem );	/* flag new entry */

	/*--- send signal to user app ---*/
	if( obj->rcvSig )
		OSS_SigSend( llHdl->osHdl, obj->rcvSig );
}

/********************************* StatChange ********************************
 *
 *  Description: Called when Status change interrupt occurs
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 status			copy of STATUS reg
 *  Output.....:
 *  Globals....: -
 ****************************************************************************/

static void StatChange( LL_HANDLE *llHdl, u_int8 status )
{
	u_int8 oldStatus = llHdl->status;
	IDBGWRT_2((DBH,"  StatChange  oldstatus=0x%x newstatus=0x%x\n",
			   oldStatus, status));

	if( (oldStatus & BS_Bit) && !(status & BS_Bit ))
		InsertErrMsg( llHdl, CPL_BUSOFF_CLR, 0 );
	else if( !(oldStatus & BS_Bit) && (status & BS_Bit ))
		InsertErrMsg( llHdl, CPL_BUSOFF_SET, 0 );

	if( (oldStatus & ES_Bit) && !(status & ES_Bit ))
		InsertErrMsg( llHdl, CPL_WARN_CLR, 0 );
	else if( !(oldStatus & ES_Bit) && (status & ES_Bit ))
		InsertErrMsg( llHdl, CPL_WARN_SET, 0 );

	llHdl->status = status;
}

/********************************* SetSig ************************************
 *
 *  Description: Setup signal to be sent on receive/transmit
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 pb				API parameter block
 *  Output.....: return       	success (0) or error code
 *  Globals....: -
 ****************************************************************************/

static int32 SetSig( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb )
{
	u_int32 nr;
	int32 signal;
	MSG_OBJ *obj;
	OSS_SIG_HANDLE **sigP;
	int32 error;

	if( pb->func == F_SET_RCVSIG ){
		nr = pb->p.set_rcvsig.nr;
		signal = pb->p.set_rcvsig.signal;
	}
	else {
		nr = pb->p.set_xmtsig.nr;
		signal = pb->p.set_xmtsig.signal;
	}
	DBGWRT_2((DBH, "  SetSig: %s nr=%d sig=%d\n",
			  pb->func == F_SET_RCVSIG ? "rcv":"xmt",
			  nr, signal ));

	if( nr >= NUM_MSG_OBJ )
		return CPL_ERR_BADMSGNUM;

	obj = &llHdl->msgObj[nr];

	if( pb->func == F_SET_RCVSIG && obj->q.dir == CPL_DIR_XMT )
		return CPL_ERR_BADDIR;

	if( pb->func == F_SET_RCVSIG )
		sigP = &obj->rcvSig;
	else
		sigP = &obj->xmtSig;

	/*--- check for signal already installed ---*/
	if( *sigP ){
		DBGWRT_ERR((DBH,"*** SJA1000:SetSig: %s signal for msg %d already"
					"installed\n", pb->func == F_SET_RCVSIG ? "rcv":"xmt",
					nr ));
		return CPL_ERR_SIGBUSY;
	}

	/*--- install signal ---*/
	if( (error = OSS_SigCreate( llHdl->osHdl, signal, sigP )))
		return error;

	return error;
}

/********************************* ClrSig ************************************
 *
 *  Description: Remove signal to be sent on receive/transmit
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 pb				API parameter block
 *  Output.....: return       	success (0) or error code
 *  Globals....: -
 ****************************************************************************/

static int32 ClrSig( LL_HANDLE *llHdl, SJA1000_CALL_PB *pb )
{
	u_int32 nr;
	MSG_OBJ *obj;
	OSS_SIG_HANDLE **sigP;
	int32 error=0;

	if( pb->func == F_CLR_RCVSIG )	nr = pb->p.clr_rcvsig.nr;
	else							nr = pb->p.clr_xmtsig.nr;

	DBGWRT_2((DBH, "  ClrSig: %s nr=%d\n",
			  pb->func == F_CLR_RCVSIG ? "rcv":"xmt", nr));

	if( nr >= NUM_MSG_OBJ )
		return CPL_ERR_BADMSGNUM;

	obj = &llHdl->msgObj[nr];

	if( pb->func == F_CLR_RCVSIG && obj->q.dir == CPL_DIR_XMT )
		return CPL_ERR_BADDIR;

	if( pb->func == F_CLR_RCVSIG )
		sigP = &obj->rcvSig;
	else
		sigP = &obj->xmtSig;

	if( *sigP )
		error = OSS_SigRemove( llHdl->osHdl, sigP );

	return error;
}

/********************************* QueueStatus *******************************
 *
 *  Description: Get number of entries in queue
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 nr				message object number
 *				 entriesP		place to store number of entries
 *  Output.....: return       	success (0) or error code
 *				 *entriesP		number of entries
 *				 *directionP	direction flag
 *  Globals....: -
 ****************************************************************************/

static int32 QueueStatus( LL_HANDLE *llHdl, u_int32 nr, u_int32 *entriesP, u_int32 *directionP )
{
	MSG_OBJ *obj;

	if( nr >= NUM_MSG_OBJ )
		return CPL_ERR_BADMSGNUM;

    obj = &llHdl->msgObj[nr];

    CHECK_OBJECT_AVAILABLE;

	if( obj->q.dir == CPL_DIR_RCV ){
		/*--- for receive queues, return number of filled entries ---*/
		*entriesP = obj->q.filled;
	}
	else if( obj->q.dir == CPL_DIR_XMT ){
		/*--- for transmit queues, return number of free messages ---*/
		*entriesP = obj->q.totEntries - obj->q.filled;
	}
	else {
		/*--- queue disabled ---*/
		*entriesP = 0;
	}
	*directionP = obj->q.dir;

	DBGWRT_2((DBH,"  QueueStatus: nr=%d rv=%d dr=%d\n", nr, *entriesP, *directionP));
	return 0;
}

/* the following functions are for internal test purposes only */

/********************************** ReadRegister *****************************
 *
 *  Description:  read controller registers
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 select		0	read current register contents
 *							1	read operating mode registers
 *							2	read reset mode registers
 *							3	read current register contents, single register
 *							4	read operating mode registers, single register
 *							5	read reset mode registers, single register
 *				 bufP           buffer pointer where the value should be stored
 *				 idx            register index
 *  Output.....: return       	success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 ReadRegister( LL_HANDLE *llHdl,
						   u_int32 select,
						   u_int32 *bufP,
						   u_int32 idx )
{
	MACCESS ma = llHdl->ma;
	int 	i;
	u_int8	*p = (u_int8 *)bufP;
	u_int8	mode;

	switch (select)
	{
		/* read current register contents */
		case 0:
			for (i=0; i<32; i++)
				*p++ = _READ_D8(ma,Register(i));
			break;

		/* read operating mode registers */
		case 1:
			if ((mode = _READ_D8(ma,ModeControlReg)) & RM_Bit)
				_SJA1000_operating_mode(llHdl);

			for (i=0; i<32; i++)
				*p++ = _READ_D8(ma,Register(i));

			if (mode & RM_Bit)
				_SJA1000_reset_mode(llHdl);
			break;

		/* read reset mode registers */
		case 2:
			if (!((mode = _READ_D8(ma,ModeControlReg)) & RM_Bit))
				_SJA1000_reset_mode(llHdl);

			for (i=0; i<32; i++)
				*p++ = _READ_D8(ma,(Register(i)));

			if (!(mode & RM_Bit))
				_SJA1000_operating_mode(llHdl);
			break;

		case 3:
			*p = _READ_D8(ma,Register(idx));
			break;

		/* read operating mode registers, single register */
		case 4:
			if ((mode = _READ_D8(ma,ModeControlReg)) & RM_Bit)
				_SJA1000_operating_mode(llHdl);

			*p = _READ_D8(ma,Register(idx));

			if (mode & RM_Bit)
				_SJA1000_reset_mode(llHdl);
			break;

			/* read reset mode registers, single register */
		case 5:
			if (!((mode = _READ_D8(ma,ModeControlReg)) & RM_Bit))
				_SJA1000_reset_mode(llHdl);

			*p = _READ_D8(ma,Register(idx));

			if (!(mode & RM_Bit))
				_SJA1000_operating_mode(llHdl);
			break;

		default:
			return (ERR_LL_ILL_PARAM);
	}
	return 0;
}

/********************************** WriteRegister *****************************
 *
 *  Description:  write controller registers
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 select		0,3	read current register contents, single register
 *							1,4	read operating mode registers, single register
 *							2,5	read reset mode registers, single register
 *				 val            register value
 *				 idx            register index
 *  Output.....: return       	success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 WriteRegister( LL_HANDLE *llHdl,
						   u_int32 select,
						   u_int32 val,
						   u_int32 idx )
{
	MACCESS ma = llHdl->ma;
	u_int8	mode;

	DBGWRT_1((DBH, "->WriteRegister Sel=%d Reg=%d Val=%d\n",select,idx,val));

	switch (select)
	{
		/* write current register contents */
		case 0:
		case 3:
			_WRITE_D8(ma,idx,val);
			break;

		/* write operating mode registers */
		case 1:
		case 4:
			if ((mode = _READ_D8(ma,ModeControlReg)) & RM_Bit)
				_SJA1000_operating_mode(llHdl);

			_WRITE_D8(ma,idx,val);

			if (mode & RM_Bit)
				_SJA1000_reset_mode(llHdl);
			break;

		/* write reset mode registers */
		case 2:
		case 5:
			if (!((mode = _READ_D8(ma,ModeControlReg)) & RM_Bit))
				_SJA1000_reset_mode(llHdl);

			_WRITE_D8(ma,idx,val);

			if (!(mode & RM_Bit))
				_SJA1000_operating_mode(llHdl);
			break;

		default:
			return (ERR_LL_ILL_PARAM);
	}
	return 0;
}

/********************************** ReadCounter ******************************
 *
 *  Description:  read interrupt counters
 *
 *  select, buffer, size
 *  0       bufP    36          read all interrupt counters
 *  1       "       4           read interrupt counter
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 select         not used, should be zero
 *				 bufP           buffer pointer where the value should be stored
 *				 size           size of buffer (min. 36 bytes)
 *  Output.....: return       	success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 ReadCounter( LL_HANDLE *llHdl,
						  u_int32 select,
						  u_int32 *bufP,
						  u_int32 size )
{
    int32   error;

    DBGWRT_1((DBH, "->ReadCounter sel=%d buf=0x%x size=%d\n",select,bufP,size));
#if	1
    error = 0;
    switch(select)
    {
   	    case 1:     *bufP = llHdl->irqCount;            break;
        case 2:     *bufP = llHdl->irqBusError;         break;
        case 3:     *bufP = llHdl->irqArbitrationLost;  break;
        case 4:     *bufP = llHdl->irqErrorPassive;     break;
        case 5:     *bufP = llHdl->irqWakeUp;           break;
        case 6:     *bufP = llHdl->irqDataOverrun;      break;
        case 7:     *bufP = llHdl->irqErrorWarning;     break;
        case 8:     *bufP = llHdl->irqTransmit;         break;
        case 9:     *bufP = llHdl->irqReceive;          break;
        case 0:
            if (size < (4*16))
	            error = CPL_ERR_BADBUFSIZE;
            else {
   	            bufP[0] = llHdl->irqCount;
                bufP[1] = llHdl->irqBusError;
                bufP[2] = llHdl->irqArbitrationLost;
                bufP[3] = llHdl->irqErrorPassive;
                bufP[4] = llHdl->irqWakeUp;
                bufP[5] = llHdl->irqDataOverrun;
                bufP[6] = llHdl->irqErrorWarning;
                bufP[7] = llHdl->irqTransmit;
                bufP[8] = llHdl->irqReceive;
            }
            break;

        default:
            error = CPL_ERR_BADPARAM;
            break;
    }
#else
	/* test */
	bufP[0] = 1;
    bufP[1] = 2;
    bufP[2] = 3;
    bufP[3] = 4;
    bufP[4] = 5;
    bufP[5] = 6;
    bufP[6] = 7;
    bufP[7] = 8;
    bufP[8] = 9;
#endif
    DBGWRT_1((DBH, "<-ReadCounter (%d)\n",error));
	return error;
}

/********************************** ReadConfiguration ************************
 *
 *  Description:  read device configuration
 *
 *
 *---------------------------------------------------------------------------
 *  Input......: llHdl			low level handle
 *				 select         not used, should be zero
 *				 bufP           buffer pointer where the value should be stored
 *				 size           size of buffer (min. 64 bytes)
 *  Output.....: return       	success (0) or error code
 *  Globals....:  ---
 ****************************************************************************/

static int32 ReadConfiguration( LL_HANDLE *llHdl,
						  		u_int32 select,
						  		u_int32 *bufP,
						  		u_int32 size )
{
#if	1
	MACCESS ma = llHdl->ma;

    switch(select)
    {
        case  1:    *bufP = _READ_D8(ma,ModeControlReg);    break;
        case  2:    *bufP = llHdl->dbgLevel;                break;

	    case  3:    *bufP = llHdl->inClk;                   break;
        case  4:    *bufP = llHdl->canClk;                  break;
        case  5:    *bufP = llHdl->irqFlag;                 break;
        case  6:    *bufP = llHdl->physIntf;                break;
        case  7:    *bufP = llHdl->cbpFlag;                 break;
        case  8:    *bufP = llHdl->dualFilterMode;          break;
        case  9:    *bufP = llHdl->mask;                    break;
        case 10:    *bufP = llHdl->acccode;                 break;
        case 11:    *bufP = llHdl->accmask;                 break;
        case 12:    *bufP = llHdl->bitrate;                 break;
        case 13:    *bufP = llHdl->spl;                     break;
        case 14:    *bufP = llHdl->tseg1;                   break;
        case 15:    *bufP = llHdl->tseg2;                   break;
        case 16:    *bufP = llHdl->extended;                break;
        case 0:
            if (size < (4*16))
	            return CPL_ERR_BADBUFSIZE;

            bufP[ 0] = _READ_D8(ma,ModeControlReg);
            bufP[ 1] = llHdl->dbgLevel;

	        bufP[ 2] = llHdl->inClk;
            bufP[ 3] = llHdl->canClk;
            bufP[ 4] = llHdl->irqFlag;
            bufP[ 5] = llHdl->physIntf;
            bufP[ 6] = llHdl->cbpFlag;
            bufP[ 7] = llHdl->dualFilterMode;
            bufP[ 8] = llHdl->mask;
            bufP[ 9] = llHdl->acccode;
            bufP[10] = llHdl->accmask;
            bufP[11] = llHdl->bitrate;
            bufP[12] = llHdl->spl;
            bufP[13] = llHdl->tseg1;
            bufP[14] = llHdl->tseg2;
            bufP[15] = llHdl->extended;
            break;

        default:
            return CPL_ERR_BADPARAM;
    }
#else
	/* test */
	bufP[0] = 1;
    bufP[1] = 2;
    bufP[2] = 3;
    bufP[3] = 4;
    bufP[4] = 5;
    bufP[5] = 6;
    bufP[6] = 7;
    bufP[7] = 8;
    bufP[8] = 9;
#endif
	return 0;
}
