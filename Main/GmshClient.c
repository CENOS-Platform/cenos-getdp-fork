/* $Id: GmshClient.c,v 1.12 2005-01-01 01:51:33 geuzaine Exp $ */
/*
 * Copyright (C) 1997-2004 C. Geuzaine, J.-F. Remacle
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished
 * to do so, provided that the above copyright notice(s) and this
 * permission notice appear in all copies of the Software and that
 * both the above copyright notice(s) and this permission notice
 * appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS NOTICE BE LIABLE FOR
 * ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 * 
 *  Please report all bugs and problems to <gmsh@geuz.org>.
 *
 * Contributor(s):
 *   Christopher Stott
 */

#ifdef MSDOS /* pure DOS/Windows code, without cygwin */

int  Gmsh_Connect(char *sockname){ return -2;}
void Gmsh_SendString(int socket, int type, char str[]){;}
void Gmsh_Disconnect(int sock){;}

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _AIX
#include <strings.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

/* private functions */

static void Socket_SendData(int socket, void *buffer, int bytes)
{
  int sofar, remaining, len;
  char *buf;
  buf = (char *)buffer;
  sofar = 0;
  remaining = bytes;
  do {
    len = write(socket, buf + sofar, remaining);
    sofar += len;
    remaining -= len;
  } while(remaining > 0);
}

static long Socket_GetTime()
{
  struct timeval tp;
  gettimeofday(&tp, (struct timezone *)0);
  return (long)tp.tv_sec * 1000000 + (long)tp.tv_usec;
}

static void Socket_Idle(double delay)
{
  long t1 = Socket_GetTime();
  while(1) {
    if(Socket_GetTime() - t1 > 1.e6 * delay)
      break;
  }
}

/* public interface */

int Gmsh_Connect(char *sockname)
{
  struct sockaddr_un addr_un;
  struct sockaddr_in addr_in;
  int len, sock;
  int tries;
  struct hostent *server;
  int portno, remotelen;
  char remote[256], *port;

  /* slight delay to be sure that the socket is bound by the
     server before we attempt to connect to it... */
  Socket_Idle(0.1);

  if(strstr(sockname, "/") || strstr(sockname, "\\") || !strstr(sockname, ":")){
    /* UNIX socket (testing ":" is not enough with Windows paths) */
    portno = -1;
  }
  else{
    /* INET socket */
    port = strstr(sockname, ":");
    if(!port)
      return -1; /* Error: Couldn't create socket */
    portno = atoi(port+1);
    remotelen = strlen(sockname) - strlen(port);
    if(remotelen > 0)
      strncpy(remote, sockname, remotelen);
    remote[remotelen] = '\0';
  }

  /* create socket */

  if(portno < 0){
    sock = socket(PF_UNIX, SOCK_STREAM, 0);
    if(sock < 0)
      return -1; /* Error: Couldn't create socket */
    /* try to connect socket to given name */
    strcpy(addr_un.sun_path, sockname);
    addr_un.sun_family = AF_UNIX;
    len = strlen(addr_un.sun_path) + sizeof(addr_un.sun_family);
    for(tries = 0; tries < 5; tries++) {
      if(connect(sock, (struct sockaddr *)&addr_un, len) >= 0)
	return sock;
      Socket_Idle(0.1);
    }
  }
  else{
    /* try to connect socket to given name */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
      return -1; /* Error: Couldn't create socket */
    if(!(server = gethostbyname(remote)))
      return -3; /* Error: No such host */
    bzero((char *) &addr_in, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&addr_in.sin_addr.s_addr, server->h_length);
    addr_in.sin_port = htons(portno);
    addr_in.sin_addr.s_addr = INADDR_ANY;
    for(tries = 0; tries < 5; tries++) {
      if(connect(sock, (struct sockaddr *)&addr_in, sizeof(addr_in)) >= 0)
	return sock;
      Socket_Idle(0.1);
    }
  }

  return -2;    /* Error: Couldn't connect */
}

void Gmsh_SendString(int socket, int type, char str[])
{
  int len = strlen(str);
  Socket_SendData(socket, &type, sizeof(int));
  Socket_SendData(socket, &len, sizeof(int));
  Socket_SendData(socket, str, len);
}

void Gmsh_Disconnect(int sock)
{
  close(sock);
}

#endif
