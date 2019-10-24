#!/bin/bash -ex

pwd=`pwd`
cd src/cgo && \
rm -rf go_config_manager.h && \
go build -buildmode=c-archive -o go_config_manager.a && \
cd $pwd && \
rm -rf build compile_commands.json && mkdir -p build && pwd
mv src/cgo/go_config_manager.h include/eis/config_manager
mv src/cgo/go_config_manager.a build/libgoconfigmanager.a
