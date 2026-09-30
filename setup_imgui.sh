#!/bin/bash
# CrocElec — récupération de Dear ImGui (branche docking, commit épinglé).
# Le pin vit dans imgui_pin.env : setup et CI lisent la même source.
set -e
cd "$(dirname "$0")"
source ./imgui_pin.env

if [[ "$OSTYPE" == "darwin"* ]]; then
    command -v brew >/dev/null && brew list glfw >/dev/null 2>&1 || brew install glfw
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    if   command -v apt    >/dev/null; then sudo apt install -y cmake libglfw3-dev libgl1-mesa-dev g++
    elif command -v dnf    >/dev/null; then sudo dnf install -y cmake glfw-devel mesa-libGL-devel gcc-c++
    elif command -v pacman >/dev/null; then sudo pacman -S --needed cmake glfw mesa gcc
    fi
fi

if [[ ! -d imgui/.git ]]; then
    rm -rf imgui
    git init -q imgui
    git -C imgui remote add origin "$IMGUI_REPO"
fi

# Le commit exact d'abord (GitHub l'autorise, le clone reste superficiel) ;
# repli sur la tête de branche avec avertissement, car `docking` est
# force-pushé à chaque rebase et la tête n'est alors plus ce qu'on a testé.
if git -C imgui fetch --depth 1 origin "$IMGUI_COMMIT" 2>/dev/null; then
    git -C imgui checkout -q FETCH_HEAD
else
    echo "  ! commit $IMGUI_COMMIT introuvable — repli sur origin/$IMGUI_BRANCH"
    git -C imgui fetch --depth 1 origin "$IMGUI_BRANCH"
    git -C imgui checkout -q FETCH_HEAD
fi

echo "Dear ImGui $IMGUI_VERSION prêt ($(git -C imgui rev-parse --short HEAD))"
