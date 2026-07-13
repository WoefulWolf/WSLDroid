#!/bin/bash
exec socat TCP-LISTEN:5555,reuseaddr,fork TCP:$1:5555