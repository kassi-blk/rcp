#!/bin/bash
aname="$1"
src="$2"
dst="$3"

# In case of the local copying, the archive is in the current directory
# without an extension
if [ -e "$aname" ]; then
    mv "$aname" "$aname.tar"
fi

if [ -e "$dst" ]; then
    if [ -d "$dst" ]; then
        tar -xkf "$aname.tar" -C "$dst"
    else
        # Do not allow overwriting an existing file
        echo "Overwrite protection"
        exit 1
    fi
else
    cd `dirname "$dst"`
    mkdir "$aname"
    tar -xkf "$aname.tar" -C "$aname"
    mv "$aname"/`basename "$src"` `basename "$dst"`
    rmdir "$aname"
fi

rm "$aname.tar"

exit 0
