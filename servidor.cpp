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
#include <vector>
#include <fstream> // ifstream

//#include "json.hpp" // json

using namespace std;
//using json = nlohmann::json;

pthread_mutex_t mutex; // prevent race condition on critical sections.

string books_list; // initialized when reading the file with the list
int books_list_size;

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

class Books{
public:
	string book_name;
	int downloads_num;
	vector<string> client_book_list; 	
};

class Statistic{
public:
	vector<Books> downloaded_books;
	vector<string> client_list;
	
};

/**
   method that shows error message and exit the program when an error occurs.

   @param message -> error message
*/
void error(const char *message){
	perror(message);
	exit(EXIT_FAILURE);
}

void send_list(int socket){
	char list[1024];
	memcpy(&list, books_list.c_str(), sizeof(list));
	write(socket, list, sizeof(list));	
}

void send_book(int socket, std::string book){
	
	// open file and go through it
	FILE* fp = fopen(book.c_str(), "r");
	fseek(fp, 0, SEEK_END);
	
	// get file size
	long file_size = ftell(fp);
	char got_file_size[1024];
	char book_name[1024];
	
	// send file size
	memcpy(&got_file_size, std::to_string(file_size).c_str(), 1024);
	write(socket, got_file_size, 1024);

	// go back to the beginning of the book
	rewind(fp);
	
	// send file name
	memcpy(&book_name, book.c_str(), sizeof(book_name));
	write(socket, book_name, sizeof(book_name));

	// start control size check
	long size_check = 0;
	char* mfcc;

	// file can't be sent in only one a TCP packet 
	if (file_size > 1499){
		mfcc = (char *) malloc(1500);

		// still data to send. read from file and set that data.
		while(size_check < file_size){
			int read = fread(mfcc, 1500, sizeof(char), fp);
			int sent = write(socket, mfcc, read);
			size_check += sent;
		}
	}

	// file can be send in one tcp packet
	else{
		mfcc = (char *) malloc(file_size + 1);
		fread(mfcc, file_size, sizeof(char), fp);
		write(socket, mfcc, file_size);
	}

	// file has been sent.
	fclose(fp);
	free(mfcc);
	
	std::cout << "[INFORMACIÓN] El archivo " << book << " ha sido enviado." << std::endl;
}

/**
   method executed by thread

   @param conn_thread 
*/
void* handle_client(void *conn_thread){
	
	ConnectionThread *ct = (ConnectionThread *) conn_thread;
	int new_socketfd = ct->get_socket();
	int data_len, command, command_solicitud;
	string book;
	
	while((data_len = read(new_socketfd, &command, sizeof(command))) > 0){
			
		switch(command){
			case 0:
				read(new_socketfd, &command_solicitud, sizeof(command_solicitud));
				break;
			case 1:
				send_list(new_socketfd);
				break;
			case 2:
				read(new_socketfd, &book, sizeof(book));
				send_book(new_socketfd, book);
				break;
			case 3:
				break;
			case 4:
				close(new_socketfd);
				pthread_exit(NULL);
				break;
		}	
	}
}

void server_help(){
  std::cout << "> LIBROS_DESCARGADOS: muestra una lista de libros que han sido descargados por el cliente.\n";
  std::cout << "> CLIENTE_QUE_CONSULTARON: muestra lista de clientes que han descargado libro.\n";
  std::cout << "> NUM_DESCARGASxLIBROxCLIENTE: muestra el numero de descargas de un libro por cliente.\n";
  std::cout << "> SALIR: termina la sesión\n";
  std::cout << "----------------------------------------------------------------------------------\n";
}

void *show_statistics(void *unused){
	std::string command;
	std:: string statistics_command;
	int command_num, data_size;

	std::cout << "Bienvenido" << std::endl;
	std::cout << "Escriba AYUDA si desea conocer la lista de comandos disponibles" << std::endl;

	while(true){
	std::cout << "Escriba el comando que desea ejecutar: ";
    std::cin >> command;

	    if(command == "AYUDA") server_help();
	    else if (command == "LIBROS_DESCARGADOS"){
	    	printf("%s \n", "LIBROS_DESCARGADOS");
	    }
	   
	    else if (command == "CLIENTE_QUE_CONSULTARON"){
	  		printf("%s \n", "CLIENTE_QUE_CONSULTARON");
		}

		else if (command == "NUM_DESCARGASxLIBROxCLIENTE"){
			printf("%s \n", "NUM_DESCARGASxLIBROxCLIENTE");
		}

		else if (command == "SALIR"){
	      exit(1);
	    }
	    else std::cout << "Comando inválido. Ejecute AYUDA si desea conocer la lista de comandos.\n";
	}
}

/**
   initializes local server listening on port_number

   @param port_number
*/
void server(int port_number){
	int socketfd; // socket file descriptor
	int new_socketfd; // new connection socket file descriptor
	int rc_1; // pthread creation return value
	int rc_2; // pthread creation return value
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

	std::cout << "Servidor corriendo en el puerto " << port_number << std::endl;
	
	/**
	    Server should create a thread to handle either incomming connections or
	    console inputs.
	*/
	pthread_t command_thread;
	rc_2 = pthread_create(&command_thread, NULL, show_statistics,NULL);

	// server is ready to wait for connections.
	while(true){
			
		// waiting for new connection
		if((new_socketfd = accept(socketfd,(struct sockaddr *)&client_addr,&client_addr_len)) < 0 )
			error("Error aceptando la conexion");

		// connection successfully accepted 
		std::cout 
		<< "\n[CONEXIÓN] IP: " << inet_ntoa(client_addr.sin_addr) 
		<< " - PUERTO: " << ntohs(client_addr.sin_port) << std::endl;

		// a new thread has to be created to handle the new connection
		pthread_t conn_thread;

		ConnectionThread *ct = new ConnectionThread(port_number, new_socketfd);

		rc_1 = pthread_create(&conn_thread, NULL, handle_client, ct);

		if (rc_1) error("Error al crear hilo para manejar la conexión");

	}

}

int read_file(char* file){
	// try to open book list text file
	ifstream book_list_stream(file);
	if(!book_list_stream.is_open()){
		cout << "No se pudo abrir el archivo " << file << endl;
		exit(-1);
	}

	string line;
	// dump into a big string
	while(getline(book_list_stream, line))
		books_list = books_list + line + "\n";

	books_list_size = sizeof(books_list);

	return 1;
}

int main(int argc, char *argv[]){

	if (argc < 3){
		cout << "usage: server port book_list_file.txt\n";
		exit(-1);
	}

	read_file(argv[2]);

	server(atoi(argv[1]));

	return 0;
}