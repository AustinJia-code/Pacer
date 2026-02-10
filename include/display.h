/**
 * @file display.h
 * @brief Display class for clean telemetry
 */

#include "types.h"
#include <array>
#include <string>

/**
 * Terminal display with in-place line rewriting
 */
class Display
{
private:
    static constexpr size_t max_events = 10;
    std::array<std::string, max_events> events {};
    size_t head = 0;
    size_t count = 0;

public:
    /**
     * Add a line to the display
     */
    void add_event (const std::string& event)
    {
        events[head] = event;
        head = (head + 1) % max_events;
        if (count < max_events)
            ++count;
    }

    /**
     * Show display
     */
    void render (const std::string& header,
                 const std::string& stats)
    {
        std::printf ("\033[H");

        auto line = [] (const std::string& text)
        {
            std::printf ("\033[2K%s\n", text.c_str ());
        };

        line (header);
        line (stats);
        line ("");
        line ("Recent:");

        size_t start = (head + max_events - count) % max_events;
        for (size_t i = 0; i < max_events; ++i)
        {
            if (i < count)
                line ("  " + events[(start + i) % max_events]);
            else
                line ("");
        }

        std::fflush (stdout);
    }
};