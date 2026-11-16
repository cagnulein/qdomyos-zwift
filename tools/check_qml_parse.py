#!/usr/bin/env python3
"""Check that every QML file compiles, with the Qt 5 QML compiler.

  python3 tools/check_qml_parse.py [root] [--github]

  root      folder with src/, by default the repository this script is in
  --github  also print GitHub annotations with file, line and column

QML is only compiled when the app starts, so the C++ build stays green on a QML file that does
not load, for example one where a merge left a property set twice. qmlcachegen compiles each file
without resolving its imports, so it finds syntax errors and duplicate properties, and a module
missing on the runner cannot fail it. It does not replace the emulator tests: types, ids and
runtime errors are not checked here.

qmlcachegen comes from the qtdeclarative5-dev-tools package.
"""
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

DEFAULT_COMPILER = "/usr/lib/qt5/bin/qmlcachegen"
ERROR_PREFIX = "Error compiling qml file: "
ERROR_SEPARATOR = ": error: "


def find_compiler():
    """Path of qmlcachegen, or None"""
    found = shutil.which("qmlcachegen")
    if found:
        return found
    return DEFAULT_COMPILER if Path(DEFAULT_COMPILER).exists() else None


def qml_files(root):
    return sorted((Path(root) / "src").rglob("*.qml"))


def parse_error(text):
    """(line, column, message) of a line of the compiler output, None if it has another form

    The form is: [Error compiling qml file: ]<path>:<line>:<column>: error: <message>
    The prefix comes once per file, so the second error of a file has none.
    """
    if text.startswith(ERROR_PREFIX):
        text = text[len(ERROR_PREFIX):]
    if ERROR_SEPARATOR not in text:
        return None
    place, message = text.split(ERROR_SEPARATOR, 1)
    parts = place.rsplit(":", 2)
    if len(parts) != 3 or not parts[1].isdigit() or not parts[2].isdigit():
        return None
    return int(parts[1]), int(parts[2]), message


def compile_file(compiler, path, output):
    """Lines of the compiler output for one file, empty when the file compiles"""
    result = subprocess.run(
        [compiler, "-o", str(output), str(path)],
        capture_output=True,
        text=True,
        errors="replace",
    )
    if result.returncode == 0:
        return []
    lines = [line for line in (result.stderr + result.stdout).splitlines() if line.strip()]
    return lines or [f"qmlcachegen exited with code {result.returncode}"]


def report(name, text, github):
    """Print one error of the file called name"""
    parsed = parse_error(text)
    if parsed:
        line, column, message = parsed
        print(f"{name}:{line}:{column}: {message}")
        if github:
            print(f"::error file={name},line={line},col={column}::{message}")
    else:
        print(f"{name}: {text}")
        if github:
            print(f"::error file={name}::{text}")


def main(argv=None):
    argv = list(sys.argv[1:] if argv is None else argv)
    github = "--github" in argv
    paths = [a for a in argv if a != "--github"]
    root = Path(paths[0]) if paths else Path(__file__).resolve().parents[1]

    compiler = find_compiler()
    if not compiler:
        print("qmlcachegen not found: install qtdeclarative5-dev-tools")
        return 2
    files = qml_files(root)
    if not files:
        print(f"no QML files in {root / 'src'}")
        return 2

    failed = 0
    with tempfile.TemporaryDirectory() as temp:
        output = Path(temp) / "out.qmlc"
        for path in files:
            lines = compile_file(compiler, path, output)
            if not lines:
                continue
            failed += 1
            name = path.relative_to(root).as_posix()
            for text in lines:
                report(name, text, github)
    print(f"QML files: {len(files)}, with errors: {failed}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
