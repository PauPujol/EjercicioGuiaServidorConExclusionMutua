#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <ctype.h>
#include <pthread.h>

int contador;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *AtenderCliente (void *socket)
{
	int sock_conn;
	int *s;
	s = (int *) socket;
	sock_conn = *s;
	
	char peticion[512];
	char respuesta[512];
	int ret;
	
	int terminar = 0;
	while (terminar == 0)
	{
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		peticion[ret]='\0';
		printf ("Peticion: %s\n",peticion);
		
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p);
		char nombre[20];
		if ((codigo !=0)&&(codigo !=6))
		{
			p = strtok( NULL, "/");
			strcpy (nombre, p);
			printf ("Codigo: %d, Nombre: %s\n", codigo, nombre);
		}
		if (codigo ==0) //peticion de desconexion
			terminar=1;
		else if (codigo == 6)
			sprintf (respuesta,"%d",contador);
		else if (codigo ==1) //piden la longitd del nombre
			sprintf (respuesta,"%d",strlen (nombre));
		else if (codigo ==2)
			// quieren saber si el nombre es bonito
			if((nombre[0]=='M') || (nombre[0]=='S'))
			strcpy (respuesta,"SI");
			else
				strcpy (respuesta,"NO");
			
			else if (codigo == 4)
			{
				// quire saber si su nombre es palindromo 
				char copia[20];
				for (int i = (strlen(nombre) -1); i >= 0; i--)
				{
					copia[strlen(nombre) -1 -i] = nombre [i]; 
				}
				copia [strlen(nombre)] = '\0';
				if (strcmp(nombre,copia)==0)
					strcpy (respuesta,"Es palindromo");
				else 
					strcpy (respuesta, "No es palindromo");
			}
			else if (codigo == 5)
			{
				for (int i = 0; nombre[i] != '\0'; i++)
				{
					nombre [i] = toupper (nombre[i]);
				}
				strcpy(respuesta,nombre);
			}
			else //quiere saber si es alto
			{
				p = strtok( NULL, "/");
				float altura =  atof (p);
				if (altura > 1.70)
					sprintf (respuesta, "%s: eres alto",nombre);
				else
					sprintf (respuesta, "%s: eresbajo",nombre);
			}
			if (codigo !=0)
			{
				
				printf ("Respuesta: %s\n", respuesta);
				// Enviamos respuesta
				write (sock_conn,respuesta, strlen(respuesta));
			}
			if ((codigo == 1)||(codigo == 2)||(codigo == 3)||(codigo == 4)||(codigo == 5))
			{
				pthread_mutex_lock( &mutex );
				contador ++;
				pthread_mutex_unlock( &mutex );
			}
	}
	// Se acabo el servicio para este cliente
	close(sock_conn); 	
}

int main(int argc, char *argv[])
{
	
	int sock_conn, sock_listen;
	struct sockaddr_in serv_adr;
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la maquina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	// establecemos el puerto de escucha
	serv_adr.sin_port = htons(9070);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	
	if (listen(sock_listen, 3) < 0)
		printf("Error en el Listen");
	
	contador = 0;
	int i;
	int sockets[100];
	pthread_t thread[100];
	// Bucle infinito
	for (i = 0; i<5; i++)
	{
		printf ("Escuchando\n");
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		sockets[i] = sock_conn;
		pthread_create (&thread[i], NULL, AtenderCliente,&sockets[i]);
	}
}


