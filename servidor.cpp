/**
   Universidad Simón Bolívar
   Departamento de Computación y Tecnología de la Información
   CI4835: Redes I

   Aplicación servidor mediante sockets.

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
#include <unistd.h> // close
#include <iostream> // cout
#include <arpa/inet.h>  // inet_ntoa

using namespace std;

pthread_mutex_t mutex; // prevent race condition on critical sections.

class ConnectionThread {
	int port;
public:
	ConnectionThread(int);	
};

ConnectionThread::ConnectionThread(int port_number){
	port = port_number;
}

/**
   method that shows error message and exit the program when an error occurs.

   @param message -> error message
*/
void error(const char *message){
	perror(message);
	exit(EXIT_FAILURE);
}

void *handle_client(void *conn_thread){
	std::cout << "Bienvenido \n";
}

void server(int port_number){
	int socketfd; // socket file descriptor
	int new_socketfd; // new connection socket file descriptor
	int rc;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;
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

	// time to bind. bind associates the socket with a port in the local machine
	if ( bind(socketfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0){
		error("Error al enlazar el puerto a la conexión (bind)");
		close(socketfd);
	}

	// listen to incoming connections. backlog queue size = 5
	if(listen(socketfd, 5))
		error("Error al escuchar conexiones entrantes.");

	// server is ready and waiting for connections.
	while(true){
		
		std::cout << "Servidor corriendo en el puerto: " << port_number << "\n";
		
		// waiting for new connection
		if( new_socketfd = accept(socketfd,(struct sockaddr *)&client_addr,&client_addr_len) < 0 )
			error("Error aceptando la conexion");

		// connection successfully accepted 
		std::cout 
		<< "Nuevo cliente conectado desde el puerto " << ntohs(client_addr.sin_port) 
		<< " y direccion IP " << inet_ntoa(client_addr.sin_addr);

		// a new thread has to be created to handle the new connection
		pthread_t conn_thread;

		ConnectionThread *ct = new ConnectionThread(port_number);

		rc = pthread_create(&conn_thread, NULL, handle_client, ct);

		if (rc)
			error("Error creando hilo para manejar la conexión");

	}

}

int main(int argc, char *argv[]){

	if (argc < 2){
		cout << "usage: server port\n";
		exit(-1);
	}

	server(atoi(argv[1]));

	return 0;
}