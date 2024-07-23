#!/bin/bash
#
# wrapper for module tester
#
# usage: ./test.sh <module> <func> <file> [path] [arg]
#
# options
#   $1: module.so
#   $2: file|dir|rel
#   file: /path/to/reldir/foo.mp3
#   path: /path/to/reldir
#   last arg: 'strace' 'gdb' to debug
#
# scripts check module for mod_func_file/mod_func_dir/mod_func_rel
#
if [ -z "$1" ]; then
  echo "usage: ./test.sh <module> [func] <file> [path]"
  echo "see script comments"
  exit 0
fi

module="$1"
func="file" # set to 'file', 'dir' or 'rel'
test="test_module" # set 'test_module' or 'do_module'

echo "$2" | grep -Eq "^file|dir|rel$" && func="$2"

test -n "$module" || {
  echo "ERROR: module not specified"
  exit 1
}

{ make clean && make tester; } || {
  echo "make tester failed, exiting"
  exit 1
}

if grep -q "$(basename "$module")" Makefile; then
  make "$module" || { echo "make module failed, exiting"; exit 1; }
else
  echo "module not in Makefile, exiting"
  exit 1
fi

test -s "${module}_debug.so" && cp -v "${module}_debug.so" "${module}.so"

# func test_module
#   tester testpre.cfg <func> <mod_name> <filepath> <dirpath> <releasepath>
#   example: ./tester testpre.cfg file ./mod_flac.so /path/to/01-foo.fla
#
# func do_module
#   tester <func> <mod_name> <filepath>
#   example: ./tester file ./mod_flac.so /path/to/01-foo.fla
#
# func_mode 1:file 2:dir 3:rel 4:do_module
#

CMD="./tester"

echo "$*" | grep -q "strace" && CMD="strace ./tester"
echo "$*" | grep -q "gdb" && CMD="gdb --args ./tester"

ARGS+="testpre.cfg ./$(basename "${module}")"

if [ "$test" = "test_module" ]; then
  _mod="${module/%.so/.c}"
  echo "$module" | grep -q "_debug.so" && _mod="${module/%_debug.so/.c}"
  grep -m1 -q "_${func}_func(" "$_mod" || \
    { echo "ERROR: module doesnt support func"; exit 1; }
  case $func in
    file) ARGS+=" 1 $3 " ;;
    dir|rel) ARGS+=" 1 $3 $4" ;;
    *) echo "ERROR: invalid func"; exit 1 ;;
  esac
else
  ARGS+=" 4 $3 $4"
fi

echo
echo "INFO: [test.sh] running tester cmd $test ..."
echo $CMD $ARGS
echo
$CMD $ARGS
echo "INFO: [test.sh] tester returned code '$?'"
echo

