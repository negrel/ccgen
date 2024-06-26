// XSTD bundle from https://github.com/negrel/xstd.h

#ifdef __clang__
#define typeof __typeof__
#else
#ifndef typeof
#define typeof assert(0 && "typeof macro is not defined")
#endif
#endif

#define type_assert_eq(X, Y)                                                   \
  _Generic((Y), typeof(X): _Generic((X), typeof(Y): (void)NULL))
#ifndef XSTD_CONSTRUCTOR_H_INCLUDE
#define XSTD_CONSTRUCTOR_H_INCLUDE

// Used by def_type_constructors macros.
#include <assert.h>

// Used by def_type_constructors macros.

#define type_init_proto
#define type_init_args
#define type_init                                                              \
  {}

#ifdef XSTD_IMPLEMENTATION
#define def_type_constructors(type, type_snake)                                \
  void type_snake##_init(type *t, type_init_proto);                            \
  void type_snake##_init(type *t, type_init_proto) {                           \
    assert(t != NULL);                                                         \
    *t = (type){0};                                                            \
    type_init                                                                  \
  }                                                                            \
                                                                               \
  type *type_snake##_new(Allocator *allocator, type_init_proto);               \
  type *type_snake##_new(Allocator *allocator, type_init_proto) {              \
    type *t = alloc_malloc(allocator, sizeof(type));                           \
    type_snake##_init(t, type_init_args);                                      \
    return t;                                                                  \
  }                                                                            \
                                                                               \
  type type_snake(type_init_proto);                                            \
  type type_snake(type_init_proto) {                                           \
    type t = {0};                                                              \
    type_snake##_init(&t, type_init_args);                                     \
    return t;                                                                  \
  }
#else
#define def_type_constructors(type, type_snake)                                \
  void type_snake##_init(type *t, type_init_proto);                            \
                                                                               \
  type *type_snake##_new(Allocator *allocator, type_init_proto);               \
                                                                               \
  type type_snake(type_init_proto);
#endif

#ifdef XSTD_IMPLEMENTATION
#define def_type_constructors_no_args(type, type_snake)                        \
  void type_snake##_init(type *t);                                             \
  void type_snake##_init(type *t) {                                            \
    assert(t != NULL);                                                         \
    *t = (type){0};                                                            \
    type_init                                                                  \
  }                                                                            \
                                                                               \
  type *type_snake##_new(Allocator *allocator);                                \
  type *type_snake##_new(Allocator *allocator) {                               \
    type *t = alloc_malloc(allocator, sizeof(type));                           \
    type_snake##_init(t);                                                      \
    return t;                                                                  \
  }                                                                            \
                                                                               \
  type type_snake(void);                                                       \
  type type_snake(void) {                                                      \
    type t = {0};                                                              \
    type_snake##_init(&t);                                                     \
    return t;                                                                  \
  }
#else
#define def_type_constructors_no_args(type, type_snake)                        \
  void type_snake##_init(type *t);                                             \
                                                                               \
  type *type_snake##_new(Allocator *allocator);                                \
                                                                               \
  type type_snake(void);
#endif

#endif
#ifndef XSTD_IFACE_H_INCLUDE
#define XSTD_IFACE_H_INCLUDE

// Used by iface_call macros.
#include <stdint.h>


// InterfaceImpl define an implementation of an interface.
typedef void *InterfaceImpl;

// Define an interface type and its virtual table.
#define iface_def(name, vtable)                                                \
  typedef struct {                                                             \
    vtable *vtable_;                                                           \
    size_t offset_;                                                            \
  } name

// Call and a method of an iface instance.

#define iface_call(instance, method, ...)                                      \
  instance->vtable_->method((void *)((uintptr_t)instance + instance->offset_), \
                            __VA_ARGS__)

#define iface_call_empty(instance, method)                                     \
  instance->vtable_->method((void *)((uintptr_t)instance + instance->offset_))

#define iface_impl_def_empty(interface, name)                                  \
  typedef struct {                                                             \
    interface iface;                                                           \
  } name

#define iface_impl_def(interface, name, impl_fields)                           \
  typedef struct {                                                             \
    interface iface;                                                           \
    impl_fields body_;                                                         \
  } name

#define iface_impl_init(instance, field, vtable, body)                         \
  do {                                                                         \
    (instance)->field = (typeof((instance)->field)){                           \
        .vtable_ = vtable,                                                     \
        .offset_ = offsetof(typeof(*instance), body_) -                        \
                   offsetof(typeof(*instance), field)};                        \
    (instance)->body_ = (body);                                                \
  } while (0);

#define iface_empty_impl_init(instance, field, vtable)                         \
  do {                                                                         \
    (instance)->field = (typeof((instance)->field)){                           \
        .vtable_ = vtable,                                                     \
        .offset_ = 0,                                                          \
    };                                                                         \
  } while (0);

#define typeof_iface_impl(impl_type) typeof(((impl_type *){0})->body_)

#define cast_iface_impl_ptr(impl_type, ptr) (typeof_iface_impl(impl_type) *)ptr
#define cast_iface_impl(impl_type, ptr) *cast_iface_impl_ptr(impl_type, ptr)

// Merged interfaces.

#define iface_merge_def(name, ...)                                             \
  typedef struct {                                                             \
    __VA_ARGS__;                                                               \
  } name

#endif
// Single file header allocator interface / vtable
// from xstd (https://github.com/negrel/xstd.h).

#ifndef XSTD_ALLOC_H_INCLUDE
#define XSTD_ALLOC_H_INCLUDE

#include <stddef.h>

#ifdef XSTD_IMPLEMENTATION
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#endif


// Allocator interface.
iface_def(
    Allocator, struct xstd_allocator_vtable {
      void *(*malloc)(InterfaceImpl allocator, size_t size);
      void (*free)(InterfaceImpl allocator, void *ptr);
      void *(*calloc)(InterfaceImpl allocator, size_t nmemb, size_t size);
      void *(*realloc)(InterfaceImpl allocator, void *ptr, size_t newsize);
    });

// alloc_malloc() allocates size bytes and returns a pointer to the allocated
// memory. The memory is not initialized.
void *alloc_malloc(Allocator *, size_t);

#ifdef XSTD_IMPLEMENTATION
void *alloc_malloc(Allocator *allocator, size_t size) {
  void *ptr = iface_call(allocator, malloc, size);
  assert(ptr != NULL);
  return ptr;
}
#endif

// alloc_free() frees the memory space pointed to by ptr, which must have been
// returned by a previous call to malloc() or related functions.
void alloc_free(Allocator *allocator, void *ptr);

#ifdef XSTD_IMPLEMENTATION
void alloc_free(Allocator *allocator, void *ptr) {
  iface_call(allocator, free, ptr);
}
#endif

// The calloc() function allocates memory for an array of nmemb elements of size
// bytes each and returns a pointer to the allocated memory. The memory is set
// to zero.
void *alloc_calloc(Allocator *allocator, size_t nmemb, size_t size);

#ifdef XSTD_IMPLEMENTATION
void *alloc_calloc(Allocator *allocator, size_t nmemb, size_t size) {
  void *ptr = iface_call(allocator, calloc, nmemb, size);
  assert(ptr != NULL);
  return ptr;
}
#endif

// The realloc() function changes the size of the memory block pointed to by ptr
// to newsize bytes. The contents of the memory will be unchanged in the range
// from the start of the region up to the minimum of the old and new sizes. If
// the new size is larger than the old size, the added memory will not be
// initialized.
void *alloc_realloc(Allocator *allocator, void *ptr, size_t newsize);

#ifdef XSTD_IMPLEMENTATION
void *alloc_realloc(Allocator *allocator, void *ptr, size_t newsize) {
  ptr = iface_call(allocator, realloc, ptr, newsize);
  assert(ptr != NULL);
  return ptr;
}
#endif

Allocator *g_libc_allocator;

#ifdef XSTD_IMPLEMENTATION
static void *libc_malloc(InterfaceImpl alloc, size_t size) {
  (void)alloc;
  return malloc(size);
}

static void libc_free(InterfaceImpl alloc, void *ptr) {
  (void)alloc;
  free(ptr);
}

static void *libc_calloc(InterfaceImpl alloc, size_t nmemb, size_t size) {
  (void)alloc;
  void *ptr = calloc(nmemb, size);
  return ptr;
}

static void *libc_realloc(InterfaceImpl alloc, void *ptr, size_t size) {
  (void)alloc;
  return realloc(ptr, size);
}

static struct xstd_allocator_vtable libc_allocator_vtable = {
    .malloc = &libc_malloc,
    .free = &libc_free,
    .calloc = &libc_calloc,
    .realloc = &libc_realloc,
};

static Allocator libc_allocator = {
    .vtable_ = &libc_allocator_vtable,
    .offset_ = 0,
};

Allocator *g_libc_allocator = &libc_allocator;
#endif

#endif
#ifndef XSTD_IO_CLOSER_H_INCLUDE
#define XSTD_IO_CLOSER_H_INCLUDE

#include <stdbool.h>
#include <stdio.h>

#ifdef XSTD_IMPLEMENTATION
#include <errno.h>
#include <stddef.h>
#endif


// Closer interface / virtual table. Closer is the interface that wraps basic
// Close method. The behavior of Close after the first call is undefined.
// Specific implementations may document their own behavior.
iface_def(
    Closer, struct xstd_closer_vtable {
      void (*close)(InterfaceImpl closer, int *error);
    });

// closer_close calls the close method of the closer.
void closer_close(Closer *closer, int *error);

#ifdef XSTD_IMPLEMENTATION
void closer_close(Closer *closer, int *error) {
  iface_call(closer, close, error);
}
#endif

// NopCloser is a no-op Closer implementation.
iface_impl_def_empty(Closer, NopCloser);

#ifdef XSTD_IMPLEMENTATION
static void nop_closer_close(void *_, int *error) {
  (void)_;
  (void)error;
}
#endif

struct xstd_closer_vtable nop_closer_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_closer_vtable nop_closer_vtable = {.close = &nop_closer_close};
#endif

// Generate nop_closer_init, nop_closer_new and nop_closer constructors.

#undef type_init
#define type_init                                                              \
  { iface_empty_impl_init(t, iface, &nop_closer_vtable) }

def_type_constructors_no_args(NopCloser, nop_closer)

    // FileCloser wraps FILE and implements the Closer interface.
    iface_impl_def(Closer, FileCloser, FILE *);

#ifdef XSTD_IMPLEMENTATION
static void file_closer_close(InterfaceImpl impl, int *error) {
  typeof_iface_impl(FileCloser) f = cast_iface_impl(FileCloser, impl);
  if (!fclose(f) && error != NULL)
    *error = errno;
}
#endif

struct xstd_closer_vtable file_closer_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_closer_vtable file_closer_vtable = {.close = &file_closer_close};
#endif

// Generate file_closer_init, file_closer_new and file_closer constructors.

#undef type_init_proto
#undef type_init_args
#undef type_init
#define type_init_proto FILE *f
#define type_init_args f
#define type_init                                                              \
  { iface_impl_init(t, iface, &file_closer_vtable, f) }

def_type_constructors(FileCloser, file_closer)

#endif
#ifndef XSTD_IO_READER_H_INCLUDE
#define XSTD_IO_READER_H_INCLUDE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef XSTD_IMPLEMENTATION
#include <errno.h>
#endif


// Reader is the interface that wraps the basic read method.
//
// read reads up to len(p) bytes into p. It returns the number of bytes read
// (0 <= n <= p_len) and any error encountered. Even if read returns n < p_len,
// it may use all of p as scratch space during the call. If some data is
// available but not p_len bytes, read conventionally returns what is available
// instead of waiting for more.
//
// When read encounters an error or end-of-file condition after successfully
// reading n > 0 bytes, it returns the number of bytes read. It may return the
// (non-null) error from the same call or return the error (and n == 0) from a
// subsequent call. An instance of this general case is that a Reader returning
// a non-zero number of bytes at the end of the input stream may return either
// err == EOF or err == nil. The next read should return 0, EOF.
//
// Callers should always process the n > 0 bytes returned before considering the
// error err. Doing so correctly handles I/O errors that happen after reading
// some bytes and also both of the allowed EOF behaviors.
//
// If p_len == 0, read should always return n == 0. It may return a non-null
// error if some error condition is known, such as EOF.
//
// Implementations of read are discouraged from returning a zero byte count with
// a nil error, except when len(p) == 0. Callers should treat a return of 0 and
// nil as indicating that nothing happened; in particular it does not indicate
// EOF.
//
// Implementations must not retain p.
iface_def(
    Reader, struct xstd_reader_vtable {
      void (*read)(InterfaceImpl reader, uint8_t *p, size_t p_len, size_t *n,
                   int *err);
    });

// reader_read calls the read method of the reader.
void reader_read(Reader *reader, uint8_t *p, size_t p_len, size_t *n, int *err);

#ifdef XSTD_IMPLEMENTATION
void reader_read(Reader *reader, uint8_t *p, size_t p_len, size_t *n,
                 int *err) {
  iface_call(reader, read, p, p_len, n, err);
}
#endif

// FileReader wraps FILE and implements Reader.
iface_impl_def(Reader, FileReader, FILE *);

// file_reader_read implements Reader.
#ifdef XSTD_IMPLEMENTATION
static void file_reader_read(InterfaceImpl impl, uint8_t *p, size_t p_len,
                             size_t *n, int *error) {
  typeof_iface_impl(FileReader) f = cast_iface_impl(FileReader, impl);
  size_t read = fread(p, sizeof(*p), p_len, f);
  if (n != NULL)
    *n = read;

  if (error != NULL) {
    *error = errno;
    if (read == 0 && feof(f)) {
      *error = EOF;
    }
    clearerr(f);
  }
}
#endif

// FileReader virtual table for Reader interface.
struct xstd_reader_vtable file_reader_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_reader_vtable file_reader_vtable = {.read = &file_reader_read};
#endif

// Generate file_writer_init, file_writer_new and file_writer constructors.

#undef type_init_proto
#undef type_init_args
#undef type_init

#define type_init_proto FILE *f
#define type_init_args f
#define type_init                                                              \
  { iface_impl_init(t, iface, &file_reader_vtable, f) }

def_type_constructors(FileReader, file_reader)

#endif
#ifndef XSTD_IO_WRITER_H_INCLUDE
#define XSTD_IO_WRITER_H_INCLUDE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef XSTD_IMPLEMENTATION
#include <errno.h>
#endif


// Writer is the interface that wraps the basic Write method.
//
// Write writes len(p) bytes from p to the underlying data stream. It returns
// the number of bytes written from p (0 <= n <= len(p)) and any error
// encountered that caused the write to stop early. Write must return a non-nil
// error if it returns n < len(p). Write must not modify the slice data, even
// temporarily.
iface_def(
    Writer, struct xstd_writer_vtable {
      void (*write)(InterfaceImpl writer, uint8_t *p, size_t p_len, size_t *n,
                    int *err);
    });

void writer_write(Writer *writer, uint8_t *p, size_t p_len, size_t *n,
                  int *error);

#ifdef XSTD_IMPLEMENTATION
void writer_write(Writer *writer, uint8_t *p, size_t p_len, size_t *n,
                  int *error) {
  iface_call(writer, write, p, p_len, n, error);
}
#endif

// FileWriter wraps FILE and implements Writer.
iface_impl_def(Writer, FileWriter, FILE *);

#ifdef XSTD_IMPLEMENTATION
static void file_writer_write(InterfaceImpl impl, uint8_t *p, size_t p_len,
                              size_t *n, int *error) {
  typeof_iface_impl(FileWriter) f = cast_iface_impl(FileWriter, impl);

  size_t write = fwrite(p, sizeof(*p), p_len, f);
  if (n != NULL)
    *n = write;

  if (error != NULL) {
    *error = errno;
    if (write == 0 && feof(f))
      *error = EOF;

    clearerr(f);
  }
}

#endif

// FileWriter virtual table for Writer interface.
struct xstd_writer_vtable file_writer_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_writer_vtable file_writer_vtable = {.write = &file_writer_write};
#endif

// Generate file_writer_init, file_writer_new and file_writer constructors.

#undef type_init_proto
#define type_init_proto FILE *f
#undef type_init_args
#define type_init_args f
#undef type_init
#define type_init                                                              \
  { iface_impl_init(t, iface, &file_writer_vtable, f); }

def_type_constructors(FileWriter, file_writer)

#endif
#ifndef XSTD_IO_READ_CLOSER_H_INCLUDE
#define XSTD_IO_READ_CLOSER_H_INCLUDE

#ifdef XSTD_IMPLEMENTATION
#include <stddef.h>
#endif


// ReadCloser is the interface that groups the basic Read and Close methods.
iface_merge_def(ReadCloser, Reader reader; Closer closer);

// FileReadCloser wraps FILE and implements ReadCloser.
iface_impl_def(ReadCloser, FileReadCloser, FILE *);

#undef type_init_proto
#define type_init_proto FILE *f
#undef type_init_args
#define type_init_args f
#undef type_init
#define type_init                                                              \
  {                                                                            \
    iface_impl_init(t, iface.reader, &file_reader_vtable, f);                  \
    iface_impl_init(t, iface.closer, &file_closer_vtable, f);                  \
  }

def_type_constructors(FileReadCloser, file_read_closer)

#endif
#ifndef XSTD_IO_READ_WRITER_H_INCLUDE
#define XSTD_IO_READ_WRITER_H_INCLUDE


// ReadWriter is the interface that groups the basic Read and Write methods.
iface_merge_def(ReadWriter, Reader reader; Writer writer);

// FileReadWriter wraps FILE and implements ReadWriter.
iface_impl_def(ReadWriter, FileReadWriter, FILE *);

#undef type_init_proto
#define type_init_proto FILE *f
#undef type_init_args
#define type_init_args f
#undef type_init
#define type_init                                                              \
  {                                                                            \
    iface_impl_init(t, iface.reader, &file_reader_vtable, f);                  \
    iface_impl_init(t, iface.writer, &file_writer_vtable, f);                  \
  }

def_type_constructors(FileReadWriter, file_read_writer)

#endif
#ifndef XSTD_IO_WRITE_CLOSER_H_INCLUDE
#define XSTD_IO_WRITE_CLOSER_H_INCLUDE

#ifdef XSTD_IMPLEMENTATION
#include <stddef.h>
#endif


// WriteCloser is the interface that groups the basic Write and Close methods.
iface_merge_def(WriteCloser, Writer writer; Closer closer);

// FileWriteCloser wraps FILE and implements WriteCloser.
iface_impl_def(WriteCloser, FileWriteCloser, FILE *);

// file_write_closer_init initializes the given FileWriteCloser.
void file_write_closer_init(FileWriteCloser *frw, FILE *f);

#undef type_init_proto
#undef type_init_args
#undef type_init
#define type_init_proto FILE *f
#define type_init_args f
#define type_init                                                              \
  {                                                                            \
    iface_impl_init(t, iface.writer, &file_writer_vtable, f);                  \
    iface_impl_init(t, iface.closer, &file_closer_vtable, f);                  \
  }

def_type_constructors(FileWriteCloser, file_write_closer)

#endif
#ifndef XSTD_BUF_READER_H_INCLUDE
#define XSTD_BUF_READER_H_INCLUDE

#include <stddef.h>
#include <stdint.h>

#ifdef XSTD_IMPLEMENTATION
#include <string.h>
#endif


// BufReader wraps a Reader
iface_impl_def(
    Reader, BufReader, struct {
      Reader *reader;
      uint8_t *buf;
      size_t buf_len;
      int read_err;
      size_t read;
      size_t write;
    });

size_t buf_reader_buffered(BufReader *buf_reader);

#ifdef XSTD_IMPLEMENTATION
size_t buf_reader_buffered(BufReader *buf_reader) {
  return buf_reader->body_.read - buf_reader->body_.write;
}
#endif

#ifdef XSTD_IMPLEMENTATION
static void buf_reader_read(InterfaceImpl impl, uint8_t *p, size_t p_len,
                            size_t *n, int *err) {
  // p is empty.
  if (p_len == 0)
    return;

  typeof_iface_impl(BufReader) *b = cast_iface_impl_ptr(BufReader, impl);

  size_t buffered = b->read - b->write;

  // No buffered data.
  if (buffered <= 0) {
    if (b->read_err != 0) {
      if (n != NULL)
        *n = 0;
      if (err != NULL)
        *err = b->read_err;
      return;
    }

    // p is larger than internal buffer,
    // read directly into p to avoid copy.
    if (p_len > b->buf_len) {
      reader_read(b->reader, p, p_len, n, err);
      return;
    }

    // Reset cursors.
    b->read = 0;
    b->write = 0;
    // Read into internal buffer.
    reader_read(b->reader, b->buf, b->buf_len, &b->read, &b->read_err);
    if (b->read == 0) {
      if (err != NULL)
        *err = b->read_err;
      return;
    }
    buffered = b->read;
  }

  size_t copied = buffered < p_len ? buffered : p_len;
  memcpy(p, &b->buf[b->write], copied);
  b->write += copied;

  if (n != NULL)
    *n = copied;
}
#endif

struct xstd_reader_vtable buf_reader_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_reader_vtable buf_reader_vtable = {.read = &buf_reader_read};
#endif

#undef type_init_proto
#undef type_init_args
#undef type_init
#define type_init_proto Reader *r, uint8_t *buf, size_t buf_len
#define type_init_args r, buf, buf_len
#define type_init                                                              \
  {                                                                            \
    iface_impl_init(t, iface, &buf_reader_vtable,                              \
                    ((typeof_iface_impl(BufReader)){                           \
                        .reader = r,                                           \
                        .buf = buf,                                            \
                        .buf_len = buf_len,                                    \
                    }));                                                       \
  }

def_type_constructors(BufReader, buf_reader)

#endif
#ifndef XSTD_SLICE_H_INCLUDE
#define XSTD_SLICE_H_INCLUDE

#include <stddef.h>
#include <stdint.h>

#ifdef XSTD_IMPLEMENTATION
#include <assert.h>
#endif

// Slice define a slice/portion of a buffer. Slice doesn't own memory it
// contains.
typedef struct {
  uint8_t *data;
  size_t len;
} Slice;

size_t slice_len(Slice *slice);

#ifdef XSTD_IMPLEMENTATION
size_t slice_len(Slice *slice) { return slice->len; }
#endif

Slice slice(uint8_t *data, size_t length);

#ifdef XSTD_IMPLEMENTATION
Slice slice(uint8_t *data, size_t length) {
  Slice s = {.data = data, .len = length};
  return s;
}
#endif

#endif
#ifndef XSTD_BYTES_BUFFER_H_INCLUDE
#define XSTD_BYTES_BUFFER_H_INCLUDE

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef XSTD_IMPLEMENTATION
#include <assert.h>
#endif


// BytesBuffer define a resizable bytes array that owns its memory.
typedef struct {
  uint8_t *bytes_;
  Allocator *allocator_;
  size_t len_;
  size_t cap_;
} BytesBuffer;

size_t bytes_buffer_capacity(const BytesBuffer *buffer);

#ifdef XSTD_IMPLEMENTATION
size_t bytes_buffer_capacity(const BytesBuffer *buffer) { return buffer->cap_; }
#endif

size_t bytes_buffer_length(const BytesBuffer *buffer);

#ifdef XSTD_IMPLEMENTATION
size_t bytes_buffer_length(const BytesBuffer *buffer) { return buffer->len_; }
#endif

size_t bytes_buffer_available(const BytesBuffer *buffer);

#ifdef XSTD_IMPLEMENTATION
size_t bytes_buffer_available(const BytesBuffer *buffer) {
  return buffer->cap_ - buffer->len_;
}
#endif

void bytes_buffer_reset(BytesBuffer *buffer);

#ifdef XSTD_IMPLEMENTATION
void bytes_buffer_reset(BytesBuffer *buffer) { buffer->len_ = 0; }
#endif

Slice bytes_buffer_slice(const BytesBuffer *buffer, size_t start, size_t end);

#ifdef XSTD_IMPLEMENTATION
Slice bytes_buffer_slice(const BytesBuffer *buffer, size_t start, size_t end) {
  assert(buffer->len_ >= start);
  assert(buffer->len_ < end);
  assert(end >= start);

  return slice(&buffer->bytes_[start], end - start);
}
#endif

Slice bytes_buffer_bytes(const BytesBuffer *buffer);

#ifdef XSTD_IMPLEMENTATION
Slice bytes_buffer_bytes(const BytesBuffer *buffer) {
  return slice(buffer->bytes_, buffer->len_);
}
#endif

bool bytes_buffer_resize(BytesBuffer *buffer, size_t new_capacity);

#ifdef XSTD_IMPLEMENTATION
bool bytes_buffer_resize(BytesBuffer *buffer, size_t new_capacity) {
  uint8_t *bytes =
      alloc_realloc(buffer->allocator_, buffer->bytes_, new_capacity);
  if (bytes == NULL)
    return false;

  buffer->bytes_ = bytes;
  buffer->cap_ = new_capacity;

  if (buffer->cap_ < buffer->len_)
    buffer->len_ = buffer->cap_;
  return true;
}
#endif

#define bytes_buffer_push(buffer, data)                                        \
  bytes_buffer_append_bytes(buffer, data, sizeof(typeof(*data)))

#define bytes_buffer_append(buffer, data, nmemb)                               \
  bytes_buffer_append_bytes(buffer, data, sizeof(typeof(*data)) * nmemb)

size_t bytes_buffer_append_bytes(BytesBuffer *buffer, void *data, size_t size);

#ifdef XSTD_IMPLEMENTATION
size_t bytes_buffer_append_bytes(BytesBuffer *buffer, void *data, size_t size) {
  // Resize if needed.
  bool need_resize = bytes_buffer_available(buffer) < size;
  while (bytes_buffer_available(buffer) < size) {
    if (buffer->cap_ == 0)
      buffer->cap_ = 1;
    else
      buffer->cap_ *= 2;
  }
  if (need_resize)
    if (!bytes_buffer_resize(buffer, buffer->cap_))
      return 0;

  memcpy(&buffer->bytes_[buffer->len_], data, size);
  buffer->len_ += size;

  return size;
}
#endif

void bytes_buffer_fill(BytesBuffer *buffer, int c);

#ifdef XSTD_IMPLEMENTATION
void bytes_buffer_fill(BytesBuffer *buffer, int c) {
  memset(buffer->bytes_, c, buffer->cap_);
  buffer->len_ = buffer->cap_;
}
#endif

void bytes_buffer_fill_available(BytesBuffer *buffer, int c);

#ifdef XSTD_IMPLEMENTATION
void bytes_buffer_fill_available(BytesBuffer *buffer, int c) {
  memset(&buffer->bytes_[buffer->len_], c, bytes_buffer_available(buffer));
  buffer->len_ = buffer->cap_;
}
#endif

#define bytes_buffer_get(buffer, index, type)                                  \
  *(type *)(bytes_buffer_get_(buffer, index, sizeof(type)))

#define bytes_buffer_get_ptr(buffer, index, type)                              \
  ((type *)(bytes_buffer_get_(buffer, index, sizeof(type))))

void *bytes_buffer_get_(const BytesBuffer *buffer, size_t index,
                        size_t elem_size);

#ifdef XSTD_IMPLEMENTATION
void *bytes_buffer_get_(const BytesBuffer *buffer, size_t index,
                        size_t elem_size) {
  assert(index * elem_size < buffer->len_);
  return (void *)&buffer->bytes_[index * elem_size];
}
#endif

void bytes_buffer_init(BytesBuffer *buffer, Allocator *allocator);

#ifdef XSTD_IMPLEMENTATION
void bytes_buffer_init(BytesBuffer *buffer, Allocator *allocator) {
  *buffer = (BytesBuffer){0};
  buffer->allocator_ = allocator;
}
#endif

BytesBuffer *bytes_buffer_new(Allocator *allocator);

#ifdef XSTD_IMPLEMENTATION
BytesBuffer *bytes_buffer_new(Allocator *allocator) {
  BytesBuffer *buf = alloc_malloc(allocator, sizeof(BytesBuffer));
  bytes_buffer_init(buf, allocator);
  return buf;
}
#endif

BytesBuffer bytes_buffer(Allocator *allocator);

#ifdef XSTD_IMPLEMENTATION
BytesBuffer bytes_buffer(Allocator *allocator) {
  BytesBuffer buf = {0};
  bytes_buffer_init(&buf, allocator);
  return buf;
}
#endif

void bytes_buffer_destroy(BytesBuffer *buf);

#ifdef XSTD_IMPLEMENTATION
void bytes_buffer_destroy(BytesBuffer *buf) {
  alloc_free(buf->allocator_, buf->bytes_);
}
#endif

void bytes_buffer_free(BytesBuffer *buf);

#ifdef XSTD_IMPLEMENTATION
void bytes_buffer_free(BytesBuffer *buf) {
  if (buf == NULL)
    return;

  bytes_buffer_destroy(buf);
  alloc_free(buf->allocator_, buf);
}
#endif

#endif
#ifndef XSTD_BYTES_READER_H_INCLUDE
#define XSTD_BYTES_READER_H_INCLUDE

#ifdef XSTD_IMPLEMENTATION
#include <string.h>
#endif


typedef struct {
  Reader reader;
  BytesBuffer *buffer_;
} BytesBufferReader;

#ifdef XSTD_IMPLEMENTATION
static void bytes_buffer_reader_read(void *reader, uint8_t *p, size_t p_len,
                                     size_t *n, int *err) {
  (void)err;

  BytesBuffer *buffer = (BytesBuffer *)reader;
  size_t copied = p_len > buffer->len_ ? buffer->len_ : p_len;
  memcpy(p, buffer->bytes_, copied);
  if (n != NULL)
    *n = copied;
}
#endif

struct xstd_reader_vtable bytes_buffer_reader_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_reader_vtable bytes_buffer_reader_vtable = {
    .read = &bytes_buffer_reader_read,
};
#endif

BytesBufferReader bytes_buffer_reader(BytesBuffer *buffer);

#ifdef XSTD_IMPLEMENTATION
BytesBufferReader bytes_buffer_reader(BytesBuffer *buffer) {
  BytesBufferReader bbw = {0};
  bbw.reader.vtable_ = &bytes_buffer_reader_vtable;
  bbw.reader.offset_ = offsetof(BytesBufferReader, buffer_);
  bbw.buffer_ = buffer;
  return bbw;
}
#endif

#endif
#ifndef XSTD_BYTES_WRITER_H_INCLUDE
#define XSTD_BYTES_WRITER_H_INCLUDE

#ifdef XSTD_IMPLEMENTATION
#include <errno.h>
#include <stddef.h>
#endif


typedef struct {
  Writer writer;
  BytesBuffer *buffer_;
} BytesBufferWriter;

#ifdef XSTD_IMPLEMENTATION
static void bytes_buffer_writer_write(void *writer, uint8_t *p, size_t p_len,
                                      size_t *n, int *err) {

  BytesBuffer *buffer = (BytesBuffer *)writer;
  size_t appended = bytes_buffer_append_bytes(buffer, p, p_len);
  if (n != NULL)
    *n = appended;

  if (err != NULL && appended != p_len)
    *err = ENOMEM;
}
#endif

struct xstd_writer_vtable bytes_buffer_writer_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_writer_vtable bytes_buffer_writer_vtable = {
    .write = &bytes_buffer_writer_write,
};
#endif

BytesBufferWriter bytes_buffer_writer(BytesBuffer *buffer);

#ifdef XSTD_IMPLEMENTATION
BytesBufferWriter bytes_buffer_writer(BytesBuffer *buffer) {
  BytesBufferWriter bbw = {0};
  bbw.writer.vtable_ = &bytes_buffer_writer_vtable;
  bbw.writer.offset_ = offsetof(BytesBufferWriter, buffer_);
  bbw.buffer_ = buffer;
  return bbw;
}
#endif

#endif
#ifndef XSTD_ITER_H_INCLUDE
#define XSTD_ITER_H_INCLUDE

#include <stdbool.h>
#include <stdint.h>


// Iterator interface.
iface_def(
    Iterator,
    struct xstd_iterator_vtable { bool (*next)(InterfaceImpl, void *); });

bool iter_next(Iterator *, void *);

#ifdef XSTD_IMPLEMENTATION
bool iter_next(Iterator *iter, void *n) { return iface_call(iter, next, n); }
#endif

iface_impl_def(
    Iterator, RangeIterator, struct {
      intmax_t value;
      intmax_t end;
      intmax_t step;
    });

#ifdef XSTD_IMPLEMENTATION
static bool range_iterator_next(InterfaceImpl impl, void *next) {
  typeof_iface_impl(RangeIterator) *b =
      cast_iface_impl_ptr(RangeIterator, impl);

  *(intmax_t *)next = b->value;

  b->value += b->step;

  return !((b->step > 0 && b->value >= b->end) ||
           (b->step < 0 && b->value <= b->end));
}
#endif

struct xstd_iterator_vtable range_iterator_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_iterator_vtable range_iterator_vtable = {
    .next = &range_iterator_next,
};
#endif

#undef type_init_proto
#undef type_init_args
#undef type_init
#define type_init_proto int64_t start, int64_t end, int64_t step
#define type_init_args start, end, step
#define type_init                                                              \
  {                                                                            \
    iface_impl_init(t, iface, &range_iterator_vtable,                          \
                    ((typeof_iface_impl(RangeIterator)){                       \
                        .end = end,                                            \
                        .step = step,                                          \
                        .value = start,                                        \
                    }))                                                        \
  }

def_type_constructors(RangeIterator, range_iterator)

#endif
#ifndef XSTD_ARENA_H_INCLUDE
#define XSTD_ARENA_H_INCLUDE

#include <stddef.h>

#ifdef XSTD_IMPLEMENTATION
#include <stdint.h>
#endif


// ArenaAllocator is an arena allocator.
iface_impl_def(
    Allocator, ArenaAllocator, struct {
      Allocator *parent_allocator_;
      size_t arena_size_;
      struct xstd_arena *arena_list_;
      size_t cursor_;
    });

struct xstd_arena {
  // Header
  struct xstd_arena *next;

  // Body
  // ...
};

void arena_allocator_init(ArenaAllocator *arena, Allocator *parent,
                          size_t arena_size);

#ifdef XSTD_IMPLEMENTATION
static void *arena_calloc(InterfaceImpl impl, size_t nmemb, size_t size) {
  typeof_iface_impl(ArenaAllocator) *arena_body =
      cast_iface_impl_ptr(ArenaAllocator, impl);

  // Detect overflow.
  size_t alloc_size = nmemb * size;
  if (alloc_size / size != nmemb ||
      alloc_size + sizeof(struct xstd_arena) < alloc_size) {
    return NULL;
  }

  if (alloc_size > arena_body->arena_size_) {
    struct xstd_arena *new_arena =
        alloc_calloc(arena_body->parent_allocator_, 1,
                     alloc_size + sizeof(struct xstd_arena));
    if (new_arena == NULL)
      return NULL;

    new_arena->next = arena_body->arena_list_;
    arena_body->arena_list_ = new_arena;
    arena_body->cursor_ = arena_body->arena_size_; // Allocated arena as full.
    return (void *)((uintptr_t)new_arena + sizeof(struct xstd_arena));
  }

  // Allocate at least size of a pointer to ensure all pointer are aligned.
  alloc_size = alloc_size < sizeof(void *) ? sizeof(void *) : alloc_size;

  // No current arena.
  if (arena_body->arena_list_ == NULL) {
    struct xstd_arena *new_arena =
        alloc_calloc(arena_body->parent_allocator_, 1,
                     arena_body->arena_size_ + sizeof(struct xstd_arena));
    if (new_arena == NULL)
      return NULL;

    new_arena->next = arena_body->arena_list_;
    arena_body->arena_list_ = new_arena;
    arena_body->cursor_ = 0;
  } else if (arena_body->arena_size_ - arena_body->cursor_ < alloc_size) {
    struct xstd_arena *new_arena =
        alloc_calloc(arena_body->parent_allocator_, 1,
                     arena_body->arena_size_ + sizeof(struct xstd_arena));
    if (new_arena == NULL)
      return NULL;

    new_arena->next = arena_body->arena_list_;
    arena_body->arena_list_ = new_arena;
    arena_body->cursor_ = 0;
  }

  void *ptr = (void *)((uintptr_t)arena_body->arena_list_ +
                       sizeof(struct xstd_arena) + arena_body->cursor_);
  arena_body->cursor_ += alloc_size;
  return ptr;
}

static void *arena_malloc(void *allocator, size_t size) {
  return arena_calloc(allocator, 1, size);
}

static void *arena_realloc(void *allocator, void *ptr, size_t size) {
  (void)ptr;
  // TODO: if last alloc, return same pointer.
  // TODO: if new size is smaller, return same pointer.

  return arena_malloc(allocator, size);
}

static void arena_free(void *allocator, void *ptr) {
  (void)allocator;
  (void)ptr;
}
#endif

struct xstd_allocator_vtable arena_allocator_vtable;

#ifdef XSTD_IMPLEMENTATION
struct xstd_allocator_vtable arena_allocator_vtable = {
    .malloc = &arena_malloc,
    .free = &arena_free,
    .calloc = &arena_calloc,
    .realloc = &arena_realloc,
};
#endif

#undef type_init_proto
#undef type_init_args
#undef type_init
#define type_init_proto Allocator *parent, size_t arena_size
#define type_init_args parent, arena_size
#define type_init                                                              \
  {                                                                            \
    iface_impl_init(t, iface, &arena_allocator_vtable,                         \
                    ((typeof_iface_impl(ArenaAllocator)){                      \
                        .parent_allocator_ = parent,                           \
                        .arena_size_ = arena_size,                             \
                    }))                                                        \
  }

def_type_constructors(ArenaAllocator, arena_allocator)

    void arena_alloc_destroy(ArenaAllocator *arena);

#ifdef XSTD_IMPLEMENTATION
void arena_alloc_destroy(ArenaAllocator *arena_alloc) {
  struct xstd_arena **arena = &arena_alloc->body_.arena_list_;
  while (*arena != NULL) {
    struct xstd_arena *a = *arena;
    *arena = a->next;
    alloc_free(arena_alloc->body_.parent_allocator_, a);
  }
  arena_alloc->body_.arena_list_ = NULL;
}
#endif

#endif
#ifndef XSTD_LIST_H_INCLUDE
#define XSTD_LIST_H_INCLUDE

#ifdef XSTD_IMPLEMENTATION
#include <stddef.h>
#include <stdint.h>
#endif


// typedef a List structure with the given name and of the given type.
#define typedef_list(type, name)                                               \
  typedef struct _xstd_##name {                                                \
    struct name *next_;                                                        \
    type value;                                                                \
  } name

#define list_prepend(list, element)                                            \
  do {                                                                         \
    type_assert_eq(*(list), element);                                          \
    list_prepend_((void **)list, (void *)element);                             \
  } while (0)

// list_prepend_ adds element to beginning of list, replacing its current head.
// An handy list_prepend macro exists so you don't have to cast arguments.
void list_prepend_(void **list, void *element);

#ifdef XSTD_IMPLEMENTATION
void list_prepend_(void **list, void *element) {
  void **element_next = (void **)element;
  *element_next = *list;
  *list = element;
}
#endif

#define list_next(list) ((typeof(list))list_next_(list))

void *list_next_(void *list);

#ifdef XSTD_IMPLEMENTATION
void *list_next_(void *list) {
  if (list == NULL)
    return NULL;
  return *((void **)list);
}
#endif

#define list_remove(list, element)                                             \
  do {                                                                         \
    type_assert_eq(list, element);                                             \
    list_remove_((void *)list, (void *)element);                               \
  } while (0)

// list_remove_ removes element from list but leave element untouched.
// An handy list_remove macro exists so you don't have to cast arguments.
void list_remove_(void *list, void *element);

#ifdef XSTD_IMPLEMENTATION
void list_remove_(void *list, void *element) {
  if (element == NULL)
    return;

  void **list_next = (void **)list;

  while (*list_next != NULL && *list_next != element) {
    list_next = (void **)*list_next;
  }

  *list_next = list_next_(element);
}
#endif

#define list_remove_next(list) (typeof(list))list_remove_next_(list)

// list_remove_next_ removes next element of the given list without altering the
// next element.
void *list_remove_next_(void *list);

#ifdef XSTD_IMPLEMENTATION
void *list_remove_next_(void *list) {
  void **list_next = (void **)list;
  void *next = *list_next;
  *list_next = list_next_(next);

  return next;
}
#endif

#define typedef_list_iterator(list_type, type_name)                            \
  typedef struct {                                                             \
    Iterator iterator;                                                         \
    list_type *list_;                                                          \
  } type_name

#define fndef_list_iterator_init(list_iter_type, list_type, fn_name)           \
  list_iter_type fn_name(list_type *list);                                     \
  list_iter_type fn_name(list_type *list) {                                    \
    list_iter_type iter = {0};                                                 \
    iter.iterator.vtable_ = &list_iterator_vtable;                             \
    iter.iterator.offset_ = offsetof(list_iter_type, list_);                   \
    iter.list_ = list;                                                         \
    return iter;                                                               \
  }

#endif
#ifndef XSTD_OPTION_H_INCLUDE
#define XSTD_OPTION_H_INCLUDE

#include <stdbool.h>

#define Option(type)                                                           \
  typedef union {                                                              \
    bool is_some;                                                              \
    type some;                                                                 \
  }

#define option_is_some(opt) opt.is_some

#define option_is_none(opt) !opt.is_some

#define OptionSome(OptionType, some)                                           \
  (OptionType) { .is_some = true, .some = some }

#define OptionNone(OptionType)                                                 \
  (OptionType) { .is_some = false }

#endif
#ifndef XSTD_RESULT_H_INCLUDE
#define XSTD_RESULT_H_INCLUDE

#include <stdbool.h>

#define Result(OkType, ErrType)                                                \
  struct {                                                                     \
    bool is_ok;                                                                \
    union {                                                                    \
      ErrType err;                                                             \
      OkType ok;                                                               \
    } data;                                                                    \
  }

#define result_is_ok(res) (res).is_ok

#define result_is_err(res) !(res).is_ok

#define ResultError(ResultType, err_value)                                     \
  (ResultType) {                                                               \
    .is_ok = false, .data = {                                                  \
      .err = err_value,                                                        \
    }                                                                          \
  }

#define ResultOk(ResultType, ok_value)                                         \
  (ResultType) {                                                               \
    .is_ok = true, .data = {                                                   \
      .ok = ok_value,                                                          \
    }                                                                          \
  }

#define ResultVoid(ErrType)                                                    \
  struct {                                                                     \
    bool is_ok;                                                                \
    union {                                                                    \
      ErrType err;                                                             \
    } data;                                                                    \
  }

#define ResultOkVoid(ResultType)                                               \
  (ResultType) { .is_ok = true }

#endif
