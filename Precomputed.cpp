#include "Precomputed.h"
#include "Types.h"
#include "Bitboard.h"

#include <array>

namespace Precomputed {
    uint64_t knightAttacks[64];
    uint64_t kingAttacks[64];
    uint64_t pawnAttacks[2][64];
    uint64_t bishopMasks[64];
    uint64_t rookMasks[64];

    namespace {
        std::array<uint64_t, 64> bishopMagics = {
            4701830649676366144ULL, 9150206650159108ULL, 2490496099296641094ULL, 1299297567764382726ULL,
            288529724635480064ULL, 14082553654280192ULL, 72621162601514016ULL, 4611709417476595776ULL,
            297307953885872273ULL, 145258748921790545ULL, 3098485342425186372ULL, 149569568769024ULL,
            1169327939586ULL, 36600715804148802ULL, 18036391024005152ULL, 4505824454067352ULL,
            1451922767598848032ULL, 4503737085329536ULL, 1441715106730754176ULL, 36170393534873600ULL,
            54606162705908352ULL, 2307602391163208738ULL, 4612816336345698306ULL, 13871686122722560004ULL,
            297272777026114056ULL, 1157469420311281792ULL, 9800976281285173570ULL, 1154048504564287616ULL,
            281546061791240ULL, 9223532565636391041ULL, 288517490761011200ULL, 11535413131635365888ULL,
            1178693649106068ULL, 1127033780314816ULL, 9295713584619718656ULL, 4614784444346663040ULL,
            18014677682884864ULL, 9367769808009169408ULL, 4612953214169645312ULL, 576744428484437024ULL,
            148917889294934016ULL, 4611758623842918400ULL, 563019880989728ULL, 4644474840029192ULL,
            576462959950201872ULL, 4688265010642718752ULL, 585476751951446280ULL, 432918002334171905ULL,
            4756372961274105856ULL, 5188191885201772544ULL, 12105676081974215714ULL, 35184917872704ULL,
            16181433538485291520ULL, 18005056620556ULL, 5190592377856999424ULL, 40552328532723840ULL,
            4616472776727923840ULL, 7882416073016586ULL, 1731634057802158188ULL, 2318790926878935040ULL,
            20274996312146464ULL, 576566879066527808ULL, 9414635455488ULL, 11530393765489672320ULL
        };

        std::array<uint64_t, 64> rookMagics = {
            1333086105553076448ULL, 162129724028489729ULL, 4683778797927927816ULL, 9403533614269956100ULL,
            2341880602359496832ULL, 1369095386299400712ULL, 288232612371108872ULL, 144115326756425476ULL,
            1189091040187940896ULL, 211174956212225ULL, 36170084531576832ULL, 290622947821883392ULL,
            4644371492503680ULL, 5189413425314284032ULL, 217439475351945728ULL, 72620561179672682ULL,
            9403520695411425280ULL, 297238400308625441ULL, 2310346746347651104ULL, 18050682661636352ULL,
            2252074758832193ULL, 1126451810402816ULL, 2026624232248640008ULL, 1162106824750563588ULL,
            72268715302879520ULL, 4612002688517800064ULL, 144132782411481216ULL, 70446054121984ULL,
            13847451752498200704ULL, 594479551007031424ULL, 2378463829165080832ULL, 18585706469277956ULL,
            70386536415872ULL, 5197171562712662080ULL, 5769111260108953728ULL, 10403605412468232194ULL,
            2344692053843118080ULL, 562984548053140ULL, 11533720303620337672ULL, 2305887540541920257ULL,
            4612110155040063488ULL, 144185694527438848ULL, 9477986520028807201ULL, 4611722302579605514ULL,
            2377909399629922308ULL, 1153486653650665600ULL, 577023736683823105ULL, 9799907625751150593ULL,
            73474590461919488ULL, 4611756389322260736ULL, 13545983522930816ULL, 2305983781062836352ULL,
            72061992218689664ULL, 9570157865238656ULL, 74311599051523072ULL, 1153204081309794816ULL,
            1276538394780162ULL, 72207200635977794ULL, 72075323679705153ULL, 4902173692488323073ULL,
            563536285667330ULL, 2306405978767165442ULL, 9008608013451780ULL, 72066529734444034ULL
        };

        std::array<int, 64> bishopBits = {
            6,5,5,5,5,5,5,6, 5,5,5,5,5,5,5,5,
            5,5,7,7,7,7,5,5, 5,5,7,9,9,7,5,5,
            5,5,7,9,9,7,5,5, 5,5,7,7,7,7,5,5,
            5,5,5,5,5,5,5,5, 6,5,5,5,5,5,5,6
        };

        std::array<int, 64> rookBits = {
            12,11,11,11,11,11,11,12, 11,10,10,10,10,10,10,11,
            11,10,10,10,10,10,10,11, 11,10,10,10,10,10,10,11,
            11,10,10,10,10,10,10,11, 11,10,10,10,10,10,10,11,
            11,10,10,10,10,10,10,11, 12,11,11,11,11,11,11,12
        };

        std::array<std::array<uint64_t, 512>, 64> bishopTable{};
        std::array<std::array<uint64_t, 4096>, 64> rookTable{};

        static bool inb(int f, int r) { return f >= 0 && f < 8 && r >= 0 && r < 8; }

        uint64_t maskBishop(int sq) {
            uint64_t m = 0ULL;
            int f = sq_file(sq), r = sq_rank(sq);
            for (int ff = f + 1, rr = r + 1; ff <= 6 && rr <= 6; ++ff, ++rr) m |= bb_of(make_sq(ff, rr));
            for (int ff = f - 1, rr = r + 1; ff >= 1 && rr <= 6; --ff, ++rr) m |= bb_of(make_sq(ff, rr));
            for (int ff = f + 1, rr = r - 1; ff <= 6 && rr >= 1; ++ff, --rr) m |= bb_of(make_sq(ff, rr));
            for (int ff = f - 1, rr = r - 1; ff >= 1 && rr >= 1; --ff, --rr) m |= bb_of(make_sq(ff, rr));
            return m;
        }

        uint64_t maskRook(int sq) {
            uint64_t m = 0ULL;
            int f = sq_file(sq), r = sq_rank(sq);
            for (int rr = r + 1; rr <= 6; ++rr) m |= bb_of(make_sq(f, rr));
            for (int rr = r - 1; rr >= 1; --rr) m |= bb_of(make_sq(f, rr));
            for (int ff = f + 1; ff <= 6; ++ff) m |= bb_of(make_sq(ff, r));
            for (int ff = f - 1; ff >= 1; --ff) m |= bb_of(make_sq(ff, r));
            return m;
        }

        uint64_t bishopAttackSlow(int sq, uint64_t occ) {
            uint64_t a = 0ULL;
            int f = sq_file(sq), r = sq_rank(sq);
            for (int ff = f + 1, rr = r + 1; ff < 8 && rr < 8; ++ff, ++rr) { int to = make_sq(ff, rr); a |= bb_of(to); if (occ & bb_of(to)) break; }
            for (int ff = f - 1, rr = r + 1; ff >= 0 && rr < 8; --ff, ++rr) { int to = make_sq(ff, rr); a |= bb_of(to); if (occ & bb_of(to)) break; }
            for (int ff = f + 1, rr = r - 1; ff < 8 && rr >= 0; ++ff, --rr) { int to = make_sq(ff, rr); a |= bb_of(to); if (occ & bb_of(to)) break; }
            for (int ff = f - 1, rr = r - 1; ff >= 0 && rr >= 0; --ff, --rr) { int to = make_sq(ff, rr); a |= bb_of(to); if (occ & bb_of(to)) break; }
            return a;
        }

        uint64_t rookAttackSlow(int sq, uint64_t occ) {
            uint64_t a = 0ULL;
            int f = sq_file(sq), r = sq_rank(sq);
            for (int rr = r + 1; rr < 8; ++rr) { int to = make_sq(f, rr); a |= bb_of(to); if (occ & bb_of(to)) break; }
            for (int rr = r - 1; rr >= 0; --rr) { int to = make_sq(f, rr); a |= bb_of(to); if (occ & bb_of(to)) break; }
            for (int ff = f + 1; ff < 8; ++ff) { int to = make_sq(ff, r); a |= bb_of(to); if (occ & bb_of(to)) break; }
            for (int ff = f - 1; ff >= 0; --ff) { int to = make_sq(ff, r); a |= bb_of(to); if (occ & bb_of(to)) break; }
            return a;
        }

        uint64_t setOccupancy(int index, int bits, uint64_t mask) {
            uint64_t occ = 0ULL;
            for (int i = 0; i < bits; ++i) {
                int sq = pop_lsb(mask);
                if (index & (1 << i)) occ |= bb_of(sq);
            }
            return occ;
        }
    }

    uint64_t bishopAttacks(int sq, uint64_t occ) {
        uint64_t blockers = occ & bishopMasks[sq];
        uint64_t index = (blockers * bishopMagics[sq]) >> (64 - bishopBits[sq]);
        return bishopTable[sq][index];
    }

    uint64_t rookAttacks(int sq, uint64_t occ) {
        uint64_t blockers = occ & rookMasks[sq];
        uint64_t index = (blockers * rookMagics[sq]) >> (64 - rookBits[sq]);
        return rookTable[sq][index];
    }

    void init() {
        static bool initialized = false;
        if (initialized) return;

        for (int sq = 0; sq < 64; ++sq) {
            int f = sq_file(sq), r = sq_rank(sq);
            uint64_t k = 0, n = 0, pw = 0, pb = 0;
            const int kd[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
            const int nd[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
            for (auto &d : kd) if (inb(f+d[0], r+d[1])) k |= bb_of(make_sq(f+d[0], r+d[1]));
            for (auto &d : nd) if (inb(f+d[0], r+d[1])) n |= bb_of(make_sq(f+d[0], r+d[1]));
            if (inb(f-1, r+1)) pw |= bb_of(make_sq(f-1, r+1));
            if (inb(f+1, r+1)) pw |= bb_of(make_sq(f+1, r+1));
            if (inb(f-1, r-1)) pb |= bb_of(make_sq(f-1, r-1));
            if (inb(f+1, r-1)) pb |= bb_of(make_sq(f+1, r-1));
            kingAttacks[sq] = k;
            knightAttacks[sq] = n;
            pawnAttacks[WHITE][sq] = pw;
            pawnAttacks[BLACK][sq] = pb;

            bishopMasks[sq] = maskBishop(sq);
            rookMasks[sq] = maskRook(sq);
        }

        for (int sq = 0; sq < 64; ++sq) {
            int bCount = 1 << bishopBits[sq];
            for (int i = 0; i < bCount; ++i) {
                uint64_t occ = setOccupancy(i, bishopBits[sq], bishopMasks[sq]);
                uint64_t idx = (occ * bishopMagics[sq]) >> (64 - bishopBits[sq]);
                bishopTable[sq][idx] = bishopAttackSlow(sq, occ);
            }

            int rCount = 1 << rookBits[sq];
            for (int i = 0; i < rCount; ++i) {
                uint64_t occ = setOccupancy(i, rookBits[sq], rookMasks[sq]);
                uint64_t idx = (occ * rookMagics[sq]) >> (64 - rookBits[sq]);
                rookTable[sq][idx] = rookAttackSlow(sq, occ);
            }
        }

        initialized = true;
    }
}
