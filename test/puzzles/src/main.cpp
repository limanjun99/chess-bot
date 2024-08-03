#include <print>

#include "config.h"
#include "data.h"
#include "solve.h"
#include "statistic.h"
int main(int argc, char* argv[]) {
  try {
    const auto config = Config::from_cli(argc, argv);

    if (config.lichess_open_db_path) {
      data::generate(config);
      return 0;
    }

    const auto data = data::get(config);
    auto new_statistics = std::vector<Statistic>{};
    auto old_statistics = std::vector<Statistic>{};

    for (const auto& [puzzle, old_statistic] : data) {
      const auto new_statistic = solve::puzzle(puzzle);
      solve::compare(new_statistic, old_statistic);
      new_statistics.push_back(std::move(new_statistic));
      old_statistics.push_back(old_statistic);
    }

    solve::compare_all(new_statistics, old_statistics);
    data::update_statistics(new_statistics, config);

  } catch (const std::runtime_error& error) {
    std::println("{}", error.what());
    return 1;
  } catch (const std::exception& error) {
    std::println("Unexpected error occured:\n{}", error.what());
    return 1;
  }

  return 0;
}