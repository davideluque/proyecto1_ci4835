/**
   Universidad Simón Bolívar
   Departamento de Computación y Tecnología de la Información
   CI4835: Redes I

   Aplicación cliente mediante sockets.

   Comunicación de clientes con tres servidores que ofrecen libros en formato
   PDF. Consulta de oferta de libros de los servidores y solicitud de
   documentos.

   @author David Cabeza <13-10191@usb.ve>
   @author Fabiola Martínez <13-10838@usb.ve>

   Prof. Wilmer Pereira
   February, 2018.
*/

#include <stdlib.h> // atoi, EXIT_FAILURE 
#include <iostream> // cout
#include <stdio.h> // perror
#include <arpa/inet.h>  // sockaddr_in, inet_ntoa
#include <netdb.h> // gethostbyname
#include <string.h> // memcpy
#include <unistd.h> // close
#include <ctype.h> // toupper
#include <vector> // vector

class Download;

// Global variable
std::vector <Download> downloads;

class Server{
  const char *ip_address;
  int port;

public:
  Server(const char*, int);

  const char* get_ip(){
    return this->ip_address;
  }

  int get_port(){
    return this->port;
  }
};

Server::Server(const char* ip, int port_number){
  ip_address = ip;
  port = port_number;
}

class Download{
  std::string book;
  int progress;

public:
  Download(std::string, int);

  std::string get_book(){
    return this->book;
  }

  int get_progress(){
    return this->progress;  
  }

  void update_progress(int new_progress){
    this->progress = new_progress;
  }

  void print_download(){
    std::cout << "[DESCARGA] " << this->book << " (" << this->progress << "%)" << std::endl;
  }  
};

Download::Download(std::string book_name, int progress_percent){
  book = book_name;
  progress = progress_percent;
}


/**
   initializes the list of servers with given ips and ports.

   @returns Vector of server objects
*/
std::vector<Server> init_servers_list(){
  std::vector <Server> servers;

  Server s1 = Server("127.0.0.1", 8081);
  Server s2 = Server("127.0.0.1", 8082);
  Server s3 = Server("127.0.0.1", 8083);

  servers.push_back(s1);
  servers.push_back(s2);
  servers.push_back(s3);

  return servers;
}

/**
   method that shows error message and exit the program when an error occurs.

   @param message -> error message
*/
void error(const char *message, int socketfd){
   perror(message);
   close(socketfd);
   if(EXIT_FAILURE) exit(EXIT_FAILURE);
   else exit(-1);
}

/**
   method that prints commands client options.
*/
void help(){
  std::cout << "> ESTADO_DESCARGAS: muestra el estado de sus descargas\n";
  std::cout << "> LISTA_LIBROS: muestra el listado de libros disponibles\n";
  std::cout << "> SOLICITUD <libro>: solicita <libro> desde cualquier servidor disponible\n";
  std::cout << "> LIBROS_DESCARGADOSxSERVIDOR: muestra los libros que ha descargado por servidor\n";
  std::cout << "> SALIR: termina la sesión\n";
  std::cout << "----------------------------------------------------------------------------------\n";
}

/**
   method that prints downloads status. It uses a global variable
*/
void print_downloads_status(){

  if (downloads.size() == 0) std::cout << "[INFORMACIÓN] No hay descargas en curso" << std::endl;
  
  else{
    for (int i = 0; i < downloads.size(); ++i)
      downloads[i].print_download();
  }  
}

/**
   method that makes a request

   @param ip, port
*/
int request_book_list(const char *ip, int port){
 int command_num = 1;
 int socketfd;
 struct sockaddr_in client_addr;
 struct hostent *server;
 struct sockaddr_in server_addr;

 /**
    create socket with params:
      domain -> IPv4 (AF_INET)
      type -> TCP (SOCK_STREAM)
      protocol -> IP (0) 
 */ 
 socketfd = socket(AF_INET, SOCK_STREAM, 0);
 if (socketfd < 0)
    error("Error al crear el socket", socketfd);

 /** 
    client_addr structure must be set to use it in bind method call
 */

 // set bytes of struct to zero
 memset(&server_addr, 0, sizeof(server_addr));

 // byte order to use -> IPV4
 server_addr.sin_family = AF_INET;

 // port must be converted from integer value into network byte order
 server_addr.sin_port = htons(port);

 if ((server = gethostbyname(ip)) == NULL){
  std::cout << "No se pudo obtener el host" << std::endl;
  close(socketfd);
  exit(-1);
 }

 // bind the client to the host of the server
 memcpy((char *)&client_addr.sin_addr.s_addr, (char *)server->h_addr, sizeof(server->h_length));

 if(connect(socketfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0){
    std::cout << "Host inalcanzable " << ip << ":" << port << std::endl; 
    return 0;
    //error("Host inalcanzable", socketfd);
 }

 char books_list[1024];

 write(socketfd, &command_num, sizeof(command_num));

 read(socketfd, books_list, sizeof(books_list));

 std::cout << books_list << std::endl;

 close(socketfd);

}

/**
   method that prints availables books. It uses a global variable
*/
void print_books_list(){
  std::cout << "-- Los libros disponibles son --" << std::endl;

  std::vector<Server> servers = init_servers_list();

  for (int i = 0; i < 3; ++i)
  	request_book_list(servers[i].get_ip(), servers[i].get_port());
}

/**
   method executed by thread

   @param conn_thread 
*/
void handle_connection(int socketfd){
  std::string command;
  std::string command_solicitud;
  int command_num, data_size;

  std::cout << "[INFORMACIÓN] Bienvenido" << std::endl;
  std::cout << 
  "[INFORMACIÓN] Escriba AYUDA para conocer la lista de comandos disponibles" 
  << std::endl;

   while(true){
    std::cout << "[INFORMACIÓN] Escriba el comando que desea ejecutar: ";
    std::cin >> command;

    if(command == "AYUDA") help();
    
    else if (command == "ESTADO_DESCARGAS") print_downloads_status();
   
    else if (command == "LISTA_LIBROS") print_books_list();

    else if (command.find("SOLICITUD") == 0){
      std::cin >> command_solicitud;
      std::cout << command_solicitud << std::endl;
      write(socketfd, &command_num, sizeof(command));
      write(socketfd, &command_solicitud, sizeof(command));
     }
    else if (command == "LIBROS_DESCARGADOSxSERVIDOR") write(socketfd, &command_num, sizeof(command));
    else if (command == "SALIR"){
      command_num = 4;
      write(socketfd, &command_num, sizeof(command_num));
      close(socketfd);
      exit(1);
    }
    else std::cout << "Comando inválido. Ejecute AYUDA si desea conocer la lista de comandos.\n";

   }   
}

/**
   method that establishes a client-server connection

   @param ip, port 
*/
int client(const char *ip, int port){
   int socketfd;
   struct sockaddr_in client_addr;
   struct hostent *server;
   struct sockaddr_in server_addr;

   /**
      create socket with params:
        domain -> IPv4 (AF_INET)
        type -> TCP (SOCK_STREAM)
        protocol -> IP (0) 
   */ 
   socketfd = socket(AF_INET, SOCK_STREAM, 0);
   if (socketfd < 0)
      error("Error al crear el socket", socketfd);

   /** 
      client_addr structure must be set to use it in bind method call
   */

   // set bytes of struct to zero
   memset(&server_addr, 0, sizeof(server_addr));

   // byte order to use -> IPV4
   server_addr.sin_family = AF_INET;

   // port must be converted from integer value into network byte order
   server_addr.sin_port = htons(port);

   if ((server = gethostbyname(ip)) == NULL){
    std::cout << "No se pudo obtener el host" << std::endl;
    close(socketfd);
    exit(-1);
   }

   // bind the client to the host of the server
   memcpy((char *)&client_addr.sin_addr.s_addr, (char *)server->h_addr, sizeof(server->h_length));

   if(connect(socketfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0){
      std::cout << "Host inalcanzable " << ip << ":" << port << std::endl; 
      return 0;
      //error("Host inalcanzable", socketfd);
   }

   handle_connection(socketfd);
}

/**
   attempts to connect to the previous specific list of available servers

   @returns -1 in case of failure
*/
int connect_to_available_servers(std::vector<Server> servers){
  int s = 0; // server num (0, 1, 2)
  int server_port; // current server port
  int connected; // 0 if connection failure
  int attempts = 0; // num of attempts to connect to available servers
  const char* server_ip; // current server ip

  while(0 <= s <= 2){
    server_ip = servers[s].get_ip();
    server_port = servers[s].get_port();

    connected = client(server_ip, server_port);

    if (!connected && s < 2) s++;
    
    else{
      attempts++;

      if(attempts > 2){
        std::cout << "Se llegó al número máximo de intentos." << std::endl;
        std::cout << "Imposibilidad de servicio. Intente más tarde." << std::endl;
        return -1;
      }
      std::cout << "No se pudo conectar a ninguno de los tres servidores." << std::endl;
      std::cout << "Se intentará conectar desde el primero en 15 segundos." << std::endl;
      std::cout << "Numero máximo de intentos es 3. Se ha intentado " << attempts;
      if (attempts > 1) std::cout << " veces." << std::endl;
      else std::cout << " vez." << std::endl;

      sleep(15);
      s = 0;
      
      continue;
    }
  }

}

int main(int argc, char *argv[]){

  std::vector<Server> servers = init_servers_list();

  // if no arguments where provided, try to connect to one of the servers
  if (argc == 1) connect_to_available_servers(servers);

  // server and port where specified. try to connect to these
  else if (argc == 3) client(argv[1], atoi(argv[2]));

  else std::cout << "usage: cliente [<host> <puerto>]\n";
  
  return -1;
}