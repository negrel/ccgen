#!/usr/bin/env -S bats --verbose-run

bats_require_minimum_version 1.5.0

TESTS_DIR="./tests"

@test "ccgen on simple c file" {
  result="$(ccgen "$TESTS_DIR/test_files/mock_file.c")"
  expected="$(
    printf "Hello from %s in %s\nHello from %s in %s in multiline comment" \
      $(realpath "$TESTS_DIR/test_files/mock_file.c") $(realpath "$TESTS_DIR/test_files") \
      $(realpath "$TESTS_DIR/test_files/mock_file.c") $(realpath "$TESTS_DIR/test_files") \
    )"

  [ "$result" = "$expected" ]
}

@test "ccgen command fail" {
  run ! ccgen "$TESTS_DIR/test_files/ccgen_fail.c"
}
