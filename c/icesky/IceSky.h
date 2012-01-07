/* -*- coding: utf-8 -*-
  Copyright (C) 2007  Denis Pauk

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA*/
#include	<unistd.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include 	<string.h>
#include	<sys/mman.h>
#define strbegin 	"<HTML>\n<HEAD>\n<META HTTP-EQUIV=\" CONTENT-TYPE\"  CONTENT=\"text/html; charset=utf-8\">\n<TITLE></TITLE>\n</HEAD>\n<BODY LANG=\"ru-RU\"> <PRE>"
#define strend 		"</PRE>\n</BODY>\n</HTML>\n"
#define stringsize 	128 
char *		dictonarymodeng;
char *		dictonarymodrus;
char *		dictonarybufeng;
char *		dictonarybufrus;
size_t		dictonarybufpos=0;
size_t		dictonarybufsize=256;
size_t		sizedic;
int		word=0;
int		nohead=0;
int		nocomment=0;
int 		sorted=1;
size_t 		charsfirst[256];//sizeof short int
size_t 		charslast[256];//sizeof short int
