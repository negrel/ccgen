#!/usr/bin/env -S bats --verbose-run

TESTS_DIR="./tests"

@test "ccgen on simple c file" {
  result="$(ccgen "$TESTS_DIR/mock_file.c")"
  expected="$(
    printf "Hello from %s in %s\nHello from %s in %s in multiline comment" \
      $(realpath "$TESTS_DIR/mock_file.c") $(realpath "$TESTS_DIR") \
      $(realpath "$TESTS_DIR/mock_file.c") $(realpath "$TESTS_DIR") \
    )"

  [ "$result" = "$expected" ]
}

