#pragma once

#include <stdio.h>
#include <stdarg.h>

#include "engine.h"
#include "threads.h"

// Universal Chess Interface
namespace uci
{
    std::string uciReadCommImpl(chess::Engine* engine, std::string input);

    class UCI
    {
    public:
        UCI();
        ~UCI();
        UCI& operator=(UCI&&);

        /**
         * @brief Get the number of commands left in the queue
         */
        int commandsLeft() { return m_queue.tasksLeft(); }

        void sendCommand(std::string comm);
        void loop();
        
    private:
        TaskQueue m_queue;
        chess::Engine m_engine;
        std::mutex m_mutex;
        std::string m_command;
        std::queue<std::string> m_result;
        bool m_ready;
    };
}

