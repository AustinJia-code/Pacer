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

/**
 * Packet with forwarding timing info
 */
struct TimedPacket
{
    ms_t out_ms;
    Packet packet;

    TimedPacket (ms_t out_ms, Packet packet)
        : out_ms (out_ms), packet (packet) {}

    bool operator > (const TimedPacket& rhs) const
    {
        if (this->out_ms == rhs.out_ms)
            return this->packet.id > rhs.packet.id;

        return this->out_ms > rhs.out_ms;
    }
};

/**
 * Runner
 */
int main (int argc, char* argv[])
{
    // Read args
    if (argc < 4)
    {
        std::cerr << "Usage: ./emulator [receiver port] [sender port] [hazard]"
                  << std::endl;
        std::cerr << "Hazards: random-loss, burst-loss, random-jitter"
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Sockets
    int receive_port = atoi (argv[1]);
    int receive_sock = create_udp_socket ();
    if (receive_sock < 0)
        return EXIT_FAILURE;
    bind_socket (receive_sock, receive_port);

    int send_port = atoi (argv[2]);
    int send_sock = create_udp_socket ();
    if (send_sock < 0)
        return EXIT_FAILURE;
    sockaddr_in dest = make_dest_addr ("127.0.0.1", send_port);

    // Hazards + pass-through
    if (debug)
        std::cout << "Emulating..." << std::endl;

    std::string hazard_name = argv[3];
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
        return EXIT_FAILURE;
    }

    Packet packet {};
    std::priority_queue<TimedPacket, std::vector<TimedPacket>,
                        std::greater<TimedPacket>> out_queue {};

    while (true)
    {
        if (receive_packet (receive_sock, packet) < 1)
        {
            std::cerr << "Issue reading from socket" << std::endl;
            continue;
        }

        ms_t time_ms = get_time_ms ();

        if (debug)
            std::cout << "Applying hazards" << std::endl;

        auto effects = std::visit (
            [&] (auto& h) { return h.get_effects (packet.id); }, hazard);

        if (effects.drop)
        {
            if (debug)
                std::cout << "Dropped ID " << packet.id << std::endl;
            continue;
        }   

        out_queue.emplace (time_ms + effects.delay, packet);

        // Send from queue
        while (!out_queue.empty () && out_queue.top ().out_ms < time_ms + 1)
        {
            Packet out_packet = out_queue.top ().packet;

            if (debug)
                std::cout << "Forwarding ID " << out_packet.id << std::endl;

            send_packet (send_sock, out_packet, dest);
            out_queue.pop ();
        }
    }
}
