/*********************  P r o g r a m  -  M o d u l e ***********************
 *
 *         Name: sja1000_api.c
 *      Project: MDIS driver for sja1000 CAN controller
 *
 *       Author: kp
 *        $Date: 2004/10/06 14:51:39 $
 *    $Revision: 1.7 $
 *
 *  Description: API functions to access the sja1000 MDIS driver
 *
 *     Required: -
 *     Switches: -
 *
 *---------------------------------------------------------------------------
 * Copyright (c) 1999-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/
/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#define _EDITION "$Id: sja1000_api.c,v 1.7 2004/10/06 14:51:39 dpfeuffer Exp $"

#include <stdio.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_err.h>
#include <MEN/mdis_api.h>
#include <MEN/sja1000_api.h>
#include <MEN/sja1000_drv.h>

/********************************* MdisCall **********************************
 *
 *  Description: Function to pass API parameter blocks to driver
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 pb			parameter block
 *				 func		function number to be set in parameter block
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
static int32 MdisCall(	/* nodoc */
	int32 path,
	SJA1000_CALL_PB *pb,
	enum SJA1000_API_FUNC func)
{
	M_SG_BLOCK sg;

	pb->func = func;

	sg.size = sizeof(*pb);
	sg.data = (void *)pb;

	return M_getstat( path, SJA1000_BLK_CALL, (int32 *)&sg );
}

/******************************* sja1000_init ********************************
 *
 *  Description: Init and reset controller
 *
 *               - disable rcv/xmt
 *               - check for controller ready
 *               - reset all registers
 *		         - config controller clock
 *				 - set global acceptance code to 0x0000
 *				 - set global acceptance mask to 0xffff
 *               - config number 0 as error object with 20 entries
 *               - enable global interrupts
 *
 *               (Bus activity is left disabled by this function)
 *
 *				 To enable controller activity configure all message object,
 *				 set bitrate, set acceptance mask, set acceptance and
 *               call sja1000_enable().
 *
 *---------------------------------------------------------------------------
 *  Input......: device		device name
 *  Output.....: return     path number or <0 = error
 *  Globals....: errno		CPL_ERR_RESETACTIVE  controller in reset state
 ****************************************************************************/
int32 __MAPILIB sja1000_init(char *device)
{
	int32 path;

	if( (path = M_open(device)) < 0 )
		return path;

    /*--- config default error object, 20 queue entries---
     * nr       :  0    error object
     * dir      :  0    (receive),
     * extended :  0    not used
     * code     :  0    not used
     * mask     :  0    not used
     * qentries : 20    number of queues
	 */
	if( sja1000_config_msg( path, 0, CPL_DIR_RCV, 0, 0, 0, 20 ) < 0)
        return -1;

	/*--- enable irq ---*/
	if( (M_setstat( path, M_MK_IRQ_ENABLE, TRUE ) < 0 ))
		return -1;
	return path;
}

/******************************* sja1000_term ********************************
 *
 *  Description: De-init controller (close path)
 *
 *               - switch off transmitter (recessive)
 *               - disable irqs
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_term(int32 path)
{
	/*--- disable irq ---*/
	M_setstat( path, M_MK_IRQ_ENABLE, FALSE );

	return M_close(path);
}

/**************************** sja1000_set_acccode ****************************
 *
 *  Description: Set global acceptance code for standard or extended ID's
 *
 *  (Bus activity should be disabled when calling this function)
 *
 *  Call sja1000_enable() before and after when using this function
 *
 *  The filter is controlled by the acceptance code and mask registers according
 *  to the algorithms given in the data sheet. The received data is compared
 *  bitwise with the value contained in the Acceptance Code register. The
 *  Acceptance Mask Register defines the bit positions, which are relevant
 *  for the comparison (0 = relevant, 1 = not relevant). For accepting a message
 *  all relevant received bits have to match the respective bits in the
 *  Acceptance Code Register (refer application note AN97076).
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               extended	message ID type
 *							0=standard, rtr mask bit is set to 0
 *							1=extended, rtr mask bit is set to 0
 *							2=standard, rtr mask bit is set to 1
 *							3=extended, rtr mask bit is set to 1
 *               code		global acceptance code
 *  Output.....: return     success or error (<0)
 *  Globals....: errno      CPL_ERR_BADMODE     controller is not in reset mode
 ****************************************************************************/
int32 __MAPILIB sja1000_set_acccode(
int32 path,
u_int32 extended,
u_int32 acc_code )
{
	SJA1000_CALL_PB pb;

	pb.p.set_acccode.extended = extended;
	pb.p.set_acccode.acc_code = acc_code;

	return MdisCall( path, &pb, F_SET_ACCCODE );
}

/**************************** sja1000_set_accmask ****************************
 *
 *  Description: Set global acceptance mask for standard or extended ID's
 *
 *	(Bus activity should be disabled when calling this function)
 *
 *  Call sja1000_enable() before and after when usind this function
 *
 *  The filter is controlled by the acceptance code and mask registers according
 *  to the algorithms given in the data sheet. The received data is compared
 *  bitwise with the value contained in the Acceptance Code register. The
 *  Acceptance Mask Register defines the bit positions, which are relevant
 *  for the comparison (0 = relevant, 1 = not relevant). For accepting a message
 *  all relevant received bits have to match the respective bits in the
 *  Acceptance Code Register (refer application note AN97076).
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               extended	message ID type
 *							0=standard, rtr mask bit is set to 0
 *							1=extended, rtr mask bit is set to 0
 *							2=standard, rtr mask bit is set to 1
 *							3=extended, rtr mask bit is set to 1
 *               mask		global acceptance mask
 *  Output.....: return     success or error (<0)
 *  Globals....: errno      CPL_ERR_BADMODE     controller is not in reset mode
 ****************************************************************************/
int32 __MAPILIB sja1000_set_accmask(
int32 path,
u_int32 extended,
u_int32 acc_mask )
{
	SJA1000_CALL_PB pb;

	pb.p.set_accmask.extended = extended;
	pb.p.set_accmask.acc_mask = acc_mask;

	return MdisCall( path, &pb, F_SET_ACCMASK );
}

/**************************** sja1000_set_accfilter **************************
 *
 *  Description: Set global acceptance code and acceptance mask for
 *                  standard or extended ID's
 *
 *  (Bus activity should be disabled when calling this function)
 *
 *  Call sja1000_enable() before and after when usind this function
 *
 *  The filter is controlled by the acceptance code and mask registers according
 *  to the algorithms given in the data sheet. The received data is compared
 *  bitwise with the value contained in the Acceptance Code register. The
 *  Acceptance Mask Register defines the bit positions, which are relevant
 *  for the comparison (0 = relevant, 1 = not relevant). For accepting a message
 *  all relevant received bits have to match the respective bits in the
 *  Acceptance Code Register (refer application note AN97076).
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               extended	message ID type
 *							0=standard, rtr mask bit is set to 0
 *							1=extended, rtr mask bit is set to 0
 *							2=standard, rtr mask bit is set to 1
 *							3=extended, rtr mask bit is set to 1
 *               code		global acceptance code
 *               mask		global acceptance mask
 *  Output.....: return     success or error (<0)
 *  Globals....: errno      CPL_ERR_BADMODE     controller is not in reset mode
 ****************************************************************************/
int32 __MAPILIB sja1000_set_accfilter(
int32 path,
u_int32 extended,
u_int32 acc_code,
u_int32 acc_mask )
{
	SJA1000_CALL_PB pb;
    int32 rval;

	pb.p.set_acccode.extended = extended;
	pb.p.set_acccode.acc_code = acc_code;

	rval = MdisCall( path, &pb, F_SET_ACCCODE );

	if (rval == 0)
	{
	    pb.p.set_accmask.extended = extended;
	    pb.p.set_accmask.acc_mask = acc_mask;

	    rval = MdisCall( path, &pb, F_SET_ACCMASK );
    }
    return rval;
}

/**************************** sja1000_set_bitrate ****************************
 *
 *  Description: Calculate and config bus timing for given bitrate
 *		         (Bus activity should be disabled when calling this function)
 *
 *               Supports all bitrates recommended by CiA Standard 102 V2.0.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               bitrate	bus bitrate specifier
 *                              0 = 1   Mbit/s
 *                              1 = 800 kbit/s
 *                              2 = 500 kbit/s
 *                              3 = 250 kbit/s
 *                              4 = 125 kbit/s
 *                              5 = 100 kbit/s
 *                              6 = 50  kbit/s
 *                              7 = 20  kbit/s
 *                              8 = 10  kbit/s
 *               spl		sample mode
 *							 0=fast, take one sample per bit
 *							 1=slow, take three samples per bit
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADSPEED       bitrate not supported
 *                          CPL_ERR_BADTMDETAILS   (should never occur)
 ****************************************************************************/
int32 __MAPILIB sja1000_set_bitrate(
int32 path,
u_int32 bitrate,
u_int32 spl )
{
	SJA1000_CALL_PB pb;

	pb.p.set_bitrate.bitrate    = bitrate;
	pb.p.set_bitrate.spl        = spl;

	return MdisCall( path, &pb, F_SET_BITRATE );
}

/******************************* sja1000_config_msg **************************
 *
 *  Description: Config a CAN message 1.... object or error object
 *
 *  The special object #0 configures the error object, i.e. the number of
 *  entries in the error queue. For this object "extended", "id" and
 *  "mask" is ignored, "dir" must be set to CPL_DIR_RCV.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....) or 0 for error obj.
 *               dir		direction
 *							CPL_DIR_RCV		receive
 *							CPL_DIR_XMT		transmit
 *							CPL_DIR_DIS		disable
 *               extended	message ID type (0=standard, 1=extended)
 *               code       local acceptance code for message object
 *							(ignored for xmt objects)
 *               mask		local acceptance mask for message object
 *							(ignored for xmt objects)
 *				 qentries 	queue depth (number of entries in rx or tx queue)
 *							must be >0...>1000
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 *							CPL_ERR_BADDIR	   object 15 is receive only
 *							ERR_OSS_MEMALLOC   can't alloc mem for queue
 ****************************************************************************/
int32 __MAPILIB sja1000_config_msg(
int32 path,
u_int32 nr,
u_int32 dir,
u_int32 extended,
u_int32 code,
u_int32 mask,
u_int32 qentries)
{
	SJA1000_CALL_PB pb;

	pb.p.config_msg.nr			= nr;
	pb.p.config_msg.dir			= dir;
	pb.p.config_msg.extended    = extended;
	pb.p.config_msg.id			= code;
	pb.p.config_msg.mask	    = mask;
	pb.p.config_msg.qentries    = qentries;

	return MdisCall( path, &pb, F_CONFIG_MSG );
}

/******************************* sja1000_read_msg ****************************
 *
 *  Description: Read a message from a CAN object's message queue
 *
 *  Reads the next message from the object's receive queue and copies the
 *  message identifier, length and data portion to the user's buffer.
 *
 *  The "timeout" parameter specifies how to wait for messages:
 *
 *  If "timeout" is -1 and no message is present in the queue, the function
 *  returns -1 and errno is set to CPL_ERR_NOMESSAGE.
 *
 *  If "timeout" is >=0 and no message is present in the queue, the function
 *  waits until a message arrives or a timeout occurs. A value of 0
 *  causes this function to wait forever. Depending on the operating system,
 *  this wait may be aborted by a deadly signal.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....)
 *				 timeout	flags if this call waits until message available
 *							(-1=don't wait, 0=wait forever, >0=tout in ms)
 *				 idP		ptr to place where message identifier will be
 *							stored. If the highest bit is set a rtr message
 *							has been received
 *				 datlenP	ptr to place where message length will be stored
 *				 data		ptr to buffer where message data will be stored
 *							(should be 32bit aligned)
 *  Output.....: *idP		message identifier
 *				 *datlenP	number of valid bytes in <data>
 *				 return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 *							CPL_ERR_NOMESSAGE  no message present (if wait=0)
 *							CPL_ERR_BADDIR	   object configured for transmit
 *						 	ERR_OSS_TIMEOUT	   timeout occurred
 ****************************************************************************/
int32 __MAPILIB sja1000_read_msg(
int32 path,
u_int32 nr,
int32 timeout,
u_int32 *idP,
u_int32 *datlenP,
u_int8 *data)
{
	SJA1000_CALL_PB pb;
	int32 rval;

	pb.p.read_msg.nr		= nr;
	pb.p.read_msg.timeout	= timeout;

	rval = MdisCall( path, &pb, F_READ_MSG );

	if( rval == 0 )
	{
		*idP		= pb.p.read_msg.id;
		*datlenP    = pb.p.read_msg.datlen;

		/*--- copy data as two longs (faster) ---*/
		*(u_int32 *)&data[0] = *(u_int32 *)&pb.p.read_msg.data[0];
		*(u_int32 *)&data[4] = *(u_int32 *)&pb.p.read_msg.data[4];
	}
	return rval;
}

/******************************* sja1000_write_msg ***************************
 *
 *  Description: Put message into CAN object's transmit queue
 *
 *  Puts the specified message into the objects message queue.
 *
 *  If mode=1, this message will be transmitted immediately once it is
 *  at the head of the transmit queue.
 *
 *  If mode=0, this message will be sent after receiving an RTR frame for
 *  the message object.
 *
 *  If mode=3, this message will be transmitted a rtr message immediately
 *	once it is at the head of the transmit queue (see sja1000_rtr()).
 *
 *  The "timeout" parameter specifies how to wait for messages:
 *
 *  If "timeout" is -1 and no space left in the queue, the function
 *  returns -1 and errno is set to CPL_ERR_QFULL.
 *
 *  If "timeout" is >=0 and no space left in the queue, the function
 *  waits until space is again available or a timeout occurs. A value of 0
 *  causes this function to wait forever. Depending on the operating system,
 *  this wait may be aborted by a deadly signal.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....)
 *				 timeout	flags if this call waits until message available
 *							(-1=don't wait, 0=wait forever, >0=tout in ms)
 *				 mode		transmit mode
 *                          0 = send message after receiving RTR frame
 *                          1 = send message immediately
 *							3 = send rtr message immediately
 *				 id			message identifier
 *				 datlen		transmit data length (0..8 bytes)
 *				 data		data to be transmitted (should be 32bit aligned)
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 *							CPL_ERR_QFULL 	   no space in queue (if wait=0)
 *							CPL_ERR_BADDIR	   object configured for receive
 *						 	ERR_OSS_TIMEOUT	   timeout occurred
 ****************************************************************************/
int32 __MAPILIB sja1000_write_msg(
int32 path,
u_int32 nr,
int32 timeout,
u_int32 mode,
u_int32 id,
u_int32 datlen,
u_int8 *data)
{
	SJA1000_CALL_PB pb;

	pb.p.write_msg.nr		= nr;
	pb.p.write_msg.timeout	= timeout;
	pb.p.write_msg.mode		= mode;
	pb.p.write_msg.id		= id;
	pb.p.write_msg.datlen	= datlen;

	*(u_int32 *)&pb.p.write_msg.data[0] = *(u_int32 *)&data[0];
	*(u_int32 *)&pb.p.write_msg.data[4] = *(u_int32 *)&data[4];

	return MdisCall( path, &pb, F_WRITE_MSG );
}

/******************************* sja1000_read_error **************************
 *
 *  Description: Read next error info from the CAN's global error queue
 *
 *  For each error detected by the CAN controller one entry is inserted
 *  into the CAN's global error queue.
 *  If no message is present in the queue, the function returns -1 and
 *  errno is set to CPL_ERR_NOMESSAGE.
 *
 *  Each entry contains:
 *
 *		errcode:
 *			CPL_BUSOFF_SET		controller has gone into bus off state
 *			CPL_BUSOFF_CLR		controller has left bus off state
 *			CPL_WARN_SET		controller has reached warning level
 *			CPL_WARN_CLR		controller has left warning level
 *			CPL_QOVERRUN 		object's receiver queue overflowed
 *
 *			CPL_DATA_OVERRUN	data overrun interrupt occurred
 *								(receive fifo full)
 *
 * 			CPL_TBUFFER_LOCKED	transmitter buffer locked
 *
 *		nr:
 *			Contains the related message object number. For global errors,
 *			"nr" is always 0, for CPL_QOVERRUN "nr" contains the related
 *			message object number (1....).
 *
 *  Note: If cause a queue overrun CPL_QOVERRUN are generated only once until
 *		  the user calls sja1000_read_msg().
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 errcodeP	ptr to place where error code will be stored
 *				 nrP		ptr to place where object nr will be stored
 *  Output.....: *errcodeP	error code
 *				 *nrP		object nr
 *				 return     success or error (<0)
 *  Globals....: errno      CPL_ERR_NOOBJECT
 *                          CPL_ERR_NOMESSAGE
 ****************************************************************************/
int32 __MAPILIB sja1000_read_error(
int32 path,
u_int32 *errcodeP,
u_int32 *nrP )
{
	SJA1000_CALL_PB pb;
	int32 rval;

	pb.p.read_error.timeout	= -1;   /* not blocked */
    pb.p.read_error.errcode = 0;
    pb.p.read_error.nr      = 0;

	rval = MdisCall( path, &pb, F_READ_ERROR );

	*errcodeP   = pb.p.read_error.errcode;
	*nrP 		= pb.p.read_error.nr;

	return rval;
}

/******************************* sja1000_read_error_timeout ******************
 *
 *  Description: Read next error info from the CAN's global error queue
 *
 *  For each error detected by the CAN controller one entry is inserted
 *  into the CAN's global error queue.
 *
 *  Each entry contains:
 *
 *		errcode:
 *			CPL_BUSOFF_SET		controller has gone into bus off state
 *			CPL_BUSOFF_CLR		controller has left bus off state
 *			CPL_WARN_SET		controller has reached warning level
 *			CPL_WARN_CLR		controller has left warning level
 *			CPL_QOVERRUN 		object's receiver queue overflowed
 *
 *			CPL_DATA_OVERRUN	data overrun interrupt occurred
 *								(receive fifo full)
 *
 * 			CPL_TBUFFER_LOCKED	transmitter buffer locked
 *
 *		nr:
 *			Contains the related message object number. For global errors,
 *			"nr" is always 0, for CPL_QOVERRUN "nr" contains the related
 *			message object number (1....).
 *
 *  	timeout:
 *          The "timeout" parameter specifies how to wait for messages:
 *
 *          If "timeout" is -1 and no message is present in the queue,
 *          the function returns -1 and errno is set to CPL_ERR_NOMESSAGE.
 *
 *          If "timeout" is >=0 and no space left in the queue, the function
 *          waits until space is again available or a timeout occurs. A value of 0
 *          causes this function to wait forever. Depending on the operating system,
 *          this wait may be aborted by a deadly signal.
 *
 *  Note: If cause a queue overrun CPL_QOVERRUN are generated only once until
 *		  the user calls sja1000_read_msg().
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 errcodeP	ptr to place where error code will be stored
 *				 nrP		ptr to place where object nr will be stored
 *				 timeout	flags if this call waits until message available
 *							(-1=don't wait, 0=wait forever, >0=tout in ms)
 *  Output.....: *errcodeP	error code
 *				 *nrP		object nr
 *				 return     success or error (<0)
 *  Globals....: errno      CPL_ERR_NOOBJECT
 *                          CPL_ERR_NOMESSAGE
 ****************************************************************************/
int32 __MAPILIB sja1000_read_error_timeout(
int32 path,
u_int32 *errcodeP,
u_int32 *nrP,
int32 timeout)
{
	SJA1000_CALL_PB pb;
	int32 rval;

	pb.p.read_error.timeout	= timeout;
    pb.p.read_error.errcode = 0;
    pb.p.read_error.nr      = 0;

	rval = MdisCall( path, &pb, F_READ_ERROR );

	*errcodeP   = pb.p.read_error.errcode;
	*nrP 		= pb.p.read_error.nr;

	return rval;
}

/******************************* sja1000_set_rcvsig **************************
 *
 *  Description: Install signal for message receiption
 *
 *  Installs a signal for the specified message object number. This signal
 *  is sent to the calling process whenever a new message is written into
 *  the object's receive queue.
 *
 *  This function is also used to install a signal for the special error
 *  object (nr=0).
 *
 *  It is possible to install different signals for different message objects.
 *  For each message object, this function must be called once.
 *
 *  Only one process can install a signal for a specific message object.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....) or 0 for error obj
 *				 signal		signal code to be installed
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 *							CPL_ERR_BADDIR	   object configured for transmit
 *							CPL_ERR_SIGBUSY	   signal already installed
 ****************************************************************************/
int32 __MAPILIB sja1000_set_rcvsig(
int32 path,
u_int32 nr,
int32 signal)
{
	SJA1000_CALL_PB pb;

	pb.p.set_rcvsig.nr		= nr;
	pb.p.set_rcvsig.signal	= signal;

	return MdisCall( path, &pb, F_SET_RCVSIG );
}

/******************************* sja1000_set_xmtsig **************************
 *
 *  Description: Install signal for message transmission
 *
 *  Installs a signal for the specified message object number. This signal
 *  is sent to the calling process whenever a message has been transmitted
 *  over the CAN bus.
 *
 *  It is possible to install different signals for different message objects.
 *  For each message object, this function must be called once.
 *
 *  Only one process can install a signal for a specific message object.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....)
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 *							CPL_ERR_SIGBUSY	   signal already installed
 ****************************************************************************/
int32 __MAPILIB sja1000_set_xmtsig(
int32 path,
u_int32 nr,
int32 signal)
{
	SJA1000_CALL_PB pb;

	pb.p.set_xmtsig.nr		= nr;
	pb.p.set_xmtsig.signal	= signal;

	return MdisCall( path, &pb, F_SET_XMTSIG );
}

/******************************* sja1000_clr_rcvsig **************************
 *
 *  Description: Deactivate signal for message receiption
 *
 *  Deactivate signal condition installed with sja1000_set_rcvsig().
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....) or 0 for error obj
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 *							CPL_ERR_BADDIR	   object configured for transmit
 ****************************************************************************/
int32 __MAPILIB sja1000_clr_rcvsig(
int32 path,
u_int32 nr)
{
	SJA1000_CALL_PB pb;

	pb.p.clr_rcvsig.nr	= nr;

	return MdisCall( path, &pb, F_CLR_RCVSIG );
}

/******************************* sja1000_clr_xmtsig **************************
 *
 *  Description: Deactivate signal for message transmission
 *
 *  Deactivate signal condition installed with sja1000_set_xmtsig().
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....)
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 ****************************************************************************/
int32 __MAPILIB sja1000_clr_xmtsig(
int32 path,
u_int32 nr)
{
	SJA1000_CALL_PB pb;

	pb.p.clr_xmtsig.nr	= nr;

	return MdisCall( path, &pb, F_CLR_XMTSIG );
}

/******************************* sja1000_queue_status ************************
 *
 *  Description: Get number of entries in an objects receive/transmit queue
 *
 *  For receive message objects returns the number of received messages in
 *  the queue.
 *
 *  For transmit message objects returns the number of free messages in the
 *  queue.
 *
 *  For the special error object (nr=0) returns the number of error entries
 *  in the error queue.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....) or 0 for error obj
 *				 entriesP	ptr to place where nr. of entries should be stored
 *				 directionP	ptr to place where direction flag should be stored
 *  Output.....: return     success or error (<0)
 *				 *entriesP		number of received (RX) or free (TX) messages
 *				 *directionP	direction flag
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 ****************************************************************************/
int32 __MAPILIB sja1000_queue_status(
int32 path,
u_int32 nr,
u_int32 *entriesP,
u_int32 *directionP)
{
	SJA1000_CALL_PB pb;
	int32 rval;

	pb.p.queue_status.nr		= nr;
	pb.p.queue_status.entries	= 0;
	pb.p.queue_status.direction = 0;

	rval = MdisCall( path, &pb, F_QUEUE_STATUS );

	*entriesP	= pb.p.queue_status.entries;
	*directionP = pb.p.queue_status.direction;

	return rval;
}

/******************************* sja1000_queue_clear *************************
 *
 *  Description: Clear message queue
 *
 *  Clears all entries in a message queue (nr>0) or error queue (nr=0).
 *
 *  For transmit message queues, if txabort is not 0, any pending transmit
 *  request that has not been completed is also aborted.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....) or 0 for error obj
 *				 txabort	Tx queues: if !=0 abort pending transmission
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 ****************************************************************************/
int32 __MAPILIB sja1000_queue_clear(
int32 path,
u_int32 nr,
u_int32 txabort)
{
	SJA1000_CALL_PB pb;

	pb.p.queue_clear.nr		    = nr;
	pb.p.queue_clear.txabort    = txabort;

	return MdisCall( path, &pb, F_QUEUE_CLEAR );
}

/**************************** sja1000_clear_busoff ***************************
 *
 *  Description: Reset controller from busoff state
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_clear_busoff(
int32 path)
{
	SJA1000_CALL_PB pb;

	return MdisCall( path, &pb, F_CLEAR_BUSOFF );
}

/**************************** sja1000_enable *********************************
 *
 *  Description: Enable/Disable controller from bus activity
 *
 *  While bus activity is disabled, no messages may transmitted or received.
 *  Transmitter line is then in recessive state.
 *
 *  Before enabling bus activity, message objects and bitrate must have been
 *	already configured.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               enable		enable/disable bus activity (0=disable, 1=enable)
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_enable(
int32 path,
u_int32 enable )
{
	SJA1000_CALL_PB pb;

	pb.p.enable.enable = enable;

	return MdisCall( path, &pb, F_ENABLE );
}

/******************************* sja1000_rtr **********************************
 *
 *  Description: Send a remote frame request
 *
 *  Requests a remote frame to be sent on the specified object. Once the
 *  frame has been received it is put into the object's receive queue and
 *  can be read using sja1000_read_msg().
 *
 *  The message object must be configured for transmit mode. The remote
 *	frame request contains no data byte and the data length is set to zero.
 *
 *	Note:
 *	This call is the same as a sja1000_write_msg call with mode 3, data length
 *	between 0..8 and timeout 1000. Not data frame are included in the transmitted
 *  frame. Nevertheless, it is necessary to specify the correct data length
 *  code which depends on the corresponding data frame with tje same identifier
 *  coding.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *               nr			message object number (1....)
 *				 id			message identifier
 *				 dl			data length
 *  Output.....: return     success or error (<0)
 *  Globals....: errno		CPL_ERR_BADMSGNUM  illegal message object number
 *						    CPL_ERR_BADDIR	   object not configured for rcv
 ****************************************************************************/
int32 __MAPILIB sja1000_rtr(
int32 path,
u_int32 nr,
u_int32 id,
u_int32	dl)
{
	SJA1000_CALL_PB pb;

	pb.p.write_msg.nr		= nr;
	pb.p.write_msg.timeout	= 1000;
	pb.p.write_msg.mode		= 3;
	pb.p.write_msg.id		= id;
#if	1
	pb.p.write_msg.datlen   = dl;

	if (dl>8)
		pb.p.write_msg.datlen = 8;
#else
	pb.p.write_msg.datlen  = 0;
#endif
	*(u_int32 *)&pb.p.write_msg.data[0] = 0;
	*(u_int32 *)&pb.p.write_msg.data[4] = 0;

	return MdisCall( path, &pb, F_WRITE_MSG );
}

/***************************** sja1000_errmsg ********************************
 *
 *  Description: Return error message string
 *
 *---------------------------------------------------------------------------
 *  Input......: error      error code returned from sja1000_xxx() function
 *  Output.....: return     error message string
 *  Globals....: -
 ****************************************************************************/
char *__MAPILIB sja1000_errmsg(int32 error)
{
	char *str;
	static char errMsg[128];

	switch(error)
	{
	    /*----- canbus error codes returned by sja1000_read_error() ---*/
        case CPL_BUSOFF_SET			: str="controller bus off";             break;
        case CPL_BUSOFF_CLR			: str="controller bus on";              break;
        case CPL_WARN_SET			: str="controller warning on";          break;
        case CPL_WARN_CLR			: str="controller warning off";         break;
        case CPL_MSGLOST			: str="controller's msg buf overwr.";   break;
        case CPL_QOVERRUN			: str="object queue overrun";           break;
        case CPL_BUS_ERROR			: str="bus error intterrupt";           break;
        case CPL_ARBITRATION_LOST	: str="arbitration lost interrupt";     break;
        case CPL_ERROR_PASSIVE		: str="error passive interrupt";        break;
        case CPL_WAKE_UP			: str="wake up interrupt";              break;
        case CPL_DATA_OVERRUN		: str="data overrun interrupt";         break;
        case CPL_ERROR_WARNING		: str="error warning interrupt";        break;
        case CPL_TBUFFER_LOCKED		: str="transmit buffer locked";         break;
        case CPL_RQUEUE_NOTREADY	: str="receive queue notready";         break;
        case CPL_RTR_FRAME			: str="rtr frame received";             break;
        case CPL_PASSIVE_STATE		: str="controller has reached the passive status";      break;
        case CPL_ACTIVE_STATE		: str="controller has enter the active status again";   break;
        case CPL_RECOVER_BUSOFF		: str="can't release busoff state";     break;

        case CPL_ERR_BADSPEED       : str="bitrate not supported";          break;
        case CPL_ERR_BADINTERFACE   : str="interface not supported";        break;
        case CPL_ERR_NOMESSAGE      : str="no message in receive buffer";   break;
        case CPL_ERR_RESETACTIVE	: str="controller reset in progress";   break;
        case CPL_ERR_BADCLOCK		: str="illegal system clock";           break;
        case CPL_ERR_BADTMDETAILS	: str="illegal timing details";         break;
        case CPL_ERR_BADMSGNUM		: str="illegal message object nr.";     break;
        case CPL_ERR_BADDIR			: str="illegal message direction";      break;
        case CPL_ERR_QFULL			: str="message queue full";             break;
        case CPL_ERR_SIGBUSY		: str="signal already installed";       break;
        case CPL_ERR_BADPARAMETER	: str="bad parameter";                  break;
        case CPL_ERR_TXQFULL		: str="transmit queue full";            break;
        case CPL_ERR_RXQFULL		: str="receive queue full";             break;
        case CPL_ERR_RXQNRDY		: str="receive queue not ready";        break;
        case CPL_ERR_BADMODE		: str="illegal mode";                   break;
        case CPL_ERR_NOOBJECT       : str="object not defined";             break;
        case CPL_ERR_BADPARAM		: str="illegal parameter";              break;
        case CPL_ERR_BADBUFSIZE		: str="illegal buffer size";            break;

		default:
			str = NULL;
			break;
	}
	if( str != NULL ){
		sprintf(errMsg,"ERROR (CPL) 0x%04x:  %s",error, str);
		return errMsg;
	}
	/*--- if unknown, use MDIS error message ---*/
	return M_errstring(error);
}

/******************************* sja1000_read_counter ************************
 *
 *  Description: read interrupt counters (single or multiple read)
 *
 *  The value of 'select' variable switch between multiple read and single read
 *  of interrupts counter.
 *
 *  select  buffer  size    description
 *  0       bufP    36      all interrupt counters
 *  1       bufP    4       global interrupts       (irqCount)
 *  2       bufP    4       bus error interrupts    (irqBusError)
 *  3       bufP    4       arbitration interupts   (irqArbitrationLost)
 *  4       bufP    4       error passive           (irqErrorPassive)
 *  5       bufP    4       wake-up                 (irqWakeUp)
 *  6       bufP    4       data overrun            (irqDataOverrun)
 *  7       bufP    4       error warning           (irqErrorWarning)
 *  8       bufP    4       transmit                (irqTransmit)
 *  9       bufP    4       receive                 (irqReceive)
 *
 *  The counter reflect the number of occurred interrupt for the specified
 *  interrupt sources. The interrupt source should be enabled in the
 *  'interrupt enable register' by setting 'INTERRUPT_MASK' at the
 *  device descriptor.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 select		not used, should be zero
 *				 *bufP		buffer pointer where irq counters will be stored
 *				 size		buffer size in number of bytes (min.36 Byte)
 *  Output.....: return     success or error (<0)
 *  Globals....: errno      CPL_ERR_BADBUFSIZE
 *                          CPL_ERR_BADPARAM
 ****************************************************************************/
int32 __MAPILIB sja1000_read_counter(
int32 path,
u_int32	select,
u_int32 *bufP,
u_int32 size)
{
	SJA1000_CALL_PB pb;

	pb.p.read_counter.select    = select;
	pb.p.read_counter.bufP		= bufP;
	pb.p.read_counter.size		= size;

	return (MdisCall( path, &pb, F_READ_COUNTER ));
}

/******************************* sja1000_read_configuration ******************
 *
 *  Description: read configuration
 *
 *  The value of 'select' variable switch between multiple read and single read
 *  of configuration date.
 *
 *  sel buffer  size    description
 *  0   bufP    36      all configutation data
 *  1   bufP    4       copy of ModeControlRegister (ModeControlReg)*
 *  2   bufP    4       debug level                 (dbgLevel)      *
 *  3   bufP    4       controller clock            (inClk)         *
 *  4   bufP    4       resulting controller clock  (canClk)
 *  5   bufP    4       interrupt enable flag       (irqFlag)
                            0:disable, 1:enable
 *  6   bufP    4       physical interface          (physIntf)      *
 *  7   bufP    4       comparator bypass function  (cbpFlag)       *
 *  8   bufP    4       dual filter mode            (dualFilterMode)*
 *  9   bufP    4       interrupt mask              (mask)          *
 * 10   bufP    4       acceptance code             (acccode)
 * 11   bufP    4       acceptance mask             (accmask)
 * 12   bufP    4       bus bitrate                 (bitrate)
 *                          0:  1 Mbit/s
 *                          1:800 kbit/s
 *                          2:500 kbit/s
 *                          3:250 kbit/s
 *                          4:125 kbit/s
 *                          5:100 kbit/s
 *                          6: 50 kbit/s
 *                          7: 20 kbit/s
 *                          8: 10 kbit/s
 * 13   bufP    4       sample mode                 (spl)
 *                          0:fast, 1:slow
 * 14   bufP    4       time segment 1              (tseg1)
 * 15   bufP    4       time segment 2              (tseg2)
 * 16   bufP    4       message type                (extended)
 *							0:standard, 1:extended,
 *                          2:standard+rtr, 3:extended+rtr
 *
 * The line marked with a '*' are copies of descriptor entries.
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 select		not used, should be zero
 *				 *bufP		buffer pointer where configurations will be stored
 *				 size		buffer size in number of bytes (min. 64 bytes)
 *  Output.....: return     success or error (<0)
 *  Globals....: errno      CPL_ERR_BADBUFSIZE
 *                          CPL_ERR_BADPARAM
 ****************************************************************************/
int32 __MAPILIB sja1000_read_configuration(
int32 path,
u_int32	select,
u_int32 *bufP,
u_int32 size)
{
	SJA1000_CALL_PB pb;

	pb.p.read_configuration.select 	= select;
	pb.p.read_configuration.bufP	= bufP;
	pb.p.read_configuration.size	= size;

	return (MdisCall( path, &pb, F_READ_CONFIGURATION ));
}

/*
 * The following functions are for internal test purposes only
 */

/******************************* sja1000_read_alcr ***************************
 *
 *  Description: read arbitration lost capture registers
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 *bufP		buffer pointer where registers will be stored
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_read_alcr(
int32 path,
u_int32 *bufP)
{
	SJA1000_CALL_PB pb;

	pb.p.read_register.select 	= 3;
	pb.p.read_register.bufP		= bufP;
	pb.p.read_register.idx		= 11;	/* register number! */

	return (MdisCall( path, &pb, F_READ_REGISTER ));
}

/******************************* sja1000_read_eccr ***************************
 *
 *  Description: read error code capture registers
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 *bufP		buffer pointer where registers will be stored
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_read_eccr(
int32 path,
u_int32 *bufP)
{
	SJA1000_CALL_PB pb;

	pb.p.read_register.select 	= 3;
	pb.p.read_register.bufP		= bufP;
	pb.p.read_register.idx		= 12;	/* register number! */

	return (MdisCall( path, &pb, F_READ_REGISTER ));
}

/******************************* sja1000_read_register ***********************
 *
 *  Description: read controller registers
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 select		0:  read current register contents
 *							1:  read operating mode registers
 *							2:	read reset mode registers
 *				 *valP		buffer pointer where registers will be stored
 *				 idx		register number
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_read_register(
int32 path,
u_int32	select,
u_int32 *valP,
u_int32 idx)
{
	SJA1000_CALL_PB pb;

	pb.p.read_register.select 	= select;
	pb.p.read_register.bufP		= valP;
	pb.p.read_register.idx		= idx;

	return (MdisCall( path, &pb, F_READ_REGISTER ));
}

/******************************* sja1000_write_register **********************
 *
 *  Description: write controller registers
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 select		0:  read current register contents
 *							1:  read operating mode registers
 *							2:	read reset mode registers
 *				 *bufP		buffer pointer where registers will be stored
 *				 idx		register number
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_write_register(
int32 path,
u_int32	select,
u_int32 val,
u_int32 idx)
{
	SJA1000_CALL_PB pb;

	pb.p.write_register.select 	= select;
	pb.p.write_register.val		= val;
	pb.p.write_register.idx		= idx;

	return (MdisCall( path, &pb, F_WRITE_REGISTER ));
}

/******************************* sja1000_read_edition_number *****************
 *
 *  Description: read driver etition number
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *				 editionnumberP
 *							pointer where the edition number shut be store
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_read_edition_number(
int32 path,
u_int32 *editionnumberP)
{
	SJA1000_CALL_PB pb;
	int32 rval;

    pb.p.read_edition.edition = 0;

	rval = MdisCall( path, &pb, F_READ_EDITION );

	*editionnumberP	= pb.p.read_edition.edition;

	return rval;
}

/******************************* sja1000_abort_transmission  *****************
 *
 *  Description: abort the current transmission
 *
 *---------------------------------------------------------------------------
 *  Input......: path		path number
 *  Output.....: return     success or error (<0)
 *  Globals....: -
 ****************************************************************************/
int32 __MAPILIB sja1000_abort_transmission(
int32 path)
{
	SJA1000_CALL_PB pb;

	return MdisCall( path, &pb, F_ABORT );
}
