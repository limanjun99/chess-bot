#include "chess/bitboard.h"

#include <iostream>

#include "doctest.h"

TEST_CASE("bishop attacks are correct on empty board") {
  using namespace bitboard;
  u64 bishop = D4;
  u64 occupancy = 0;
  u64 attacks = bishop_attacks(bishop, occupancy);
  u64 expected_attacks = A1 | B2 | C3 | E3 | F2 | G1 | C5 | B6 | A7 | E5 | F6 | G7 | H8;
  REQUIRE(attacks == expected_attacks);
}

TEST_CASE("bishop attacks are correct with a few blockers") {
  using namespace bitboard;
  u64 bishop = H5;
  u64 occupancy = F3 | E8;
  u64 attacks = bishop_attacks(bishop, occupancy);
  u64 expected_attacks = G4 | F3 | G6 | F7 | E8;
  REQUIRE(attacks == expected_attacks);
}

TEST_CASE("bishop attacks are correct when fully blocked") {
  using namespace bitboard;
  u64 bishop = A1;
  u64 occupancy = B2 | H8;
  u64 attacks = bishop_attacks(bishop, occupancy);
  u64 expected_attacks = B2;
  REQUIRE(attacks == expected_attacks);
}

TEST_CASE("rooks attacks are correct on empty board") {
  using namespace bitboard;
  u64 rook = E5;
  u64 occupancy = 0;
  u64 attacks = rook_attacks(rook, occupancy);
  u64 expected_attacks = A5 | B5 | C5 | D5 | F5 | G5 | H5 | E1 | E2 | E3 | E4 | E6 | E7 | E8;
  REQUIRE(attacks == expected_attacks);
}

TEST_CASE("rook attacks are correct with a few blockers") {
  using namespace bitboard;
  u64 rook = B8;
  u64 occupancy = A8 | B5 | B8;
  u64 attacks = rook_attacks(rook, occupancy);
  u64 expected_attacks = A8 | B5 | B6 | B7 | C8 | D8 | E8 | F8 | G8 | H8;
  REQUIRE(attacks == expected_attacks);
}

TEST_CASE("rook attacks are correct when fully blocked") {
  using namespace bitboard;
  u64 rook = H1;
  u64 occupancy = G1 | H2 | B1;
  u64 attacks = rook_attacks(rook, occupancy);
  u64 expected_attacks = G1 | H2;
  REQUIRE(attacks == expected_attacks);
}