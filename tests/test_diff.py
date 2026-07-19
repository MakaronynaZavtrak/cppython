import sys
import subprocess
import difflib
from pathlib import Path
import os
import pytest

PROGRAMS_DIR = Path(__file__).parent / "programs"

DIFF_EXCLUDE = {"07_errors_runtime"}

DIFF_PROGRAMS = sorted(
    p for p in PROGRAMS_DIR.glob("*.py")
    if p.stem not in DIFF_EXCLUDE
)


def _run(argv, prog):
    env = {
        **os.environ,
        "PYTHONIOENCODING": "utf-8",
        "PYTHONUTF8": "1",
    }
    proc = subprocess.run(
        argv + [prog.name],
        cwd=prog.parent,
        capture_output=True,
        timeout=30,
        env=env,
        )
    return proc.stdout.decode("utf-8").replace("\r\n", "\n")


@pytest.mark.parametrize("prog", DIFF_PROGRAMS, ids=lambda p: p.stem)
def test_matches_cpython(prog, cppython_bin):
    mine = _run([str(cppython_bin)], prog)
    ref  = _run([sys.executable], prog)

    if mine == ref:
        return

    diff = "\n".join(difflib.unified_diff(
        ref.splitlines(),
        mine.splitlines(),
        fromfile="CPython",
        tofile="cppython",
        lineterm="",
    ))
    pytest.fail(f"Расхождение с CPython в {prog.name}:\n{diff}", pytrace=False)