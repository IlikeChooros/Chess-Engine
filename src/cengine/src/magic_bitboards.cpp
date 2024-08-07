#include <cengine/magic_bitboards.h>

// Bits used for given square, number of bits set to 1 in the mask
// For example for square 56 (a1):
// . . . . . . . .
// 1 . . . . . . .
// 1 . . . . . . .
// 1 . . . . . . .
// 1 . . . . . . .
// 1 . . . . . . .
// 1 . . . . . . .
// . 1 1 1 1 1 1 .
// Total of 12 bits set to 1
const int MagicBitboards::RBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12,
};

const int MagicBitboards::BBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
};

uint64_t MagicBitboards::bishopAttacks[512];
uint64_t MagicBitboards::rookAttacks[4096];

Magic MagicBitboards::bishopMagics[64] = {
    {0x40201008040200ULL, 0x1d8f90ec9212278dULL, 58},
    {0x402010080400ULL, 0xe0c2047790ce6954ULL, 59},
    {0x4020100a00ULL, 0x9f978882128d384cULL, 59},
    {0x40221400ULL, 0xfc279df80bdd4d27ULL, 59},
    {0x2442800ULL, 0xd9a78b7d6a9f31c4ULL, 59},
    {0x204085000ULL, 0x3a04f5f84dfdf597ULL, 59},
    {0x20408102000ULL, 0xf1e647e9f6d17a6ULL, 59},
    {0x2040810204000ULL, 0x8bd889fbffe6add7ULL, 58},
    {0x20100804020000ULL, 0x27591ea75b95d89eULL, 59},
    {0x40201008040000ULL, 0xce28700d1584ec80ULL, 59},
    {0x4020100a0000ULL, 0x96df05d695b2c1ULL, 59},
    {0x4022140000ULL, 0xfb76ca09aa56d0dcULL, 59},
    {0x244280000ULL, 0x831b5a77ac4f757eULL, 59},
    {0x20408500000ULL, 0xd04c0c30d4f353bfULL, 59},
    {0x2040810200000ULL, 0xa8431b4e7785c8ebULL, 59},
    {0x4081020400000ULL, 0x230e9d7e0d76e0a7ULL, 59},
    {0x10080402000200ULL, 0x6320df2b6985f59eULL, 59},
    {0x20100804000400ULL, 0x5be0a943bf6edd04ULL, 59},
    {0x4020100a000a00ULL, 0x20499c252be681b9ULL, 57},
    {0x402214001400ULL, 0x963a7be626f086e4ULL, 57},
    {0x24428002800ULL, 0x7f23db47c4d0c8baULL, 57},
    {0x2040850005000ULL, 0xab286b07180f662dULL, 57},
    {0x4081020002000ULL, 0x130a1beca0d790bdULL, 59},
    {0x8102040004000ULL, 0x291ee48bfa181677ULL, 59},
    {0x8040200020400ULL, 0xba05e4fadc5609b9ULL, 59},
    {0x10080400040800ULL, 0xdccf8a7c11191ff8ULL, 59},
    {0x20100a000a1000ULL, 0x751535fc62729c8fULL, 57},
    {0x40221400142200ULL, 0xac1cb375cf25f59bULL, 55},
    {0x2442800284400ULL, 0x49a7f30dfd279963ULL, 55},
    {0x4085000500800ULL, 0xf33a21ba3b5fe500ULL, 57},
    {0x8102000201000ULL, 0x3861b303594561ebULL, 59},
    {0x10204000402000ULL, 0x393dca42aa9e222eULL, 59},
    {0x4020002040800ULL, 0x6a762c7cd01e6daeULL, 59},
    {0x8040004081000ULL, 0x269e971a41bcc04bULL, 59},
    {0x100a000a102000ULL, 0x8e29cee178c2478aULL, 57},
    {0x22140014224000ULL, 0x4dc4778982056ca5ULL, 55},
    {0x44280028440200ULL, 0xc4bd57c5856aeb29ULL, 55},
    {0x8500050080400ULL, 0x97e136fee34536f4ULL, 57},
    {0x10200020100800ULL, 0xb923c1e707572286ULL, 59},
    {0x20400040201000ULL, 0xa8898ba4a2161e33ULL, 59},
    {0x2000204081000ULL, 0x20fcb61485bd159cULL, 59},
    {0x4000408102000ULL, 0xbb1281f412dca279ULL, 59},
    {0xa000a10204000ULL, 0xc79230c56e7139deULL, 57},
    {0x14001422400000ULL, 0x159950bd5f940d41ULL, 57},
    {0x28002844020000ULL, 0x3daffb21a580231ULL, 57},
    {0x50005008040200ULL, 0xddac8570d4e5f20ULL, 57},
    {0x20002010080400ULL, 0x8ee6d4bcec4de747ULL, 59},
    {0x40004020100800ULL, 0x3c98e77a1a4b677ULL, 59},
    {0x20408102000ULL, 0x7dc4f225ad75a5e5ULL, 59},
    {0x40810204000ULL, 0x857d270c58db1c12ULL, 59},
    {0xa1020400000ULL, 0x250ff9046715433aULL, 59},
    {0x142240000000ULL, 0x766d0de562781a49ULL, 59},
    {0x284402000000ULL, 0x9caf197b788b3636ULL, 59},
    {0x500804020000ULL, 0xeb836916d8b5aea2ULL, 59},
    {0x201008040200ULL, 0x18f4cef440bd4c39ULL, 59},
    {0x402010080400ULL, 0xfeb69cadad4bd20cULL, 59},
    {0x2040810204000ULL, 0x7db98876e9ef3849ULL, 58},
    {0x4081020400000ULL, 0x14a17a6bf11c8fb3ULL, 59},
    {0xa102040000000ULL, 0xae3b192c342f530bULL, 59},
    {0x14224000000000ULL, 0x8902184457ded46eULL, 59},
    {0x28440200000000ULL, 0xc0ecb3b721a81d85ULL, 59},
    {0x50080402000000ULL, 0xdadcb38f6b8e4308ULL, 59},
    {0x20100804020000ULL, 0x9b7a99e4474c08a6ULL, 59},
    {0x40201008040200ULL, 0xe1081b520f6bdb60ULL, 58},
};

Magic MagicBitboards::rookMagics[64] = {
    {0x101010101017eULL, 0xc32d0fe875afd88eULL, 52},
    {0x202020202027cULL, 0xee1a194118512f2ULL, 53},
    {0x404040404047aULL, 0xd6047b3e58d52a8eULL, 53},
    {0x8080808080876ULL, 0xb7b48683e3823befULL, 53},
    {0x1010101010106eULL, 0x56a97742af03b329ULL, 53},
    {0x2020202020205eULL, 0x25dddb418884a307ULL, 53},
    {0x4040404040403eULL, 0x6052a65d8afdf675ULL, 53},
    {0x8080808080807eULL, 0xb719a71a4c06e462ULL, 52},
    {0x1010101017e00ULL, 0xc758c851fb0f342ULL, 53},
    {0x2020202027c00ULL, 0x854dd8aa1e94cb7eULL, 54},
    {0x4040404047a00ULL, 0xbb638097b92b1913ULL, 54},
    {0x8080808087600ULL, 0xec4e184cc1d633a2ULL, 54},
    {0x10101010106e00ULL, 0xb90c97c531551328ULL, 54},
    {0x20202020205e00ULL, 0x4e1b81ca70028a75ULL, 54},
    {0x40404040403e00ULL, 0xe15cded7c16452d6ULL, 54},
    {0x80808080807e00ULL, 0xef1a65d77546ec11ULL, 53},
    {0x10101017e0100ULL, 0x5a38763eb192668aULL, 53},
    {0x20202027c0200ULL, 0xe1d254039a3b112dULL, 54},
    {0x40404047a0400ULL, 0x237fdc3ad76d8c9cULL, 54},
    {0x8080808760800ULL, 0x6d15a74a1b0744a9ULL, 54},
    {0x101010106e1000ULL, 0x8e18fd6b49e945a1ULL, 54},
    {0x202020205e2000ULL, 0xd8f61fed6884f416ULL, 54},
    {0x404040403e4000ULL, 0xba4256c2e51395d2ULL, 54},
    {0x808080807e8000ULL, 0xe39573e4f829fdeULL, 53},
    {0x101017e010100ULL, 0xb1c6b97022031a8aULL, 53},
    {0x202027c020200ULL, 0x766f8d9e9a89bbb4ULL, 54},
    {0x404047a040400ULL, 0xba7817a74ae0dddeULL, 54},
    {0x8080876080800ULL, 0x921b2d77fd7823cULL, 54},
    {0x1010106e101000ULL, 0xaa55f60ec700178bULL, 54},
    {0x2020205e202000ULL, 0x4a207945d6d91011ULL, 54},
    {0x4040403e404000ULL, 0xf0fe7fe6f9e1d85cULL, 54},
    {0x8080807e808000ULL, 0x88d4d68a6f196e0fULL, 53},
    {0x1017e01010100ULL, 0x1db55b2f650f3f30ULL, 53},
    {0x2027c02020200ULL, 0x1e2a08a3a901362eULL, 54},
    {0x4047a04040400ULL, 0x6c87cde786bc1ecfULL, 54},
    {0x8087608080800ULL, 0x1b3dbf1c6d9259afULL, 54},
    {0x10106e10101000ULL, 0x1c165b0bbe2cb573ULL, 54},
    {0x20205e20202000ULL, 0xe3eb1fca07c44923ULL, 54},
    {0x40403e40404000ULL, 0x2004a3bbebc2d8c9ULL, 54},
    {0x80807e80808000ULL, 0x68b95c16a2f7a5feULL, 53},
    {0x17e0101010100ULL, 0xeb7a49513dd5352ULL, 53},
    {0x27c0202020200ULL, 0x5a8b397ad51b684bULL, 54},
    {0x47a0404040400ULL, 0x2b8c8265f094efcULL, 54},
    {0x8760808080800ULL, 0xe6c99e83ac84396cULL, 54},
    {0x106e1010101000ULL, 0x5effa5ac6f4d3c2aULL, 54},
    {0x205e2020202000ULL, 0x4d806164a44a0022ULL, 54},
    {0x403e4040404000ULL, 0xf96fce5999e5336bULL, 54},
    {0x807e8080808000ULL, 0x77d4843053663ba9ULL, 53},
    {0x7e010101010100ULL, 0x8324199d65142f45ULL, 53},
    {0x7c020202020200ULL, 0x6ebeb8ef5d0b2bbfULL, 54},
    {0x7a040404040400ULL, 0xb7d4a3321cec0a01ULL, 54},
    {0x76080808080800ULL, 0x4cafd2ac4904f26fULL, 54},
    {0x6e101010101000ULL, 0x19e79c64851e4495ULL, 54},
    {0x5e202020202000ULL, 0xfc774e0ad43c4ad2ULL, 54},
    {0x3e404040404000ULL, 0x55ccc48c2c2b30afULL, 54},
    {0x7e808080808000ULL, 0x1163ebf598f660c1ULL, 53},
    {0x7e01010101010100ULL, 0xe64dc25d10e34e43ULL, 52},
    {0x7c02020202020200ULL, 0x4798aaa32b4b323ULL, 53},
    {0x7a04040404040400ULL, 0x48014fe7f5ade5e3ULL, 53},
    {0x7608080808080800ULL, 0x37590c89938f8578ULL, 53},
    {0x6e10101010101000ULL, 0xecf1f5d96cb56377ULL, 53},
    {0x5e20202020202000ULL, 0x3887abd573c6002ULL, 53},
    {0x3e40404040404000ULL, 0x11e9eacf0bc26b75ULL, 53},
    {0x7e80808080808000ULL, 0x8cf51df34d5abd85ULL, 52},
};

// Generate a random 64-bit number, has probably the biggest impact on the generation speed
// If you want to generate the same magics every time, you can seed the generator
uint64_t genRandom()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    
    uint64_t u1 = gen() & 0xFFFF;
    uint64_t u2 = gen() & 0xFFFF;
    uint64_t u3 = gen() & 0xFFFF;
    uint64_t u4 = gen() & 0xFFFF;

    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64_t bishopMask(int sq)
{
    uint64_t result = 0ULL;
    int rk = sq/8, fl = sq%8, r, f;
    for(r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
    for(r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
    for(r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
    for(r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
    return result;
}

uint64_t rookMask(int sq)
{
    uint64_t result = 0ULL;
    int rk = sq/8, fl = sq%8, r, f;
    for(r=rk+1; r<=6; r++) result |= (1ULL << (fl + r*8));
    for(r=rk-1; r>=1; r--) result |= (1ULL << (fl + r*8));
    for(f=fl+1; f<=6; f++) result |= (1ULL << (f + rk*8));
    for(f=fl-1; f>=1; f--) result |= (1ULL << (f + rk*8));
    return result;
}

uint64_t genMailboxAttacks(int type, int sq, uint64_t blockers)
{
    using namespace chess;
    uint64_t attacks = 0;
    for (int rays = 0; rays < Board::n_piece_rays[type]; rays++){
        for(int n = sq;;) {
            n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][rays]];
            if(n == -1) 
                break;
            attacks |= (1ULL << n);
            if (blockers & (1ULL << n) || !Board::is_piece_sliding[type]) 
                break;
        }
    }
    return attacks;
}

// Generate possible bishop attacks for a given square
uint64_t bAttacks(int sq, uint64_t blockers)
{
    using namespace chess;
    return genMailboxAttacks(Board::BISHOP_TYPE, sq, blockers);
}

// Generate possible rook attacks for a given square
uint64_t rAttacks(int sq, uint64_t blockers)
{
    using namespace chess;
    return genMailboxAttacks(Board::ROOK_TYPE, sq, blockers);
}

void populateAttacks(int sq, int bits, uint64_t mask, uint64_t* attacks, uint64_t *occ, bool bishop)
{
    uint64_t (*attackFunc)(int, uint64_t) = bishop ? bAttacks : rAttacks;
    for (int i = 0; i < (1 << bits); i++){
        for (int j = 0; j < bits; j++){
            int lsb = pop_lsb1(mask);
            if (i & (1 << j)){
                // If the j-th bit is set in i, set the lsb-th bit in the occupancy bitboard
                // we do that because we want to generate all possible occupancy combinations
                occ[i] |= (1ULL << lsb);
            }
        }
        // Precompute the attacks for this occupancy
        attacks[i] = attackFunc(sq, occ[i]);
    }
}

uint64_t findMagic(int sq, int shift, bool bishop)
{
    // How it works:
    // 1. First precompute all possible occupancy bitboards for the given square
    // 2. Generate valid attacks for each occupancy
    // 3. Generate a random magic number
    // 4. Check if the magic number is valid:
    //      - Check if the magic number has at least 6 bits set in the high bits
    //      - Check if there are no collisions:
    //          - Generate index = occupancy * magic >> shift
    //          - check if the index is not already used 
    //          - check if the attacks at `index` are the same as the precomputed ones for given occupancy
    // 5. If the magic number is valid, return it
    // 6. If not, repeat the process

    uint64_t attacks[4096], occ[4096], used[4096]; // 4096 = 2^12 (max number of occupancy combinations)
    uint64_t mask = bishop ? bishopMask(sq) : rookMask(sq); // get the mask
    int bits = pop_count(mask); // number of bits set in mask
    
    // Precompute all possible occupancy bitboards for the given square
    populateAttacks(sq, bits, mask, attacks, occ, bishop);

    uint64_t magic = 0;
    for (int tries = 0; tries < 10000000; tries++)
    {
        // Generate a random magic number
        magic = genRandom();

        // Check if the magic number is valid (has at least 6 bits set in the high bits)
        // It is important to have at least 6 bits set in the high bits to avoid collisions
        if (pop_count((mask * magic) & 0xFF00000000000000ULL) < 6)
        {
            // Reset the used array
            memset(used, 0, sizeof(used));
            bool fail = false;

            for (int i = 0; i < (1 << bits); i++)
            {
                // Get the index to look up the attack in the attacks array
                int index = (occ[i] * magic) >> shift;
                // If we have a collision, break and try a new magic number
                // (that is is we have used already this index or the attacks are different)
                if (used[index] && attacks[index] != attacks[i])
                {
                    fail = true;
                    break;
                }
                used[index] = 1;
            }
            // Found valid magic number for this square
            if (!fail){
                break;
            }
        }
    }

    if (magic == 0)
        std::cout << "Failed to find magic for square " << sq << "\n";

    return magic;
}

// Run the magic bitboard generation in parallel
void run_magics(bool bishop)
{
    struct M {
        int index;
        Magic magic;
    };

    // Run in parallel
    std::vector<M> vmagic;
    std::mutex mutex;
    ThreadPool pool;

    for (int i = 0; i < 64; i++){
        pool.enqueue([i, &vmagic, &mutex, &bishop](){
            Magic m;
            const int *bitB = bishop ? MagicBitboards::BBits : MagicBitboards::RBits;
            m.shift = 64 - bitB[i];
            m.mask = bishop ? bishopMask(i) : rookMask(i);
            m.magic = findMagic(i, m.shift, bishop);
            mutex.lock();
            vmagic.push_back({i, m});
            mutex.unlock();
        });
    }

    // Wait for all threads to finish
    pool.stop();
    
    // sort the results
    std::sort(vmagic.begin(), vmagic.end(), [](const M &a, const M &b){
        return a.index < b.index;
    });

    // Print the results in C++ format
    std::string piece = bishop ? "bishopMagics" : "rookMagics";
    std::cout << "Magic MagicBitboards::" << piece << "[64] = {\n";
    for (auto &m : vmagic)
        std::cout << "\t{0x" << std::hex << m.magic.mask << "ULL, 0x" << m.magic.magic << "ULL, "<< std::dec << m.magic.shift << "},\n";
    std::cout << "};\n\n";
}


void init_magics(bool recalculate)
{
    if (recalculate)
    {
        std::cout << "Recalculating magics...\n";
        run_magics(true);
        run_magics(false);
    }

    // Use precomputed magics, fill the attacks
    uint64_t occ[4096];
    for (int sq = 0; sq < 64; sq++){
        // For the bishop
        memset(occ, 0, sizeof(occ));
        uint64_t mask = bishopMask(sq);
        populateAttacks(sq, pop_count(mask), mask, MagicBitboards::bishopAttacks, occ, true);

        // For the rook
        memset(occ, 0, sizeof(occ));
        mask = rookMask(sq);
        populateAttacks(sq, pop_count(mask), mask, MagicBitboards::rookAttacks, occ, false);
    }
}