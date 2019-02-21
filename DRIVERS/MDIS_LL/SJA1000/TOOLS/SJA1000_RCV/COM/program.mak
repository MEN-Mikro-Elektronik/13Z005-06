#***************************  M a k e f i l e  *******************************
#
#         Author: kp
#          $Date: 2004/10/06 14:51:05 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for SJA1000 tool
#
#-----------------------------------------------------------------------------
#   Copyright (c) 1999-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=sja1000_rcv

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/sja1000_api$(LIB_SUFFIX) \
		 $(LIB_PREFIX)$(MEN_LIB_DIR)/mdis_api$(LIB_SUFFIX)    \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_oss$(LIB_SUFFIX)     \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/usr_utl$(LIB_SUFFIX)     \

MAK_INCL=$(MEN_INC_DIR)/sja1000_api.h \
         $(MEN_INC_DIR)/men_typs.h    \
         $(MEN_INC_DIR)/mdis_api.h    \
         $(MEN_INC_DIR)/mdis_err.h    \
         $(MEN_INC_DIR)/usr_oss.h     \
         $(MEN_INC_DIR)/usr_err.h     \
         $(MEN_INC_DIR)/usr_utl.h     \

MAK_INP1=sja1000_rcv$(INP_SUFFIX)

MAK_INP=$(MAK_INP1)



