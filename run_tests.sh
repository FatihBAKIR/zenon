#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

#CLANG_OPT="-fsanitize=address"

cd tests/

function compile() {
  OUTPUT=$3
  if [ "$2" = "no_compile" ]; then
    EXPECTED_RET="1"
  else
    EXPECTED_RET="0"
  fi
  ../zenon $1 -o $OUTPUT --cpp "clang++ $CLANG_OPT" 2> /dev/null
  RESULT=$?
  if [ "$RESULT" = "2" ]; then
    echo -e "$1: $RED C++ compilation failed$NC"
    return 1
  fi
  if [ "$RESULT" -gt "2" ]; then
    echo -e "$1: ${RED} Compiler crashed$NC"
    return 1
  fi
  if [ "$RESULT" != "$EXPECTED_RET" ]; then
    if [ "$EXPECTED_RET" = "0" ]; then
      echo -e "$1: $RED Compilation failed$NC"
    else
      echo -e "$1: $RED Compilation succeeded$NC"
    fi
    return 1
  else
    return 0
  fi
}

BINARY_TMP=$(mktemp)

for I in `ls *.znn`; do 
  EXPECTED=`head -n 1 $I | cut -c 4-`
#echo -n "Running $I Expecting: $EXPECTED"
  if [ "$EXPECTED" = "" ]; then
    echo -e "$I: $RED No expected value specified$NC"
    continue
  fi
  compile $I $EXPECTED $BINARY_TMP
  if [ "$?" != "0" ]; then
    continue
  fi
  if [ "$EXPECTED" = "no_compile" ]; then
#    echo -e "$GREEN Success$NC"
    continue
  fi
  $BINARY_TMP
  RESULT=$?
  if [ "$RESULT" != "$EXPECTED" ]; then
    echo -e "$I: $RED Expected $EXPECTED, got $RESULT$NC"
    continue
  fi
#echo -e "$GREEN Success$NC"
done

for D in `ls -d */`; do
  EXPECTED=`head -n 1 $D/main.znn | grep "//"| cut -c 4-`
#echo -n "Running directory $D Expecting: $EXPECTED"
  if [ "$EXPECTED" = "" ]; then
    echo -e "$D: $RED No expected value specified$NC"
    continue
  fi
   OBJECTS=""
  FILES=`ls $D`
  if [ "$EXPECTED" = "no_compile" ]; then
    FILES="main.znn"
  fi
  for I in $FILES; do
#    echo "Compiling $I"
    OPATH=$(mktemp)
    compile "$D/$I -c" $EXPECTED $OPATH
    if [ "$?" != "0" ]; then
      continue 2
    fi
    if [ "$EXPECTED" = "no_compile" ]; then
#echo -e "$GREEN Success$NC"
      continue 2
    fi
    OBJECTS="$OBJECTS $OPATH"
  done
#  echo "Linking $OBJECTS"
  clang++ $OBJECTS -o $BINARY_TMP $CLANG_OPT
  $BINARY_TMP
  RESULT=$?
  if [ "$RESULT" != "$EXPECTED" ]; then
    echo -e "$D: $RED Expected $EXPECTED, got $RESULT$NC"
    continue
  fi
#echo -e "$GREEN Success$NC"
  rm $OPATH
done

rm $BINARY_TMP
