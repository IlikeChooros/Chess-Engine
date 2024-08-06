#pragma once

#include <stdio.h>
#include <stdarg.h>

#include "manager.h"
#include "test.h"
#include "threads.h"

// Universal Chess Interface
namespace uci
{

    class UCI
    {
    public:
        UCI();
        void sendCommand(std::string comm);
        bool isReady();
        std::string getResult();
    private:
        TaskQueue m_queue;
        std::mutex m_mutex;
        std::string m_command;
        std::string m_result;
        bool m_ready;
    };


    /**
     * @brief Start the UCI loop, will read from input and write to output
     * 
     * @param input The input stream
     * @param output The output stream
     */
    void uciLoop(FILE* input, FILE* output);
}

