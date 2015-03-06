// Server Chat

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct haha {
    int sockcli;
    char ip_active[16];
} haha;

char user[100][16];

int cek_IP(char *msg) {
    //printf(">> %s, %s <<", msg, user[1]);
    int i;
    for (i=4; i<100; i++) {
        if (strcmp(user[i], msg) == 0) {
            //printf("%i\n", i);
            return i;
        }
    }
    return -1;
}

void send_who(int dest) {
    int i;
    char temp[128];
    bzero(temp, 128);
    strcpy(temp, "LIST\r\n");
    write(dest, temp, strlen(temp));
    fflush(stdout);
    for (i=4; i<100; i++) {
        if (strcmp(user[i], "") == 0) {
            break;
        } else {
            strcpy(temp, user[i]);
            strcat(temp, "\r\n");
            write(dest, temp, strlen(temp));
            fflush(stdout);
        }
    }
    strcpy(temp, "END\r\n");
    write(dest, temp, strlen(temp));
    fflush(stdout);
    return;
}

void *acc(void *ptr) {
    haha * handler = (haha *)ptr;
    
    printf("%d", handler->sockcli);
    int retval = 0;
    
    char buf[2], msg[255], comment[255], msg_send[4096];
    int dest = -1;
    
    /* here */
    
    
    strcpy(msg_send, "WELCOME :)\r\n");
    write(handler->sockcli, msg_send, strlen(msg_send));
    fflush(stdout);
    
    while(1) {
        bzero(msg, 255);
        bzero(comment, 255);
        bzero(msg_send, 4096);
        do {
            fflush(stdin);
            retval = read(handler->sockcli, buf, 1);
            buf[retval] = '\0';
            strcat(msg, buf);
            bzero(buf, 2);
        } while (strstr(msg, "\r\n") == NULL);
        
        int i;
        for (i=0; i<4; i++) {
            if (msg[i] >= 'a' && msg[i] <= 'z') {
                msg[i] = msg[i] - 'a' + 'A';
            }
        }
        
        //printf("%s", msg);
        
        if (strstr(msg, "USER") != NULL) {
            sscanf(msg, "USER %s %[^\r\n]", comment, msg_send);
            if (strcmp(comment, "") == 0) {
                sprintf(msg_send, "IP?\r\n");
            } else {
                dest = cek_IP(comment);
                if (dest > -1) {
                    //printf("%d", dest);
                    strcat(msg_send, "\r\n");
                    write(dest, msg_send, strlen(msg_send));
                    fflush(stdout);
                    bzero(msg_send, 4096);
                } else {
                    sprintf(msg_send, "USER with IP = %s not found\r\n", comment);
                }
            }
        }
        
        else if (strstr(msg, "WHO?") != NULL) {
             send_who(handler->sockcli);
             bzero(msg_send, 4096);
        }
        
        write(handler->sockcli, msg_send, strlen(msg_send));
        fflush(stdout);
    }
        
    close(handler->sockcli);
        
    return;
}

void main()
{
     int sockfd = 0, sockcli = 0;
     int retval;
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
     socklen_t clisize = sizeof(cliaddr);
     
     // thread acc //
    void *join;
     
     pthread_t acc_t;
    int acc_i;
	pthread_mutex_t acc_m = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // end //
	
    while(sockcli = accept(sockfd, (struct sockaddr*)&cliaddr , &clisize)) {
     
        haha *handler = (haha *) malloc( sizeof ( haha ) );
	    printf("sockcli --> %d\n", sockcli);
	    printf("Ada klien masuk dari %s %d\n", inet_ntoa(cliaddr.sin_addr), (int) ntohs(cliaddr.sin_port));
	    
	    strcpy(user[sockcli], inet_ntoa(cliaddr.sin_addr));
	    
	    //printf("%d %s\n", sockcli, user[sockcli]);
	    
	    char str[INET_ADDRSTRLEN];
	    inet_ntop(AF_INET, &(cliaddr.sin_addr), str, INET_ADDRSTRLEN);
	    printf("%s\n", str);
	    
	    struct sockaddr_in localAddress;
	    socklen_t addressLength = sizeof(localAddress);
	    getsockname(sockcli, (struct sockaddr*)&localAddress, &addressLength);
	    
	    char ip_active[16];
	    strcpy(ip_active, inet_ntoa( localAddress.sin_addr));
	    printf("local address: %s\n", ip_active);
	    printf("local port: %d\n", (int) ntohs(localAddress.sin_port));
 	   
         
         if (sockcli < 0)
         {
                     perror(strerror(errno));
                     exit(-1);
         }
         
     // baca dan tulis pesan disini
     
     handler->sockcli = sockcli;
	    printf("%d\n", handler->sockcli);
	    strcpy(handler->ip_active, ip_active);
    	
	    pthread_mutex_lock( &acc_m );
		acc_i = pthread_create( &acc_t, &attr, acc, (void *)handler);
		
		pthread_mutex_unlock( &acc_m );
		
		//pthread_join(acc_t, &join);
	}
     close(sockfd);           
}
 