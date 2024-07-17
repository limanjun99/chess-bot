#include "parsing.h"

#include <algorithm>

std::vector<std::string_view> command::parsing::split_string(std::string_view string) {
  std::vector<std::string_view> words;
  auto word_start{string.begin()};

  while (word_start != string.end()) {
    while (word_start != string.end() && std::isspace(*word_start)) word_start++;

    auto word_end{word_start};
    while (word_end != string.end() && !std::isspace(*word_end)) word_end++;

    if (word_start != word_end) {
      words.emplace_back(word_start, word_end);
      word_start = word_end;
    }
  }

  return words;
}

std::string_view command::parsing::first_word(std::string_view string) {
  const auto is_space = [](auto c) { return std::isspace(c); };
  const auto word_start{std::find_if_not(string.begin(), string.end(), is_space)};
  if (word_start == string.end()) return std::string_view{};
  const auto word_end{std::find_if(word_start, string.end(), is_space)};
  return std::string_view{word_start, word_end};
}