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
#include "IceSky.h"
void    makenewdict(){
size_t i;
char * dictonary;
	for(i=0;i<255;++i)charsfirst[i]=0;
	for(i=0;i<255;++i)charslast[i]=0;
	//"read dictonary\n";
	int fddictonary = open("fullbase.db",O_RDONLY);
	sizedic=lseek(fddictonary,0,SEEK_END);
	lseek(fddictonary,0,SEEK_SET);
	dictonary=(char*)mmap(NULL,sizedic,PROT_READ,MAP_PRIVATE,fddictonary,0);
	int count=0;
	for(i=0;i<sizedic;++i)
	 {
	  if(*(dictonary+i)=='/'){
	  ++count;
	 };
	};  
	//"alloc dictonary"
	int j;
	int posb=0;
	count++;
	dictonarymodrus=(char*)calloc(count		,stringsize*sizeof(char));
	dictonarymodeng=(char*)calloc(count		,stringsize*sizeof(char));
	dictonarybufrus=(char*)calloc(dictonarybufsize+1,stringsize*sizeof(char));
	dictonarybufeng=(char*)calloc(dictonarybufsize+1,stringsize*sizeof(char));	
	count=0;
	posb=0;
	for(i=1;i<sizedic;++i)
	 {
	  if(*(dictonary+i)=='/')
		  {
 		  j=posb+1;
		  while((*(dictonary+j)!='-')&((j-(posb+1))<stringsize))
			++j;
		  strncpy(dictonarymodeng+count,dictonary+posb+1,j-(posb+1));
		  strncpy((dictonarymodrus+count),dictonary+j+1,i-(j+1));
		  if(charsfirst[(*(dictonarymodeng+count))]==0)
			charsfirst[(*(dictonarymodeng+count))]=count; 
		  charslast[(*(dictonarymodeng+count))]=count; 
		  posb=i;
		  count=count+stringsize; 
	 	 }
        };  
	word=(count/stringsize);
	word--;
	munmap(dictonary,sizedic);
	close(fddictonary); 
};
void	get_translatemod(char *buffer){
int i;
int res;
	if(buffer==NULL)return;
	if(strlen(buffer)==0)return;
	//search in hash buffer
    	for(i=0;i<dictonarybufsize*stringsize;i+=stringsize)
	{
	 if(strncmp(dictonarybufeng+i,buffer,stringsize)==0)
	  {
	  strncpy(buffer,dictonarybufrus+i,stringsize);
	  buffer[strlen(buffer)-1]=0;
	  return ;
	  }
       };
	//search in main dictonary    
 	i=0;
	i=charsfirst[(*buffer)];
       	for(;i<word*stringsize;i+=stringsize)
	{
	 if((res=strncmp(dictonarymodeng+i,buffer,stringsize))==0)
	  {
        	    {
	    	    strncpy(dictonarybufrus+dictonarybufpos*stringsize,dictonarymodrus+i,stringsize);
    		    strncpy(dictonarybufeng+dictonarybufpos*stringsize,dictonarymodeng+i,stringsize);
    	    	    dictonarybufpos=(dictonarybufpos+1)%(dictonarybufsize);	
		    }   
	  strncpy(buffer,dictonarymodrus+i,stringsize);
	  buffer[strlen(buffer)-1]=0;
	  return ;
	  }
	  if(sorted)//for sorted speed up
	    if(res>0)
		if(strlen(buffer)>(strlen(dictonarymodeng+i)))//for not corectly sort (short word after long)
			    return;        
	  if(charslast[(*buffer)]<i)return;
       };
};
//Siple connver to UpChar
void ToUp(unsigned char* buffer)
{
	if(buffer==NULL)
		return;
       	if((buffer[0]>='a') &(buffer[0]<='z'))
		{
			buffer[0]=buffer[0]+('A'-'a');
		}
	if((buffer[0]==208))
		if(buffer[1]>175)
		{
			buffer[1]-=32;
		}
	if((buffer[0]==209))
		if(buffer[1]>127)
                {
			buffer[0]=208;
			buffer[1]+=32;
		}
}


int main(int argc, char *argv[])
{
	char buffer[stringsize];
	char ch;
	if(!nohead)
	     printf("%s",strbegin);
	makenewdict();
	int fd0;
		fd0 = open(argv[1],O_RDONLY);
	int changet=0;
	char pos=0;
	while (read(fd0,&ch,1)>0)
	{
        	if((ch>='A') &(ch<='Z'))
		{
			ch=ch-('A'-'a');
		 	changet=1;
		}	
		buffer[pos]=ch;
        	if(!((ch>='a') &(ch<='z')))
		{
    			buffer[pos]=0;
			get_translatemod(buffer);
			if(changet==1)
				ToUp(buffer);
			changet=0;
			if(!nocomment)
			    printf("<span style=\"color: rgb(0, 0, 255);\">");
			printf("%s", buffer);
			if(!nocomment)
			    printf("</span>");
			printf("%c", ch);
    			pos=-1;
		};
		++pos;
	};
	free(dictonarymodeng);
	free(dictonarymodrus);
	free(dictonarybufeng);
	free(dictonarybufrus);
	if(!nohead)
    		printf("%s",strend);
	if(!nohead)
    		printf("%c",'\n');
	return 0;
};
    

