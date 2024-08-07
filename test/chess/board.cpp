#include "chess/board.h"

#include <doctest/doctest.h>

#include <iostream>

#include "chess/stack_repetition_tracker.h"
#include "chess/uci.h"

using namespace chess;

TEST_SUITE("board.from_fen") {
  TEST_CASE("castling rights") {
    auto board = Board::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0");
    REQUIRE(board.get_player<Color::White>().can_castle_kingside());
    REQUIRE(board.get_player<Color::White>().can_castle_queenside());
    REQUIRE(board.get_player<Color::Black>().can_castle_kingside());
    REQUIRE(board.get_player<Color::Black>().can_castle_queenside());
    REQUIRE(!board.get_en_passant());
  }

  TEST_CASE("no castling rights") {
    auto board = Board::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 0");
    REQUIRE(!board.get_player<Color::White>().can_castle_kingside());
    REQUIRE(!board.get_player<Color::White>().can_castle_queenside());
    REQUIRE(!board.get_player<Color::Black>().can_castle_kingside());
    REQUIRE(!board.get_player<Color::Black>().can_castle_queenside());
    REQUIRE(!board.get_en_passant());
  }

  TEST_CASE("en passant with castling") {
    auto board = Board::from_fen("rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 0");
    REQUIRE(board.get_player<Color::White>().can_castle_kingside());
    REQUIRE(board.get_player<Color::White>().can_castle_queenside());
    REQUIRE(board.get_player<Color::Black>().can_castle_kingside());
    REQUIRE(board.get_player<Color::Black>().can_castle_queenside());
    REQUIRE(board.get_en_passant() == Bitboard::D6);
  }

  TEST_CASE("en passant without castling") {
    auto board = Board::from_fen("rnbqkb1r/ppp1pppp/5n2/3pP3/8/8/PPPP1PPP/RNBQKBNR w - d6 0 0");
    REQUIRE(!board.get_player<Color::White>().can_castle_kingside());
    REQUIRE(!board.get_player<Color::White>().can_castle_queenside());
    REQUIRE(!board.get_player<Color::Black>().can_castle_kingside());
    REQUIRE(!board.get_player<Color::Black>().can_castle_queenside());
    REQUIRE(board.get_en_passant() == Bitboard::D6);
  }
}

TEST_SUITE("board.get_hash") {
  TEST_CASE("equal boards have equal hashes") {
    auto board1{Board::initial()};
    auto board2{Board::initial()};
    REQUIRE(board1.get_hash() == board2.get_hash());
  }

  TEST_CASE("boards with different en passant bits have different hashes") {
    auto en_passant_board{Board::from_fen("rnbqkbnr/ppppp1pp/8/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 0")};
    auto no_en_passant_board{Board::from_fen("rnbqkbnr/ppppp1pp/8/4Pp2/8/8/PPPP1PPP/RNBQKBNR w KQkq - 0 0")};
    REQUIRE(en_passant_board.get_hash() != no_en_passant_board.get_hash());
  }

  TEST_CASE("boards with different castling rights have different hashes") {
    auto can_castle_board{Board::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0")};
    auto cannot_castle_board{Board::from_fen("rnbqkbnr/ppppp1pp/8/4Pp2/8/8/PPPP1PPP/RNBQKBNR w - - 0 0")};
    REQUIRE(can_castle_board.get_hash() != cannot_castle_board.get_hash());
  }

  TEST_CASE("boards with different player's turn have different hashes") {
    auto white_turn_board{Board::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 0")};
    auto black_turn_board{Board::from_fen("rnbqkbnr/ppppp1pp/8/4Pp2/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 0")};
    REQUIRE(white_turn_board.get_hash() != black_turn_board.get_hash());
  }
}

TEST_SUITE("board.get_score") {
  TEST_CASE("draw by repetition") {
    auto board{Board::initial()};
    StackRepetitionTracker tracker{};
    tracker.push(board);
    board = board.apply_move(uci::move("b1c3", board));
    tracker.push(board);
    REQUIRE(!board.get_score(tracker).has_value());
    board = board.apply_move(uci::move("b8c6", board));
    tracker.push(board);
    REQUIRE(!board.get_score(tracker).has_value());
    board = board.apply_move(uci::move("c3b1", board));
    tracker.push(board);
    REQUIRE(!board.get_score(tracker).has_value());
    board = board.apply_move(uci::move("c6b8", board));
    tracker.push(board);
    REQUIRE(!board.get_score(tracker).has_value());
    board = board.apply_move(uci::move("b1c3", board));
    tracker.push(board);
    REQUIRE(!board.get_score(tracker).has_value());
    board = board.apply_move(uci::move("b8c6", board));
    tracker.push(board);
    REQUIRE(!board.get_score(tracker).has_value());
    board = board.apply_move(uci::move("c3b1", board));
    tracker.push(board);
    REQUIRE(!board.get_score(tracker).has_value());
    board = board.apply_move(uci::move("c6b8", board));
    tracker.push(board);
    REQUIRE(board.get_score(tracker).value() == 0);
  }

  TEST_CASE("draw by fifty move rule") {
    auto board{Board::from_fen("8/p7/3k4/8/8/3K4/P7/8 w - - 98 0")};
    REQUIRE(!board.get_score().has_value());
    board = board.apply_move(uci::move("d3e3", board));
    REQUIRE(!board.get_score().has_value());
    board = board.apply_move(uci::move("d6e6", board));
    REQUIRE(board.get_score().value() == 0);
  }

  TEST_CASE("white wins") {
    auto board{Board::from_fen("r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4")};
    REQUIRE(!board.get_score().has_value());
    board = board.apply_move(uci::move("h5f7", board));
    REQUIRE(board.get_score().value() == 1);
  }
}

TEST_SUITE("board.to_fen") {
  TEST_CASE("from initial board") {
    auto board = chess::Board::initial();
    REQUIRE(board.to_fen() == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0");
    board = board.apply_move(chess::uci::move("e2e4", board));
    REQUIRE(board.to_fen() == "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0");
  }

  TEST_CASE("partial castling") {
    auto board = chess::Board::initial();
    board = board.apply_move(chess::uci::move("h2h4", board));
    board = board.apply_move(chess::uci::move("a7a5", board));
    board = board.apply_move(chess::uci::move("h1h3", board));
    board = board.apply_move(chess::uci::move("a8a6", board));
    REQUIRE(board.to_fen() == "1nbqkbnr/1ppppppp/r7/p7/7P/7R/PPPPPPP1/RNBQKBN1 w Qk - 2");
  }

  TEST_CASE("no castling") {
    auto board = chess::Board::initial();
    board = board.apply_move(chess::uci::move("e2e4", board));
    board = board.apply_move(chess::uci::move("e7e5", board));
    board = board.apply_move(chess::uci::move("e1e2", board));
    board = board.apply_move(chess::uci::move("e8e7", board));
    REQUIRE(board.to_fen() == "rnbq1bnr/ppppkppp/8/4p3/4P3/8/PPPPKPPP/RNBQ1BNR w - - 2");
  }
}