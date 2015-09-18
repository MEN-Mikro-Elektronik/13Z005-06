#***************************  M a k e f i l e  *******************************
#
#         Author: kp
#          $Date: 2004/10/06 14:51:23 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the SJA1000 example program
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.2  2004/10/06 14:51:23  dpfeuffer
#   minor modifications for MDIS4/2004 conformity
#
#   Revision 1.1  1999/08/03 10:55:54  ww
#   Initial Revision
#
#   Revision 1.1  1999/03/31 15:10:51  kp
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 1998 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=sja1000_simp

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/sja1000_api$(LIB_SUFFIX)    \
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)    \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)     \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)     \

MAK_INCL=$(MEN_INC_DIR)/sja1000_drv.h     \
		 $(MEN_INC_DIR)/sja1000_api.h	\
         $(MEN_INC_DIR)/men_typs.h    \
         $(MEN_INC_DIR)/mdis_api.h    \
         $(MEN_INC_DIR)/mdis_err.h    \
         $(MEN_INC_DIR)/usr_oss.h     \
         $(MEN_INC_DIR)/usr_utl.h     \

MAK_INP1=sja1000_simp$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)
