#include "error_output.h"

ErrorOutput::ErrorOutput(std::string error_message) : Output{}, error_message{std::move(error_message)} {}

std::string ErrorOutput::to_string() const { return error_message; }