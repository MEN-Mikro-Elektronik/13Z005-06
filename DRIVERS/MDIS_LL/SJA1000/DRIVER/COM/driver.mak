#***************************  M a k e f i l e  *******************************
#
#         Author: ww
#          $Date: 2004/10/06 14:50:30 $
#      $Revision: 1.6 $
#
#    Description: Makefile definitions for the SJA1000 driver
#
#	 MAK_SWITCH:	SBC5
#					M51
#					M75
#-----------------------------------------------------------------------------
#   Copyright (c) 1999-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=sja1000

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED

MAK_LIBS=$(LIB_PREFIX)$(MEN_LIB_DIR)/desc$(LIB_SUFFIX)    \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/oss$(LIB_SUFFIX)     \
         $(LIB_PREFIX)$(MEN_LIB_DIR)/dbg$(LIB_SUFFIX)     \


MAK_INCL=$(MEN_INC_DIR)/sja1000_drv.h   \
         $(MEN_INC_DIR)/men_typs.h      \
         $(MEN_INC_DIR)/oss.h           \
         $(MEN_INC_DIR)/mdis_err.h      \
         $(MEN_INC_DIR)/maccess.h       \
         $(MEN_INC_DIR)/desc.h          \
         $(MEN_INC_DIR)/mdis_api.h      \
         $(MEN_INC_DIR)/mdis_com.h      \
         $(MEN_INC_DIR)/ll_defs.h       \
         $(MEN_INC_DIR)/ll_entry.h      \
         $(MEN_INC_DIR)/dbg.h           \
         $(MEN_INC_DIR)/sja1000.h       \
         $(MEN_INC_DIR)/sja1000_api.h   \
		 $(MEN_MOD_DIR)/sja1000_int.h


MAK_INP1=sja1000_drv$(INP_SUFFIX)
MAK_INP2=accmask$(INP_SUFFIX)
MAK_INP3=acccode$(INP_SUFFIX)
MAK_INP4=bustime$(INP_SUFFIX)
MAK_INP5=config$(INP_SUFFIX)
MAK_INP6=physintf$(INP_SUFFIX)
MAK_INP7=misc$(INP_SUFFIX)
MAK_INP8=init$(INP_SUFFIX)

MAK_INP=$(MAK_INP1) \
        $(MAK_INP2) \
        $(MAK_INP3) \
        $(MAK_INP4) \
        $(MAK_INP5) \
        $(MAK_INP6) \
        $(MAK_INP7) \
        $(MAK_INP8)
