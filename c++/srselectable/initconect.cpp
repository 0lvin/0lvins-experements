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

#include "initconect.h"
#ifdef EXEPCPP 
#include  "exeption.h"
#endif
InitConect::InitConect(){
}
void InitConect::NewServer(int port,int maxclient){
  char local_name[80];
  gethostname(local_name,60);
  opensoket(port,local_name,maxclient);
}
int InitConect::CreateClient(conects& arg,int port ,int maxparalel,char * local_name)
{
  openconect(port,local_name,maxparalel);
  char  buffer[BUFFERSIZE];
  arg.adress=new(struct  sockaddr);
  memcpy(arg.adress,&addrclient,sizeof(struct sockaddr));
  arg.sockaddr_in_len=sizeof(struct sockaddr);
  arg.socket=cs;
  return 1;
};
int InitConect::WaitnewClient(conects& arg){
char  buffer[BUFFERSIZE];
  if(error=recivesocet(buffer)){
     return 0;
     }
     else
     {
     if(cs==-1){
     return 0;}     
     if (error<0)exit(1);
     }
  arg.adress=new(struct  sockaddr);
  memcpy(arg.adress,&addrclient,sizeof(struct sockaddr));
  arg.sockaddr_in_len=sizeof(struct sockaddr);
  arg.socket=cs;
  return 1;
  }
InitConect::~InitConect(){
}
int InitConect::openconect(int port,char * local_name,int maxclient)
{
   char message[] = "Hello world!"; // отправляемое сообщение
   int sd = makesocet(local_name, port);
   if (sd == -1) {
 #ifdef EXEPCPP 
 throw Exeption("Не могу открыть сокет клиента",1); 
 #else
      perror("Не могу открыть сокет клиента");
      return -1;
 #endif     
   }
   return sd;
};
int InitConect::opensoket(int port,char * local_name,int maxclient)
{
int options;
  if((sd=socket( PF_INET, SOCK_STREAM, 0))==-1)
  {perror("can't creat socet");
   return -1;
   }
   struct hostent *hp;
	if ((hp= gethostbyname(local_name)) == 0) return -1;
	memset(&saddr, 0, sizeof(saddr));
	memcpy(hp->h_addr, &saddr.sin_addr, hp->h_length);
	saddr.sin_family = AF_INET;
	saddr.sin_port =htons(port);
	if(bind(sd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in))==-1)
  {
    perror("can bind");
    return -1;
  }
 SRsoket=sd;
 options=O_NONBLOCK|fcntl(SRsoket,F_GETFL);
 error=fcntl(SRsoket, F_SETFL, options);///global error
 listen(SRsoket,maxclient);//lisen maxclient client
return sd;
}
int InitConect::makesocet(char *server_name,int port_num)
{
int options;
  struct hostent *hp; // данные о сервере подключения
   struct sockaddr_in addr; // данные подключения

   // транcлируем доменное имя в адрес
   if ((hp = gethostbyname(server_name)) == 0) return -1;

   // заполнение данных подключения
   memset(&addr, 0, sizeof(addr));
   memcpy(hp->h_addr, &addr.sin_addr, hp->h_length); // данные
   addr.sin_family = hp->h_addrtype; // тип адреса
   addr.sin_port = htons(port_num); // порт подключения

   // создаем сокет
   if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;

   // подключаемся к серверу
   if (connect(sd, (struct sockaddr *)(&addr), sizeof(addr)) == -1) return -1;
   SRsoket=sd;
 options=O_NONBLOCK|fcntl(SRsoket,F_GETFL);
 error=fcntl(SRsoket, F_SETFL, options);///global error
   cs=sd;
   return sd;
}
int InitConect::recivesocet(char buffer[BUFFERSIZE])
{
  socklen_t addr_len = sizeof(&addrclient);
        memset(&addrclient, 0, sizeof(addrclient));
        if ((cs=accept(SRsoket,(struct sockaddr*)(&addrclient), &addr_len)) <0) {
          return -1;
        }
int	 options=O_NONBLOCK|fcntl(cs,F_GETFL);
	 error=fcntl(cs, F_SETFL, options);///global error
	return 0;//must not blocked
}
int InitConect::sendsoket(char buffer[BUFFERSIZE])
{
	  return 0;
}
int InitConect::closesoket()
{
    return  close(sd);
}

