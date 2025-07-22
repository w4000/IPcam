#!/bin/bash

grep -rl "IP42-327" --include="*.c" --include="*.ini" . | xargs sed -i 's/IP42-327/21C26IP6/g'



