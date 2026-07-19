import os, shutil
from pathlib import Path
import pytest

def pytest_addoption(parser):
    parser.addoption("--update-golden", action="store_true",
                     help="перегенерировать .out вместо сравнения")

@pytest.fixture(scope="session")
def cppython_bin():

    if env := os.environ.get("CPPYTHON_BIN"):
        p = Path(env)
        return p if p.exists() else pytest.skip(f"CPPYTHON_BIN={env} не найден")

    root = Path(__file__).resolve().parent.parent

    for d in ("cmake-build-debug", "cmake-build-release", "build", "build/Debug"):
        for name in ("cppython.exe", "cppython"):
            if (cand := root / d / name).exists():
                return cand

    if found := shutil.which("cppython"):
        return Path(found)
    pytest.skip("бинарник cppython не найден; задай CPPYTHON_BIN")