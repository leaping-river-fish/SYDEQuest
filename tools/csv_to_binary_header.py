#!/usr/bin/env python3
"""Convert CSV level files to C headers: row-major int8_t tile arrays + LevelMetadata for Pico.

Regenerate embedded Pico headers after editing a level CSV, then diff to verify no drift::

    python tools/csv_to_binary_header.py levels/Level1.csv Pico/assets/level1_data.h level1
    python tools/csv_to_binary_header.py levels/Level2.csv Pico/assets/level2_data.h level2
    python tools/csv_to_binary_header.py levels/Level3.csv Pico/assets/level3_data.h level3
    python tools/csv_to_binary_header.py levels/Level4.csv Pico/assets/level4_data.h level4
    python tools/csv_to_binary_header.py levels/Level5.csv Pico/assets/level5_data.h level5

Use ``git diff Pico/assets/level*_data.h`` (or your VCS) to review changes before committing.
"""
import sys
from pathlib import Path


def parse_csv_level(csv_path):
    """Parse CSV file and separate tile data from metadata."""
    with open(csv_path, "r", encoding="utf-8") as f:
        lines = f.readlines()

    tile_lines = []
    metadata_lines = []

    for line in lines:
        line = line.strip()
        if not line:
            continue

        if line[0].isalpha():
            metadata_lines.append(line)
        else:
            tile_lines.append(line)

    return tile_lines, metadata_lines


def parse_tile_rows(tile_lines):
    """Parse comma-separated tile rows into lists of int (-1..15)."""
    rows = []
    for raw in tile_lines:
        parts = [p.strip() for p in raw.split(",")]
        row = []
        for p in parts:
            if not p:
                continue
            v = int(p)
            # -1 air; 0–15 index the 4x4 terrain sheet (see DesktopRenderer::drawTile)
            if v < -1 or v > 15:
                raise ValueError(f"Tile value out of range [-1, 15]: {v}")
            row.append(v)
        rows.append(row)
    if not rows:
        raise ValueError("No tile rows in CSV")
    w = len(rows[0])
    for i, row in enumerate(rows):
        if len(row) != w:
            raise ValueError(f"Row {i} width {len(row)} != first row width {w}")
    return rows


def parse_metadata(metadata_lines, level_name):
    """Parse metadata lines and convert to C struct format."""
    spawn = None
    enemies = []
    health_packs = []
    portals = []
    objectives = []
    bosses = []

    level_id_map = {
        "../levels/Level1.csv": 0,
        "../levels/Level2.csv": 1,
        "../levels/Level3.csv": 2,
        "../levels/Level4.csv": 3,
        "../levels/Level5.csv": 4,
        "Level1.csv": 0,
        "Level2.csv": 1,
        "Level3.csv": 2,
        "Level4.csv": 3,
        "Level5.csv": 4,
    }

    for line in metadata_lines:
        if line.startswith("SPAWN "):
            parts = line.split()
            if len(parts) >= 3:
                spawn = {"x": float(parts[1]), "y": float(parts[2])}

        elif line.startswith("ENEMY_BASIC "):
            parts = line.split()
            if len(parts) >= 3:
                enemies.append(
                    {"x": float(parts[1]), "y": float(parts[2]), "type": "BASIC"}
                )

        elif line.startswith("ENEMY_RANGED "):
            parts = line.split()
            if len(parts) >= 3:
                enemies.append(
                    {"x": float(parts[1]), "y": float(parts[2]), "type": "RANGED"}
                )

        elif line.startswith("ENEMY "):
            parts = line.split()
            if len(parts) >= 3:
                enemies.append(
                    {"x": float(parts[1]), "y": float(parts[2]), "type": "BASIC"}
                )

        elif line.startswith("HEALTH_PACK "):
            parts = line.split()
            if len(parts) >= 3:
                health_packs.append({"x": float(parts[1]), "y": float(parts[2])})

        elif line.startswith("PORTAL "):
            parts = line.split()
            if len(parts) >= 8:
                target_file = parts[5]
                target_id = level_id_map.get(target_file, 0)
                portals.append(
                    {
                        "x": float(parts[1]),
                        "y": float(parts[2]),
                        "w": float(parts[3]),
                        "h": float(parts[4]),
                        "targetLevelId": target_id,
                        "targetX": float(parts[6]),
                        "targetY": float(parts[7]),
                    }
                )

        elif line.startswith("OBJECTIVE "):
            parts = line.split()
            if len(parts) >= 4:
                objectives.append(
                    {"x": float(parts[1]), "y": float(parts[2]), "type": parts[3]}
                )

        elif line.startswith("BOSS "):
            parts = line.split()
            if len(parts) >= 4:
                bosses.append(
                    {"x": float(parts[1]), "y": float(parts[2]), "type": parts[3]}
                )

    return {
        "spawn": spawn if spawn else {"x": 100.0, "y": 100.0},
        "enemies": enemies,
        "health_packs": health_packs,
        "portals": portals,
        "objectives": objectives,
        "bosses": bosses,
    }


def generate_c_structs(metadata, level_name):
    """Generate C struct definitions from parsed metadata."""
    lines = []

    if metadata["enemies"]:
        lines.append(f"const EnemySpawnData {level_name}_enemies[] = {{")
        for enemy in metadata["enemies"]:
            enemy_type = f"LevelEnemyType::{enemy['type']}"
            lines.append(f"    {{{enemy['x']}f, {enemy['y']}f, {enemy_type}}},")
        lines.append("};")
        lines.append("")

    if metadata["health_packs"]:
        lines.append(f"const SpawnData {level_name}_healthPacks[] = {{")
        for hp in metadata["health_packs"]:
            lines.append(f"    {{{hp['x']}f, {hp['y']}f}},")
        lines.append("};")
        lines.append("")

    if metadata["portals"]:
        lines.append(f"const PortalData {level_name}_portals[] = {{")
        for portal in metadata["portals"]:
            lines.append(
                f"    {{{portal['x']}f, {portal['y']}f, {portal['w']}f, {portal['h']}f, "
                f"{portal['targetLevelId']}, {portal['targetX']}f, {portal['targetY']}f}},"
            )
        lines.append("};")
        lines.append("")

    if metadata["objectives"]:
        lines.append(f"const ObjectiveSpawnData {level_name}_objectives[] = {{")
        for obj in metadata["objectives"]:
            obj_type = f"LevelObjectiveType::{obj['type']}"
            lines.append(f"    {{{obj['x']}f, {obj['y']}f, {obj_type}}},")
        lines.append("};")
        lines.append("")

    if metadata["bosses"]:
        lines.append(f"const BossSpawnData {level_name}_bosses[] = {{")
        for b in metadata["bosses"]:
            b_type = f"LevelBossType::{b['type']}"
            lines.append(f"    {{{b['x']}f, {b['y']}f, {b_type}}},")
        lines.append("};")
        lines.append("")

    lines.append(f"const LevelMetadata {level_name}_metadata = {{")
    lines.append(f"    .spawn = {{{metadata['spawn']['x']}f, {metadata['spawn']['y']}f}},")

    if metadata["enemies"]:
        lines.append(f"    .enemies = {level_name}_enemies,")
        lines.append(f"    .enemyCount = {len(metadata['enemies'])},")
    else:
        lines.append("    .enemies = nullptr,")
        lines.append("    .enemyCount = 0,")

    if metadata["health_packs"]:
        lines.append(f"    .healthPacks = {level_name}_healthPacks,")
        lines.append(f"    .healthPackCount = {len(metadata['health_packs'])},")
    else:
        lines.append("    .healthPacks = nullptr,")
        lines.append("    .healthPackCount = 0,")

    if metadata["portals"]:
        lines.append(f"    .portals = {level_name}_portals,")
        lines.append(f"    .portalCount = {len(metadata['portals'])},")
    else:
        lines.append("    .portals = nullptr,")
        lines.append("    .portalCount = 0,")

    if metadata["objectives"]:
        lines.append(f"    .objectives = {level_name}_objectives,")
        lines.append(f"    .objectiveCount = {len(metadata['objectives'])},")
    else:
        lines.append("    .objectives = nullptr,")
        lines.append("    .objectiveCount = 0,")

    if metadata["bosses"]:
        lines.append(f"    .bosses = {level_name}_bosses,")
        lines.append(f"    .bossCount = {len(metadata['bosses'])},")
    else:
        lines.append("    .bosses = nullptr,")
        lines.append("    .bossCount = 0,")

    lines.append("};")

    return "\n".join(lines)


def emit_tile_array(f, level_name, tile_rows):
    """Write constexpr dimensions and row-major int8_t array."""
    height = len(tile_rows)
    width = len(tile_rows[0])
    f.write("#include <cstdint>\n\n")
    f.write(f"constexpr int {level_name}_width = {width};\n")
    f.write(f"constexpr int {level_name}_height = {height};\n\n")
    f.write(f"static const int8_t {level_name}_tiles[{width} * {height}] = {{\n")
    for y, row in enumerate(tile_rows):
        cells = ", ".join(str(v) for v in row)
        f.write(f"    {cells},\n")
    f.write("};\n\n")


def csv_to_binary_header(csv_path, output_path, level_name):
    """Convert CSV level to C header with int8 tile grid + metadata."""
    tile_lines, metadata_lines = parse_csv_level(csv_path)
    tile_rows = parse_tile_rows(tile_lines)
    metadata = parse_metadata(metadata_lines, level_name)

    width = len(tile_rows[0])
    height = len(tile_rows)
    tile_bytes = width * height

    with open(output_path, "w", encoding="utf-8") as f:
        f.write(f"// Auto-generated from {csv_path.name}\n")
        f.write("#pragma once\n")
        f.write('#include "../../Shared/game/level/LevelMetadata.h"\n\n')
        emit_tile_array(f, level_name, tile_rows)
        f.write(generate_c_structs(metadata, level_name))
        f.write("\n")

    print(f"Generated {output_path}")
    print(f"  - Tile grid: {width}x{height} = {tile_bytes} int8_t")
    print(f"  - Enemies: {len(metadata['enemies'])}")
    print(f"  - Health packs: {len(metadata['health_packs'])}")
    print(f"  - Portals: {len(metadata['portals'])}")
    print(f"  - Objectives: {len(metadata['objectives'])}")
    print(f"  - Bosses: {len(metadata['bosses'])}")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: csv_to_binary_header.py <input.csv> <output.h> <level_name>")
        print("Example: csv_to_binary_header.py levels/Level1.csv Pico/assets/level1_data.h level1")
        sys.exit(1)

    csv_to_binary_header(Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3])
