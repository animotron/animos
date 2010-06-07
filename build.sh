#! /bin/bash
#
# you must stand in phantom home when you run this script
#
export PHANTOM_HOME="`pwd`"
[ -L phantom/kernel ] || ln -s ../oldtree/kernel/phantom phantom/kernel
make -C phantom "$@"
make -C phantom/dev "$@"
make -C oldtree/kernel/phantom/i386 "$@"
make -C oldtree/kernel/phantom/threads "$@"
make -C oldtree/kernel/phantom "$@"
