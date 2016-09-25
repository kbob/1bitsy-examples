##
## This file is part of the libopencm3 project.
##
## Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
##
## This library is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.
##

SOURCES := actors.c ballop.c clockr.c demons.c dgame.c dinit.c dso1.c	\
	   dso2.c dso3.c dso4.c dso5.c dso6.c dso7.c dsub.c dverb1.c	\
	   dverb2.c gdt.c lightp.c local.c nobjs.c np.c np1.c np2.c	\
	   np3.c nrooms.c objcts.c rooms.c sobjs.c supp.c sverbs.c	\
	   verbs.c villns.c
OBJS    := $(patsubst %.cpp, %.o, $(SOURCES:.c=.o))

BINARY = dmain

LDSCRIPT = ../stm32f4-1bitsy.ld

include ../Makefile.include
CPPFLAGS += -Wno-parentheses -Werror