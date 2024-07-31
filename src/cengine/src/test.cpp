#include <cengine/test.h>


namespace test
{
    Preft::Preft(Board* board)
    {
        m_board = board;
    }

    Preft& Preft::operator=(Preft&& other)
    {
        m_board = other.m_board;
        return *this;
    }

    int Preft::run(int depth)
    {
        return preft(depth);
    }

    int Preft::preft(int depth){
        ManagerImpl manager(m_board);

        manager.generateMoves();

        if(depth == 1){
            return manager.n_moves;
        }

        int nodes = 0;
        for(int i = 0; i < manager.n_moves; i++){
            auto move = Move(manager.move_list[i]);
            manager.make(move);
            nodes += preft(depth - 1);
            manager.unmake();
        }
        
        return nodes;
    }
}