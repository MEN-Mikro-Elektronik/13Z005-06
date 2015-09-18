#***************************  M a k e f i l e  *******************************
#
#         Author: dieter.pfeuffer@men.de
#          $Date: 2004/10/06 14:50:32 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the SJA1000 driver (swapped version)
#
#---------------------------------[ History ]---------------------------------
#
#   $Log: driver_sw.mak,v $
#   Revision 1.1  2004/10/06 14:50:32  dpfeuffer
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright by MEN mikro elektronik GmbH, Nuernberg, Germany
#*****************************************************************************

MAK_NAME=sja1000_sw

MAK_SWITCH=$(SW_PREFIX)MAC_MEM_MAPPED \
		   $(SW_PREFIX)MAC_BYTESWAP \
		   $(SW_PREFIX)SJA1000_VARIANT=SJA1000_SW

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
