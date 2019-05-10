#***************************  M a k e f i l e  *******************************
#
#         Author: dieter.pfeuffer@men.de
#          $Date: 2004/10/06 14:50:32 $
#      $Revision: 1.1 $
#
#    Description: Makefile definitions for the SJA1000 driver (swapped version)
#
#-----------------------------------------------------------------------------
#   Copyright (c) 2019, MEN Mikro Elektronik GmbH
#*****************************************************************************
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
