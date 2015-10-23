#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <string.h>

#include <sys/event.h>

#include <errno.h>

#include <algorithm>

#define PORT_NUMBER 8888

#ifdef __APPLE__
#define MSG_NOSIGNAL 0x2000
#endif

int main() {
    int ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(PORT_NUMBER); 
    SockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    connect(ClientSocket, (const struct sockaddr*) &SockAddr, sizeof (SockAddr));
    
    fd_set Set;
    int fd = fileno(stdin);
    
    while (1) {
        FD_ZERO(&Set);
        FD_SET(fd, &Set);
        FD_SET(ClientSocket, &Set);
        int retval = select(std::max(fd, ClientSocket) + 1, &Set, NULL, NULL, NULL);
        if (retval == -1) {
            perror("Connection terminated\n");
            break;
        } else {
            static char Buffer[1024];
            if (FD_ISSET(fd, &Set)) {
                fgets(Buffer, sizeof(Buffer), stdin);
                send(ClientSocket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
            } 

            if (FD_ISSET(ClientSocket, &Set)) {
                int RecvSize = recv(ClientSocket, Buffer, sizeof(Buffer), MSG_NOSIGNAL);
                if (RecvSize <= 0) {
                    printf("Connection terminated\n");
                    break;
                } else {
                    printf("new message was received:%s\n", Buffer);
                }
            }
        }
    }
    close(ClientSocket);
    return 0;
}
