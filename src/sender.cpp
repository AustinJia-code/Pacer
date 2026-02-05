/**
 * @file sender.cpp
 * @brief Sends packets
 */

#include "network.h"
#include "helpers.h"
#include "consts.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>

/**
 * Runner
 */
int main (int argc, char* argv[])
{
    /**** START RECEIVE ****/
    if (argc < 3)
    {
        std::cerr << "Usage: ./sender [bind port] [dest port]" << std::endl;
        return EXIT_FAILURE;
    }

    int bind_port = atoi (argv[1]);
    int dest_port = atoi (argv[2]);

    int sock = create_udp_socket ();
    if (sock < 0)
        return EXIT_FAILURE;

    // Ack receive
    if (bind_socket (sock, bind_port) < 0)
    {
        std::cerr << "Issue binding sender" << std::endl;
        return EXIT_FAILURE;
    }

    // Data send
    sockaddr_in data_dest_addr = make_dest_addr ("127.0.0.1", dest_port);

    /**** START SEND ****/
    if (debug)
        std::cout << "Sending..." << std::endl;

    DataPacket data_packet;
    AckPacket ack_packet;

    for (id_t id = 0; id < (2 << 10); ++id)
    {
        // Send data
        data_packet.header = {.type = PacketType::Data, .id = id};
        data_packet.byte_count = 0;

        if (debug)
            std::cout << "Sending ID " << data_packet.header.id << std::endl;

        if (send_data (sock, data_packet, data_dest_addr) < 0)
            std::cerr << "Issue sending to socket" << std::endl;

        // Listen for ack, retransmit if needed (sloppy implementation)
        if (receive_ack (sock, ack_packet, ack_timeout) < 1)
            --id;

        usleep (sec_to_us ({sec_t {0.1}}));
    }
}
