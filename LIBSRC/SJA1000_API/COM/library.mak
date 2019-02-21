#***************************  M a k e f i l e  *******************************
#  
#         Author: kp
#          $Date: 2004/10/06 14:51:41 $
#      $Revision: 1.2 $
#                      
#    Description: Makefile descriptor file for SJA1000_API lib
#                      
#-----------------------------------------------------------------------------
#   Copyright (c) 1999-2019, MEN Mikro Elektronik GmbH
#*****************************************************************************

MAK_NAME=sja1000_api

MAK_LIBS=


MAK_INCL=$(MEN_INC_DIR)/men_typs.h    	\
		 $(MEN_INC_DIR)/mdis_err.h		\
         $(MEN_INC_DIR)/mdis_api.h		\
		 $(MEN_INC_DIR)/sja1000_api.h	\
		 $(MEN_INC_DIR)/sja1000_drv.h

MAK_INP1   =   sja1000_api$(INP_SUFFIX)

MAK_INP =   $(MAK_INP1) 

