#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>




//Fonctions---------------------------------------------------------------------

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

//------------------------------------------------------------------------------

int do_socket(int domain, int type, int protocol) {
  int socket1;
  int yes = 1;

  //create the socket
  socket1 = socket(domain,type,protocol);

  //check for socket validity
  if(socket1 == -1){
    error("ERROR socket creation");
  }

  // set socket option, to prevent "already in use" issue when rebooting the server right on
  if (setsockopt(socket1, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    error("ERROR setting socket options");
  }

  return socket1;
}

//------------------------------------------------------------------------------

void get_addr_info(const char* port, struct sockaddr_in* serv_addr) {

  int portno;

  //clean the serv_add structure
  memset(serv_addr,'\0',sizeof(serv_addr));

  //cast the port from a string to an int
  portno = atoi(port);

  //internet family protocol
  serv_addr->sin_family = AF_INET;

  //we bind to any ip form the host
  serv_addr->sin_addr.s_addr = INADDR_ANY;

  //we bind on the tcp port specified
  serv_addr->sin_port = htons(portno);

}


//------------------------------------------------------------------------------

void do_bind(int socket, const struct sockaddr_in pointeur_serv_addr){
  int i = bind(socket, (struct sockaddr*) &pointeur_serv_addr, sizeof(pointeur_serv_addr));
  if(i == -1){
    error("ERROR bind server");
  }
}



//------------------------------------------------------------------------------

void listen_client(int socket, int backlog){
  int i = listen(socket, backlog);
  if(i == -1){
    error("ERROR listen server");
  }
}


//------------------------------------------------------------------------------

int do_accept(int socket, struct sockaddr_in pointeur_host_addr){
  size_t host_addr_size = sizeof(struct sockaddr_in);
  int i = accept(socket, (struct sockaddr *) &pointeur_host_addr,(socklen_t *) &host_addr_size);
  if(i == -1){
    error("ERROR accepte server");
  }
  return i;
}


//------------------------------------------------------------------------------

void do_read(int socket, char *buffer){
  int i = read(socket,buffer,255);
  if(i == -1){
    error("ERROR read server");
  }
}

//------------------------------------------------------------------------------

void do_write(int fd, const void *buffer){
  size_t i = write(fd,buffer,255);
  if(i == -1){
    error("ERROR write server");
  }
}

//------------------------------------------------------------------------------

void close_socket(int socket){
  int i = close(socket);
  if(i == -1){
    error("ERROR close socket");
  }
}



//Corps-------------------------------------------------------------------------

int main(int argc, char** argv)
{

  if (argc != 2)
  {
    fprintf(stderr, "usage: RE216_SERVER port\n");
    return 1;
  }

  char buffer[256];

  //get the socket--------------------------------------------------------------
  printf("Etape : Création socket\n");
  int socket = do_socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);


  //init the serv_add structure
  printf("Etape : Informations serveur\n");
  struct sockaddr_in pointeur_serv_addr;
  get_addr_info(argv[1], &pointeur_serv_addr);


  //perform the binding---------------------------------------------------------
  printf("Etape : Bind\n");
  do_bind(socket,pointeur_serv_addr);


  //listen for at most 20 concurrent client-------------------------------------
  printf("Etape : Ecoute\n");
  listen_client(socket,20);

  int nb_co=1;
  int event_fd;
  struct pollfd fds[21];
  fds[0].fd=socket;
  fds[0].events=POLLIN;


while(1){

  while(1){

  //accept connection from client-------------------------------------------
  printf("Etape : Acceptation\n");

for (int i = 0; i < nb_co; i++){
  printf("%i\n",fds[i].fd );
}

  event_fd=poll(fds,nb_co,-1);
  printf("fd events %i\n",event_fd);

for (int i = 1; i <= nb_co; i++) {
  printf("soke %i\n",socket);

  printf("%i\n",i);
    if(fds[0].revents==POLLIN){
            printf("%i\n",i);
            printf("dadada\n");
            struct sockaddr_in pointeur_host_addr;
            int new_socket = do_accept(socket,pointeur_host_addr);
            fds[i].fd=new_socket;
            fds[0].revents=POLLOUT;
            fds[i].revents=POLLIN;
            nb_co++;



    }
    else
        {

          if(fds[i].revents==POLLIN){

            printf("dddd\n");

            //read what the client has to say------------------------------------------
            printf("Etape : Lecture\n");
            memset (buffer, '\0', sizeof (buffer));

            do_read(fds[i].fd,buffer);
            printf("Le message reçu est: %s\n",buffer);


              //clean up client socket-------------------------------------------------
              if(strcmp(buffer, "/quit") == 0 ){
                printf("Fermeture socket client\n");
                close_socket(fds[i].fd);
                break;
              }

            //we write back to the client---------------------------------------------
            printf("Etape : Ecriture\n");
            do_write(fds[i].fd,buffer);
          }



 }


}
}








  //clean up server socket------------------------------------------------------
  printf("Fermeture socket serveur\n");
  close_socket(socket);



  return 0;
}
}
