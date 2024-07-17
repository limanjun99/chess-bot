#pragma once

#include <istream>
#include <memory>
#include <ostream>

#include "commands/command.h"
#include "outputs/output.h"

class UciInput;
class UciOutput;

class UciIO {
public:
  explicit UciIO(std::istream& input_stream, std::ostream& output_stream);

  struct ReadError;
  // Read the next command from its input stream (blocking read).
  // Returns a ReadError of type EndOfFile if eof has been reached.
  // Returns a ReadError of type InvalidCommand with a corresponding error message if the command is invalid.
  std::expected<std::unique_ptr<Command>, ReadError> read();

  // Write output to its output stream.
  void write(const Output& output);

private:
  std::istream& input_stream;
  std::ostream& output_stream;
};

struct UciIO::ReadError {
  enum Type { EndOfFile, InvalidCommand };

  Type error_type;
  std::string error_message;
};