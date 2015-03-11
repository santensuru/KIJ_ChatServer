// Server Chat

#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

struct user {
    int sockcli;
    char ip_active[16];
    char username[128];
    struct user *next;
};

struct user *awal;

void push(int cli, char *ip, char *name) {
    struct user *baru = (struct user*) malloc (sizeof(struct user));
    baru->sockcli = cli;
    strcpy(baru->ip_active, ip);
    strcpy(baru->username, name);
    baru->next = NULL;
    
    if (awal == NULL) {
        awal = baru;
    } else {
        struct user *temp = awal;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = baru;
    }
    return;
}

void pop(int cli) {
    struct user *temp = awal;
    struct user *temp2 = awal->next;
    if (temp->sockcli == cli) {
        awal = temp2;
        free(temp);
    } else {
        do {
            if (temp2->sockcli == cli) {
                temp->next = temp2->next;
                free(temp2);
            } else {
                temp = temp->next;
                temp2 = temp2->next;
            }
        } while (temp2->next != NULL);
    }
    return;
}

void set_name(int cli, char *name) {
    struct user *temp = awal;
    do {
       if (temp->sockcli == cli) {
           strcpy(temp->username, name);
       }
       temp = temp->next;
    } while (temp != NULL);
    return;
}

int cek_user(char *msg) {
    struct user *temp = awal;
    do {
       if (strcmp(temp->username, msg) == 0) {
           return temp->sockcli;
       }
       temp = temp->next;
    } while (temp != NULL);
    return -1;
}

void send_who(int dest) {
    char msg_temp[4096];
    bzero(msg_temp, 4096);
    strcpy(msg_temp, "<LIST>");
    
    struct user *temp = awal;
    do {
        strcat(msg_temp, "-");
        strcat(msg_temp, temp->username);
        temp = temp->next;
    } while (temp != NULL);
    
    strcat(msg_temp, "\r\n");
    write(dest, msg_temp, strlen(msg_temp));
    fflush(stdout);
    return;
}

typedef struct haha {
    int sockcli;
    char username[128];
} haha;

void *broadcast(void *ptr) {
    haha *handler = (haha *)ptr;
    send_who(handler->sockcli);
    return;
}

void broadcast_IP() {
     // thread acc //
    void *join;
     
    pthread_t broadcast_t;
    int broadcast_i;
	pthread_mutex_t broadcast_m = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // end //
    
    haha *handler = (haha *) malloc( sizeof ( haha ) );
    
    struct user *temp = awal;
    while (temp != NULL) {
        handler->sockcli = temp->sockcli;
        
        pthread_mutex_lock( &broadcast_m );
        
        broadcast_i = pthread_create( &broadcast_t, &attr, broadcast, (void *)handler);
        
        pthread_mutex_unlock( &broadcast_m );
        
        pthread_join(broadcast_t, &join);
        
        temp = temp->next;
    }
    return;
}

void *acc(void *ptr) {
    char version[128];
    strcpy(version, "0.0.1c beta");

    haha * handler = (haha *)ptr;
    
    printf("%d", handler->sockcli);
    int retval = 0;
    
    char buf[2], msg[4096], comment[4096], msg_temp[4096], msg_send[4096];
    int dest = -1;
    
    /* here */
    
    sprintf(msg_send, "<HAY.>-:) Welcome to chat server KIJ %s\r\n", version);
    write(handler->sockcli, msg_send, strlen(msg_send));
    fflush(stdout);
    
    while(1) {
        bzero(msg, 4096);
        bzero(comment, 4096);
        bzero(msg_send, 4096);
        do {
            fflush(stdin);
            retval = read(handler->sockcli, buf, 1);
            buf[retval] = '\0';
            strcat(msg, buf);
            bzero(buf, 2);
        } while (strstr(msg, "\r\n") == NULL);
        
        int i;
        for (i=1; i<5; i++) {
            if (msg[i] >= 'a' && msg[i] <= 'z') {
                msg[i] = msg[i] - 'a' + 'A';
            }
        }
        
        if (strstr(msg, "<NAME>") != NULL) {
            sscanf(msg, "<NAME>-%[^\r\n]", comment);
            strcpy(handler->username, comment);
            set_name(handler->sockcli, handler->username);
            broadcast_IP();

        } else if (strstr(msg, "<USER>") != NULL) {
            sscanf(msg, "<USER>-%[^-]-%[^\r\n]", comment, msg_send);
            if (strcmp(comment, "") == 0) {
                sprintf(msg_send, "<REPL>-?\r\n");
                
            } else {
                sscanf(comment, "%s", msg_temp);
                strcpy(comment, msg_temp);
                dest = cek_user(comment);
                if (dest > -1) {
                    if (strcmp(msg_send, "") != 0) {
                        strcpy(msg_temp, "<FROM>-");
                        strcat(msg_temp, handler->username);
                        strcat(msg_temp, "-");
                        strcat(msg_temp, msg_send);
                        strcat(msg_temp, "\r\n");
                        write(dest, msg_temp, strlen(msg_temp));
                        fflush(stdout);
                        strcpy(msg_send, "<SEND>\r\n");
                        
                    } else {
                        sprintf(msg_send, "<REPL>-Message for NAME = %s ?\r\n", comment);
                    
                    }
                    
                } else {
                    sprintf(msg_send, "<REPL>-User with NAME = %s not found\r\n", comment);
                    
                }
            }
            
        } else if (strstr(msg, "<WHO?>") != NULL) {
            send_who(handler->sockcli);
            bzero(msg_send, 4096);
            
        } else if (strstr(msg, "<OUT.>") != NULL) {
            strcpy(msg_send, "<BYE.>-:) Thank You....");
            write(handler->sockcli, msg_send, strlen(msg_send));
            fflush(stdout);
            pop(handler->sockcli);
            break;
            
        } else {
            strcpy(msg_send, "<???.>-@x@ Command not recognized....\r\n");
        
        }
        
        write(handler->sockcli, msg_send, strlen(msg_send));
        fflush(stdout);
    }
    
    close(handler->sockcli);
    
    broadcast_IP();
    
    return;
}

void main()
{
    awal = NULL;
    
    int sockfd = 0, sockcli = 0;
    int retval;
    struct sockaddr_in servaddr, cliaddr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    
    bzero(&servaddr, sizeof(servaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6060);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    retval = bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    
    if (retval < 0)
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
        
        push(sockcli, inet_ntoa(cliaddr.sin_addr), "anonymous");
        
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
        strcpy(handler->username, "anonymous");
        
        pthread_mutex_lock( &acc_m );
        
        acc_i = pthread_create( &acc_t, &attr, acc, (void *)handler);
        
        pthread_mutex_unlock( &acc_m );
    }
    close(sockfd);           
}
 
