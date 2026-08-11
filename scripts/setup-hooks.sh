#!/bin/sh
# Install git hooks
HOOK_DIR=$(git rev-parse --git-dir 2>/dev/null)/hooks
if [ -d "$HOOK_DIR" ]; then
  DIR=$(dirname "$0")
  cp "$DIR/pre-commit" "$HOOK_DIR/pre-commit"
  chmod +x "$HOOK_DIR/pre-commit"
  echo "✓ pre-commit hook installed"
fi
