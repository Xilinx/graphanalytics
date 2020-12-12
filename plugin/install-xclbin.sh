#!/bin/bash

echo "Checking TigerGraph installation directory"
tg_root_dir=$(cat $HOME/.tg.cfg | jq .System.AppRoot | tr -d \")
echo "Found TigerGraph installation in $tg_root_dir"

# Copy xclbins to TG root directory
mkdir -p $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin
cp *.xclbin* $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin

echo "XCLBIN files are copied to $tg_root_dir/dev/gdk/gsql/src/QueryUdf/xclbin"
