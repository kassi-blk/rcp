#!/bin/bash
aname="$1"
src="$2"

tar -cf "$aname" --exclude="$aname" -C `dirname "$src"` `basename "$src"`

exit 0
