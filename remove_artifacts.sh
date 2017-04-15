#!/bin/bash -x
#
# (c) 2017 Ribose Inc.
#

# Post-make build artifacts that can't be removed via `make maintainer-clean`
artifacts="
  Makefile.in
  aclocal.m4
  buildaux/
  configure
  include/Makefile.in
  m4/
  src/Makefile.in
  src/lib/Makefile.in
  src/lib/config.h.in
  src/lib/config.h.in~
  src/libmj/Makefile.in
  src/rnp/Makefile.in
  src/rnpkeys/Makefile.in
  src/rnpv/Makefile.in
  src/rnpv/config.h
  tests/Makefile.in
"

[[ -s Makefile ]] &&
  make maintainer-clean

for file in ${artifacts}; do
  rm -rf ${file}
done
