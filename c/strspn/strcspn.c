/* Copyright (C) 1991, 1994, 1996, 1997, 2003 Free Software Foundation, Inc.
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

/* Return the length of the maximum initial segment of S
   which contains no characters from REJECT.  */
size_t
strcspn_test (s, reject)
     const char *s;
     const char *reject;
{
  size_t count = 0;

  while (*s != '\0')
    if (strchr (reject, *s++) == NULL)
      ++count;
    else
      return count;

  return count;
}

//without call strchr
size_t
strcspn_test1 (s, reject)
     const char *s;
     const char *reject;
{
  size_t count = 0;
  const char * r;
  const char * p;

  p = s;
  while (*p != '\0'){
	for (r = reject; *r != '\0'; ++r)
		if (*p == *r)
		      return count;
	++count;
	++p;
  }

  return count;
}

//without call strchr and count variable
size_t
strcspn_test2 (const char *s, const char *reject)
{
  const char * r;
  const char * p;

  p = s;
  while (*p != '\0'){
	for (r = reject; *r != '\0'; ++r)
		if (*p == *r)
		      return p -s ;
	++p;
  }

  return p - s;
}

//without call strchr and count variable and with binary set
size_t
strcspn_test3 (const char *s, const char *reject)
{
	const char * r;
	const char * p;
	char testbuffer[256];

	memset(testbuffer,0,256);

	for (r = reject; *r != '\0'; ++r)
		testbuffer[(unsigned char) *r] = 1;

	p = s;
	while (*p != '\0'){
		if(testbuffer[(unsigned char)*p] != 0 )
			return p -s ;
		++p;
  	}

	return p - s;
}

//without call strchr and count variable and with binary set + delete pointer to char
size_t
strcspn_test4 (const char *s, const char *reject)
{
	const char * r;
	const char * p;
	char testbuffer[256];

	memset(testbuffer,0,256);

	r = reject;

	unsigned char curr = (unsigned char)*r;
	for (r = reject;curr != '\0'; ++r, curr = (unsigned char)*r)
		testbuffer[curr] = 1;

	p = s;
	curr = (unsigned char)*p;

	while (curr != '\0'){
		if(testbuffer[curr] != 0 )
			return p -s ;
		++p;
		curr = (unsigned char)*p;
  	}

	return p - s;
}

//test4 + min/max filter 
size_t
strcspn_test5 (const char *s, const char *reject)
{
	const char * r;
	const char * p;
	char testbuffer[256];
	char filter_max = 0;
	char filter_min = 255;

	memset(testbuffer,0,256);

	r = reject;

	unsigned char curr = (unsigned char)*r;
	for (r = reject;curr != '\0'; ++r, curr = (unsigned char)*r)
	{
		testbuffer[curr] = 1;
		filter_max |= curr;
		filter_min &= curr;
	}

	p = s;
	curr = (unsigned char)*p;

	while (curr != '\0'){
		if( (filter_max | curr) == filter_max)
			if( (filter_min & curr) == filter_min)
				if(testbuffer[curr] != 0 )
					return p -s ;
		++p;
		curr = (unsigned char)*p;
  	}

	return p - s;
}

//test3 + min/max filter 
size_t
strcspn_test6 (const char *s, const char *reject)
{
	const char * r;
	const char * p;
	char testbuffer[256];
	char filter_max = 0;
	char filter_min = 255;

	memset(testbuffer,0,256);

	for (r = reject;(unsigned char)*r != '\0'; ++r)
	{
		testbuffer[(unsigned char)*r] = 1;
		filter_max |= (unsigned char)*r;
		filter_min &= (unsigned char)*r;
	}

	p = s;
	while (*p != '\0'){
		if( (filter_max | *p) == filter_max)
			if( (filter_min & *p) == filter_min)
				if(testbuffer[(unsigned char)*p] != 0 )
					return p -s ;
		++p;
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
	memset(buffer,'b',SIZEBUFFER);
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strcspn(buffer,"a");
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
			result += strcspn_test (buffer,"a");
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
			result += strcspn_test1 (buffer,"a");
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
			result += strcspn_test2 (buffer,"a");
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
			result += strcspn_test3 (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test3 \tresult = %x time= %lld\n", result, 
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strcspn_test4 (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test4 \tresult = %x time= %lld\n", result, 
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strcspn_test5 (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test5 \tresult = %x time= %lld\n", result, 
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	{
		result = 0;
		getrusage(RUSAGE_SELF,&start);
		for(i = 0 ; i < TESTCOUNT; i ++)
			result += strcspn_test6 (buffer,"a");
		getrusage(RUSAGE_SELF,&stop);
		printf("Standart test6 \tresult = %x time= %lld\n", result, 
			(long long)(stop.ru_utime.tv_usec - start.ru_utime.tv_usec) +
			(long long)(stop.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000
		);
	}
	return 0;
}
