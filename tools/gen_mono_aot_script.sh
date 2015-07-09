#!/bin/bash
rm mono_aot.sh
find /usr/lib/mono/  *.dll | xargs printf "mono -O=all --aot=full %s\n" >> mono_aot.sh

