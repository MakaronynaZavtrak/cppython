import subprocess
from pathlib import Path
import pytest

GOLDEN_ONLY = {"07_errors_runtime"}
PROGRAMS = sorted(
    p for p in (Path(__file__).parent / "programs").glob("*.py")
    if p.stem in GOLDEN_ONLY
)

def run(cppython_bin, prog: Path) -> str:
    proc = subprocess.run(
        [str(cppython_bin), prog.name],
        cwd=prog.parent,
        capture_output=True, timeout=30,
    )

    return proc.stdout.decode("utf-8").replace("\r\n", "\n")

@pytest.mark.parametrize("prog", PROGRAMS, ids=lambda p: p.stem)
def test_program_output(prog, cppython_bin, request):
    actual = run(cppython_bin, prog)
    golden = prog.with_suffix(".out")

    if request.config.getoption("--update-golden"):
        golden.write_bytes(actual.encode("utf-8"))
        pytest.skip(f"обновил {golden.name}")

    assert golden.exists(), f"нет эталона {golden.name}: запусти pytest --update-golden"
    expected = golden.read_bytes().decode("utf-8").replace("\r\n", "\n")
    assert actual == expected