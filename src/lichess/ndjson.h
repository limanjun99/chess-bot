#pragma once

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

using json = nlohmann::json;

// An adapter for cpr::WriteCallback used to handle HTTP streams of ndjson.
// Calls the given handler for each json object recevied (instead of every buffer write).
class NdjsonWriteCallback {
public:
  // The handler returns true if we should continue listening and reading from the HTTP stream.
  NdjsonWriteCallback(std::function<bool(const json&)> handler);

  bool operator()(std::string data, intptr_t);

private:
  std::function<bool(const json&)> handler;
  std::string line;
};