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

int main(int argc, char *argv[]){

	int socketfd; // socket file descriptor
	int new_socket; // new connection socket file descriptor.

	// create socket with params domain: IPv4 (AF_INET), type: TCP (SOCK_STREAM), 
	// protocol: IP (0)
	socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (socketfd < 0){
		perror("Error al crear el socket");
		exit(EXIT_FAILURE);
	}

	return 0;
}