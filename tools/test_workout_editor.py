#!/usr/bin/env python3
"""Regression checks for Workout Editor rower resistance support."""

from pathlib import Path
import re


SOURCE = Path(__file__).parents[1] / "src/inner_templates/workouteditor/workout-editor-app.js"


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")

    resistance_field = re.search(
        r"\{ key: 'resistance'.*?devices: \[(?P<devices>[^]]+)\]",
        source,
    )
    assert resistance_field, "Workout Editor must define a resistance field"
    assert "'rower'" in resistance_field.group("devices"), (
        "Workout Editor resistance field must be available for rowers"
    )

    rower_series = re.search(
        r"rower:\s*\[(?P<body>.*?)\n\s*\],\n\s*jumprope:",
        source,
        re.DOTALL,
    )
    assert rower_series, "Workout Editor must define the rower chart series"
    assert "key: 'resistance'" in rower_series.group("body"), (
        "Workout Editor rower preview must include resistance"
    )

    rower_defaults = re.search(
        r"case 'rower':(?P<body>.*?)\n\s*case 'treadmill':",
        source,
        re.DOTALL,
    )
    assert rower_defaults, "Workout Editor must define rower defaults"
    assert "base.resistance = 1" in rower_defaults.group("body"), (
        "Rower intervals must start from the machine's level-1 default"
    )
    assert "base.__enabled_resistance = false" in rower_defaults.group("body"), (
        "Rower resistance must remain opt-in for existing workouts"
    )

    # Elevation-based targets are exclusive to treadmill and must survive editor save/load.
    elevation_field = re.search(
        r"\{ key: 'elevationGain'.*?devices: \[(?P<devices>[^]]+)\]",
        source,
    )
    assert elevation_field, "Workout Editor must expose an elevation gain field"
    assert "'treadmill'" in elevation_field.group("devices")
    assert "out.__enabled_elevationGain !== true" in source
    assert "row.__enabled_elevationGain = false" in source
    assert "row.elevationGain = target" in source
    assert "interval.__enabled_elevationGain === true" in source
    assert "duration = 60;" in source, "D+ chart duration is illustrative, not an ETA"

    print("Workout Editor rower resistance and elevation target regression checks passed")


if __name__ == "__main__":
    main()
