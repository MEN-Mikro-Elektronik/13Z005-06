#***************************  M a k e f i l e  *******************************
#  
#         Author: kp
#          $Date: 2004/10/06 14:51:41 $
#      $Revision: 1.2 $
#                      
#    Description: Makefile descriptor file for SJA1000_API lib
#                      
#---------------------------------[ History ]---------------------------------
#
#   $Log: library.mak,v $
#   Revision 1.2  2004/10/06 14:51:41  dpfeuffer
#   minor modifications for MDIS4/2004 conformity
#
#   Revision 1.1  1999/08/03 10:56:03  ww
#   Initial Revision
#
#   Revision 1.1  1999/03/31 15:11:01  kp
#   Initial Revision
#
#-----------------------------------------------------------------------------
#   (c) Copyright 1999 by MEN mikro elektronik GmbH, Nuernberg, Germany 
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

