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
#include <stdlib.h> // EXIT_FAILURE, atoi
#include <unistd.h> // close
#include <iostream> // cout
#include <arpa/inet.h>  // sockaddr_in, inet_ntoa
#include <string.h> // memset


using namespace std;

pthread_mutex_t mutex; // prevent race condition on critical sections.

class ConnectionThread {
	int port;
	int new_socketfd;

public:
	ConnectionThread(int, int);

	int get_socket(){
		return this->new_socketfd;
	}

};

ConnectionThread::ConnectionThread(int port_number, int socket){
	port = port_number;
	new_socketfd = socket;
}

/**
   method that shows error message and exit the program when an error occurs.

   @param message -> error message
*/
void error(const char *message){
	perror(message);
	exit(EXIT_FAILURE);
}

void send_list(int socketfd){
	std::string list = "Hola cliente :)\n";
	write(socketfd, &list, sizeof(list));
}

/**
   method executed by thread

   @param conn_thread 
*/
void* handle_client(void *conn_thread){
	
	ConnectionThread *ct = (ConnectionThread *) conn_thread;
	int new_socketfd = ct->get_socket();
	int data_len;
	
	int command;
	string command_solicitud;

	cout << new_socketfd << endl;
	data_len = read(new_socketfd, &command, sizeof(command));
	printf("Lei algo\n");
	cout << data_len << endl;
	while(data_len > 0){
		cout << "recibi algo" << endl;
		switch(command){
			case 0:
				read(new_socketfd, &command_solicitud, sizeof(command_solicitud));
				break;
			case 1:
				send_list(new_socketfd);
				break;
			case 2:
				break;
			case 3:
				break;
		}
	
	}

}

/**
   initializes local server listening on port_number

   @param port_number
*/
void server(int port_number){
	int socketfd; // socket file descriptor
	int new_socketfd; // new connection socket file descriptor
	int rc; // pthread creation return value
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len = sizeof(struct sockaddr_in);

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

   // set bytes of struct to zero
   memset(&server_addr, 0, sizeof(server_addr));

	// byte order to use -> IPV4
	server_addr.sin_family = AF_INET;

	// port must be converted from integer value into network byte order
	server_addr.sin_port = htons(port_number);

	// bind the server to the localhost
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// time to bind. bind associates the socket with a port in the local machine
	if ( bind(socketfd,(struct sockaddr *)&server_addr, client_addr_len) < 0){
		error("Error al enlazar el puerto a la conexión (bind)");
	}

	// listen to incoming connections. backlog queue size = 5
	if(listen(socketfd, 5) == -1)
		error("Error al escuchar conexiones entrantes.");

	std::cout << "Servidor corriendo en el puerto " << port_number << "\n";
	
	// server is ready and waiting for connections.
	while(true){
			
		// waiting for new connection
		if((new_socketfd = accept(socketfd,(struct sockaddr *)&client_addr,&client_addr_len)) < 0 )
			error("Error aceptando la conexion");

		// connection successfully accepted 
		std::cout 
		<< "[CONEXIÓN] IP: " << inet_ntoa(client_addr.sin_addr) 
		<< " - PUERTO: " << ntohs(client_addr.sin_port) << std::endl;

		// a new thread has to be created to handle the new connection
		pthread_t conn_thread;

		ConnectionThread *ct = new ConnectionThread(port_number, new_socketfd);

		rc = pthread_create(&conn_thread, NULL, handle_client, ct);

		if (rc) error("Error al crear hilo para manejar la conexión");

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