#!/bin/bash
# build for all copters boards

set -e

# specify boards
BOARDS="AP3-GA22 AP3-GA22-dev AP3-GA45 AP3-GA45-dev "
BOARDS+="AP6-GA22 AP6-GA22-dev AP6-GA45 AP6-GA45-dev "

# prepare align-build folder
if [ -d "align-build" ]; then
    rm -rf align-build
fi
mkdir align-build

# compile
for b in $BOARDS; do
    echo "Building $b"
    ./waf configure --board $b
    ./waf clean
    ./waf rover
    cp build/$b/bin/ardurover.apj align-build/$b.apj
done

echo "align builds completed"

exit 0