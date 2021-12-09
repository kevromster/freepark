#!/bin/sh

# This script runs CarDetector application on the set of tests stored under "tests/" subdirectory.
# Usage:
#   ./run_tests.sh [--override-expected] [filter]
#
# The option '--override-expected' allows to overwrite expected output files into
# "tests/expected/" subdirectory by actual ones. Use this option to update exected
# output files of tests.
#
# The filter given as a regexp allows to specify input tests to be run, for example:
#   ./run_tests.sh mirgo
# will run all tests with "mirgo" in their name and will skip all other tests.
# If no input arguments specified, all tests are run.

if [ "$1" = "--override-expected" ]; then
  OVERRIDE_EXPECTED=1
  shift
fi

INPUT_MATCH=$*
CUR_DIR="`pwd`"
EXE_DIR="$CUR_DIR/../../.detector-build/CarDetector"
TESTS_DIR="$CUR_DIR/tests"
LOGS_DIR="$CUR_DIR/.logs"
CONFIG_DIR="$CUR_DIR"

TOTAL_TESTS_COUNT=0
CURRENT_TEST_IDX=0
TEST_START_TIME=0
TOTAL_TIME=0

rotate_logs() {
  [ -d "${LOGS_DIR}_prev_prev" ] && rm -rf "${LOGS_DIR}_prev_prev"
  [ -d "${LOGS_DIR}_prev" ] && mv -f "${LOGS_DIR}_prev" "${LOGS_DIR}_prev_prev"
  [ -d "$LOGS_DIR" ] && mv -f "$LOGS_DIR" "${LOGS_DIR}_prev"
  mkdir "$LOGS_DIR"
}

get_option_from_file() {
  local f="$1"
  local option_name="$2"
  echo $(cat "$f" | grep "$option_name" | awk -F= '
    function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
    function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
    function trim(s) { return rtrim(ltrim(s)); }
    {print trim($2)}')
}

# returns 0 if succeeded, non-zero if failed
run_test() {
  local config_file="$1"
  local input_file="$2"
  local expected_output_file="$3"
  local log_file="$4"

  # replace 'camera_uri' option by our input file
  local tmp_config="_config_$$"
  cat "$config_file" | sed 's,^camera_uri.*$,camera_uri = file://'"$input_file"',g' > "$tmp_config"

  local output_file="_output_$$"
  > "$output_file"
  ./CarDetector --test "$output_file" "$tmp_config" >> "$log_file" 2>&1
  local result=$?

  if [ $result != 0 ]; then
    echo "Error running CarDetector: result $result" >> "$log_file"
  else
    # check if diff is empty
    if [ -n "`diff "$expected_output_file" "$output_file"`" ]; then
      echo "diff in output!" >> "$log_file"
      result=1
    fi
  fi

  # move all logs and detection images into logs directory
  local local_logs_dir="$(dirname "$log_file")"

  mv -f "$tmp_config" "$local_logs_dir/config.txt"
  mv -f "$output_file" "$local_logs_dir/actual.out"
  mv -f *.png "$local_logs_dir"

  if [ "$OVERRIDE_EXPECTED" = "1" ]; then
    cp -a "$expected_output_file" "$local_logs_dir/expected.out"
    cp -a "$local_logs_dir/actual.out" "$expected_output_file"
  else
    ln -s "$expected_output_file" "$local_logs_dir/expected.out"
  fi

  return $result
}

echo_res() {
  local res="$1"
  local test_end_time=$(date +%s)
  local spent_time=$(($test_end_time - $TEST_START_TIME))
  TOTAL_TIME=$(($TOTAL_TIME + $spent_time))
  printf '%-5s  %2s/%s    %3s sec' $res $CURRENT_TEST_IDX $TOTAL_TESTS_COUNT $spent_time
  [ "$res" = "FAIL" -a "$OVERRIDE_EXPECTED" = "1" ] && printf '    override!'
  echo
}

# filters input tests by input matching string if given
find_tests() {
  if [ -z "$INPUT_MATCH" ]; then
    ls "$TESTS_DIR"/*.test
    return
  fi

  for tst in `ls "$TESTS_DIR"/*.test`; do
    (echo "`basename -s .test $tst`" | grep -Eq "$INPUT_MATCH") && echo $tst
  done
}

main() {
  echo "========== CarDetector tests =========="
  cd "$EXE_DIR"

  # check file exists and is executable
  if [ ! -x "CarDetector" ]; then
    echo "No 'CarDetector' executable found."
    return 1
  fi

  rotate_logs
  rm -f *.png

  local succeed_cnt=0
  local failed_cnt=0
  local skipped_cnt=0
  local tests="$(find_tests)"

  TOTAL_TESTS_COUNT=`echo "$tests" | wc -l`
  echo "Number of tests to run: $TOTAL_TESTS_COUNT"

  for test_file in $tests; do
    CURRENT_TEST_IDX=$(($CURRENT_TEST_IDX+1))
    TEST_START_TIME=$(date +%s)

    printf '%-50s' `basename $test_file`

    local local_log_filename="$(basename -s .test "$test_file")"
    local local_logs_dir="$LOGS_DIR/$local_log_filename"
    mkdir "$local_logs_dir"
    local log_file="${local_logs_dir}/${local_log_filename}.log"

    local config=$(get_option_from_file "$test_file" "config")
    local input=$(get_option_from_file "$test_file" "input")
    local expected=$(get_option_from_file "$test_file" "expected")

    if [ -z "$config" ]; then
      echo "'config' option is not given, skip test" >> "$log_file"
      echo_res "SKIP"
      skipped_cnt=$(($skipped_cnt+1))
      continue
    fi
    if [ -z "$input" ]; then
      echo "'input' option is not given, skip test" >> "$log_file"
      echo_res "SKIP"
      skipped_cnt=$(($skipped_cnt+1))
      continue
    fi
    if [ -z "$expected" ]; then
      echo "'expected' option is not given, skip test" >> "$log_file"
      echo_res "SKIP"
      skipped_cnt=$(($skipped_cnt+1))
      continue
    fi

    local config_file="$CONFIG_DIR/$config"
    local input_file="$TESTS_DIR/$input"
    local expected_output_file="$TESTS_DIR/$expected"

    if [ ! -f "$config_file" ]; then
      echo "config file not found: '$config_file', skip test" >> "$log_file"
      echo_res "SKIP"
      skipped_cnt=$(($skipped_cnt+1))
      continue
    fi
    if [ ! -f "$input_file" ]; then
      echo "input file not found: '$input_file', skip test" >> "$log_file"
      echo_res "SKIP"
      skipped_cnt=$(($skipped_cnt+1))
      continue
    fi
    if [ ! -f "$expected_output_file" ]; then
      echo "exected output file not found: '$expected_output_file', skip test" >> "$log_file"
      echo_res "SKIP"
      skipped_cnt=$(($skipped_cnt+1))
      continue
    fi

    run_test "$config_file" "$input_file" "$expected_output_file" "$log_file"
    local result=$?

    if [ $result = 0 ]; then
      echo "test succeeded" >> "$log_file"
      echo_res "OK"
      succeed_cnt=$(($succeed_cnt+1))
    else
      echo "test failed" >> "$log_file"
      echo_res "FAIL"
      failed_cnt=$(($failed_cnt+1))
    fi
  done

  echo
  echo "===== Final results:"
  echo "total tests: $TOTAL_TESTS_COUNT"
  echo "total spent time: $TOTAL_TIME secs"
  echo "skipped: $skipped_cnt"
  echo "failed: $failed_cnt"
  echo "succeeded: $succeed_cnt"

  local final_result=1
  [ $succeed_cnt = $TOTAL_TESTS_COUNT ] && final_result=0
  return $final_result
}

main
exit $?
