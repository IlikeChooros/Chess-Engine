#pragma once

#include "manager_impl.h"

namespace test
{
    using namespace chess;

    class Preft
    {
    public:
        Preft(Board* board = nullptr);
        Preft(const Preft& other) = delete;
        Preft& operator=(Preft&& other);

        int run(int depth = 5);
    private:
        int preft(int depth);

        Board *m_board;
    };
}