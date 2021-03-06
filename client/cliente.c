/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: cliente.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Cliente de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <winsock.h>
#include <time.h>
#include <conio.h>

#include "protocol.h"





int main(int *argc, char *argv[])
{
	SOCKET sockfd;
	struct sockaddr_in server_in;
	char buffer_in[1024], buffer_out[1024],input[1024];
	int recibidos=0,enviados=0;
	int estado=S_HELO;
	char option;
	//new variables added for the SUM functionality
	char cnum1[5] = "", cnum2[5] = "";
	int strcnum = 0;	//variable used for counting the number of figures of the number inserted by the user
	//new variables addad for the ECHO functionality
	char secho[1017] = "";	//char vector(string) for the ECHO functionality

	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

    char ipdest[16];
	char default_ip[16]="127.0.0.1";
	
	//Inicialización Windows sockets
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1)
	{
		WSACleanup();
		return(0);
	}
	//Fin: Inicialización Windows sockets

	do{
		sockfd=socket(AF_INET,SOCK_STREAM,0);

		if(sockfd==INVALID_SOCKET)
		{
			printf("CLIENTE> ERROR AL CREAR SOCKET\r\n");
			exit(-1);
		}
		else
		{
			printf("CLIENTE> SOCKET CREADO CORRECTAMENTE\r\n");

		
			printf("CLIENTE> Introduzca la IP destino (pulsar enter para IP por defecto): ");
			gets(ipdest);

			if(strcmp(ipdest,"")==0)
				strcpy(ipdest,default_ip);


			server_in.sin_family=AF_INET;
			server_in.sin_port=htons(TCP_SERVICE_PORT);
			server_in.sin_addr.s_addr=inet_addr(ipdest);
			
			estado=S_HELO;
		
			// establece la conexion de transporte
			if(connect(sockfd,(struct sockaddr*)&server_in,sizeof(server_in))==0)
			{
				printf("CLIENTE> CONEXION ESTABLECIDA CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			
		
				//Inicio de la máquina de estados
				do{
					switch(estado)
					{
					case S_HELO:
						// Se recibe el mensaje de bienvenida
						break;
					case S_USER:
						// establece la conexion de aplicacion 
						printf("CLIENTE> Introduzca el usuario (enter para salir): ");
						gets(input);
						if(strlen(input)==0)
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else

						sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",SC,input,CRLF);
						break;
					case S_PASS:
						printf("CLIENTE> Introduzca la clave (enter para salir): ");
						gets(input);
						if(strlen(input)==0)
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s %s%s",PW,input,CRLF);
						break;
					case S_DATA:
						printf("CLIENTE> Introduzca datos (enter o QUIT para salir): ");
						gets(input);
						if(strlen(input)==0)
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",SD,CRLF);
							estado=S_QUIT;
						}
						//NEW CODE ADDED from here for SUM functionality
						else if(strcmp(input, SUM) == 0)
						{
							printf("CLIENTE> Please, insert the first number(4 figures or less): ");
							gets(cnum1);
							printf("CLIENTE> Please, insert the second number(4 figures or less): ");
							gets(cnum2);
							//creating the string with the following format: SUM SP NUM1 SP NUM2 CRLF
							//it will be send to the server side and it will process it for returning
							//OK SP SUMA CRLF	where SUMA is NUM1+NUM2(5-digit number)
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s%s",SUM, cnum1, cnum2, CRLF);
						//END OF NEW CODE
						}
						//NEW CODE ADDED FOR THE ECHO FUNCTIONALITY
						else if(strcmp(input, ECHO) == 0)
						{
							printf("CLIENTE> Please, insert a message for ECHO: ");
							gets(secho);
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s", ECHO, secho, CRLF);
						}
						else if(strcmp(input, SD) == 0)
						{
							printf("CLIENTE> CLOSING CONNECTION:%s", CRLF);
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s", input, CRLF);
							estado = S_QUIT;
						}
						else
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s",input, CRLF);
						break;
				 
				
					}
					//Envio
					if(estado!=S_HELO)
					{
					// Ejercicio: Comprobar el estado de envio
						enviados=send(sockfd,buffer_out,(int)strlen(buffer_out),0);
						if(enviados <= 0)
						{
							DWORD error = GetLastError();
							//-7 value chosen for send error code
							//return(-7);
							if(enviados < 0)
							{
								printf("CLIENTE> FAIL SENDING DATA TO THE SERVER: %d%s", error, CRLF);
								estado=S_QUIT;
							}
							else
							{
								printf("CLIENTE> END CONEXION WITH THE SERVER%s", CRLF);
								estado=S_QUIT;
							}
						}
					}
					//Recibo
					recibidos=recv(sockfd,buffer_in,512,0);
					if(recibidos<=0)
					{
						DWORD error=GetLastError();
						if(recibidos<0)
						{
							printf("CLIENTE> Error %d en la recepción de datos\r\n",error);
							estado=S_QUIT;
						}
						else
						{
							printf("CLIENTE> Conexión con el servidor cerrada\r\n");
							estado=S_QUIT;
						}
					}else
					{
						buffer_in[recibidos]=0x00;
						printf(buffer_in);
						if(estado!=S_DATA && strncmp(buffer_in,OK,2)==0) 
							estado++;  
					}
				}while(estado!=S_QUIT);
				
	
		
			}
			else
			{
				printf("CLIENTE> ERROR AL CONECTAR CON %s:%d\r\n",ipdest,TCP_SERVICE_PORT);
			}		
			// fin de la conexion de transporte
			closesocket(sockfd);
			
		}	
		printf("-----------------------\r\n\r\nCLIENTE> Volver a conectar (S/N)\r\n");
		option=_getche();

	}while(option!='n' && option!='N');

	
	
	return(0);

}
