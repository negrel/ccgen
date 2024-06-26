#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

/**
 * read_char reads a single byte from f.
 */
static size_t read_char(FILE *f, char *c) {
  return fread(c, sizeof(char), 1, f);
}

static size_t read_line(FILE *f, char **buf, size_t *buf_cap) {
  size_t buf_len = 0;

  do {
    // Grow buffer if needed.
    while (buf_len + 1 >= *buf_cap) {
      *buf_cap = *buf_cap == 0 ? 128 : *buf_cap * 2;
      *buf = realloc(*buf, *buf_cap);
      if (*buf == NULL) {
        perror("realloc() failed");
        exit(1);
      }
    }

    size_t read = read_char(f, &(*buf)[buf_len]);
    buf_len += read;

    // Read failed, let caller handle error.
    if (read == 0)
      goto end;

  } while ((*buf)[buf_len - 1] != '\n');

end:
  return buf_len;
}

static int ccgen_file(FILE *f, char **buf, size_t *buf_cap) {
  while (true) {
    size_t line_len = read_line(f, buf, buf_cap);
    if (ferror(f)) {
      fprintf(stderr, "fread() failed: %zu\n", line_len);
      return 1;
    }
    if (line_len == 0 && feof(f)) {
      return 0;
    }

    char *line = *buf;

    // Skip whitespaces.
    while (isspace(line[0])) {
      line = &line[1];
      line_len--;
    }

    // Skip non comment lines.
    if (line_len < strlen("//ccgen") || line[0] != '/' ||
        (line[1] != '/' && line[1] != '*')) {
      goto nextline;
    }

    // Only multiline (/* foo ... */) comment on a single line are
    // supported.
    if (line[1] == '*') {
      for (size_t i = 0; i < line_len - 2; i++) {
        if (line[i] == '\n')
          goto nextline;

        // Close comment.
        if (line[i] == '*' && line[i + 1] == '/') {
          line_len = i + 1;
          break;
        }
      }
    }
    // Replace '\n' or '*/' by '\0'
    line[line_len - 1] = '\0';

    // Remove /* or //
    line = &line[2];
    line_len -= 2;

    // Skip whitespaces after start comment token.
    while (isspace(line[0])) {
      line = &line[1];
      line_len--;
    }

    // Skip non ccgen comments.
    if (strncmp(line, "ccgen:", strlen("ccgen:")) != 0)
      goto nextline;

    // Strip prefix "ccgen:"
    line = &line[strlen("ccgen:")];
    line_len -= strlen("ccgen:");

    int exit_code = system(line);
    if (exit_code)
      return exit_code;

  nextline:;
  }

  return 0;
}

int main(int argc, char **argv) {
  int error = 0;

  if (argc == 1) {
    fprintf(stderr, "please specify a list of file to scan");
    return EXIT_FAILURE;
  }

  char *buf = NULL;
  size_t buf_cap = 0;

  for (int i = 1; i < argc; i++) {
    FILE *f = fopen(argv[i], "r");
    if (!f) {
      fprintf(stderr, "failed to open file %s\n", argv[i]);
      return EXIT_FAILURE;
    }

    error = set_ccgen_env(argv[i]);
    if (error) {
      fprintf(stderr, "failed to putenv CCGEN_FILE for file %s: %s\n", argv[i],
              strerror(errno));
      return EXIT_FAILURE;
    }

    error = ccgen_file(f, &buf, &buf_cap);
    if (error) {
      fprintf(stderr, "failed to process file %s: %s\n", argv[i],
              strerror(error));
      return EXIT_FAILURE;
    }

    fclose(f);
  }

  return error;
}
