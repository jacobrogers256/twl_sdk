#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - CARD - libraries
# File:     Makefile
#
# Copyright 2007-2008 Nintendo. All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law. They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

SUBDIRS			 =

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL
TWL_CODEGEN     ?= ALL
TWL_PROC         = ARM9

# Create libraries for both HYBRID and LIMITED
TWL_ARCHGEN		?= ALL

SRCDIR           = ../ARM9/src ../common/src
INCDIR           = ../ARM9/include ../common/include

SRCS             = \
				   card_api.c	\
				   card_common.c	\
				   card_hook.c	\
				   card_task.c	\
				   card_utility.c	\
				   card_spi.c		\
				   card_backup.c	\
				   card_rom.c		\
				   card_hash.c \
				   card_pullOut.c	\

TARGET_LIB       = libcard$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS  = $(TARGETS)
INSTALL_DIR      = $(TWL_INSTALL_LIBDIR)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
