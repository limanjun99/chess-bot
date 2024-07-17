#include "output.h"

std::ostream& operator<<(std::ostream& os, const Output& output) { return os << output.to_string(); }