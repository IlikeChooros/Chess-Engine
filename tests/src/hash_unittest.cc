#include <gtest/gtest.h>
#include <cengine/cengine.h>


class HashTest: public ::testing::Test
{
    protected:
    void SetUp() override
    {
        chess::Manager::init();
        b.init();
        chess::Zobrist::set_hash(&b);
        gh.push(&b, Move());
    }

    chess::Board b;
    chess::GameHistory gh;
};


TEST_F(HashTest, Hashing)
{
    uint64_t hash = chess::Zobrist::get_hash(&b);
    EXPECT_EQ(hash, b.hash());
}

TEST_F(HashTest, makePawnDoubleMove)
{
    // Make a move
    Move move(Move(chess::E2, chess::E4, Move::FLAG_DOUBLE_PAWN));
    make(move, &b, &gh);
    uint64_t hash = chess::Zobrist::get_hash(&b);
    EXPECT_EQ(hash, b.hash()); // Check if the hash is updated correctly
}

TEST_F(HashTest, castleKingSide)
{
    // Make a move
    Move move(Move(chess::E2, chess::E4, Move::FLAG_DOUBLE_PAWN));
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E7, chess::E5, Move::FLAG_DOUBLE_PAWN);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::G1, chess::F3, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::B8, chess::C6, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::F1, chess::C4, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::G8, chess::F6, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E1, chess::G1, Move::FLAG_KING_CASTLE);
    make(move, &b, &gh);
    EXPECT_EQ(chess::Zobrist::get_hash(&b), b.hash()); // Check if the hash is updated correctly
}


TEST_F(HashTest, loseCastlingRights)
{
    // Make a move
    Move move(Move(chess::E2, chess::E4, Move::FLAG_DOUBLE_PAWN));
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E7, chess::E5, Move::FLAG_DOUBLE_PAWN);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::G1, chess::F3, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::B8, chess::C6, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::F1, chess::C4, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::G8, chess::F6, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::H1, chess::G1, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly
}

TEST_F(HashTest, loseCastlingRightsKing)
{
        // Make a move
    Move move(Move(chess::E2, chess::E4, Move::FLAG_DOUBLE_PAWN));
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E7, chess::E5, Move::FLAG_DOUBLE_PAWN);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::G1, chess::F3, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::B8, chess::C6, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::F1, chess::C4, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::G8, chess::F6, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E1, chess::F1, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly
}

TEST_F(HashTest, capture)
{
    Move move(Move(chess::E2, chess::E4, Move::FLAG_DOUBLE_PAWN));
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::D7, chess::D5, Move::FLAG_DOUBLE_PAWN);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E4, chess::D5, Move::FLAG_CAPTURE);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly
}

TEST_F(HashTest, enpassantCapture)
{
    Move move(chess::E2, chess::E4, Move::FLAG_DOUBLE_PAWN);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::D7, chess::D5, Move::FLAG_DOUBLE_PAWN);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E4, chess::E5, 0);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::F7, chess::F5, Move::FLAG_DOUBLE_PAWN);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly

    move = Move(chess::E5, chess::F6, Move::FLAG_ENPASSANT_CAPTURE);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly
}

TEST_F(HashTest, promotion)
{
    b.loadFen("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9");
    chess::Zobrist::set_hash(&b);

    Move move(chess::D7, chess::D8, Move::FLAG_QUEEN_PROMOTION);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly
}

TEST_F(HashTest, promotionCapture)
{
    b.loadFen("rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9");
    chess::Zobrist::set_hash(&b);

    Move move(chess::D7, chess::C8, Move::FLAG_QUEEN_PROMOTION_CAPTURE);
    make(move, &b, &gh);
    EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)); // Check if the hash is updated correctly
}


TEST_F(HashTest, playGame)
{
    std::vector<std::string> moves = {
        "e2e4", "e7e5", "g1f3", "b8c6", 
        "f1c4", "g8f6", "e1g1", "d7d5", 
        "c4d5", "f6d5", "e4d5", "c6e7", 
        "f1e1", "e7g6", "d2d4", "f8e7", 
        "c1e3", "c8g4", "d1d2", "g4f3",
        "g2f3", "d8d7", "b1c3", "a8b8"
    };

    for (auto& m: moves)
    {
        EXPECT_TRUE(gh.ply() < 100);
        Move move(m);
        make(move, &b, &gh);
        EXPECT_EQ(b.hash(), chess::Zobrist::get_hash(&b)) << "Move: " << move.notation() << " failed";
    }
}