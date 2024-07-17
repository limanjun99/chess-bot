#include "id_output.h"

#include <format>

IdOutput::IdOutput(std::string name, std::string author) : Output{}, name{std::move(name)}, author{std::move(author)} {}

std::string IdOutput::to_string() const { return std::format("id name {}\nid author {}", name, author); }