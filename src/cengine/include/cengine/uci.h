#pragma once

#include <stdio.h>
#include <stdarg.h>

#include "manager.h"
#include "test.h"
#include "threads.h"

// Universal Chess Interface
namespace uci
{
    std::string uciReadCommImpl(chess::Manager* manager, std::string input);

    class UCI
    {
    public:
        UCI();

        void sendCommand(std::string comm);
        bool isReady();
        std::string getResult();
        void loop();
    private:
        TaskQueue m_queue;
        chess::Board m_board;
        chess::Manager m_manager;
        std::mutex m_mutex;
        std::string m_command;
        std::queue<std::string> m_result;
        bool m_ready;
    };
}

