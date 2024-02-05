#include "bitboard.h"

#include "magic_bitboard.h"

// These values are hardcoded because:
// 1. Generating them requires random, but there is currently no compile time random in std.
// 2. It's a waste of time to keep regenerating them every time we run.
// 3. It will be much easier to update these values in the future if better magics are found, especially if done by
// long-running bruteforce.
MagicBitboard bishop_magic{
    {{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}},
    {58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 57, 57, 57, 57,
     59, 59, 59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57,
     57, 57, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 58},
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
    {9196383984746768U,     9297753800810515U,     162696952847663200U,   2308117353311789064U, 4614017259063742468U,
     321402066468864U,      1297323670068277768U,  708088039016452U,      355837559470851584U,  2884590750199382336U,
     342346156644278304U,   164392658691719220U,   37296603724251649U,    641765180854248516U,  144139386191484928U,
     2594078063131103232U,  6919780982345224705U,  1166467625401585728U,  40537103935213600U,   147528080926310400U,
     73184877194903744U,    903044327549699072U,   5197435451414188052U,  2054002072315496449U, 154252823260303360U,
     22526796650643584U,    13835242784241500480U, 594492777392275536U,   4800839401837973504U, 12105823134758752768U,
     9224499595223126273U,  7320548556211330U,     434773320262487553U,   4615068530658902528U, 7066183206278004864U,
     565151124685186U,      147493489092199680U,   1252036166397001892U,  289365073630204176U,  576604797990405129U,
     284842500932016U,      37155916799283329U,    1230045734158795784U,  6052978920276451458U, 2261149961815042U,
     11534923779172491520U, 92324909119705476U,    14268566115776465408U, 1137174465003524U,    1155362438172380164U,
     18577900437569536U,    12252042881040384000U, 145241159554695176U,   8865080967168U,       45038272740786690U,
     2254033230242832U,     18577906818260992U,    18702571997248U,       9296625902832722192U, 290271222859776U,
     1442278368674325521U,  297802861889397264U,   292752255194763332U,   2352154507699323140U}};

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

MagicBitboard rook_magic{
    {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}},
    {52, 53, 53, 53, 53, 53, 53, 52, 53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54,
     54, 53, 53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54,
     54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52},
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
    {108086491992817698U,   9241395368935030848U, 36037730551989632U,   5008020379970043940U, 2377902819589029904U,
     108087499165532288U,   648519445928607772U,  36029514278715520U,   4974929513453486128U, 70369820020736U,
     11530481820911013888U, 8361073995591733248U, 149885665621049472U,  563018808525312U,     13835621009608679936U,
     5188709738409378562U,  1307312178603245568U, 1157425379114221770U, 36311371780071425U,   72093878139752448U,
     3459047088913946624U,  141287311312384U,     1763616785248322U,    603519733472165953U,  4629841362731712512U,
     9270941896333590689U,  1144887959355649U,    9728771361246044224U, 144119588270375040U,  6918091986189157392U,
     2306569837939394576U,  2594073703193089025U, 4629770787836920096U, 81099979820646400U,   13835199411270983684U,
     140806216222720U,      578712569305368704U,  4612952673010844161U, 9835862722263126544U, 282029061050436U,
     211108398923784U,      576495938017705984U,  184790521504464960U,  288529443868180488U,  2315414464629440516U,
     2306406543350431760U,  1246373430811689104U, 54628690906185737U,   5199687794533878016U, 5206304132597613056U,
     325528009868068352U,   1189126567100419328U, 1459307033936790144U, 145241096576892992U,  9511604646407308288U,
     1271177184037376U,     144255929860579393U,  18647717483929857U,   9033724977221889U,    92333705699266561U,
     375487688188920898U,   725361169607034881U,  8864837800196U,       2324420908523201538U}};

u64 bitboard::rook_attacks(u64 rook, u64 occupancy) { return rook_magic.attacks(rook, occupancy); }

u64 bitboard::queen_attacks(u64 queen, u64 occupancy) {
  return bishop_magic.attacks(queen, occupancy) | rook_magic.attacks(queen, occupancy);
}

namespace {
constexpr std::array<std::array<u64, 64>, 64> between_array = []() {
  std::array<std::array<u64, 64>, 64> between_array{{{0}}};
  std::array<std::pair<int, int>, 8> directions{{{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};
  for (int from_index = 0; from_index < 64; from_index++) {
    const int from_y = from_index / 8;
    const int from_x = from_index % 8;
    for (int distance = 2; distance < 8; distance++) {
      for (auto [delta_y, delta_x] : directions) {
        const int to_y = from_y + distance * delta_y;
        const int to_x = from_x + distance * delta_x;
        const int to_index = to_y * 8 + to_x;
        if (to_x < 0 || to_x >= 8 || to_y < 0 || to_y >= 8) continue;
        const int prev_y = to_y - delta_y;
        const int prev_x = to_x - delta_x;
        const int prev_index = prev_y * 8 + prev_x;
        between_array[from_index][to_index] = between_array[from_index][prev_index] | u64(1) << prev_index;
      }
    }
  }
  return between_array;
}();
}  // namespace
u64 bitboard::between(u64 from, u64 to) {
  const int from_index = bit::to_index(from);
  const int to_index = bit::to_index(to);
  return between_array[from_index][to_index];
}

namespace {
constexpr std::array<std::array<u64, 64>, 64> beyond_array = []() {
  std::array<std::array<u64, 64>, 64> beyond_array{{{0}}};
  std::array<std::pair<int, int>, 8> directions{{{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};
  for (int from_index = 0; from_index < 64; from_index++) {
    const int from_y = from_index / 8;
    const int from_x = from_index % 8;
    for (int distance = 7; distance > 0; distance--) {
      for (auto [delta_y, delta_x] : directions) {
        const int to_y = from_y + distance * delta_y;
        const int to_x = from_x + distance * delta_x;
        const int to_index = to_y * 8 + to_x;
        const int prev_y = to_y + delta_y;
        const int prev_x = to_x + delta_x;
        const int prev_index = prev_y * 8 + prev_x;
        if (prev_x < 0 || prev_x >= 8 || prev_y < 0 || prev_y >= 8) continue;
        beyond_array[from_index][to_index] = beyond_array[from_index][prev_index] | u64(1) << prev_index;
      }
    }
  }
  return beyond_array;
}();
}  // namespace
u64 bitboard::beyond(u64 from, u64 to) {
  const int from_index = bit::to_index(from);
  const int to_index = bit::to_index(to);
  return beyond_array[from_index][to_index];
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