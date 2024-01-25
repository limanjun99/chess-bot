#include "bitboard.h"

#include "magic_bitboard.h"

// These values are hardcoded because:
// 1. Generating them requires random, but there is currently no compile time random in std.
// 2. It's a waste of time to keep regenerating them every time we run.
// 3. It will be much easier to update these values in the future if better magics are found, especially if done by
// long-running bruteforce.
MagicBitboard<55> bishop_magic{
    {{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}},
    {18049651735527936U, 70506452091904U,    275415828992U,      1075975168U,        38021120U,
     8657588224U,        2216338399232U,     567382630219776U,   9024825867763712U,  18049651735527424U,
     70506452221952U,    275449643008U,      9733406720U,        2216342585344U,     567382630203392U,
     1134765260406784U,  4512412933816832U,  9024825867633664U,  18049651768822272U, 70515108615168U,
     2491752130560U,     567383701868544U,   1134765256220672U,  2269530512441344U,  2256206450263040U,
     4512412900526080U,  9024834391117824U,  18051867805491712U, 637888545440768U,   1135039602493440U,
     2269529440784384U,  4539058881568768U,  1128098963916800U,  2256197927833600U,  4514594912477184U,
     9592139778506752U,  19184279556981248U, 2339762086609920U,  4538784537380864U,  9077569074761728U,
     562958610993152U,   1125917221986304U,  2814792987328512U,  5629586008178688U,  11259172008099840U,
     22518341868716544U, 9007336962655232U,  18014673925310464U, 2216338399232U,     4432676798464U,
     11064376819712U,    22137335185408U,    44272556441600U,    87995357200384U,    35253226045952U,
     70506452091904U,    567382630219776U,   1134765260406784U,  2832480465846272U,  5667157807464448U,
     11333774449049600U, 22526811443298304U, 9024825867763712U,  18049651735527936U},
    {41662763923148816U,    3647929751576162625U, 9223442990289137668U, 18062781324984320U,    11529251020991365120U,
     9513132969833808400U,  4627642287131754496U, 61641506948161U,      10412466516778913920U, 108121592609932312U,
     5188155850292330512U,  72136793505468417U,   144133880310431744U,  578856038988791880U,   9227897643905057168U,
     579277744044509184U,   9059980778938432U,    324331758122699520U,  45106674389753900U,    4756927107500613760U,
     2341946573644759104U,  9078154530400521U,    9225905311720677377U, 144119587213152260U,   5080018732581008U,
     35493685248772U,       5192659200827723872U, 1154056200674476040U, 2314995344582123520U,  4521741573439586U,
     13837336331960910592U, 9013281012383904U,    2306124625926439972U, 19793358913680U,       9326195703420928U,
     722854162831903232U,   9412524046912061698U, 3479048308693932032U, 1319576681320548384U,  1130300369306144U,
     72215924249461762U,    4611740995105595488U, 9295996979110758656U, 4503616809337856U,     36311375819314056U,
     74784008407136U,       2534391649798916U,    19703420237643840U,   72069761814511616U,    18016597868576768U,
     9148489518854305U,     10742669348U,         577187530596810760U,  2305848235920561184U,  5803170832364863824U,
     650209536959119488U,   576478366232780928U,  36028900400693312U,   2305848266623026304U,  144273534946942984U,
     9223442715040027148U,  90078625059051906U,   281917492842624U,     725721659648114704U}};

u64 bitboard::bishop_attacks(u64 bishop, u64 occupancy) { return bishop_magic.attacks(bishop, occupancy); }

consteval std::array<u64, 64> generate_king_lookup() {
  using namespace bitboard;
  std::array<u64, 64> king_lookup;
  for (int index = 0; index < 64; index++) {
    u64 king = u64(1) << index;
    king_lookup[index] = 0;
    king_lookup[index] |= (king & ~RANK_8 & ~FILE_H) << 9;  // Up right.
    king_lookup[index] |= (king & ~RANK_8) << 8;            // Up.
    king_lookup[index] |= (king & ~RANK_8 & ~FILE_A) << 7;  // Up left.
    king_lookup[index] |= (king & ~FILE_H) << 1;            // Right.
    king_lookup[index] |= (king & ~FILE_A) >> 1;            // Left.
    king_lookup[index] |= (king & ~RANK_1 & ~FILE_H) >> 7;  // Down right.
    king_lookup[index] |= (king & ~RANK_1) >> 8;            // Down.
    king_lookup[index] |= (king & ~RANK_1 & ~FILE_A) >> 9;  // Down left.
  }
  return king_lookup;
}
constexpr std::array<u64, 64> king_lookup = generate_king_lookup();

u64 bitboard::king_attacks(u64 king) { return king_lookup[bit::to_index(king)]; }

consteval std::array<u64, 64> generate_knight_lookup() {
  using namespace bitboard;
  std::array<u64, 64> knight_lookup;
  for (int index = 0; index < 64; index++) {
    u64 knight = u64(1) << index;
    knight_lookup[index] = 0;
    knight_lookup[index] |= (knight & ~RANK_8 & ~RANK_7 & ~FILE_H) << 17;  // Up up right.
    knight_lookup[index] |= (knight & ~RANK_8 & ~RANK_7 & ~FILE_A) << 15;  // Up up left.
    knight_lookup[index] |= (knight & ~RANK_8 & ~FILE_H & ~FILE_G) << 10;  // Up right right.
    knight_lookup[index] |= (knight & ~RANK_8 & ~FILE_A & ~FILE_B) << 6;   // Up left left.
    knight_lookup[index] |= (knight & ~RANK_1 & ~FILE_H & ~FILE_G) >> 6;   // Down right right.
    knight_lookup[index] |= (knight & ~RANK_1 & ~FILE_A & ~FILE_B) >> 10;  // Down left left.
    knight_lookup[index] |= (knight & ~RANK_1 & ~RANK_2 & ~FILE_H) >> 15;  // Down down right.
    knight_lookup[index] |= (knight & ~RANK_1 & ~RANK_2 & ~FILE_A) >> 17;  // Down down left.
  }
  return knight_lookup;
}
constexpr std::array<u64, 64> knight_lookup = generate_knight_lookup();

u64 bitboard::knight_attacks(u64 knight) { return knight_lookup[bit::to_index(knight)]; }

consteval std::array<std::array<u64, 64>, 2> generate_pawn_lookup() {
  using namespace bitboard;
  std::array<std::array<u64, 64>, 2> pawn_lookup;
  for (int is_white = 0; is_white < 2; is_white++) {
    for (int index = 0; index < 64; index++) {
      u64 pawn = u64(1) << index;
      u64 left = is_white ? pawn << 7 : pawn >> 9;
      u64 right = is_white ? pawn << 9 : pawn >> 7;
      pawn_lookup[is_white][index] = (left & ~FILE_H) | (right & ~FILE_A);
    }
  }
  return pawn_lookup;
}
constexpr std::array<std::array<u64, 64>, 2> pawn_lookup = generate_pawn_lookup();

u64 bitboard::pawn_attacks(u64 pawn, bool is_white) { return pawn_lookup[is_white][bit::to_index(pawn)]; }

u64 bitboard::pawn_pushes(u64 pawn, u64 occupancy, bool is_white) {
  if (is_white) return ((((pawn & bitboard::RANK_2) << 8 & ~occupancy) | pawn) << 8) & ~occupancy;
  return ((((pawn & bitboard::RANK_7) >> 8 & ~occupancy) | pawn) >> 8) & ~occupancy;
}

MagicBitboard<52> rook_magic{
    {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}},
    {282578800148862U,     565157600297596U,     1130315200595066U,    2260630401190006U,    4521260802379886U,
     9042521604759646U,    18085043209519166U,   36170086419038334U,   282578800180736U,     565157600328704U,
     1130315200625152U,    2260630401218048U,    4521260802403840U,    9042521604775424U,    18085043209518592U,
     36170086419037696U,   282578808340736U,     565157608292864U,     1130315208328192U,    2260630408398848U,
     4521260808540160U,    9042521608822784U,    18085043209388032U,   36170086418907136U,   282580897300736U,
     565159647117824U,     1130317180306432U,    2260632246683648U,    4521262379438080U,    9042522644946944U,
     18085043175964672U,   36170086385483776U,   283115671060736U,     565681586307584U,     1130822006735872U,
     2261102847592448U,    4521664529305600U,    9042787892731904U,    18085034619584512U,   36170077829103616U,
     420017753620736U,     699298018886144U,     1260057572672512U,    2381576680245248U,    4624614895390720U,
     9110691325681664U,    18082844186263552U,   36167887395782656U,   35466950888980736U,   34905104758997504U,
     34344362452452352U,   33222877839362048U,   30979908613181440U,   26493970160820224U,   17522093256097792U,
     35607136465616896U,   9079539427579068672U, 8935706818303361536U, 8792156787827803136U, 8505056726876686336U,
     7930856604974452736U, 6782456361169985536U, 4485655873561051136U, 9115426935197958144U},
    {36033745362354192U,   18014467231055940U,    2328365681885581568U, 1297041228973615168U, 13907116750039704100U,
     1450231648334643202U, 108088590088601664U,   4791835501082462336U, 9260527010873147909U, 648589129554067472U,
     288283153249075346U,  4640977079075112960U,  180214362584122496U,  4625337572011311169U, 2882374688609140992U,
     691654389779136656U,  36028934460018689U,    297809356022353928U,  2305878472759732802U, 27026614319726608U,
     3458907450634143745U, 9295430799136396288U,  9448103427457600U,    289356585312979080U,  9174911285313537U,
     9552137287983177752U, 22519133618839616U,    434812877984106240U,  144196570727055624U,  5699902907219969U,
     577588851537643776U,  279173537930U,         2310347729295377408U, 1302402447020654672U, 6953848645619943424U,
     18194718953390162U,   4665730313535095872U,  1266775103144002U,    603554918917210368U,  71470404601122U,
     571748462497792U,     1164743591076168708U,  2390285536605511680U, 648887791107444752U,  35192963089408U,
     9320325187633184900U, 9369752519536017922U,  2288152484118656U,    1549379010650186816U, 1161946450834489384U,
     288243587542959112U,  10543086225581408264U, 9015997495314448U,    612771032956141704U,  586595088449159296U,
     9232767096193155328U, 140738713150097U,      1229693805058359627U, 2309432983934756997U, 103591862677610562U,
     1787511964162U,       2305922451143393537U,  17595440842756U,      108090868587578114U}};

u64 bitboard::rook_attacks(u64 rook, u64 occupancy) { return rook_magic.attacks(rook, occupancy); }

u64 bitboard::queen_attacks(u64 queen, u64 occupancy) {
  return bishop_magic.attacks(queen, occupancy) | rook_magic.attacks(queen, occupancy);
}

// Generate bitboards of squares that block an attacker from checking the king.
consteval std::array<std::array<u64, 64>, 64> generate_block_checks(
    const std::initializer_list<std::pair<int, int>> directions) {
  std::array<std::array<u64, 64>, 64> block_checks;
  for (int king_index = 0; king_index < 64; king_index++) {
    const int king_y = king_index / 8;
    const int king_x = king_index % 8;
    for (int attacker_index = 0; attacker_index < 64; attacker_index++) {
      block_checks[king_index][attacker_index] = 0;
      for (const auto& [delta_y, delta_x] : directions) {
        bool is_correct_direction = false;
        int y = attacker_index / 8 + delta_y;
        int x = attacker_index % 8 + delta_x;
        while (0 <= y && y < 8 && 0 <= x && x < 8) {
          if (y == king_y && x == king_x) {
            is_correct_direction = true;
            break;
          }
          block_checks[king_index][attacker_index] |= u64(1) << (y * 8 + x);
          y += delta_y;
          x += delta_x;
        }
        if (!is_correct_direction) {
          block_checks[king_index][attacker_index] = 0;
        } else {
          break;
        }
      }
    }
  }
  return block_checks;
}
constexpr std::array<std::array<u64, 64>, 64> block_bishop_checks =
    generate_block_checks({{-1, -1}, {-1, 1}, {1, -1}, {1, 1}});
constexpr std::array<std::array<u64, 64>, 64> block_rook_checks =
    generate_block_checks({{-1, 0}, {1, 0}, {0, -1}, {0, 1}});

u64 bitboard::block_slider_check(u64 king, u64 slider) {
  size_t king_index = bit::to_index(king);
  size_t slider_index = bit::to_index(slider);
  return block_bishop_checks[king_index][slider_index] | block_rook_checks[king_index][slider_index];
}

std::string bitboard::to_string(u64 bitboard) {
  std::string s;
  s.reserve(8 * 8 + 7);
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      s += (bitboard & (u64(1) << (y * 8 + x))) ? '1' : '0';
    }
    if (y > 0) s += '\n';
  }
  return s;
}

u64 bit::from_algebraic(std::string_view algebraic) {
  int x = static_cast<int>(algebraic[0] - 'a');
  int y = static_cast<int>(algebraic[1] - '1');
  return u64(1) << (y * 8 + x);
}

std::string bit::to_algebraic(u64 bit) {
  std::string algebraic;
  int index = bit::to_index(bit);
  int rank = index / 8;
  int file = index % 8;
  algebraic += static_cast<char>(file + 'a');
  algebraic += static_cast<char>(rank + '1');
  return algebraic;
}