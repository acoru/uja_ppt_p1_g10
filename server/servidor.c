/*******************************************************
Protocolos de Transporte
Grado en Ingeniería Telemática
Dpto. Ingeníería de Telecomunicación
Univerisdad de Jaén

Fichero: servidor.c
Versión: 1.0
Fecha: 23/09/2012
Descripción:
	Servidor de eco sencillo TCP.

Autor: Juan Carlos Cuevas Martínez

*******************************************************/
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <Winsock2.h>

#include "protocol.h"




main()
{

	WORD wVersionRequested;
	WSADATA wsaData;
	SOCKET sockfd,nuevosockfd;
	struct sockaddr_in  local_addr,remote_addr;
	char buffer_out[1024],buffer_in[1024], cmd[10], usr[10], pas[10];
	int err,tamanio;
	int fin=0, fin_conexion=0;
	int recibidos=0,enviados=0;
	int estado=0;
	//NEW VARIABLES FOR SUM FROM HERE
	//here is where the numbers will be saved before the SUM
	int num1 = 0, num2 = 0;
	int suma = 0;
	//END OFF NEW VARIABLES
	//NEW VARIABLES FOR ECHO
	char cECHO[5] = "";
	//END OF NEW VARIABLES

	/** INICIALIZACION DE BIBLIOTECA WINSOCK2 **
	 ** OJO!: SOLO WINDOWS                    **/
	wVersionRequested=MAKEWORD(1,1);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0){
		return(-1);
	}
	if(LOBYTE(wsaData.wVersion)!=1||HIBYTE(wsaData.wVersion)!=1){
		WSACleanup() ;
		return(-2);
	}
	/** FIN INICIALIZACION DE BIBLIOTECA WINSOCK2 **/


	sockfd=socket(AF_INET,SOCK_STREAM,0);//Creación del socket

	if(sockfd==INVALID_SOCKET)	{
		return(-3);
	}
	else {
		local_addr.sin_family		=AF_INET;			// Familia de protocolos de Internet
		local_addr.sin_port			=htons(TCP_SERVICE_PORT);	// Puerto del servidor
		local_addr.sin_addr.s_addr	=htonl(INADDR_ANY);	// Direccion IP del servidor Any cualquier disponible
													// Cambiar para que conincida con la del host
	}
	
	// Enlace el socket a la direccion local (IP y puerto)
	if(bind(sockfd,(struct sockaddr*)&local_addr,sizeof(local_addr))<0)
		return(-4);
	
	//Se prepara el socket para recibir conexiones y se establece el tamaño de cola de espera
	if(listen(sockfd,5)!=0)
		return (-6);
	
	tamanio=sizeof(remote_addr);

	do
	{
		printf ("SERVIDOR> ESPERANDO NUEVA CONEXION DE TRANSPORTE\r\n");
		
		nuevosockfd=accept(sockfd,(struct sockaddr*)&remote_addr,&tamanio);

		if(nuevosockfd==INVALID_SOCKET) {
			
			return(-5);
		}

		printf ("SERVIDOR> CLIENTE CONECTADO\r\nSERVIDOR [IP CLIENTE]> %s\r\nSERVIDOR [CLIENTE PUERTO TCP]>%d\r\n",
					inet_ntoa(remote_addr.sin_addr),ntohs(remote_addr.sin_port));

		//Mensaje de Bienvenida
		sprintf_s (buffer_out, sizeof(buffer_out), "%s Bienvenindo al servidor de ECO%s",OK,CRLF);
		
		enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);
		//TODO Comprobar error de envío
		if(enviados <= 0)
		{
			DWORD error = GetLastError();
			if(enviados < 0)
			{
				//-7 value chosen for send error code
				//return(-7);
				printf("SERVER> ERROR SENDING DATA %d%s", error, CRLF);
				//provisional solution for error sending welcome message to the client, just close the socket
				fin_conexion = 1;
			}
			else
			{
				fin_conexion = 1;
			}
		}
		//Se reestablece el estado inicial
		estado = S_USER;
		fin_conexion = 0;

		printf ("SERVIDOR> Esperando conexion de aplicacion\r\n");
		do
		{
			//Se espera un comando del cliente
			recibidos = recv(nuevosockfd,buffer_in,1023,0);
			//TODO Comprobar posible error de recepción
			if(recibidos <= 0)
			{
				//if an error occurs, the server will close the socket, if recieves 0 the server will close the socket
				if(recibidos < 0)
				{
					DWORD error = GetLastError();
					//-8 value chosen for recv(reception) error code
					//return(-8);
					printf("SERVER> ERROR ON RECEPTION%d%s", error, CRLF);
					//provisional solution for reciving error from client, just close the socket
					fin_conexion = 1;
				}
				else
				{
					fin_conexion = 1;
				}
			}
			else
			{
				buffer_in[recibidos] = 0x00;
				printf ("SERVIDOR [bytes recibidos]> %d\r\nSERVIDOR [datos recibidos]>%s", recibidos, buffer_in);
			
				switch (estado)
				{
					case S_USER:    /*****************************************/
						strncpy_s ( cmd, sizeof(cmd),buffer_in, 4);
						cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000

						if ( strcmp(cmd,SC)==0 ) // si recibido es solicitud de conexion de aplicacion
						{
							sscanf_s (buffer_in,"USER %s\r\n",usr,sizeof(usr));
						
							// envia OK acepta todos los usuarios hasta que tenga la clave
							sprintf_s (buffer_out, sizeof(buffer_out), "%s%s", OK,CRLF);
						
							estado = S_PASS;
							printf ("SERVIDOR> Esperando clave\r\n");
						} else
						if ( strcmp(cmd,SD)==0 )
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
							fin_conexion=1;
						}
						else
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
						}
					break;

					case S_PASS: /******************************************************/

					
						strncpy_s ( cmd, sizeof(cmd), buffer_in, 4);
						cmd[4]=0x00; // en C los arrays finalizan con el byte 0000 0000

						if ( strcmp(cmd,PW)==0 ) // si comando recibido es password
						{
							sscanf_s (buffer_in,"PASS %s\r\n",pas,sizeof(usr));

							if ( (strcmp(usr,USER)==0) && (strcmp(pas,PASSWORD)==0) ) // si password recibido es correcto
							{
								// envia aceptacion de la conexion de aplicacion, nombre de usuario y
								// la direccion IP desde donde se ha conectado
								sprintf_s (buffer_out, sizeof(buffer_out), "%s %s IP(%s)%s", OK, usr, inet_ntoa(remote_addr.sin_addr),CRLF);
								estado = S_DATA;
								printf ("SERVIDOR> Esperando comando\r\n");
							}
							else
							{
								sprintf_s (buffer_out, sizeof(buffer_out), "%s Autenticación errónea%s",ER,CRLF);
							}
						} else
						if ( strcmp(cmd,SD)==0 )
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
							fin_conexion=1;
						}
						else
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
						}
					break;

					case S_DATA: /***********************************************************/
					
						buffer_in[recibidos] = 0x00;
					
						strncpy_s(cmd,sizeof(cmd), buffer_in, 4);

						printf ("SERVIDOR [Comando]>%s\r\n",cmd);
					
						if ( strcmp(cmd,SD)==0 )
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Fin de la conexión%s", OK,CRLF);
							fin_conexion=1;
						}
						else if (strcmp(cmd,SD2)==0)
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Finalizando servidor%s", OK,CRLF);
							fin_conexion=1;
							fin=1;
						}
						//NEW CODE ADDED from here for SUM
						else if(strcmp(cmd, "SUM ") == 0)
						{
							sscanf_s(buffer_in, "SUM %d %d\r\n", &num1, &num2);
							if(num1 > 0 && num1 < 9999 && num2 > 0 && num2 < 9999)
							{
								suma = num1 + num2;
								printf("%d", suma);
								sprintf_s(buffer_out, sizeof(buffer_out), "%s %d%s", OK, suma, CRLF);
							}
							else
							{
								sprintf_s(buffer_out, sizeof(buffer_out), "%s%s", ER, CRLF);
							}
						}
						//END OF NEW CODE
						//NEW CODE ADDED FOR ECHO FUNCTIONALITY
						else if(strcmp(cmd, ECHO) == 0)
						{
							sprintf_s(buffer_out, sizeof(buffer_out), "%s %s %s", OK, buffer_in + 5, CRLF);
						}
						//END OF NEW CODE
						else
						{
							sprintf_s (buffer_out, sizeof(buffer_out), "%s Comando incorrecto%s",ER,CRLF);
						}
						break;

					default:
						break;
					
				} // switch

				enviados=send(nuevosockfd,buffer_out,(int)strlen(buffer_out),0);
				//TODO 
				if(enviados <= 0)
				{
					if(enviados < 0)
					{
						DWORD error = GetLastError();
						//if an error occurs sending data from the state machine, it will return as error code "-9"
						//return(-9);
						printf("SERVER> ERROR SENDING DATA: %d%s", error, CRLF);
						//provisional solution for sending error, just close the socket created for the current client conection
						fin_conexion = 1;
					}
					else
					{
						fin_conexion = 1;
					}
				}
			}

		} while (!fin_conexion);
		printf ("SERVIDOR> CERRANDO CONEXION DE TRANSPORTE\r\n");
		shutdown(nuevosockfd,SD_SEND);
		closesocket(nuevosockfd);

	}while(!fin);

	printf ("SERVIDOR> CERRANDO SERVIDOR\r\n");

	return(0);
} 
