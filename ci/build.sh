#!/bin/bash

scriptdir="$(dirname "$0")"
export NOAUTODEPS=1
export SNMP_VERBOSE=1
if [ -z "$OSTYPE" ]; then
    case "$(uname)" in
        Linux)  OSTYPE=linux;;
        Darwin) OSTYPE=darwin;;
        *)      OSTYPE="UNKNOWN:$(uname)";;
    esac
    export OSTYPE
fi
"${scriptdir}"/net-snmp-configure master || exit $?
make -s                                  || exit $?
[ "$OSTYPE" = cygwin ]			 && exit 0
"${scriptdir}"/net-snmp-run-tests        || exit $?
