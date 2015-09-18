/***********************  I n c l u d e  -  F i l e  ************************
 *
 *         Name: SJA1000_int.h
 *
 *       Author: ww
 *        $Date: 2004/10/06 14:50:12 $
 *    $Revision: 1.10 $
 *
 *  Description: Internal include file for SJA1000 driver
 *
 *     Switches: BYTEALIGN
 *
 *-------------------------------[ History ]---------------------------------
 *
 * $Log: sja1000_int.h,v $
 * Revision 1.10  2004/10/06 14:50:12  dpfeuffer
 * minor modifications for MDIS4/2004 conformity
 *
 * Revision 1.9  2004/03/23 12:02:25  ww
 * new edition number (4)
 *
 * Revision 1.8  2003/09/23 11:09:34  ww
 * changed return value of the _SJA1000_clear_busoff() routine from void to int32
 *
 * Revision 1.7  2002/02/06 14:56:14  Schmidt
 * BYTEALIGN switch for byte aligned controller addresses added
 *
 * Revision 1.6  2001/04/20 16:19:56  ww
 * included inquiry for big and little endian defines
 *
 * Revision 1.5  2001/01/25 11:28:50  ww
 * moved the prototypes of the main entry calls to sja1000_drv.c
 *
 * Revision 1.4  2001/01/23 14:59:33  ww
 * included defines for M74 module
 *
 * Revision 1.3  2000/02/10 12:29:19  ww
 * changed return value of _SJA1000_enable() from void to int32
 *
 * Revision 1.2  1999/08/18 10:04:18  ww
 * changed address space size
 *
 * Revision 1.1  1999/08/03 10:55:33  ww
 * Initial Revision
 *---------------------------------------------------------------------------
 * (c) Copyright 1995 by MEN mikro elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

#ifdef __cplusplus
	extern "C" {
#endif

#define _NO_LL_HANDLE		/* ll_defs.h: don't define LL_HANDLE struct */

#include <MEN/men_typs.h>   /* system dependend definitions   */
#include <MEN/maccess.h>    /* hw access macros and types     */
#include <MEN/dbg.h>        /* debug functions                */
#include <MEN/oss.h>        /* oss functions                  */
#include <MEN/desc.h>       /* descriptor functions           */
#include <MEN/mdis_api.h>   /* MDIS global defs               */
#include <MEN/mdis_com.h>   /* MDIS common defs               */
#include <MEN/mdis_err.h>   /* MDIS error codes               */
#include <MEN/ll_defs.h>    /* low level driver definitions   */

#include <MEN/sja1000.h>
#include <MEN/sja1000_api.h>

/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/
#ifndef	EDITION
#define	EDITION			4
#endif

/* general MDIS defs */
#define CH_NUMBER			1			/* nr of device channels */
#define ADDRSPACE_COUNT		1			/* nr of required address spaces */
#define USE_IRQ				TRUE		/* interrupt required  */

/* debug settings */
#define DBG_MYLEVEL			llHdl->dbgLevel
#define DBH					llHdl->dbgHdl

/* others */
#define NUM_MSG_OBJ			6			/* number of message objects */
#define	ERROR_OBJ			0

/* MQUEUE_HEAD.errSent */
#define ERRSENT_OVERRUN		0x1			/* overrun has been sent */
#define ERRSENT_MSGLOST		0x2			/* msg lost has been sent */

/*--------------------------------------+
|   DEFINE HOW TO ACCESS CONTROLLER     |
|   - define how to access controller   |
|     (USE_FUNCTIONS or USE_MACROS)     |
|   - define the registers location     |
|     (calculated via HA macro)         |
+--------------------------------------*/

#ifdef BYTEALIGN

#  define	BusWide	1
#  define	Offset	0

#else /* BYTEALIGN */

   /* sbc5, m51, m74 */
#  define	BusWide	2

#  if (defined(_BIG_ENDIAN_) && !defined(MAC_BYTESWAP)) || (defined(_LITTLE_ENDIAN_) && defined(MAC_BYTESWAP))
#    define	Offset	1	/* motorola */
#  else
#    define	Offset	0	/* intel */
#  endif /* _BIG_ENDIAN_ */

#endif /* BYTEALIGN */

#define	HA(regno) 		(((regno*BusWide)+Offset))

#define	Register(_x_)	(_x_)
#define	RxBuffer(_x_)	(_x_)
#define	TxBuffer(_x_)	(_x_)

#define ADDRSPACE_SIZE	64			/* size of address space */
#define	DFLT_CLOCK		16000000

/*--------------------------------------+
|   CONTROLLER ACCESS VIA MACROS        |
|   (convert offs via HA() macro)       |
+--------------------------------------*/
#define USE_MACROS

#ifdef USE_MACROS
# define _READ_D8(ma,offs) 			MREAD_D8(ma,(HA(offs)))
# define _READ_D16(ma,offs) 		MREAD_D16(ma,HA(offs))
# define _WRITE_D8(ma,offs,val) 	MWRITE_D8(ma,HA(offs),val)
# define _WRITE_D16(ma,offs,val) 	MWRITE_D16(ma,HA(offs),val)
# define _SETMASK_D8(ma,offs,mask) 	MSETMASK_D8(ma,HA(offs),mask)
# define _SETMASK_D16(ma,offs,mask) MSETMASK_D16(ma,HA(offs),mask)
# define _CLRMASK_D8(ma,offs,mask) 	MCLRMASK_D8(ma,HA(offs),mask)
# define _CLRMASK_D16(ma,offs,mask) MCLRMASK_D16(ma,HA(offs),mask)
#endif

/*--------------------------------------------------------+
|  Define externally visible function names for variants  |
+--------------------------------------------------------*/
#define  _SJA1000_init				SJA1000_GLOBNAME(SJA1000_VARIANT,init)
#define  _SJA1000_term 				SJA1000_GLOBNAME(SJA1000_VARIANT,term)
#define  _SJA1000_ctrlval 			SJA1000_GLOBNAME(SJA1000_VARIANT,ctrlval)
#define  _SJA1000_set_accmask 		SJA1000_GLOBNAME(SJA1000_VARIANT,set_accmask)
#define  _SJA1000_set_acccode 		SJA1000_GLOBNAME(SJA1000_VARIANT,set_acccode)
#define  _SJA1000_set_bitrate 		SJA1000_GLOBNAME(SJA1000_VARIANT,set_bitrate)
#define  _SJA1000_enable 			SJA1000_GLOBNAME(SJA1000_VARIANT,enable)
#define  _SJA1000_config_msg 		SJA1000_GLOBNAME(SJA1000_VARIANT,config_msg)
#define  _SJA1000_irq_enable 		SJA1000_GLOBNAME(SJA1000_VARIANT,irq_enable)
#define  _SJA1000_config_msg15 		SJA1000_GLOBNAME(SJA1000_VARIANT,config_msg15)
#define  _SJA1000_transmit_abort	SJA1000_GLOBNAME(SJA1000_VARIANT,transmit_abort)
#define  _SJA1000_set_physintf 		SJA1000_GLOBNAME(SJA1000_VARIANT,set_physintf)
#define  _SJA1000_clear_busoff 		SJA1000_GLOBNAME(SJA1000_VARIANT,clear_busoff)
#define  _SJA1000_rtr 				SJA1000_GLOBNAME(SJA1000_VARIANT,rtr)
#define  _SJA1000_check_reset_mode	SJA1000_GLOBNAME(SJA1000_VARIANT,check_reset_mode)
#define  _SJA1000_reset_mode		SJA1000_GLOBNAME(SJA1000_VARIANT,reset_mode)
#define  _SJA1000_operating_mode	SJA1000_GLOBNAME(SJA1000_VARIANT,operating_mode)

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/

/* queue entry structures */
typedef struct mqueue_ent {
	struct mqueue_ent *next;		/* ptr to next entry */
	union {
		struct {					/* data for rx/tx queues */
			u_int32 id;				/* msg identifier */
			u_int8  datlen;			/* msg len */
			u_int8	mode;			/* tx mode (tx queues only)
									   0=rtr 1=normal */
			u_int8	_pad[2];		/* preserve long alignment */
			u_int8	data[8];		/* msg data */
		} m;
		struct {					/* data for error queue */
			u_int32	errcode;		/* error code */
			u_int32	nr;				/* related object number */
		} e;
	} d;
} MQUEUE_ENT;

/* queue header structure */
typedef struct {
	MQUEUE_ENT 	*first,				/* start of memory used for entries */
		        *nxtIn,				/* next entry to fill */
		        *nxtOut;			/* next entry to extract */
	u_int32		memAlloc;			/* allocated mem for entries */
	u_int32		totEntries;			/* total number of entries */
	u_int32		filled;				/* number of filled entries */
	u_int8		ready;				/* flags if queue is fully initialized */
	u_int8		xmtHalt;			/* flags if transmission halted */
	u_int8		errSent;			/* flags if overrun/datalost error has
									   been sent */
	u_int8		dir;				/* direction 0=receive 1=transmit */
	u_int8		extended;			/* 0=standard, 1=extended */
	u_int8		rsrv;				/* reserved */

	u_int32		code;				/* local acceptance code */
	u_int32		mask;				/* local acceptance mask */
	OSS_SEM_HANDLE *sem;			/* semaphore counting entries */
} MQUEUE_HEAD;

/* per message object structure */
typedef struct {
	u_int32			nr;				/* message object number (redundant) */
	MQUEUE_HEAD		q;				/* message queue header */
	OSS_SIG_HANDLE	*rcvSig;		/* signal installed for receive */
	OSS_SIG_HANDLE	*xmtSig;		/* signal installed for transmit */
} MSG_OBJ;

/* ll handle */
typedef struct {
	/* general */
    int32           memAlloc;		/* size allocated for the handle */
    OSS_HANDLE      *osHdl;         /* oss handle */
    OSS_IRQ_HANDLE  *irqHdl;        /* irq handle */
    DESC_HANDLE     *descHdl;       /* desc handle */
    MACCESS         ma;             /* hw access handle */
	MDIS_IDENT_FUNCT_TBL idFuncTbl;	/* id function table */
	OSS_SEM_HANDLE	*devSemHdl;		/* device semaphore handle */
	/* debug */
    u_int32         dbgLevel;		/* debug level */
	DBG_HANDLE      *dbgHdl;        /* debug handle */

	/* misc */
	MSG_OBJ			msgObj[NUM_MSG_OBJ]; /* message object structures */

	u_int32			offset;			/* address offset */
	u_int32			canClk;			/* resulting can clock */
	u_int32			inClk;			/* CAN controller input clock */

	u_int32			physIntf;		/* CAN physical interface */
	u_int32			cbpFlag;		/* CAN comperator bypass flag */
	u_int32			dualFilterMode;	/* CAN dual filter mode */
	u_int32			mask;			/* CAN controller interrupt mask */

	u_int32			extended;
	u_int32			acccode;
	u_int32			accmask;
	u_int32			bitrate;
	u_int32			spl;
	u_int32			tseg1;
	u_int32			tseg2;

	u_int32			irqCount;		/* interrupt counter */
    u_int32         irqBusError;
    u_int32         irqArbitrationLost;
    u_int32         irqErrorPassive;
    u_int32         irqWakeUp;
    u_int32         irqDataOverrun;
    u_int32         irqErrorWarning;
    u_int32         irqTransmit;
    u_int32         irqReceive;

	int32			irqFlag;		/* interrupt enable flag */

	u_int8			reserved;
	u_int8			status;			/* CAN controller status */
	u_int8			ecapture;
	u_int8			acapture;

	u_int8			xmtHalt;		/* transmitter is halted */
	u_int8			xmtRtr;			/* message waiting for transmit */

} LL_HANDLE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>   /* low level driver jumptable  */
#include <MEN/sja1000_drv.h>   /* SJA1000 driver header file */

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
int32 _SJA1000_init(LL_HANDLE *llHdl, u_int32 sysclock, u_int32 *canclock);
int32 _SJA1000_term(LL_HANDLE *llHdl);
u_int8 _SJA1000_ctrlval(u_int8 flags);
int32 _SJA1000_set_accmask(
LL_HANDLE *llHdl,
u_int32 extended,
u_int32 acc_mask);
int32 _SJA1000_set_acccode(
LL_HANDLE *llHdl,
u_int32 extended,
u_int32 acc_code);
int32 _SJA1000_set_bitrate(
LL_HANDLE *llHdl,
u_int32 canclock,
u_int32 bitrate,
u_int32 spl);
int32 _SJA1000_enable(
LL_HANDLE *llHdl,
u_int32 enable);
void _SJA1000_irq_enable(
LL_HANDLE *llHdl,
u_int32 val);
int32 _SJA1000_config_msg(
LL_HANDLE *llHdl,
u_int32 nr,
u_int32 dir,
u_int32 extended,
u_int32 id,
u_int32 mask);
int32 _SJA1000_transmit_abort(
LL_HANDLE *llHdl,
u_int32 nr);
int32 _SJA1000_set_physintf(
LL_HANDLE *llHdl,
u_int32 phys_intf,
u_int32 user_cfg);
int32 _SJA1000_clear_busoff(
LL_HANDLE *llHdl);
int32 _SJA1000_rtr(
LL_HANDLE *llHdl,
u_int32 nr);
int32 _SJA1000_check_reset_mode(
LL_HANDLE *llHdl);
int32 _SJA1000_reset_mode(
LL_HANDLE *llHdl);
int32 _SJA1000_operating_mode(
LL_HANDLE *llHdl);

#ifdef __cplusplus
	}
#endif

