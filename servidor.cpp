/**
   Universidad Simón Bolívar
   Departamento de Computación y Tecnología de la Información
   CI4835: Redes I

   Aplicación cliente/servidor mediante sockets.

   Comunicación de clientes con tres servidores que ofrecen libros en formato
   PDF. Consulta de oferta de libros de los servidores y solicitud de
   documentos.

   @author David Cabeza <13-10191@usb.ve>
   @author Fabiola Martínez <13-10838@usb.ve>

   Prof. Wilmer Pereira
   February, 2018.
*/
#include <pthread.h> // pthread_mutex_t
#include <netinet/in.h> // AF_INET
#include <stdio.h> // perror
#include <stdlib.h> // EXIT_FAILURE

pthread_mutex_t mutex; // prevent race condition on critical sections.

/**
   method that shows error message and exit the program when an error occurs.

   @param message -> error message
*/
void error(const char *message){
	perror(message);
	exit(EXIT_FAILURE);
}

void server(int port_number){
	int socketfd; // socket file descriptor
	int new_socket; // new connection socket file descriptor.
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;

	/**
	   create socket with params:
	     domain -> IPv4 (AF_INET)
	     type -> TCP (SOCK_STREAM)
	     protocol -> IP (0) 
	*/ 
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0)
		error("Error al crear el socket");

	/* server_addr structure must be set to use in bind method call */

	// byte order to use -> IPV4
	server_addr.sin_family = AF_INET;

	// bind the server to the localhost
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// port must be converted from integer value into network byte order
	server_addr.sin_port = htons(port_number);

}

int main(int argc, char *argv[]){

	server(65500);

	return 0;
}