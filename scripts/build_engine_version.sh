#!/bin/bash

# Exit on error.
set -e

# Help instructions.
print_help() {
  echo "This script helps to build the engine_cli binary for the current commit or a given tag,"
  echo "then outputs the binary to 'engine_builds/engine_cli_[tag]'."
  echo ""
  echo "Usage: $0 [options]"
  echo ""
  echo "  -c             Build the current commit."
  echo "  -t <tag>       Build the commit with the given tag."
  echo ""
  echo "E.g. To build the version tagged v0.0.1:  build_engine.sh -t v0.0.1"
  echo "E.g. To build the current commit       :  build_engine.sh -c"
}



# Parse options
OPTSTRING=":ct:"
build_current=false
build_tag=false
while getopts ${OPTSTRING} opt; do
  case ${opt} in
    c)
      build_current=true
      ;;
    t)
      build_tag=true
      git_tag=${OPTARG}
      ;;
    :)
      echo "Invalid usage: option -${OPTARG} requires an argument [version tag]."
      exit 1
      ;;
    ?)
      echo "Invalid option: -${OPTARG}."
      exit 1
    ;;
  esac
done

# User must choose to build current commit or a given tag, not both.
if [ "$build_current" = true ] && [ "$build_tag" = true ]; then
  echo "Invalid usage: exactly one of -c or -t must be specified, but both were provided."
  exit 1
fi

# User must choose to build current commit or a given tag.
if [ "$build_current" = false ] && [ "$build_tag" = false ]; then
  print_help
  exit 1
fi



# Go to root directory.
scriptdir="$(dirname "$0")"
cd $scriptdir
cd ../

# Checkout version.
if [ "$build_tag" = true ]; then

  exit_status=0
  git_error=$(git checkout $git_tag 2>&1) || exit_status=$?

  if [ $exit_status -ne 0 ]; then
    echo "Build failed: Could not git checkout the given tag '$git_tag'. Command failed with error:"
    echo ""
    echo $git_error
    exit 1
  fi
fi

# Check that build script exists.
if [ ! -f ./scripts/build_engine.sh ]; then
  echo "Build failed: scripts/build_engine.sh not found."
  echo "This script only supports commits where a scripts/build_engine.sh is found."
  echo "Otherwise, we do not know how to build the binary for the given commit."
  exit 1
fi

# Run build script.
if [ "$build_tag" = true ]; then
  bash ./scripts/build_engine.sh $git_tag
else
  bash ./scripts/build_engine.sh current
fi

# Go back to current git commit
if [ "$build_tag" = true ]; then
  git checkout -
fi
