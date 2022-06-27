#!/bin/bash

my_args=( "$@" )

has_config_dir=false

while [[ $# -gt 0 ]]; do
  case $1 in
    --config-dir)
      has_config_dir=true
      shift
      ;;
    *)
      shift
      ;;
  esac
done

if [ "$has_config_dir" = true ] ; then
    prismatik "${my_args[@]}"
else
    prismatik --config-dir "$XDG_CONFIG_HOME/prismatik" "${my_args[@]}"
fi
