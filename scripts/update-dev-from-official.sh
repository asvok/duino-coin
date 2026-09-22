#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

if [[ "$(git branch --show-current)" != "dev" ]]; then
  echo "Switch to the dev branch before updating." >&2
  exit 1
fi

if [[ -n "$(git status --porcelain)" ]]; then
  echo "Commit or stash local changes before updating dev." >&2
  exit 1
fi

if ! git remote get-url upstream >/dev/null 2>&1; then
  git remote add upstream https://github.com/duino-coin/duino-coin.git
fi

upstream_url="$(git remote get-url upstream)"
case "$upstream_url" in
  https://github.com/duino-coin/duino-coin.git|git@github.com:duino-coin/duino-coin.git) ;;
  *)
    echo "The upstream remote does not point to duino-coin/duino-coin." >&2
    exit 1
    ;;
esac

git fetch upstream master

if git merge-base --is-ancestor upstream/master HEAD; then
  echo "dev already contains the latest official master."
  exit 0
fi

git merge --no-ff --no-edit upstream/master
echo "Official updates are now in local dev. Review, build, then push dev to your own repository."
