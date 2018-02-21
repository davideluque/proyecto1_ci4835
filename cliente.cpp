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

void error(const char *message, int socketfd){
   perror(message);
   close(socketfd);
   if(EXIT_FAILURE) exit(EXIT_FAILURE);
   else exit(-1);
}

void help(){
  std::cout << "> ESTADO_DESCARGAS: muestra el estado de sus descargas\n";
  std::cout << "> LISTA_LIBROS: muestra el listado de libros disponibles\n";
  std::cout << "> SOLICITUD <libro>: solicita <libro> desde cualquier servidor disponible\n";
  std::cout << "> LIBROS_DESCARGADOSxSERVIDOR: muestra los libros que ha descargado por servidor\n";
  std::cout << "> SALIR: termina la sesión\n";
  std::cout << "----------------------------------------------------------------------------------\n";
}

void handle_connection(int socketfd){
  std::string command;
  std::string command_solicitud;
  int command_num;

  std::cout << "Bienvenido" << std::endl;
  std::cout << "Escriba AYUDA si desea conocer la lista de comandos disponibles" << std::endl;

   while(true){
    std::cout << "Escriba el comando que desea ejecutar: ";
    std::cin >> command;

    if(command == "AYUDA") help();
    else if (command == "ESTADO_DESCARGAS") write(socketfd, &command_num, sizeof(command));
    else if (command == "LISTA_LIBROS"){
      std::string book_list;
      int strlen = htonl(sizeof("HOLA"));
      std::string holas = "HOLA";
      write(socketfd, &strlen, sizeof(strlen));
      write(socketfd, &holas, sizeof(holas));
      read(socketfd, &book_list, sizeof(book_list));
    }
    else if (command.find("SOLICITUD") == 0){
      std::cin >> command_solicitud;
      std::cout << command_solicitud << std::endl;
      write(socketfd, &command_num, sizeof(command));
      write(socketfd, &command_solicitud, sizeof(command));
     }
    else if (command == "LIBROS_DESCARGADOSxSERVIDOR") write(socketfd, &command_num, sizeof(command));
    else if (command == "SALIR"){
      close(socketfd);
      exit(1);
    }
    else std::cout << "Comando inválido. Ejecute AYUDA si desea conocer la lista de comandos.\n";

   }   
}

void cliente(char *ip, int port){
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

   /* client_addr structure must be set to use in bind method call */

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
      error("Host inalcanzable", socketfd);
   }

   handle_connection(socketfd);
}

int main(int argc, char *argv[]){

   if (argc <3){
      std::cout << "usage: cliente <host> <puerto>\n";
      exit(-1);
   }

   cliente(argv[1], atoi(argv[2]));
   
   return 0;
}