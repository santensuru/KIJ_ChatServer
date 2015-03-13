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
    char username[128];
} haha;

struct user {
    int weight;
    haha *data;
    struct user *left,
                *right;
};

struct user *awal;

int height(struct user **temp) {
    int kiri = 0, kanan = 0, nilai = 0;
    if ((*temp)->left != NULL) {
        kiri += height(&(*temp)->left);
        if (kiri - 1 == 0)
            kiri = -2;
        else
            kiri -= 1;
    }
    if ((*temp)->right != NULL) {
        kanan += height(&(*temp)->right);
        if (kanan + 1 == 0)
            kanan = 2;
        else
            kanan += 1;
    }
    
    nilai = kiri + kanan;
        
    if (nilai == -2) {
        struct user *temp2 = (*temp);
        if (temp2->left->weight == -1) {
            temp2->left->right = (*temp);
            (*temp) = temp2->left;
            (*temp)->right->left = NULL;
        }
        else {
            temp2->left->right->right = (*temp);
            temp2->left->right->left = (*temp)->left;
            (*temp) = temp2->left->right;
            (*temp)->left->right = NULL;
            (*temp)->right->left = NULL;
        }
        (*temp)->left->weight = 0;
        (*temp)->right->weight = 0;
        nilai = 0;
    }
    if (nilai == 2) {
        struct user *temp2 = (*temp);
        if (temp2->right->weight == 1) {
            temp2->right->left = (*temp);
            (*temp) = temp2->right;
            (*temp)->left->right = NULL;
        }
        else {
            temp2->right->left->left = (*temp);
            temp2->right->left->right = (*temp)->right;
            (*temp) = temp2->right->left;
            (*temp)->right->left = NULL;
            (*temp)->left->right = NULL;
        }
        (*temp)->right->weight = 0;
        (*temp)->left->weight = 0;
        nilai = 0;
    }
    
    (*temp)->weight = nilai;
    return nilai;
}

int push(haha *isi) {
    struct user *baru = (struct user*) malloc (sizeof(struct user));
    baru->weight = 0;
    baru->data = isi;
    baru->left = NULL;
    baru->right = NULL;
    
    if (awal == NULL) {
        awal = baru;
    } else {
        struct user *temp = awal;
        while (1) {
            if (strcmp(temp->data->username, isi->username) > 0) {
                if (temp->left != NULL) {
                    temp = temp->left;
                }
                else {
                    temp->left = baru;
                    break;
                }
            } else if (strcmp(temp->data->username, isi->username) < 0) {
                if (temp->right != NULL) {
                    temp = temp->right;
                }
                else {
                     temp->right = baru;
                     break;
                }
            } else {
                return -1;
            }
        }
    }
    height(&awal);
    return 1;
}

void pop(int cli, struct user **temp) {
    struct user *temp2;
    if ((*temp)->data->sockcli == cli) {
        if ((*temp)->left != NULL && (*temp)->right != NULL) {
            temp2 = (*temp)->left;
            if (temp2->right != NULL) {
                while (temp2->right->right != NULL) {
                    temp2 = temp2->right;
                }
                (*temp)->data = temp2->right->data;
                temp2->right = temp2->right->left;
            } else {
                temp2 = (*temp)->right;
                if (temp2->left != NULL) {
                    while (temp2->left->left != NULL) {
                        temp2 = temp2->left;
                    }
                    (*temp)->data = temp2->left->data;
                    temp2->left = temp2->left->right;
                } else {
                    (*temp)->data = temp2->data;
                    (*temp)->right = temp2->right;
                }
            }
        } else if ((*temp)->left != NULL) {
            temp2 = (*temp)->left;
            if (temp2->right != NULL) {
                while (temp2->right->right != NULL) {
                    temp2 = temp2->right;
                }
                (*temp)->data = temp2->right->data;
                temp2->right = temp2->right->left;
            } else {
                (*temp)->data = temp2->data;
                (*temp)->left = temp2->left;
            }
        } else if ((*temp)->right != NULL) {
            temp2 = (*temp)->right;
            if (temp2->left != NULL) {
                while (temp2->left->left != NULL) {
                    temp2 = temp2->left;
                }
                (*temp)->data = temp2->left->data;
                temp2->left = temp2->left->right;
            } else {
                (*temp)->data = temp2->data;
                (*temp)->right = temp2->right;
            }
        } else {
            (*temp) = NULL;
        }
        height(&awal);
        return;
    } else {
        if ((*temp)->left != NULL)
            pop(cli, &(*temp)->left);
        if ((*temp)->right != NULL)
            pop(cli, &(*temp)->right);
    }
    return;
}

int cek_user(char *msg, struct user *temp) {
    int a = -1, b = -1;
    if (temp->left != NULL)
        a = cek_user(msg, temp->left);
        
    if (strcmp(temp->data->username, msg) == 0)
        return temp->data->sockcli;
    
    if (temp->right != NULL) 
        b = cek_user(msg, temp->right);
    
    return a >= b ? a : b ;
}

void send_who_msg(int dest, struct user *temp, int depth) {
    char msg_temp[128];
    bzero(msg_temp, 128);
    if (temp->left != NULL)
        send_who_msg(dest, temp->left, depth+1);
    
    if (temp != NULL) {
        strcpy(msg_temp, "-");
        strcat(msg_temp, temp->data->username);
        write(dest, msg_temp, strlen(msg_temp));
        fflush(stdout);
    }
    
    if (temp->right != NULL)
        send_who_msg(dest, temp->right, depth+1);
    return;
}

void send_who(int dest) {
    char msg_temp[128];
    bzero(msg_temp, 128);
    strcpy(msg_temp, "<LIST>");
    write(dest, msg_temp, strlen(msg_temp));
    fflush(stdout);
    
    send_who_msg(dest, awal, 0);
    
    strcpy(msg_temp, "\r\n");
    write(dest, msg_temp, strlen(msg_temp));
    fflush(stdout);
    return;
}

void *broadcast(void *ptr) {
    haha *handler = (haha *)ptr;
    send_who(handler->sockcli);
    return;
}

void broadcast_IP(struct user *temp) {
    // thread acc //
    void *join;
    
    pthread_t broadcast_t;
    int broadcast_i;
    pthread_mutex_t broadcast_m = PTHREAD_MUTEX_INITIALIZER;
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // end //
    
    haha *handler = (haha *) malloc( sizeof ( haha ) );
    
    handler->sockcli = temp->data->sockcli;
    
    pthread_mutex_lock( &broadcast_m );
    
    broadcast_i = pthread_create( &broadcast_t, &attr, broadcast, (void *)handler);
    
    pthread_mutex_unlock( &broadcast_m );
    
    if (temp->left != NULL)
        broadcast_IP(temp->left);
    
    if (temp->right != NULL)
        broadcast_IP(temp->right);
    
    return;
}


void *acc(void *ptr) {
    char version[128];
    strcpy(version, "0.0.1f beta");

    haha *handler = (haha *)ptr;
    
    printf("%d", handler->sockcli);
    int retval = 0;
    
    char buf[2], msg[4096], command[128], msg_temp[4096], msg_send[4096];
    int dest = -1;
    
    /* here */
    
    sprintf(msg_send, "<HAY.>-:) Welcome to chat server KIJ %s\r\n", version);
    write(handler->sockcli, msg_send, strlen(msg_send));
    fflush(stdout);
    
    while(1) {
        bzero(msg, 4096);
        bzero(command, 128);
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
            sscanf(msg, "<NAME>-%[^\r\n]", command);
            strcpy(handler->username, command);
            push(handler);
            broadcast_IP(awal);

        } else if (strstr(msg, "<USER>") != NULL) {
            sscanf(msg, "<USER>-%[^-]-%[^\r\n]", command, msg_send);
            if (strcmp(command, "") == 0) {
                sprintf(msg_send, "<REPL>-?\r\n");
                
            } else {
                sscanf(command, "%s", msg_temp);
                strcpy(command, msg_temp);
                dest = cek_user(command, awal);
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
                        sprintf(msg_send, "<REPL>-Message for NAME = %s ?\r\n", command);
                    
                    }
                    
                } else {
                    sprintf(msg_send, "<REPL>-User with NAME = %s not found\r\n", command);
                    
                }
            }
            
        } else if (strstr(msg, "<WHO?>") != NULL) {
            send_who(handler->sockcli);
            bzero(msg_send, 4096);
            
        } else if (strstr(msg, "<OUT.>") != NULL) {
            strcpy(msg_send, "<BYE.>-:) Thank You....");
            write(handler->sockcli, msg_send, strlen(msg_send));
            fflush(stdout);
            pop(handler->sockcli, &awal);
            break;
            
        } else {
            strcpy(msg_send, "<???.>-@x@ Command not recognized....\r\n");
        
        }
        
        write(handler->sockcli, msg_send, strlen(msg_send));
        fflush(stdout);
    }
    
    close(handler->sockcli);
    
    broadcast_IP(awal);
    
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
 
