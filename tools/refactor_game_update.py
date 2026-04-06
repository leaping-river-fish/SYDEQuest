#!/usr/bin/env python3
"""One-shot splice: refactor Game::update into processPlayerInput/Movement, resolveCollisions, checkDeath, updateLevelState."""
from __future__ import annotations

import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
GAME_CPP = ROOT / "Shared" / "game" / "Game.cpp"


def main() -> int:
    text = GAME_CPP.read_text(encoding="utf-8")

    u_start = text.find("void Game::update() {")
    u_end = text.find("void Game::notifyPlayerDamageHaptics() {")
    if u_start < 0 or u_end < 0 or u_end <= u_start:
        print("Could not find update() / notifyPlayerDamageHaptics() boundaries", file=sys.stderr)
        return 1

    before = text[:u_start]
    middle = text[u_start:u_end]
    after = text[u_end:]

    playing_idx = middle.find("    // Playing")
    physics_idx = middle.find("    // Physics")
    proj_idx = middle.find("    // Update all projectiles")
    stop_idx = middle.find("    // Stop frame after death")
    if playing_idx < 0 or physics_idx < 0 or proj_idx < 0 or stop_idx < 0 or not (playing_idx < physics_idx < proj_idx < stop_idx):
        print("Could not find Playing/Physics/projectiles/stop markers", file=sys.stderr)
        return 1

    preamble = middle[:playing_idx]
    process_move_body = middle[physics_idx:proj_idx]
    resolve_body = middle[proj_idx:stop_idx]

    pit_old = (
        "            return;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    // Check enemy collision damage"
    )
    pit_new = (
        "            return true;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    // Check enemy collision damage"
    )
    if pit_old not in resolve_body:
        print("Pit return pattern not found — aborting", file=sys.stderr)
        return 1
    resolve_body = resolve_body.replace(pit_old, pit_new, 1)
    resolve_body = resolve_body.rstrip() + "\n    return false;\n"

    new_playing = """    // Playing
    playerDiedThisFrame = false;
    ++debugUpdateFrameIndex;

    processPlayerInput(dt);
    processPlayerMovement(dt);
    if (resolveCollisions(dt)) {
        return;
    }
    checkDeath();
    if (state == GameState::GameOver) {
        return;
    }
    if (player.health <= 0) {
        return;
    }
    if (playerDiedThisFrame) {
        return;
    }
    checkPortalCollisions();
    updateLevelState();
}
"""

    new_update = preamble + new_playing

    # Do not use f-strings for helpers: resolve_body contains lambdas with { }.
    helpers = (
        "void Game::processPlayerInput(float dt) {\n"
        "    controller.update(player, input, dt);\n"
        "}\n\n"
        "void Game::processPlayerMovement(float dt) {\n"
        + process_move_body
        + "}\n\n"
        "bool Game::resolveCollisions(float dt) {\n"
        + resolve_body
        + "}\n\n"
        "void Game::checkDeath() {\n"
        "    if (state == GameState::GameOver) {\n"
        "        playerDiedThisFrame = true;\n"
        "    }\n"
        "}\n\n"
        "void Game::updateLevelState() {\n"
        "    camera.follow(player, level);\n"
        "}\n\n"
    )

    new_text = before + helpers + new_update + after

    # Patch checkPortalCollisions: insert guards after opening brace
    portal_sig = "void Game::checkPortalCollisions() {\n    if (player.health <= 0) {"
    portal_repl = """void Game::checkPortalCollisions() {
    if (playerDiedThisFrame) {
        return;
    }
    if (state != GameState::Playing) {
        return;
    }
    if (player.health <= 0) {"""
    if portal_sig not in new_text:
        print("checkPortalCollisions signature not found — aborting", file=sys.stderr)
        return 1
    new_text = new_text.replace(portal_sig, portal_repl, 1)

    GAME_CPP.write_text(new_text, encoding="utf-8", newline="\n")
    print("Wrote", GAME_CPP)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
