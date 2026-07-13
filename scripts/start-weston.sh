#!/bin/bash
exec weston \
    --backend=rdp-backend.so \
    --port=3390 \
    --rdp-tls-cert=$HOME/.local/share/weston/tls.crt \
    --rdp-tls-key=$HOME/.local/share/weston/tls.key \
    --width="$1" \
    --height="$2"