// This program calculates occupancy masks and magic numbers for sliding piece attacks.

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <map>
#include <random>
#include <utility>
#include <vector>
using u64 = uint64_t;

class MagicBitboard {
public:
  MagicBitboard(std::array<int, 64> found_shifts, std::array<u64, 64> found_magics,
                std::array<std::pair<int, int>, 4> directions)
      : found_shifts{found_shifts}, found_magics{found_magics}, directions{directions} {}

  // Calculate occupancy masks, moves for each occupancy, and magic numbers.
  void generate() {
    generate_masks();
    generate_moves();
    generate_magics();
  }

  // Output the shift and magic values if a better (larger) shift was found.
  void print() {
    std::vector<int> better_indexes;
    for (int i = 0; i < 64; i++) {
      if (found_shifts[i] == shifts[i]) continue;
      better_indexes.push_back(i);
    }

    if (!better_indexes.empty()) {
      std::cout << "Found better shifts for the following indexes: ";
      for (int index : better_indexes) std::cout << index << " ";
      std::cout << "\n\nShifts:\n";
      for (int i = 0; i < 64; i++) {
        std::cout << shifts[i];
        if (i < 63) std::cout << ", ";
      }
      std::cout << "\nMasks:\n";
      for (int i = 0; i < 64; i++) {
        std::cout << masks[i] << "U";
        if (i < 63) std::cout << ", ";
      }
      std::cout << "\nMagics:\n";
      for (int i = 0; i < 64; i++) {
        std::cout << magics[i] << "U";
        if (i < 63) std::cout << ", ";
      }
    } else {
      std::cout << "Failed to find any better shifts";
    }
  }

private:
  std::array<int, 64> found_shifts;
  std::array<u64, 64> found_magics;
  std::array<std::pair<int, int>, 4> directions;
  std::array<int, 64> shifts;
  std::array<u64, 64> magics;
  std::array<u64, 64> masks;
  std::map<u64, u64> moves_bitmap[64];

  // Generate occupancy masks.
  void generate_masks() {
    for (int index = 0; index < 64; index++) {
      masks[index] = 0;
      int y = index / 8;
      int x = index % 8;
      for (auto [delta_y, delta_x] : directions) {
        int to_y = y + delta_y;
        int to_x = x + delta_x;
        while (true) {
          int next_y = to_y + delta_y;
          int next_x = to_x + delta_x;
          if (next_y < 0 || next_y >= 8 || next_x < 0 || next_x >= 8) break;
          masks[index] ^= u64(1) << (to_y * 8 + to_x);
          to_y = next_y;
          to_x = next_x;
        }
      }
    }
  }

  void generate_moves() {
    // Pre-compute moves bitmap for all possible occupancies.
    for (int index = 0; index < 64; index++) {
      int y = index / 8;
      int x = index % 8;
      u64 mask = masks[index];
      for (u64 subset = mask;; subset = (subset - 1) & mask) {
        u64 move_bitmap = 0;
        for (auto [delta_y, delta_x] : directions) {
          int to_y = y + delta_y;
          int to_x = x + delta_x;
          while (0 <= to_y && to_y < 8 && 0 <= to_x && to_x < 8) {
            u64 to_bit = u64(1) << (to_y * 8 + to_x);
            move_bitmap ^= to_bit;
            if (to_bit & subset) break;
            to_y += delta_y;
            to_x += delta_x;
          }
        }
        moves_bitmap[index][subset] = move_bitmap;
        if (subset == 0) break;
      }
    }
  }

  // Check if the magic at the given index is valid.
  bool check_magic(int index) {
    std::vector<u64> moves(1 << (64 - shifts[index]), 0);
    for (auto [occupancy, move_bitmap] : moves_bitmap[index]) {
      u64 hash_index = (magics[index] * occupancy) >> shifts[index];
      if (moves[hash_index] == 0) {
        moves[hash_index] = move_bitmap;
      } else if (moves[hash_index] != move_bitmap) {
        return false;
      }
    }
    return true;
  }

  // Generate magic numbers.
  void generate_magics() {
    std::mt19937_64 rng{static_cast<unsigned long long>(std::time(NULL))};
    std::uniform_int_distribution<u64> distrib;

    for (int index = 0; index < 64; index++) {
      // Check that the provided magic is valid, in case we mess up the stored values.
      shifts[index] = found_shifts[index];
      magics[index] = found_magics[index];
      assert(check_magic(index));

      // Try to increase shift until we fail.
      while (true) {
        u64 prev_magic = magics[index];
        shifts[index]++;
        bool found = false;
        for (int attempt = 0; attempt < 1000000000; attempt++) {
          magics[index] = distrib(rng) & distrib(rng) & distrib(rng);
          if (check_magic(index)) {
            found = true;
            break;
          }
        }
        if (!found) {
          magics[index] = prev_magic;
          shifts[index]--;
          break;
        }
      }
    }
  }
};

int main() {
  std::array<int, 64> bishop_shifts{{58, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 59, 59,
                                     59, 59, 57, 57, 57, 57, 59, 59, 59, 59, 57, 55, 55, 57, 59, 59,
                                     59, 59, 57, 55, 55, 57, 59, 59, 59, 59, 57, 57, 57, 57, 59, 59,
                                     59, 59, 59, 59, 59, 59, 59, 59, 58, 59, 59, 59, 59, 59, 59, 58}};
  std::array<u64, 64> bishop_magics{
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

  MagicBitboard bishop_magic{bishop_shifts, bishop_magics, {{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}}};
  bishop_magic.generate();
  std::cout << "========== Bishop magic ==========\n";
  bishop_magic.print();
  std::cout << '\n';

  std::array<int, 64> rook_shifts{{52, 53, 53, 53, 53, 53, 53, 52, 53, 54, 54, 54, 54, 54, 54, 53,
                                   53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
                                   53, 54, 54, 54, 54, 54, 54, 53, 53, 54, 54, 54, 54, 54, 54, 53,
                                   53, 54, 54, 54, 54, 54, 54, 53, 52, 53, 53, 53, 53, 53, 53, 52}};
  std::array<u64, 64> rook_magics{
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

  MagicBitboard rook_magic{rook_shifts, rook_magics, {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}}};
  rook_magic.generate();
  std::cout << "========== Rook magic ==========\n";
  rook_magic.print();
  std::cout << '\n';
}