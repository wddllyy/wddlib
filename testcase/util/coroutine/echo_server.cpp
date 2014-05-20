#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include<stdio.h>
#include<sys/time.h>
#include<unistd.h>

#define MAXEVENTS 10240+10

static int make_socket_non_blocking (int sfd)
{
    int flags, s;

    flags = fcntl (sfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror ("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl (sfd, F_SETFL, flags);
    if (s == -1)
    {
        perror ("fcntl");
        return -1;
    }

    return 0;
}

static int create_and_bind (char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
    hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
    hints.ai_flags = AI_PASSIVE;     /* All interfaces */

    s = getaddrinfo (NULL, port, &hints, &result);
    if (s != 0)
    {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        //int sock_reuse = 1;
        //setsockopt(sfd ,
        //    SOL_SOCKET,
        //    SO_REUSEADDR,
        //    (char *)&sock_reuse,
        //    sizeof(sock_reuse));


        s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0)
        {
            /* We managed to bind successfully! */
            break;
        }

        close (sfd);
    }

    if (rp == NULL)
    {
        fprintf (stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo (result);

    s = make_socket_non_blocking (sfd);
    if (s == -1)
        abort ();

    s = listen (sfd, SOMAXCONN);
    if (s == -1)
    {
        perror ("listen");
        abort ();
    }


    return sfd;
}


int socket_send(int sockfd, const char* buffer, size_t buflen)
{
    int tmp;
    size_t total = buflen;
    const char *p = buffer;

    while(1)
    {
        tmp = send(sockfd, p, total, 0);
        if(tmp < 0)
        {
            // 当send收到信号时,可以继续写,但这里返回-1.
            if(errno == EINTR)
                return -1;

            // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
            // 在这里做延时后再重试.
            if(errno == EAGAIN)
            {
                usleep(1000);
                continue;
            }

            return -1;
        }

        if((size_t)tmp == total)
            return buflen;

        total -= tmp;
        p += tmp;
    }

    return tmp;
}



int  main (int argc, char *argv[])
{
    int sfd, s;
    int efd;
    struct epoll_event event;
    struct epoll_event *events;
    int successcount = 1;
    if (argc != 2)
    {
        fprintf (stderr, "Usage: %s [port]\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    sfd = create_and_bind (argv[1]);
    if (sfd == -1)
        abort ();

    efd = epoll_create1 (0);
    if (efd == -1)
    {
        perror ("epoll_create");
        abort ();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
    if (s == -1)
    {
        perror ("epoll_ctl");
        abort ();
    }

    /* Buffer where events are returned */
    events = (epoll_event*)calloc (MAXEVENTS, sizeof event);
    struct timeval starttime ;
    gettimeofday(&starttime, NULL);
    /* The event loop */
    while (1)
    {
        int n, i;

        n = epoll_wait (efd, events, MAXEVENTS, -1);
        //printf("epoll_wait %d\n",n);
        if (successcount > 100000 )
        {
            struct timeval now ;
            gettimeofday(&now, NULL);

            float diff =   (now.tv_sec-starttime.tv_sec) + ((now.tv_usec - starttime.tv_usec))/1000000.f;
            printf("count %d, tps %f\n",successcount, (float)successcount / diff);
            starttime = now;
            successcount = 1;
        }
        
        for (i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN)))
            {
                /* An error has occured on this fd, or the socket is not
                ready for reading (why were we notified then?) */
                int err = errno;
                fprintf (stderr, "epoll error  fd:%d  errorcode:%d event:%d\n", events[i].data.fd, err, events[i].events);
                fprintf (stderr, "%d %d %d\n", (events[i].events & EPOLLERR), (events[i].events & EPOLLHUP) , (!(events[i].events & EPOLLIN)));
                close (events[i].data.fd);
                continue;
            }

            else if (sfd == events[i].data.fd)
            {
                /* We have a notification on the listening socket, which
                means one or more incoming connections. */
                while (1)
                {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    infd = accept (sfd, &in_addr, &in_len);
                    if (infd == -1)
                    {
                        if ((errno == EAGAIN) ||
                            (errno == EWOULDBLOCK))
                        {
                            /* We have processed all incoming
                            connections. */
                            break;
                        }
                        else
                        {
                            perror ("accept");
                            break;
                        }
                    }

                    s = getnameinfo (&in_addr, in_len,
                        hbuf, sizeof hbuf,
                        sbuf, sizeof sbuf,
                        NI_NUMERICHOST | NI_NUMERICSERV);
                    if (s == 0)
                    {
                        printf("Accepted connection on descriptor %d "
                            "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                    }

                    /* Make the incoming socket non-blocking and add it to the
                    list of fds to monitor. */
                    s = make_socket_non_blocking (infd);
                    if (s == -1)
                        abort ();

                    event.data.fd = infd;
                    event.events = EPOLLIN ;
                    s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                    if (s == -1)
                    {
                        perror ("epoll_ctl");
                        abort ();
                    }
                }
                continue;
            }
            else
            {
                /* We have data on the fd waiting to be read. Read and
                display it. We must read whatever data is available
                completely, as we are running in edge-triggered mode
                and won't get a notification again for the same
                data. */
                int done = 0;

                while (1)
                {
                    ssize_t count;
                    char buf[512];

                    count = read (events[i].data.fd, buf, sizeof buf);
                    if (count == -1)
                    {
                        /* If errno == EAGAIN, that means we have read all
                        data. So go back to the main loop. */
                        if (errno != EAGAIN)
                        {
                            perror ("read");
                            done = 1;
                        }
                        break;
                    }
                    else if (count == 0)
                    {
                        /* End of file. The remote has closed the
                        connection. */

                        printf("fd:%d close\n",events[i].data.fd);
                        done = 1;
                        break;
                    }

                    /* Write the buffer to standard output */
                    buf[count]=0;
                    //printf("fd:%d recv:%s\n",events[i].data.fd, buf);
                    ++successcount;
                    //s = write (1, buf, count);
                    //if (s == -1)
                    //{
                    //    perror ("write");
                    //    abort ();
                    //}

                    static int aa = 0;
                    if ( aa % 100000 == 0)
                    {
                        printf("fd:%d recv:count%d:%s \n",events[i].data.fd, (int)count ,buf+4);
                    }
                    ++aa;

                    s = socket_send(events[i].data.fd, buf, count);
                    if (s == -1)
                    {
                        perror ("socket_send");
                        done = 1;
                        break;
                    }
                }

                if (done)
                {
                    printf ("Closed connection on descriptor %d\n", events[i].data.fd);
                    /* Closing the descriptor will make epoll remove it
                    from the set of descriptors which are monitored. */
                    close (events[i].data.fd);
                }
            }
        }
    }

    free (events);

    close (sfd);

    return EXIT_SUCCESS;
}