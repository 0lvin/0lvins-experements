/***************************************************************************
 *   Copyright (C) 2005 by PaukDenis                                       *
 *   den@localhost.localdomain                                             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "selectableconect.h"
int voidfunction(char * param){
  printf("%s",param);
  strcpy(param,"server au!au!au!au!au!");
  return 1;
 }
int SelectAbleConect::senddata(conects& name,char * param){
//if(param[0]!=0) return  send(name.socket,param, 65536, 0);
if(param[0]!=0)return  sendto(name.socket,param, 65536, 0, (struct sockaddr*)&name.adress,name.sockaddr_in_len);
return 0;
   }
SelectAbleConect::SelectAbleConect(){
 lostclient=10;
}
SelectAbleConect::~SelectAbleConect(){

}
int SelectAbleConect::ConectList(conects *clients,int maxparalel){
  for(int i=0;i<maxparalel;++i)
  {
    clients[i].adress=NULL;
    clients[i].function=&voidfunction;
    clients[i].socket=0;
  }
  _clients=clients;
  _maxparalel=maxparalel;
  return maxparalel;
}
int SelectAbleConect::getdata(conects& name,char * buffer){
  int res;
  signal(SIGPIPE, SIG_IGN);
  res=name.socket;
//if(0==(res=recv(name.socket,buffer, 65536, 0)))buffer[0]=0;
if(0==(res=recvfrom(name.socket,buffer, 65536, 0, (struct sockaddr *)
&name.adress,&name.sockaddr_in_len)))buffer[0]=0;
return res;
}
void  SelectAbleConect::service(){
  char *buffer;
  int max=0;
  fd_set rfds;//for name socket;
  struct timeval tv;
  FD_ZERO(&rfds);
  for(int i=0;i<_maxparalel;++i)
    if(_clients[i].adress!=NULL){
       if (max<_clients[i].socket)max=_clients[i].socket;
       FD_SET(_clients[i].socket,&rfds);
	}
     tv.tv_sec = 1;
     tv.tv_usec = 0;
    if(int res =select(max+1, &rfds, NULL, NULL, &tv)>0){
      buffer=new char[65536];
	   for(int i=0;i<_maxparalel;++i)
	    if(_clients[i].adress!=NULL){
	      if(getdata(_clients[i],buffer)>0){
      		_clients[i].function(buffer);
	         senddata(_clients[i],buffer);
      		};
	      };
      delete buffer;
	 };
  FD_ZERO(&rfds);
  if(lostclient==0)exit(0);
}
void  SelectAbleConect::work(){
//клиентская программа если требуется передать данные при получение пустой строки должна составить запрос
//и вернуть его в параметрах клиенская программа сама следит по параметрам за обменом данными 
//и если хочет что либо паслать возвращает не нулевой результат
//clientskaya programma esli trebuetcya pereslat danua na poluchenie 0-stoki dolgna sostaviti
//zapros i pri poluchenii ne pustoy obrabotat esli otvet ne poluchen novogo ne sozdeavat
  char *buffer;
  int max=0;
  fd_set wfds;//for name socket;
  struct timeval tv;
  FD_ZERO(&wfds);
  for(int i=0;i<_maxparalel;++i)
    if(_clients[i].adress!=NULL){
       if (max<_clients[i].socket)max=_clients[i].socket;
       FD_SET(_clients[i].socket,&wfds);
	}
     tv.tv_sec = 1;
     tv.tv_usec = 0;
    if(int res =select(max+1,NULL,&wfds, NULL, &tv)>0){
      buffer=new char[65536];
	   for(int i=0;i<_maxparalel;++i)
	    if(_clients[i].adress!=NULL){
              buffer[0]=0;
	      if(_clients[i].function(buffer)>0)
	       {
	      	senddata(_clients[i],buffer);
	        if(getdata(_clients[i],buffer)>0)_clients[i].function(buffer);
	       }
      	   };
      delete buffer;
  };
  FD_ZERO(&wfds);
  if(lostclient==0)exit(0);
}


