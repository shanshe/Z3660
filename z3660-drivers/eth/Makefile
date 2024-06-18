###############################################################################
#
# makefile for vbcc or gcc
#
# author: Henryk Richter <henryk.richter@gmx.net>
#
# concept:
#
# tools required:
#  - vbcc, defaulting to m68k-amigaos
#  - vlink
#  (- vasm)
#
# porting:
#
#  see Common.mk
#
###############################################################################
VBCC=/opt/vbcc
#VBCC=/usr/local/opt/vbcc

###############################################################################
# Date, version, extra objects to build
#
###############################################################################
DEVICEVERSION=1
DEVICEREVISION=0
DEVICEDATE=31.07.2019
#ASMOBJECTS = $(SUBDIR)/enc624j6l.o $(SUBDIR)/intervaltimer.o kprintf.o

###############################################################################
# Devices to build (1 or 2, keep DEVICEID2 empty if only one build is desired)
#
###############################################################################

DEVICEID=Z3660Net.device
DEFINES = #
ASMDEFS = #
CPU = 68060

DEVICEID2= #test.device_68000
DEFINES2 = # -DPROTO_V2EXPNET
ASMDEFS2 = # -DPROTO_V2EXPNET
CPU2 = #68000

###############################################################################
# import generic ruleset
# 
###############################################################################
include Common.mk



