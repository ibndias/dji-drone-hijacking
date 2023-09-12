/*
 * puppet.c --- Control small machine ether interface to send packet that
 *              has been crafted from bigger machine
 * 
 * This file is a part of PAPPET (Packet Puppet) project
 * more informations at http://derrylab.com
 *
 * Copyright (C) 2023  Derry Pratama <derryprata@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/ether.h>

#define PORT 1234

#define DEFAULT_IF "eth0"
#define MY_DEST_MAC0 0xff
#define MY_DEST_MAC1 0xff
#define MY_DEST_MAC2 0xff
#define MY_DEST_MAC3 0xff
#define MY_DEST_MAC4 0xff
#define MY_DEST_MAC5 0xff

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_ll socket_address;
    struct ifreq if_idx, if_mac;
    char ifName[IFNAMSIZ];

    int sock, client_sock, read_size;
    struct sockaddr_in server, client;
    unsigned char client_message[4096] = {0};
    unsigned int nextlen;

    // create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }
    int optval = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // bind the socket to a specific port
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Bind failed\n");
        return 1;
    }

    // listen for incoming connections
    listen(sock, 3);
reconnect:
    // accept incoming connections
    puts("Waiting for incoming connections...");
    client_sock = accept(sock, (struct sockaddr *)&client, (socklen_t *)&read_size);
    if (client_sock < 0)
    {
        perror("accept");
        printf("Accept failed\n");
        return 1;
    }

    //==================ETHERNET SETUP============================

    /* Get interface name */
    if (argc > 1)
        strcpy(ifName, argv[1]);
    else
        strcpy(ifName, DEFAULT_IF);

    /* Open RAW socket to send on */
    if ((sockfd = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW)) == -1)
    {
        perror("socket");
    }

    /* Get the index of the interface to send on */
    memset(&if_idx, 0, sizeof(struct ifreq));
    strncpy(if_idx.ifr_name, ifName, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
        perror("SIOCGIFINDEX");
    /* Get the MAC address of the interface to send on */
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, ifName, IFNAMSIZ - 1);
    if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");

    /* Index of the network device */
    socket_address.sll_ifindex = if_idx.ifr_ifindex;
    /* Address length*/
    socket_address.sll_halen = ETH_ALEN;
    /* Destination MAC */
    socket_address.sll_addr[0] = MY_DEST_MAC0;
    socket_address.sll_addr[1] = MY_DEST_MAC1;
    socket_address.sll_addr[2] = MY_DEST_MAC2;
    socket_address.sll_addr[3] = MY_DEST_MAC3;
    socket_address.sll_addr[4] = MY_DEST_MAC4;
    socket_address.sll_addr[5] = MY_DEST_MAC5;

    //==================END ETHERNET SETUP============================

    // main loop for receiving and printing messages
    while (1)
    {   
        read_size = recv(client_sock, client_message, 1, 0);

        if(read_size == 1 && client_message[0] == '-')
        {   
            printf("- FOUND\n");
            // memset(client_message, 0, sizeof(client_message));
            read_size = recv(client_sock, client_message, 2, 0);
            if(read_size == 2)
            {
                nextlen = 0;
                nextlen = (unsigned int)(client_message[0] << 8) | client_message[1];
                printf("LEN FOUND: %d\n",nextlen);
                printf("%d\n",client_message[0]);
                printf("%d\n",client_message[1]);
                read_size = recv(client_sock, client_message, nextlen, 0);
                if (read_size <= 0) goto disc;
            }
            else
                goto disc;

        }
        else{
            disc:
            perror("recv");
            printf("Client disconnected\n");
            fflush(stdout);
            return 1;
            // close(client_sock);
            // goto reconnect;
        }

        // // receive message from client
        // read_size = recv(client_sock, client_message, 4096, 0);
        // if (read_size == 0)
        // {
        //     printf("Client disconnected\n");
        //     fflush(stdout);
        //     return 1;
        // }
        // print message and send response
        printf("Received message: %s\n", client_message);

        // Convert the message from hex to bytes
        size_t len = strlen(client_message) / 2;
        unsigned char bytes[len];
        int i;
        for (i = 0; i < len; i++)
        {
            sscanf(client_message + 2 * i, "%2hhx", &bytes[i]);
        }

        printf("Received bytes: ");
        for (i = 0; i < len; i++)
        {
            printf("%02x ", bytes[i]);
        }
        printf("\n");

        /* Send packet */
        ssize_t ret = sendto(sockfd, bytes, len, 0, (struct sockaddr *)&socket_address, sizeof(struct sockaddr_ll));
        if (ret == -1)
        {
            perror("send");
            printf("Send failed\n");
            send(client_sock, "Send failed, please check the puppet", 17, 0);
        }
        else
        {
            printf("Sent %ld packets\n", ret);
            send(client_sock, "Message received", 17, 0);
        }
        send(client_sock, "Message received", 17, 0);
        // clear the message buffer for next message
        memset(client_message, 0, sizeof(client_message));
    }

    return 0;
}
