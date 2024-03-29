PHP_ARG_ENABLE(fast_io,
  [Whether to enable the "fast_io" extension],
  [ --enable-fast_io   Enable "fast_io" extension support])

if test "$PHP_FAST_IO" != "no"; then
  PHP_NEW_EXTENSION(fast_io, fast_io.c, $ext_shared)
fi