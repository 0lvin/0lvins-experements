/* Copyright (C) 1991, 1997, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

/* Return the length of the maximum initial segment
   of S which contains only characters in ACCEPT.  */
size_t
strspn_test (s, accept)
     const char *s;
     const char *accept;
{
  const char *p;
  const char *a;
  size_t count = 0;

  for (p = s; *p != '\0'; ++p)
    {
      for (a = accept; *a != '\0'; ++a)
	if (*p == *a)
	  break;
      if (*a == '\0')
	return count;
      else
	++count;
    }

  return count;
}

//delete count variable
size_t
strspn_test1 (const char *s, const char *accept)
{
	const char *p;
	const char *a;

	for (p = s; *p != '\0'; ++p){
		for (a = accept; *a != '\0'; ++a)
			if (*p == *a)
				break;
		if (*a == '\0')
			return p - s;
	}
	return p - s;
}

//use 'binary' set for test existing chars in accept
size_t
strspn_test2 (const char *s, const char *accept)
{
	const char *p;
	const char *a;
	char testbuffer[256];

	memset(testbuffer,0,256);

	for (a = accept; *a != '\0'; ++a)
		testbuffer[(unsigned char) *a] = 1;

	for (p = s; *p != '\0'; ++p){
		if(testbuffer[(unsigned char) *p] == 0)
			return p -s;
	}

	return p - s;
}

//use  'binary' set for test existing chars in accept and binary maximum and minimum value
size_t
strspn_test3 (const char *s, const char *accept)
{
	const char *p;
	const char *a;
	char testbuffer[256];
	char filter_max = 0;
	char filter_min = 255;

	memset(testbuffer,0,256);

	for (a = accept; *a != '\0'; ++a)
	{
		unsigned char curr=(unsigned char)*a;
		testbuffer[curr] = 1;
		filter_max |= curr;
		filter_min &= curr;
	}

	for (p = s; *p != '\0'; ++p){
		unsigned char curr=(unsigned char)*p;
		
		if( (filter_max | curr) != filter_max)
			return p -s;
		
		if( (filter_min & curr) != filter_min)
			return p -s;
		
		if(testbuffer[curr] == 0)
			return p -s;
	}

	return p - s;
}


#define SIZEBUFFER 1024*1024
#define TESTCOUNT 1024
int
main(){

	struct rusage start;
	struct rusage stop;
	int result=0;
	int i=0;
	char * buffer = calloc(1,SIZEBUFFER+1);
	memset(buffer,'a',SIZEBUFFER);
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strspn(buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart \tresult = %x time= %lld\n", result,
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strspn_test (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test \tresult = %x time= %lld\n", result,
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strspn_test1 (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test1 \tresult = %x time= %lld\n", result,
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strspn_test2 (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test2 \tresult = %x time= %lld\n", result, 
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strspn_test3 (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test3 \tresult = %x time= %lld\n", result, 
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	return 0;
}
