#pragma once

#include "config.h"

class Lichess {
public:
  Lichess(const Config& config);

  // Start listening and responding to incoming events from Lichess API.
  void listen();

private:
  const Config& config;

  // Either accept or decline the challenge with given `challenge_id`.
  // Returns false if challenge was not found.
  bool handle_challenge(const std::string& challenge_id, bool accept);

  // Handle an incoming event. Returns true if handled successfully.
  bool handle_incoming_event(std::string data);
};