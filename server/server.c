#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

// https://www.sciencedirect.com/topics/computer-science/registered-port#:~:text=Ports%200%20through%201023%20are,be%20used%20dynamically%20by%20applications.
// /etc/services
#define PORT 49157
#define BACKLOG 5
#define BUF_SIZE 256


int main(int argc, const char * argv[])
{
    struct sockaddr_in addr;
    int sfd;
    
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(sfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    listen(sfd, BACKLOG);
    
    for(;;)
    {
        int cfd;
        ssize_t num_read;
        char buf[BUF_SIZE];

        cfd = accept(sfd, NULL, NULL);
         do {
            num_read = read(cfd, buf, BUF_SIZE);
            write(STDOUT_FILENO, buf, num_read);
            write(cfd, "xd", 2);

        } while(num_read != 0);
        close(cfd);
    }
    
    // close(sfd); <- never reached because for(;;) never ends.

    return EXIT_SUCCESS;
}