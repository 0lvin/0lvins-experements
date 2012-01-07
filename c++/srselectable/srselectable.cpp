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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <cstdlib>
#define maxparalel 10
#include <iostream>
#include <stdlib.h>
#include "conects.h"
#include "initconect.h"
#include "exeption.h"
#include "selectableconect.h"
///////////////////
int voidfunctionsr(char * param){
if(param!=NULL)
  printf("%s",param);
  strcpy(param,"all right i can talk you!");
  return 1;
 }
//////////////////
int main(int argc, char *argv[])
{
int numclients=0;
conects clients[maxparalel];
using namespace std;
  cout << "Hello, World!It is " <<argv[0]<<"version:"<<
#ifdef HAVE_CONFIG_H
  VERSION<<
#endif
"Have "<<argc<<"lib.version"<<ver<<endl;
try{
InitConect conect;
conect.NewServer(65535,maxparalel);
for(;;){
SelectAbleConect selecta;
selecta.ConectList((conects*)&clients,maxparalel);//пустые обработчики
for(int i=0;i<maxparalel;){
clients[i].function=&voidfunctionsr;//замена на реальный обработчик
i=i+conect.WaitnewClient(clients[i]);
selecta.service(); //<<other process may be
}
numclients+=10;
printf("\nstart new serwer ....");
if(fork()!=0){
	printf("     done  \n");
	for(;;)selecta.service();
}
else
  printf("\nold conect not dead.Work %d clients\n ",numclients);
} 
}catch(Exeption exe)
{
cout<<"\nerror number="<<exe.getexep()<<endl;
}
   return EXIT_SUCCESS;
}
