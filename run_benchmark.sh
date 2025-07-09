BIN_DIR=build/gcc/Release/bin/
BINARY_NAME=benchmark_runner

# Check if the binary exists
if [ ! -f "$BIN_DIR$BINARY_NAME" ]; then
  echo "Binary '$BINARY_NAME' not found in '$BIN_DIR'. Building the project..."
  ./build.sh -r
fi

export NUM_KEYS=1000  # Set the number of keys to sort
export KEY_SIZE=16    # Set the size of each key in bytes
export N_RUNS=7       # Set the number of runs for each benchmark
$BIN_DIR$BINARY_NAME
