#pragma once

#include <cstdint>
#include <string>

using u64 = uint64_t;

namespace bitboard {
constexpr u64 A1 = u64(1) << 0;
constexpr u64 B1 = u64(1) << 1;
constexpr u64 C1 = u64(1) << 2;
constexpr u64 D1 = u64(1) << 3;
constexpr u64 E1 = u64(1) << 4;
constexpr u64 F1 = u64(1) << 5;
constexpr u64 G1 = u64(1) << 6;
constexpr u64 H1 = u64(1) << 7;
constexpr u64 A2 = u64(1) << 8;
constexpr u64 B2 = u64(1) << 9;
constexpr u64 C2 = u64(1) << 10;
constexpr u64 D2 = u64(1) << 11;
constexpr u64 E2 = u64(1) << 12;
constexpr u64 F2 = u64(1) << 13;
constexpr u64 G2 = u64(1) << 14;
constexpr u64 H2 = u64(1) << 15;
constexpr u64 A3 = u64(1) << 16;
constexpr u64 B3 = u64(1) << 17;
constexpr u64 C3 = u64(1) << 18;
constexpr u64 D3 = u64(1) << 19;
constexpr u64 E3 = u64(1) << 20;
constexpr u64 F3 = u64(1) << 21;
constexpr u64 G3 = u64(1) << 22;
constexpr u64 H3 = u64(1) << 23;
constexpr u64 A4 = u64(1) << 24;
constexpr u64 B4 = u64(1) << 25;
constexpr u64 C4 = u64(1) << 26;
constexpr u64 D4 = u64(1) << 27;
constexpr u64 E4 = u64(1) << 28;
constexpr u64 F4 = u64(1) << 29;
constexpr u64 G4 = u64(1) << 30;
constexpr u64 H4 = u64(1) << 31;
constexpr u64 A5 = u64(1) << 32;
constexpr u64 B5 = u64(1) << 33;
constexpr u64 C5 = u64(1) << 34;
constexpr u64 D5 = u64(1) << 35;
constexpr u64 E5 = u64(1) << 36;
constexpr u64 F5 = u64(1) << 37;
constexpr u64 G5 = u64(1) << 38;
constexpr u64 H5 = u64(1) << 39;
constexpr u64 A6 = u64(1) << 40;
constexpr u64 B6 = u64(1) << 41;
constexpr u64 C6 = u64(1) << 42;
constexpr u64 D6 = u64(1) << 43;
constexpr u64 E6 = u64(1) << 44;
constexpr u64 F6 = u64(1) << 45;
constexpr u64 G6 = u64(1) << 46;
constexpr u64 H6 = u64(1) << 47;
constexpr u64 A7 = u64(1) << 48;
constexpr u64 B7 = u64(1) << 49;
constexpr u64 C7 = u64(1) << 50;
constexpr u64 D7 = u64(1) << 51;
constexpr u64 E7 = u64(1) << 52;
constexpr u64 F7 = u64(1) << 53;
constexpr u64 G7 = u64(1) << 54;
constexpr u64 H7 = u64(1) << 55;
constexpr u64 A8 = u64(1) << 56;
constexpr u64 B8 = u64(1) << 57;
constexpr u64 C8 = u64(1) << 58;
constexpr u64 D8 = u64(1) << 59;
constexpr u64 E8 = u64(1) << 60;
constexpr u64 F8 = u64(1) << 61;
constexpr u64 G8 = u64(1) << 62;
constexpr u64 H8 = u64(1) << 63;

constexpr u64 RANK_1 = u64(0b11111111);
constexpr u64 RANK_2 = RANK_1 << 8;
constexpr u64 RANK_3 = RANK_1 << 16;
constexpr u64 RANK_4 = RANK_1 << 24;
constexpr u64 RANK_5 = RANK_1 << 32;
constexpr u64 RANK_6 = RANK_1 << 40;
constexpr u64 RANK_7 = RANK_1 << 48;
constexpr u64 RANK_8 = RANK_1 << 56;

constexpr u64 FILE_A = A1 | A2 | A3 | A4 | A5 | A6 | A7 | A8;
constexpr u64 FILE_B = FILE_A << 1;
constexpr u64 FILE_C = FILE_A << 2;
constexpr u64 FILE_D = FILE_A << 3;
constexpr u64 FILE_E = FILE_A << 4;
constexpr u64 FILE_F = FILE_A << 5;
constexpr u64 FILE_G = FILE_A << 6;
constexpr u64 FILE_H = FILE_A << 7;

// Returns a bitboard of all attacked squares by the bishop.
u64 bishop_attacks(u64 bishop, u64 occupancy);

// Returns a bitboard of all attacked squares by the king.
u64 king_attacks(u64 king);

// Returns a bitboard of all attacked squares by the knight.
u64 knight_attacks(u64 knight);

// Returns a bitboard of all attacked squares by the pawn.
u64 pawn_attacks(u64 pawn, bool is_white);

// Returns a bitboard of all attacked squares by the queen.
u64 queen_attacks(u64 queen, u64 occupancy);

// Returns a bitboard of all attacked squares by the rook.
u64 rook_attacks(u64 rook, u64 occupancy);

// Counts the number of set bits in the given bitboard.
int count(u64 bitboard);

// A utility function to convert bitboards into a 8x8 binary string for visualization.
std::string to_string(u64 bitboard);
}  // namespace bitboard

namespace bit {
// Given a square in algebraic notation (e.g. 'a1'), return a bitboard with that bit set.
u64 from_algebraic(std::string_view algebraic);

// Given a square, return the algebraic notation (e.g. 'a1') of it.
std::string to_algebraic(u64 bit);

// Returns the index of a single set bit.
int to_index(u64 bit);

// Returns the least significant bit.
u64 lsb(u64 bitboard);
}  // namespace bit