#***************************  M a k e f i l e  *******************************
#
#         Author: kp
#          $Date: 2004/10/06 14:51:23 $
#      $Revision: 1.2 $
#
#    Description: Makefile definitions for the SJA1000 example program
#
#-----------------------------------------------------------------------------
#   Copyright (c) 1998-2019, MEN Mikro Elektronik GmbH
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
