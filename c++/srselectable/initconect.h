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
#ifndef INITCONECT_H
#define INITCONECT_H
#include "conects.h"

/**
  *@author root
  */

class InitConect {
  struct sockaddr_in addrclient;
 	struct sockaddr_in saddr;
  int SRsoket;/*main server socet*/
  int cs;/*cs - client socket*/
  int sd;
  int error;//for all error in class
  int opensoket(int port,char * local_name,int maxclient);//create socket
  int recivesocet(char buffer[BUFFERSIZE]); //get message
  int sendsoket(char buffer[BUFFERSIZE]);   //send message
  int closesoket();                         //close socket
  int openconect(int port,char * local_name,int maxclient);
  int makesocet(char *,int);
public:
	InitConect();
void NewServer(int port,int maxclient);//create new server
int  WaitnewClient(conects&);//wait new for conection
	~InitConect();
int CreateClient(conects&,int,int,char *);
};

#endif
