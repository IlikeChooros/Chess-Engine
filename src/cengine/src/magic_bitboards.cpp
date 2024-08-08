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

uint64_t MagicBitboards::bishopAttacks[64][512];
uint64_t MagicBitboards::rookAttacks[64][4096];

Magic MagicBitboards::bishopMagics[64] = {
        {0x40201008040200ULL, 0x836c04c6e7ec0101ULL, 58},
        {0x402010080400ULL, 0xc17cc3fbba0a0000ULL, 59},
        {0x4020100a00ULL, 0xe85801a92203014aULL, 59},
        {0x40221400ULL, 0xa58208124e14022ULL, 59},
        {0x2442800ULL, 0x10320610cc712000ULL, 59},
        {0x204085000ULL, 0x45ee02b522700010ULL, 59},
        {0x20408102000ULL, 0xa7f0c0c619b40120ULL, 59},
        {0x2040810204000ULL, 0x81c6030259100802ULL, 58},
        {0x20100804020000ULL, 0xb8342cc828080088ULL, 59},
        {0x40201008040000ULL, 0xa06a8d2a9e5a0200ULL, 59},
        {0x4020100a0000ULL, 0x20a91c4d3fe5c800ULL, 59},
        {0x4022140000ULL, 0xd331040c00820420ULL, 59},
        {0x244280000ULL, 0xfc1f540c201b0410ULL, 59},
        {0x20408500000ULL, 0xb810988220200000ULL, 59},
        {0x2040810200000ULL, 0xe64d9e8ba8384010ULL, 59},
        {0x4081020400000ULL, 0x8300383c1db41000ULL, 59},
        {0x10080402000200ULL, 0xac8e8d4048282100ULL, 59},
        {0x20100804000400ULL, 0x881053aa0c070411ULL, 59},
        {0x4020100a000a00ULL, 0x79007c80c224030ULL, 57},
        {0x402214001400ULL, 0x4792020426020000ULL, 57},
        {0x24428002800ULL, 0x46c040a020a0080ULL, 57},
        {0x2040850005000ULL, 0x41c8c00a02d00400ULL, 57},
        {0x4081020002000ULL, 0x8e1c200a94030804ULL, 59},
        {0x8102040004000ULL, 0x10c9c06461141000ULL, 59},
        {0x8040200020400ULL, 0x39e8140158a00800ULL, 59},
        {0x10080400040800ULL, 0x80687e55ac140810ULL, 59},
        {0x20100a000a1000ULL, 0x9b550243f0140040ULL, 57},
        {0x40221400142200ULL, 0xe99400c1f0090040ULL, 55},
        {0x2442800284400ULL, 0xc530030024600800ULL, 55},
        {0x4085000500800ULL, 0x84c886002fe21020ULL, 57},
        {0x8102000201000ULL, 0x8a58410502030344ULL, 59},
        {0x10204000402000ULL, 0x4fde830360290800ULL, 59},
        {0x4020002040800ULL, 0x84462020de111200ULL, 59},
        {0x8040004081000ULL, 0xf2451002c1c80820ULL, 59},
        {0x100a000a102000ULL, 0x5001c3ee09100400ULL, 57},
        {0x22140014224000ULL, 0xe36f640108440100ULL, 55},
        {0x44280028440200ULL, 0x4983c880206e0200ULL, 55},
        {0x8500050080400ULL, 0x4f35900f80770089ULL, 57},
        {0x10200020100800ULL, 0x20a22ec852644400ULL, 59},
        {0x20400040201000ULL, 0x3a4800e503004900ULL, 59},
        {0x2000204081000ULL, 0xa3dc222610224088ULL, 59},
        {0x4000408102000ULL, 0x8b5ff6b12d942011ULL, 59},
        {0xa000a10204000ULL, 0x433bd1b290040800ULL, 57},
        {0x14001422400000ULL, 0xd189c6e013030800ULL, 57},
        {0x28002844020000ULL, 0x2cca082d01420c00ULL, 57},
        {0x50005008040200ULL, 0x392f66088d020600ULL, 57},
        {0x20002010080400ULL, 0xe5617b1f0d011200ULL, 59},
        {0x40004020100800ULL, 0xb588050c11e90282ULL, 59},
        {0x20408102000ULL, 0x266ffed108951000ULL, 59},
        {0x40810204000ULL, 0x958bf1c81c7c0000ULL, 59},
        {0xa1020400000ULL, 0x25db8a0b06880009ULL, 59},
        {0x142240000000ULL, 0xdc60c6508c80120ULL, 59},
        {0x284402000000ULL, 0x11b414c8210502c3ULL, 59},
        {0x500804020000ULL, 0xda8320d30e120000ULL, 59},
        {0x201008040200ULL, 0x2f50202e43862000ULL, 59},
        {0x402010080400ULL, 0x8a9b7058d3150010ULL, 59},
        {0x2040810204000ULL, 0x742bfac40c1c4008ULL, 58},
        {0x4081020400000ULL, 0x585df29403080604ULL, 59},
        {0xa102040000000ULL, 0x855f142300451010ULL, 59},
        {0x14224000000000ULL, 0x45a51263b1940400ULL, 59},
        {0x28440200000000ULL, 0x40af6e2115a0200ULL, 59},
        {0x50080402000000ULL, 0x3946a32c907a4200ULL, 59},
        {0x20100804020000ULL, 0xb10a6c68b05d0201ULL, 59},
        {0x40201008040200ULL, 0x836b3000d5210200ULL, 58},
};

Magic MagicBitboards::rookMagics[64] = {
        {0x101010101017eULL, 0x2080065140008120ULL, 52},
        {0x202020202027cULL, 0x4140002005445000ULL, 53},
        {0x404040404047aULL, 0x80100082200108ULL, 53},
        {0x8080808080876ULL, 0x80080104100080ULL, 53},
        {0x1010101010106eULL, 0x4100024500280010ULL, 53},
        {0x2020202020205eULL, 0x100040002089500ULL, 53},
        {0x4040404040403eULL, 0x9000a0000ac0300ULL, 53},
        {0x8080808080807eULL, 0x5100002700054082ULL, 52},
        {0x1010101017e00ULL, 0x8000800840003080ULL, 53},
        {0x2020202027c00ULL, 0x42802008400082ULL, 54},
        {0x4040404047a00ULL, 0x3001041082000ULL, 54},
        {0x8080808087600ULL, 0x840801800500281ULL, 54},
        {0x10101010106e00ULL, 0x400800400480080ULL, 54},
        {0x20202020205e00ULL, 0x1002401000806ULL, 54},
        {0x40404040403e00ULL, 0x11000405001200ULL, 54},
        {0x80808080807e00ULL, 0x11001203a3c100ULL, 53},
        {0x10101017e0100ULL, 0x8080004000200142ULL, 53},
        {0x20202027c0200ULL, 0x1140088020014080ULL, 54},
        {0x40404047a0400ULL, 0x420030010284100ULL, 54},
        {0x8080808760800ULL, 0x808010008804ULL, 54},
        {0x101010106e1000ULL, 0x5488008018040080ULL, 54},
        {0x202020205e2000ULL, 0x808042000400ULL, 54},
        {0x404040403e4000ULL, 0x2080040002100841ULL, 54},
        {0x808080807e8000ULL, 0x8402000242810cULL, 53},
        {0x101017e010100ULL, 0x4100410200228202ULL, 53},
        {0x202027c020200ULL, 0x8010084040012000ULL, 54},
        {0x404047a040400ULL, 0x10100080802000b2ULL, 54},
        {0x8080876080800ULL, 0x308210100083000ULL, 54},
        {0x1010106e101000ULL, 0xa551011100042800ULL, 54},
        {0x2020205e202000ULL, 0x8002000a00440810ULL, 54},
        {0x4040403e404000ULL, 0x102101400081205ULL, 54},
        {0x8080807e808000ULL, 0x1000c020006c281ULL, 53},
        {0x1017e01010100ULL, 0x804000800821ULL, 53},
        {0x2027c02020200ULL, 0x8208810021004000ULL, 54},
        {0x4047a04040400ULL, 0x2380801003802000ULL, 54},
        {0x8087608080800ULL, 0x2000d00180800800ULL, 54},
        {0x10106e10101000ULL, 0x2600480101001004ULL, 54},
        {0x20205e20202000ULL, 0x40c440080802200ULL, 54},
        {0x40403e40404000ULL, 0xa2421004000843ULL, 54},
        {0x80807e80808000ULL, 0x260041804200050cULL, 53},
        {0x17e0101010100ULL, 0x3002400034808000ULL, 53},
        {0x27c0202020200ULL, 0x210124020004000ULL, 54},
        {0x47a0404040400ULL, 0x80d00d020010040ULL, 54},
        {0x8760808080800ULL, 0x810018188008030ULL, 54},
        {0x106e1010101000ULL, 0x488008004008018ULL, 54},
        {0x205e2020202000ULL, 0x2001004020029ULL, 54},
        {0x403e4040404000ULL, 0x180102240010ULL, 54},
        {0x807e8080808000ULL, 0x8802008ac1020004ULL, 53},
        {0x7e010101010100ULL, 0x4000801020410300ULL, 53},
        {0x7c020202020200ULL, 0x880804008a01480ULL, 54},
        {0x7a040404040400ULL, 0x831000200180ULL, 54},
        {0x76080808080800ULL, 0x4000201048420200ULL, 54},
        {0x6e101010101000ULL, 0x54008016880080ULL, 54},
        {0x5e202020202000ULL, 0x409020040801ULL, 54},
        {0x3e404040404000ULL, 0x10822101c00ULL, 54},
        {0x7e808080808000ULL, 0x4008403084200ULL, 53},
        {0x7e01010101010100ULL, 0x2800427004011ULL, 52},
        {0x7c02020202020200ULL, 0x2040104000806905ULL, 53},
        {0x7a04040404040400ULL, 0x245008c220010031ULL, 53},
        {0x7608080808080800ULL, 0x8001091000a10005ULL, 53},
        {0x6e10101010101000ULL, 0x102001004582022ULL, 53},
        {0x5e20202020202000ULL, 0x101000214001841ULL, 53},
        {0x3e40404040404000ULL, 0x81021001008814ULL, 53},
        {0x7e80808080808000ULL, 0x1001000201a04081ULL, 52},
};

// Generate a random 64-bit number, has probably the biggest impact on the generation speed
// If you want to generate the same magics every time, you can seed the generator
uint64_t random_uint64()
{
    uint64_t u1 = random() & 0xFFFFFFFF;
    uint64_t u2 = random() & 0xFFFFFFFF;
    uint64_t u3 = random() & 0xFFFFFFFF;
    uint64_t u4 = random() & 0xFFFFFFFF;

    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64_t genRandom()
{
    return random_uint64() & random_uint64() & random_uint64();
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


uint64_t genMailboxAttacks(int type, int square, uint64_t occupied)
{
    using namespace chess;
    uint64_t attacks = 0;
    for(int j = 0; j < Board::n_piece_rays[type]; j++){
        for(int n = square;;){
            n = Board::mailbox[Board::mailbox64[n] + Board::piece_move_offsets[type][j]];
            if (n == -1){// outside of the board
                break;
            }
            attacks |= 1ULL << n;
            if (occupied & (1ULL << n)){
                break;
            }
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

uint64_t index_occupied(int index, int bit, uint64_t mask)
{
    int pop;
    uint64_t occ = 0;
    for (int i = 0; i < bit; i++){
        pop = pop_lsb1(mask);
        if (index & (1 << i)){
            occ |= (1ULL << pop);
        }
    }
    return occ;
}

void populateAttacks(int sq, int bits, uint64_t mask, uint64_t* attacks, uint64_t *occ, bool bishop)
{
    uint64_t (*attackFunc)(int, uint64_t) = bishop ? bAttacks : rAttacks;
    int max_count = 1 << bits;
    for (int i = 0; i < max_count; i++){
        occ[i] = index_occupied(i, bits, mask);
        attacks[i] = attackFunc(sq, occ[i]);
    }
}

/*
Find the magic number for a given square
Heavily referenced from:
https://www.chessprogramming.org/Looking_for_Magics
*/
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

    int size = bishop ? 512 : 4096;
    uint64_t attacks[size], occ[size], used[size]; // 4096 = 2^12 (max number of occupancy combinations)
    uint64_t mask = bishop ? bishopMask(sq) : rookMask(sq); // get the mask
    int bits = pop_count(mask); // number of bits set in mask
    
    // Precompute all possible occupancy bitboards for the given square
    populateAttacks(sq, bits, mask, attacks, occ, bishop);

    uint64_t magic = 0;
    for (uint64_t tries = 0; tries < 1000000000UL; tries++)
    {
        // Generate a random magic number
        magic = genRandom();

        // Check if the magic number is valid (has at least 6 bits set in the high bits)
        // It is important to have at least 6 bits set in the high bits to avoid collisions
        if (pop_count((mask * magic) & 0xFF00000000000000UL) < 6)
        {
            // Reset the used array
            memset(used, 0, sizeof(used));
            bool fail = false;

            for (int i = 0; i < (1 << bits); i++)
            {
                // Get the index to look up the attack in the attacks array
                auto index = (occ[i] * magic) >> shift;
                // If we have a collision, break and try a new magic number
                // (that is is we have used already this index or the attacks are different)
                if (used[index] == 0)
                    used[index] = attacks[i]; // not the same as attacks[index] !!!
                else if (used[index] != attacks[i])
                {
                    magic = 0;
                    fail = true;
                    break;
                }
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
    vmagic.reserve(64);

    for (int i = 0; i < 64; i++){
        pool.enqueue([i, &vmagic, &mutex, &bishop](){
            Magic m;
            m.shift = 64 - (bishop ? MagicBitboards::BBits[i] : MagicBitboards::RBits[i]);
            m.mask = bishop ? bishopMask(i) : rookMask(i);
            m.magic = findMagic(i, m.shift, bishop);

            // After we found the magic, we can print the progress & save the result
            std::lock_guard<std::mutex> lock(mutex);
            std::cout << "Progress: " << vmagic.size() + 1 << "/64\n";
            vmagic.push_back({i, m});
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

// Initialize the magics, if recalculate is true, run the magic generation again, the
// results will be printed in C++ format, otherwise use the precomputed magics and just
// set the correct order of attacks
void init_magics(bool recalculate)
{
    if (recalculate)
    {
        std::cout << "Recalculating magics...\n";
        run_magics(true);
        run_magics(false);
    }

    // Use precomputed magics, find the correct order of attacks
    uint64_t occ[4096], bishopAtt[512], rookAtt[4096];
    for (int sq = 0; sq < 64; sq++){
        // For the bishop
        memset(bishopAtt, 0, sizeof(bishopAtt));
        memset(rookAtt, 0, sizeof(rookAtt));
        memset(occ, 0, sizeof(occ));
        uint64_t mask = bishopMask(sq);
        int bits = pop_count(mask);
        
        // Populate the attacks
        for(int i = 0; i < (1 << bits); i++){
            occ[i] = index_occupied(i, bits, mask);
            bishopAtt[i] = bAttacks(sq, occ[i]);

            // Set the attack in the correct order
            auto index = (occ[i] * MagicBitboards::bishopMagics[sq].magic) >> MagicBitboards::bishopMagics[sq].shift;
            MagicBitboards::bishopAttacks[sq][index] = bishopAtt[i];
        }

        // For the rook
        memset(occ, 0, sizeof(occ));
        mask = rookMask(sq);
        bits = pop_count(mask);

        // Populate the attacks
        for(int i = 0; i < (1 << bits); i++){
            occ[i] = index_occupied(i, bits, mask);
            rookAtt[i] = rAttacks(sq, occ[i]);

            // Set the attack in the correct order
            auto index = (occ[i] * MagicBitboards::rookMagics[sq].magic) >> MagicBitboards::rookMagics[sq].shift;
            MagicBitboards::rookAttacks[sq][index] = rookAtt[i];
        }
    }
}