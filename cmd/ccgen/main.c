#include <ctype.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define XSTD_IMPLEMENTATION
#include "xstd.h"

static int set_ccgen_env(char *filename) {
  filename = realpath(filename, NULL);

  int exit_code = setenv("CCGEN_FILE", filename, true);
  if (exit_code) {
    fprintf(stderr, "failed to setenv CCGEN_FILE for file %s: %s\n", filename,
            strerror(errno));
    exit(exit_code);
  }

  char *dir = dirname(filename);
  exit_code = setenv("CCGEN_DIR", dir, true);
  if (exit_code) {
    fprintf(stderr, "failed to setenv CCGEN_DIR for file %s: %s\n", filename,
            strerror(errno));
    exit(exit_code);
  }

  return 0;
}

typedef Result(char, int) ReadCharResult;
static ReadCharResult read_char(Reader *reader) {
  uint8_t current_char = 0;
  size_t read = 0;
  int err = 0;
  reader_read(reader, &current_char, sizeof(current_char), &read, &err);
  if (read > 0)
    return ResultOk(ReadCharResult, current_char);

  return ResultError(ReadCharResult, err);
}

typedef Result(Slice, int) ReadLineResult;
static ReadLineResult read_line(Reader *reader, BytesBuffer *buf) {
  bytes_buffer_reset(buf);

  ReadCharResult result = {0};
  do {
    result = read_char(reader);
    if (result_is_err(result)) {
      if (result.data.err == EOF && bytes_buffer_length(buf) > 0)
        goto ok;
      return ResultError(ReadLineResult, result.data.err);
    }

    bytes_buffer_append_bytes(buf, &result.data.ok, 1);
  } while (result.data.ok != '\n');

ok:
  return ResultOk(ReadLineResult, bytes_buffer_bytes(buf));
}

static int ccgen_reader(Reader *reader, Allocator *allocator) {
  BytesBuffer buf = bytes_buffer(allocator);

  while (true) {
    ReadLineResult line_result = read_line(reader, &buf);
    if (result_is_err(line_result)) {
      if (line_result.data.err == EOF)
        break;
      else {
        bytes_buffer_destroy(&buf);
        return line_result.data.err;
      }
    }

    Slice line = line_result.data.ok;

    // Skip whitespaces.
    while (isspace(line.data[0])) {
      line = slice(&line.data[1], line.len - 1);
    }

    // Skip non comment lines.
    if (line.len < strlen("//ccgen") || line.data[0] != '/' ||
        (line.data[1] != '/' && line.data[1] != '*')) {
      goto nextline;
    }

    // Only multiline (/* foo ... */) comment on a single line are
    // supported.
    if (line.data[1] == '*') {
      for (size_t i = 0; i < line.len - 2; i++) {
        if (line.data[i] == '\n')
          goto nextline;

        // Close comment.
        if (line.data[i] == '*' && line.data[i + 1] == '/') {
          line.len = i + 1;
          break;
        }
      }
    }
    // Replace '\n' or '*/' by '\0'
    line.data[line.len - 1] = '\0';

    // Remove /* or //
    line.data = &line.data[2];
    line.len -= 2;

    // Skip whitespaces after start comment token.
    while (isspace(line.data[0])) {
      line = slice(&line.data[1], line.len - 1);
    }

    // Skip non ccgen comments.
    if (strncmp((const char *)line.data, "ccgen:", strlen("ccgen:")) != 0)
      goto nextline;

    // Strip prefix "ccgen:"
    line.data = &line.data[strlen("ccgen:")];
    line.len -= strlen("ccgen:");

    int exit_code = system((const char *)line.data);
    if (exit_code)
      return exit_code;

  nextline:;
  }

  return 0;
}

int main(int argc, char **argv) {
  int exit_code = 0;

  if (argc == 1) {
    fprintf(stderr, "please specify a list of file to scan");
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    FILE *f = fopen(argv[i], "r");
    if (!f) {
      fprintf(stderr, "failed to open file %s\n", argv[i]);
      return 1;
    }

    exit_code = set_ccgen_env(argv[i]);
    if (exit_code) {
      fprintf(stderr, "failed to putenv CCGEN_FILE for file %s: %s\n", argv[i],
              strerror(errno));
      return exit_code;
    }

    FileReader freader = file_reader(f);
    exit_code = ccgen_reader(&freader.iface, g_libc_allocator);
    if (exit_code) {
      fprintf(stderr, "failed to process file %s: %s\n", argv[i],
              strerror(exit_code));
      return exit_code;
    }
  }

  return exit_code;
}
