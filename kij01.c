// Server Chat

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

void main()
{
     int sockfd, sockcli;
     int retval, clisize;
     struct sockaddr_in servaddr, cliaddr;
     
     sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     
     bzero(&servaddr, sizeof(servaddr));
     
     servaddr.sin_family = AF_INET;
     servaddr.sin_port = htons(6060);
     servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
     
     retval = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
     
     if (retval <0)
     {
                perror(strerror(errno));
                exit(-1);
     }
     
     printf("Server mengikat port 6060\n");
     retval = listen(sockfd, 5);
     printf("Server menunggu panggilan...\n");
     bzero(&cliaddr, sizeof(cliaddr));
     clisize = 0;
     
     sockcli = accept(sockfd, (struct sockaddr*)&cliaddr , &clisize);
     
     printf("Ada klien masuk dari %s\n", inet_ntoa(cliaddr.sin_addr));
     
     if (sockcli < 0)
     {
                 perror(strerror(errno));
                 exit(-1);
     }
     
     // baca dan tulis pesan disini
     
     char buf[225], msg[225];
     while(1) {
          memset(msg, 0, sizeof(msg[225]));
          do {
               fflush(stdin);
               retval = read(sockcli, buf, sizeof(buf)-1);
               buf[retval] = '\0';
               strcat(msg, buf);
          } while(strstr(buf, "\r\n") == NULL && retval < 225);
          write(sockcli, msg, strlen(msg));
          fflush(stdout);
     }
     
     /*
     char msg[20] ="Selamat datang kawan";
     retval = write(sockcli,msg,strlen(msg));
     printf("selesai kirim pesan\n");
     */
     
     close(sockcli);
     close(sockfd);           
}
 
