#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/event.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#define PORT_NUMBER 8888

#ifdef __APPLE__
#define MSG_NOSIGNAL 0x2000
#endif

int set_nonblock(int fd)
{
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
} 

void print_string(std::string s) 
{
    const int MAX_BYTES = 1024;
    int i = 0;
    while (s[i]) {
        for (; s[i] && i < MAX_BYTES - 1; i++) {
            printf("%c", s[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    FILE *logs = fopen("logs", "w");

    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int optval = 1;
    setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    optval = 1;
    setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    
    if (MasterSocket == -1)
    {
        std::cout << strerror(errno) << std::endl;
        return 1;
    }

    struct sockaddr_in SockAddr;
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(PORT_NUMBER);
    SockAddr.sin_addr.s_addr = INADDR_ANY;

    int Result = bind(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));

    if(Result == -1)
    {
        std::cout << strerror(errno) << std::endl;
        return 1;
    }

    set_nonblock(MasterSocket);

    Result = listen(MasterSocket, SOMAXCONN);

    if(Result == -1)
    {
        std::cout << strerror(errno) << std::endl;
        return 1;
    }


    int KQueue = kqueue();

    struct kevent KEvent;
    bzero(&KEvent, sizeof(KEvent));

    EV_SET(&KEvent, MasterSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
    kevent(KQueue, &KEvent, 1, NULL, 0, NULL);

    std::vector<int> clients;
    std::string tmp;

    while (true)
    {
        bzero(&KEvent, sizeof(KEvent));
        kevent(KQueue, NULL, 0, &KEvent, 1, NULL);

        if (KEvent.filter == EVFILT_READ)
        {
            if (KEvent.ident == MasterSocket)
            {
                int SlaveSocket = accept(MasterSocket, 0, 0);
                set_nonblock(SlaveSocket);
                
                clients.push_back(SlaveSocket);
                fprintf(stdout, "%s\n", "accepted connection");
                fflush(stdout);

                char msg[] = "Welcome!\n";
                send(SlaveSocket, msg, sizeof(msg), MSG_NOSIGNAL);
                bzero(&KEvent, sizeof(KEvent));
                EV_SET(&KEvent, SlaveSocket, EVFILT_READ, EV_ADD, 0, 0, 0);
                kevent(KQueue, &KEvent, 1, NULL, 0, NULL);
            }
            else
            {
                static char Buffer[1025];
                int RecvSize = recv(KEvent.ident, Buffer, 1024, MSG_NOSIGNAL);
                if (RecvSize <= 0)
                {
                    clients.erase(find(clients.begin(), clients.end(), KEvent.ident));
                    fprintf(stdout, "%s\n", "connection terminated");
                    fflush(stdout);
                    close(KEvent.ident);
                }
                else
                {   
                    for (int client : clients) 
                    {
                        send(client, Buffer, RecvSize, MSG_NOSIGNAL);
                    }
                    fprintf(stdout, "new message:%s", Buffer);
                    fflush(stdout);
                }

            }
        }
    }

    fclose(logs);

    return 0;
}