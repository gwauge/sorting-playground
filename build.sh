#!/bin/bash

set -e

# Default values
BUILD_TYPE="Debug"
COMPILER="gcc"
CLEAR_BUILD_DIR=false
CORES=$(nproc) # Default to all cores
RUN_BENCHMARKS=false

# Parse CLI flags
while getopts "rcbj:" opt; do
  case $opt in
    r)
      BUILD_TYPE="Release"
      ;;
    c)
      CLEAR_BUILD_DIR=true
      ;;
    b)
      RUN_BENCHMARKS=true
      ;;
    j)
      CORES=$OPTARG
      ;;
    *)
      echo "Usage: $0 [-r] [-c] [-b] [-j <number_of_cores>]"
      exit 1
      ;;
  esac
done
shift $((OPTIND - 1))

# Build directory structure
BUILD_DIR="build/$COMPILER/$BUILD_TYPE"

if $CLEAR_BUILD_DIR; then
  if [ -d $BUILD_DIR ]; then
    echo "Clearing build directory '$BUILD_DIR' ..."
    rm -rf $BUILD_DIR
  else
    echo "Build directory '$BUILD_DIR' does not exist. Nothing to clear."
  fi
fi

mkdir -p $BUILD_DIR


# Configure and build
CMAKE_ARGS=("-DCMAKE_BUILD_TYPE=$BUILD_TYPE" "-B" "${BUILD_DIR}")
if [[ "$COMPILER" == "clang" ]]; then
  CMAKE_ARGS+=("-DCMAKE_C_COMPILER=clang" "-DCMAKE_CXX_COMPILER=clang++")
fi

cmake "${CMAKE_ARGS[@]}"
# ninja -C ${BUILD_DIR} hyriseBenchmarkTPCDS # hyriseMicroBenchmarks
make -j $CORES -C $BUILD_DIR benchmark_runner

if ! $RUN_BENCHMARKS; then
  echo "Skipping benchmarks. Use -b to run them."
  exit 0
fi

# Run benchmarks
ISO_TIME=$(date +%Y-%m-%dT%H-%M-%S)
BENCHMARK_FOLDER=data/benchmarks
mkdir -p $BENCHMARK_FOLDER
./$BUILD_DIR/bin/benchmark_runner --benchmark_filter=BM_Sort.* --benchmark_out_format=json --benchmark_out="${BENCHMARK_FOLDER}/${BUILD_TYPE}_${COMPILER}_$ISO_TIME.json"
