#include "ndjson.h"

NdjsonWriteCallback::NdjsonWriteCallback(std::function<bool(const json&)> handler) : handler{handler} {}

bool NdjsonWriteCallback::operator()(std::string data, intptr_t) {
  size_t start = 0;
  while (true) {
    size_t end = data.find_first_of('\n', start);
    if (end == data.npos) {
      line.append(data, start, data.npos);
      break;
    }
    line.append(data, start, end - start);
    const auto object = line.empty() ? json() : json::parse(line);
    if (!handler(object)) return false;
    line.clear();
    start = end + 1;
  }
  return true;
}
