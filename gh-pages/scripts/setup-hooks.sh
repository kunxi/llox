#!/bin/sh
# Install git pre-commit hook for prettier formatting
HOOK_DIR=$(git rev-parse --git-dir 2>/dev/null)/hooks
if [ -d "$HOOK_DIR" ]; then
  cp "$(dirname "$0")/pre-commit" "$HOOK_DIR/pre-commit"
  chmod +x "$HOOK_DIR/pre-commit"
  echo "✓ pre-commit hook installed"
fi
