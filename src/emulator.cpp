/**
 * @file emulator.cpp
 * @brief Emulate network hazards between sender and receiver.
 *        This is the "world" that the protocol needs to handle.
 */

#include "network.h"
#include "consts.h"
#include "hazards.h"
#include "metrics.h"
#include "helpers.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <variant>
#include <queue>
#include <optional>
#include <poll.h>

/**
 * DataPacket with forwarding timing info
 */
struct TimedPacket
{
    ms_t out_ms;
    UnionPacket packet;

    TimedPacket (ms_t out_ms, UnionPacket packet)
        : out_ms (out_ms), packet (packet) {}

    bool operator > (const TimedPacket& rhs) const
    {
        if (this->out_ms == rhs.out_ms)
            return this->packet.ack_packet.header.id >
                         rhs.packet.ack_packet.header.id;

        return this->out_ms > rhs.out_ms;
    }
};

/**
 * Parsed arguments for emulator
 */
struct Args
{
    int receive_sock;
    int send_sock;
    sockaddr_in ack_dest_addr;
    sockaddr_in data_dest_addr;
    std::variant<RandomLoss, BurstLoss, RandomJitter> hazard;
};

/**
 * Argument Parser, returns nullopt on failure
 */
std::optional<Args> parse_args (int argc, char* argv[])
{
    if (argc < 6)
    {
        std::cerr << "Usage: ./emulator [recv bind] [ack bind] [receiver port] [sender port] [hazard]"
                  << std::endl;

        return std::nullopt;
    }

    // [recv bind] - receive data from sender
    int recv_bind = atoi (argv[1]);
    int receive_sock = create_udp_socket ();
    if (receive_sock < 0)
        return std::nullopt;

    if (bind_socket (receive_sock, recv_bind) < 0)
    {
        std::cerr << "Issue binding emulator receive" << std::endl;
        return std::nullopt;
    }

    // [ack bind] - receive ACKs from receiver
    int ack_bind = atoi (argv[2]);
    int send_sock = create_udp_socket ();
    if (send_sock < 0)
        return std::nullopt;

    if (bind_socket (send_sock, ack_bind) < 0)
    {
        std::cerr << "Issue binding emulator send" << std::endl;
        return std::nullopt;
    }

    // [receiver port] - forward data to receiver
    int receiver_port = atoi (argv[3]);
    sockaddr_in data_dest_addr = make_dest_addr ("127.0.0.1", receiver_port);

    // [sender port] - forward ACKs to sender
    int sender_port = atoi (argv[4]);
    sockaddr_in ack_dest_addr = make_dest_addr ("127.0.0.1", sender_port);

    // [hazard]
    std::string hazard_name = argv[5];
    std::variant<RandomLoss, BurstLoss, RandomJitter> hazard;

    if (hazard_name == "random-loss")
        hazard = RandomLoss {};
    else if (hazard_name == "burst-loss")
        hazard = BurstLoss {};
    else if (hazard_name == "random-jitter")
        hazard = RandomJitter {};
    else
    {
        std::cerr << "Unknown hazard: " << hazard_name << std::endl;
        std::cerr << "Hazards: random-loss, burst-loss, random-jitter"
                  << std::endl;
        return std::nullopt;
    }

    // Return
    return Args {.receive_sock = receive_sock,
                 .send_sock = send_sock,
                 .ack_dest_addr = ack_dest_addr,
                 .data_dest_addr = data_dest_addr,
                 .hazard = hazard};
}

/**
 * Runner
 */
int main (int argc, char* argv[])
{
    /**** PARSE ARGS ****/
    auto args = parse_args (argc, argv);
    if (!args)
        return EXIT_FAILURE;
    
    constexpr size_t nfds = 2;
    pollfd pollfds[nfds] = {{.fd = args->receive_sock, .events = POLLIN},
                            {.fd = args->send_sock, .events = POLLIN}};
    constexpr int timeout = (int) (ms_t {1});

    /**** START EMULATE ****/
    if (debug)
        std::cout << "Starting Emulation..." << std::endl;

    UnionPacket packet {};
    std::priority_queue<TimedPacket, std::vector<TimedPacket>,
                        std::greater<TimedPacket>> out_queue {};

    while (true)
    {
        ms_t time_ms = get_time_ms ();
        int ready = poll (pollfds, nfds, timeout);
        if (ready < 1)
            continue;

        /*** PASS DATA FROM SENDER TO RECEIVER ***/
        if (pollfds[0].revents & POLLIN)
        {
            if (receive_data (args->receive_sock, packet.data_packet) < 1)
            {
                std::cerr << "Issue reading from socket" << std::endl;
                continue;
            }
        }

        /*** PASS ACK FROM RECEIVER TO SENDER ***/
        // TODO: Drops if send, fix
        else if (pollfds[1].revents & POLLIN)
        {
            if (receive_ack (args->send_sock, packet.ack_packet, ms_t {0}) < 1)
            {
                std::cerr << "Issue reading from socket" << std::endl;
                continue;
            }

            time_ms = get_time_ms ();
        }

        /*** APPLY HAZARDS ***/
        if (debug)
            std::cout << "Applying hazards" << std::endl;

        auto effects = std::visit (
            [&] (auto& h) { return h.get_effects (packet.ack_packet.header.type,
                                                  packet.ack_packet.header.id); },
            args->hazard);

        if (effects.drop)
        {
            if (debug)
                std::cout << "Dropped ID " << packet.ack_packet.header.id
                            << std::endl;
            continue;
        }   

        out_queue.emplace (time_ms + effects.delay, packet);

        /*** FORWARD PACKETS ***/
        while (!out_queue.empty () && out_queue.top ().out_ms < time_ms + 1)
        {
            switch (out_queue.top ().packet.ack_packet.header.type)
            {
                case PacketType::Data:
                {
                    DataPacket out_data = out_queue.top ().packet.data_packet;

                    if (debug)
                        std::cout << "Forwarding ID " << out_data.header.id
                                  << std::endl;

                    send_data (args->send_sock, out_data, args->data_dest_addr);
                    break;
                }
                case PacketType::Ack:
                {
                    AckPacket out_ack = out_queue.top ().packet.ack_packet;

                    if (debug)
                        std::cout << "Forwarding ACK " << out_ack.header.id
                                  << std::endl;

                    send_ack (args->receive_sock, out_ack, args->ack_dest_addr);
                    break;
                }
                default:
                {
                    std::cerr << "Unknown packet type on queue" << std::endl;
                    break;
                }
            }
            
            out_queue.pop ();
        }
    }
}
