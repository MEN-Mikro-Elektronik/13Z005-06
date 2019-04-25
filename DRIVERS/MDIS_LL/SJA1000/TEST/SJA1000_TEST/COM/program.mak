#***************************  M a k e f i l e  *******************************
#
#         Author: kp
#          $Date: 2004/10/06 14:51:27 $
#      $Revision: 1.4 $
#
#    Description: Makefile definitions for I82527 tool
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: program.mak,v $
#   Revision 1.4  2004/10/06 14:51:27  dpfeuffer
#   minor modifications for MDIS4/2004 conformity
#
#   Revision 1.3  2002/03/01 13:42:31  Schmidt
#   bug from last "cosmetic corrections" fixed :-(
#
#   Revision 1.2  2001/04/20 16:21:51  ww
#   cosmetic corrections
#
#   Revision 1.1  1999/08/03 10:55:56  ww
#   Initial Revision
#
#   Revision 1.1  1999/03/31 15:11:08  kp
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=sja1000_test

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/sja1000_api$(LIB_SUFFIX) \
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)    \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)     \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)     \

MAK_INCL=$(MEN_INC_DIR)/sja1000_api.h     \
         $(MEN_INC_DIR)/men_typs.h    \
         $(MEN_INC_DIR)/mdis_api.h    \
         $(MEN_INC_DIR)/mdis_err.h    \
         $(MEN_INC_DIR)/usr_oss.h     \
         $(MEN_INC_DIR)/usr_utl.h     \

MAK_INP1=sja1000_test$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)


