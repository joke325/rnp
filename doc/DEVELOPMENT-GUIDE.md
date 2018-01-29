# Introduction

The following are a set of conventions and items that are relevant to
contributors.

# Contributing

## Pull Requests

See also: [Guides - Code Review](https://github.com/riboseinc/guides/tree/master/code-review)

Pull Requests should be used for any non-trivial changes. This presents
an opportunity for feedback and allows the CI tests to complete prior to
merging.

The `master` branch should generally always be in a buildable and
functional state.

Pull Requests should be:

* Focused. Do not include changes that are unrelated to the main purpose
  of the PR.
* As small as possible. Sometimes large pull requests may be necessary
  for adding complex features, but generally they should be kept as small
  as possible to ensure a quick and thorough review process.
* Related to a GH issue to which you are assigned. If there is none,
  file one (but search first!). This ensures there is no duplication of
  effort and allows for a discussion prior to beginning work.
  (This may not be necessary for PRs that are purely documentation updates)
* Approved by **2** reviewers before merging.
  (Updates related to policies, like this section, should be approved by
  the project owner)
* Merged by a reviewer via the most appropriate method
  (see [here](https://github.com/riboseinc/guides/tree/master/protocol/git)).

## Branches

See also: [Guides - Protocol / Git](https://github.com/riboseinc/guides/tree/master/protocol/git)

Git branches should be used generously. Most branches should be topic branches,
created for adding a specific feature or fixing a specific bug.

Keep branches short-lived (treat them as disposable/transient) and try to
avoid long-running branches.

A good example of using a branch would be:

* User `@joe` notices a bug where a NULL pointer is dereferenced during
  key export. He creates GH issue `#500`.
* He creates a new branch to fix this bug named
  `joe-500-fix-null-deref-in-pgp_export_key`.
* Joe commits a fix for the issue to this new branch.
* Joe creates a Pull Request to merge this branch in to master.
* Once merged, Joe deletes the branch since it is no longer useful.

Branch names may vary but should be somewhat descriptive, with words
separated by hyphens. It is also helpful to start the branch name with
your github username, to make it clear who created the branch and
prevent naming conflicts.

Remember that branch names may be preserved permanently in the commit
history of `master`, depending on how they are merged.

## Commits

* Try to keep commits as small as possible. This may be difficult or
  impractical at times, so use your best judgement.
* Each commit should be buildable and should pass all tests. This helps
  to ensure that git bisect remains a useful method of pinpointing issues.
* Commit messages should follow 50/72 rule.
* When integrating pull requests, merge function should be prefered over
  squashing. From the other hand, developers should squash commits and
  create meaningful commit stack before PR is merged into mainstream branch.
  Merging commits like "Fix build" or "Implement comments from code review"
  should be avoided.

# Continuous Integration (Travis CI)

Travis CI is used for continuously testing new commits and pull
requests.

We use the sudo-less beta Ubuntu Trusty containers, which do not permit
root access.

See the file `.travis.yml` and the scripts in `ci/` for the most
up-to-date details.

## Reproducing Locally

Sometimes tests fail in Travis CI and you will want to reproduce them
locally for easier troubleshooting.

We can use a container for this, like so:

``` sh
./travis.sh
# or
# docker run -ti --rm travisci/ci-garnet:packer-1490989530 bash -l
```

(Refer to
[here](https://docs.travis-ci.com/user/common-build-problems/#Troubleshooting-Locally-in-a-Docker-Image)
and [here](https://hub.docker.com/r/travisci/ci-garnet/tags/))

Inside the container, you will need to perform steps like the following:

``` sh
cd ~/
git clone https://github.com/riboseinc/rnp
# or if testing local copy
# git clone /usr/local/rnp
cd rnp
export LOCAL_BUILDS="$HOME/local-builds"
export BOTAN_INSTALL="${LOCAL_BUILDS}/botan-install"
export CMOCKA_INSTALL="${LOCAL_BUILDS}/cmocka-install"
export JSONC_INSTALL="${LOCAL_BUILDS}/jsonc-install"
export GPG21_INSTALL="${LOCAL_BUILDS}/gpg21-install"
export BUILD_MODE=normal
ci/install.sh
env CC=clang ci/main.sh
```

(The above uses clang as the compiler -- use `CC=gcc` for GCC)
Refer to the current `.travis.yml` for the most up-to-date information
on what environment variables need to be set.

# Code Coverage

CodeCov is used for assessing our test coverage.
The current coverage can always be viewed here: https://codecov.io/github/riboseinc/rnp/

# Security / Bug Hunting

## Static Analysis

### Coverity Scan

Coverity Scan is used for occasional static analysis of the code base.

To initiate analysis, a developer must push to the `coverity_scan` branch.
You may wish to perform a clean clone for this, like so:

``` sh
cd /tmp

git clone https://github.com/riboseinc/rnp
# or
# git clone git@github.com:riboseinc/rnp.git
cd rnp

# switch to the coverity_scan branch
git checkout coverity_scan

# replay all commits from master onto coverity_scan
git rebase master coverity_scan

# forcefully push the coverity_scan branch
git push -u origin coverity_scan -f
```

Note: The `master` and `coverity_scan` branches have separate
`.travis.yml` files, so you may need to perform a manual merge. In
general, the `coverity_scan` branch's `.travis.yml` is identical to
`master`'s, but with a build matrix of only one entry.

The results can be accessed on
https://scan.coverity.com/projects/riboseinc-rnp. You will need to
create an account and request access to the riboseinc/rnp project.

Since the scan results are not updated live, line numbers may no longer
be accurate against the `master` branch, issues may already be resolved,
etc.

### Clang Static Analyzer

Clang includes a useful static analyzer that can also be used to locate
potential bugs.

To use it, pass the build command to `scan-build`:

``` sh
./configure
scan-build make -j4
[...]
scan-build: 6 bugs found.
scan-build: Run 'scan-view /tmp/scan-build-2017-05-29-223318-9830-1' to examine bug reports.
```

Then use `scan-view`, as indicated above, to start a web server and use
your web browser to view the results.

## Dynamic Analysis

### Fuzzer

It is often useful to utilize a fuzzer like
["american fuzzy lop" ("AFL")](http://lcamtuf.coredump.cx/afl/) to find
ways to improve the robustness of the code base.

Currently, we have a very simple test program in
`src/fuzzers/fuzz_keys`, which will attempt to load an armored key file
passed on the command line. We can use this with AFL to try to produce
crashes, which we can then analyze for issues.

1. Install AFL.
2. Rebuild, using the `afl-gcc` compiler.
    * It's probably easiest to also do a static build, using the
      `--disable-shared` option to `configure`.
    * It may be helpful to occasionally enable the address sanitizer,
      which tends to help produce crashes that may not otherwise be found.
      Read the documentation for AFL first to understand the challenges
      with ASan and AFL.
3. Create directories for input files, and for AFL output.
4. Run `afl-fuzz`.
5. When satisfied, exit with `CTRL-C`.
6. Analyze the crashes/hangs in the output directory.

Here is an example:

``` sh
env CC=afl-gcc AFL_HARDEN=1 CFLAGS=-ggdb ./configure --disable-shared
make -j$(grep -c '^$' /proc/cpuinfo) clean all
mkdir afl_in afl_out
cp some_tests/*.asc afl_in/
afl-fuzz -i afl_in -o afl_out src/fuzzing/fuzz_keys @@
ctrl-c to exit
valgrind -q src/fuzzing/fuzz_keys < afl_out/[...]
```

#### Further Reading

* AFL's `README`, `parallel_fuzzing.txt`, and other bundled documentation.
* See [Tutorial: Instrumented fuzzing with american fuzzy lop](https://fuzzing-project.org/tutorial3.html)

### Clang Sanitizer

Clang and GCC both support a number of sanitizers that can help locate
issues in the code base during runtime.

To use them, you should rebuild with the sanitizers enabled, and then
run the tests (or any executable):

``` sh
env CC=clang CFLAGS="-fsanitize=address,undefined" LDFLAGS="-fsanitize=address,undefined" ./configure
make -j4
src/tests/rnp_tests
```

Here we are using the
[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html)
and
[UndefinedBehaviorSanitizer](https://clang.llvm.org/docs/UndefinedBehaviorSanitizer.html).

This will produce output showing any memory leaks, heap overflows, or
other issues.

# Code Conventions

C is a very flexible and powerful language. Because of this, it is
important to establish a set of conventions to avoid common problems and
to maintain a consistent code base.

## Code Formatting

`clang-format` (v4.0.0) can be used to format the code base, utilizing
the `.clang-format` file included in the repository.

### clang-format git hook

A git pre-commit hook exists to perform this task automatically, and can
be enabled like so:

``` sh
cd rnp
git-hooks/enable.sh
```

If you do not have clang-format v4.0.0 available, you can use a docker
container for this purpose by setting `USE_DOCKER="yes"` in
`git-hooks/pre-commit.sh`.

This should generally work if you commit from the command line.

Note that if you have unstaged changes on some of the files you are
attempting to commit, which have formatting issues detected, you will
have to resolve this yourself (the script will inform you of this).

If your commit does not touch any `.c`/`.h` files, you can skip the
pre-commit hook with git's `--no-verify`/`-n` option.

### clang-format (manually)

If you are not able to use the git hook, you can run `clang-format`
manually in a docker container.

Create a suitable container image with:

``` sh
docker run --name=clang-format alpine:latest apk --no-cache add clang
docker commit clang-format clang-format
docker rm clang-format
```

You can then reformat a file (say, `src/lib/bn.c`) like so:

``` sh
cd rnp
docker run --rm -v $PWD:/rnp -w /rnp clang-format clang-format -style=file -i src/lib/bn.c
```

Also you may wish to reformat all modified uncommited files:
``` sh
docker run --rm -v $PWD:/rnp -w /rnp clang-format clang-format -style=file -i `git ls-files -m |grep "\.\(c\|h\)\$"`
```
...or files, modified since referenced commit, say `54c5476`:
``` sh
docker run --rm -v $PWD:/rnp -w /rnp clang-format clang-format -style=file -i `git diff --name-only 54c5476..HEAD |grep "\.\(c\|h\)\$"`
```

## Style Guide

In order to keep the code base consistent, we should define and adhere
to a single style.

When in doubt, consult the existing code base.

### Naming

The following are samples that demonstrate the style for naming
different things.

* Functions: `some_function`
* Variables: `some_variable`
* Filenames: `packet-parse.c` `packet-parse.h`
* Struct: `pgp_key_t`
* Typedefed Enums: `pgp_pubkey_alg_t`
* Enum Values: `PGP_PKA_RSA = 1`
* Constants (macro): `RNP_BUFSIZ`

### General Guidelines

Do:

* Do use header guards (`#ifndef SOME_HEADER_H [...]`) in headers.
* Do use `sizeof(variable)`, rather than `sizeof(type)`. Or
  `sizeof(*variable)` as appropriate.
* Do use commit messages that close GitHub issues automatically, when
  applicable. `Fix XYZ. Closes #78.` See
  [here](https://help.github.com/articles/closing-issues-via-commit-messages/).
* Do declare functions `static` when they do not need to be referenced
  outside the current source file.
* Do always use braces for conditionals, even if the block only contains a
  single statement.
  ```c
  if (something) {
    return val;
  }
  ```
* Do use a default failure (not success) value for `ret` variables. Example:
  ```
  rnp_result_t ret = RNP_ERROR_GENERIC;
  // ...

  return ret;
  ```

Do not:
* Do not use the static storage class for local variables, *unless* they
  are constant.

  **Not OK**
  ```c
  int somefunc() {
    static char buffer[256];
    //...
  }
  ```
  **OK**
  ```c
  int somefunc() {
    static const uint16_t some_data[] = {
      0x00, 0x01, 0x02, //...
    };
  }
  ```
* Do not use `pragma`, and try to avoid `__attribute__` as well.
* Do not use uninitialized memory. Try to ensure your code will not cause any errors in valgrind and other memory checkers.

### Documentation
Documentation is done in Doxygen comments format, which must be put in header files.

Exception are static or having only definition functions - it is not required to document them,
however if they are documented then this should be done in the source file and using the @private tag.

Comments should use doxygen markdown style, like the following example:

```c
/** Some comments regarding the file purpose, like 'PGP packet parsing utilities'
 *  @file
 */

/** brief description of the sample function which does something, keyword 'brief' is ommitted
 *  Which may be continued here
 *
 *  After an empty line you may add detailed description in case it is needed. You may put
 *  details about the memory allocation, what happens if function fails and so on.
 *
 *  @param param1 first parameter, null-terminated string which should not be NULL
 *  @param param2 integer, some number representing something
 *  @param size number of bytes available to store in buffer
 *  @param buffer buffer to store results, may be NULL. In this case size can be used to
 *                obtain the required buffer length
 *  @return 0 if operation succeeds, or error code otherwise. If operation succeeds then buffer
 *          is populated with the resulting data, and size contains the length of this data.
 *          if error code is E_BUF_TOOSMALL then size will contain the required size to store
 *          the result
 **/
rnp_result_t
rnp_do_operation(const char *param1, const int param2, int *size, char *buffer);
```

# Reviewers and Responsibility areas

The individuals are responsible for the following areas of `rnp`.
When submitting a Pull Request please seek reviews by whoever is
responsible according to this list.

General:

* Code style: @dewyatt
* Algorithms: @randombit, @dewyatt, @flowher, @catap, @ni4
* Performance: @catap, @ni4
* CLI: @ni4
* GnuPG compatibility: @MohitKumarAgniotri, @frank-trampe, @ni4
* Security Testing/Analysis: @MohitKumarAgniotri, @flowher
* Autotools: @randombit, @zgyarmati, @catap

Data formats:

* OpenPGP Packet: @randombit, @catap, @ni4
* Keystore: @catap
* JSON: @zgyarmati
* SSH: @ni4

Bindings:

* FFI: @dewyatt
* Ruby: @dewyatt
* Java/JNI: @catap
* Obj-C/Swift: @ni4
* Python: @dewyatt, @ni4

Platforms:

* RHEL/CentOS: @dewyatt
* BSD:
* Windows:
* macOS / iOS / homebrew: @ni4
* Debian: @zgyarmati
