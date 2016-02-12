#!/bin/bash

set -e

# Execute Yaafe as command if needed
if [[ "$1" == -* ]]; then
	exec "yaafe" "$@"
fi

exec "$@"
