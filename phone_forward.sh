#!/bin/bash

PROGRAM=""
REDIRECTIONS_FILE=""
Y=0

# Program must have 3 arguments.
if [ $# -ge 3 ]; then
  PROGRAM=$1
  REDIRECTIONS_FILE=$2
  Y=$3
else
  echo "ERROR BAD ARGUMENTS" >&2
  exit 1
fi

# If path is relative we add './' at the beggining, so when user enters
# 'main' instead of './main' script should work just fine anytime.
if [[ $PROGRAM != "/"* ]] && [[ $PROGRAM != "~"* ]]; then
  PROGRAM="./$PROGRAM"
fi

# The same way we fix the input file.
if [[ $REDIRECTIONS_FILE != "/"* ]] && [[ $REDIRECTIONS_FILE != "~"* ]]; then
  REDIRECTIONS_FILE="./$REDIRECTIONS_FILE"
fi

# Check if program is valid:
if [ ! -x $PROGRAM ]; then
  echo "ERROR PROGRAM" >&2
  exit 1
fi

# Check if redirections file is valid:
if [ ! -f $REDIRECTIONS_FILE ]; then
  echo "ERROR REDIRECTIONS FILE" >&2
  exit 1
fi

# Check if number is valid:
number_regex='^[0-9]+$'
if ! [[ $Y =~ $number_regex ]] ; then
  echo "ERROR Y" >&2
  exit 1
fi

PHFWD_INPUT=`mktemp`
PHFWD_OUTPUT=`mktemp`
PHFDW_ERR=`mktemp`
RESULT_CANDIDATES=`mktemp`

echo "NEW DB " > $PHFWD_INPUT
cat $REDIRECTIONS_FILE >> $PHFWD_INPUT
echo "? $Y" >> $PHFWD_INPUT

# Check the file size. If everything is OK, nothing should have been printed to
# stderr, so $PHFDW_ERR should have size 0. If error was found we just print in
# on the screen and exit.
$PROGRAM < $PHFWD_INPUT > $PHFWD_OUTPUT 2> $PHFDW_ERR
if [[ -s $PHFDW_ERR ]]; then
  cat $PHFDW_ERR >&2
  exit 1
fi

# Save the output in RESULT_CANDIDATES file.
cat $PHFWD_OUTPUT > $RESULT_CANDIDATES

# We modify the output file, so we can append it to the input and call the
# program once again.
sed -i 's/$/ \?/' $PHFWD_OUTPUT

# Construct the input file once again:
echo "NEW DB " > $PHFWD_INPUT
cat $REDIRECTIONS_FILE >> $PHFWD_INPUT
cat $PHFWD_OUTPUT >> $PHFWD_INPUT

# Run the program again, now check the phnumGet for all values of phnumReverse
# done in the previous program call.
$PROGRAM < $PHFWD_INPUT > $PHFWD_OUTPUT 2> $PHFDW_ERR
if [[ -s $PHFDW_ERR ]]; then
  cat $PHFDW_ERR >&2
  exit 1
fi

LINES_IN_CANDIDATES=`wc -l < $RESULT_CANDIDATES`
LINES_IN_OUTPUT=`wc -l < $PHFWD_OUTPUT`
# We assume that these values are the same. Otherwise something has gone wrong
# so we exit with an information about undefined internal error.
if [ "$LINES_IN_CANDIDATES" -ne "$LINES_IN_OUTPUT" ]; then
  echo "INTERNAL ERROR" >&2
  exit 1
fi

# Read both files, line by line, and save their content into the arrays.
CANDIDATES_ARR=()
OUTPUT_ARR=()

IDX=0
while IFS='' read -r line || [[ -n "$line" ]]; do
  CANDIDATES_ARR[$IDX]="$line"
  IDX=$(( IDX + 1 ))
done < $RESULT_CANDIDATES

IDX=0
while IFS='' read -r line || [[ -n "$line" ]]; do
  OUTPUT_ARR[$IDX]="$line"
  IDX=$(( IDX + 1 ))
done < $PHFWD_OUTPUT

# Not we just check if output of phnumGet was $Y, and if it was we print the
# corresponding candidate as a part of the result.
for (( i=0; i<=$LINES_IN_OUTPUT; i++ )); do
  if [ "$Y" == "${OUTPUT_ARR[$i]}" ]; then
   echo ${CANDIDATES_ARR[$i]}
  fi
done

rm $PHFWD_OUTPUT
rm $PHFWD_INPUT
rm $PHFDW_ERR
rm $RESULT_CANDIDATES
