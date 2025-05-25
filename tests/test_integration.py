import subprocess
import os
import sys
import pytest
import platform

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

if platform.system() == "Windows":
    MYPYTHON = os.path.join(ROOT, "cmake-build-debug", "cppython.exe")
else:
    MYPYTHON = os.path.join(ROOT, "build", "cppython")

if not os.path.isfile(MYPYTHON):
    raise FileNotFoundError(f"Не найден ваш REPL: {MYPYTHON}")

PYTHON = sys.executable

def run_cppython(cmds: str | list[str]) -> str:
    """
    Выполняет команды Python с использованием подпроцесса, запускающего интерпретатор Python, и 
    возвращает окончательный обработанный вывод, полученный из подпроцесса.
    
    :param cmds: Строка или список строк, содержащих команды Python для выполнения. 
                 Если предоставлена строка, она разбивается на строки для обработки.
    :return: Строка, представляющая окончательный обработанный вывод из интерпретатора Python, 
             или пустая строка, если действительный результат не получен.
    """
    if isinstance(cmds, str):
        lines = cmds.splitlines()
    else:
        lines = list(cmds)

    stdin = "\n".join(lines) + "\n\nexit\n"
    p = subprocess.run(
        [MYPYTHON],
        input=stdin.encode("utf-8"),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=5,
    )

    payloads = []
    for raw in p.stdout.decode("utf-8", "ignore").splitlines():
        s = raw.lstrip()
        if s.startswith(">>> ") or s.startswith("... "):
            parts = s.replace(">>> ", "###").replace("... ", "###").split("###")
            val = parts[-1].strip() if parts else ""
            if not val:
                continue
            payloads.append(val)

    return payloads[-1] if payloads else ""

def run_cpython(cmds: str | list[str]) -> str:
    """
    Выполняет код или выражение Python, вычисляет конечное выражение и захватывает его вывод. Метод
    динамически создает скрипт Python на основе предоставленных входных данных для оценки одиночного
    выражения или выполнения блока кода с последующей оценкой конечного выражения. Возвращает
    строковое представление результата вычисления.
    
    :param cmds: Строка, содержащая код Python или выражение, либо список строк, где каждая строка
        представляет строку кода Python или выражение.
    :return: Строковое представление вычисленного результата из предоставленного выражения или кода.
    :rtype: str
    """
    if isinstance(cmds, str):
        lines = cmds.splitlines()
    else:
        lines = list(cmds)
        print(lines)

    if len(lines) == 1:
        expr = lines[0]
        code = f"import sys; sys.stdout.write(repr({expr}))"
    else:
        code_to_exec = "\n".join(lines[:-1])
        last_expr = lines[-1].strip()

        code = f"""
import sys
exec('''{code_to_exec}''')
sys.stdout.write(repr({last_expr}))
"""

    p = subprocess.run([PYTHON, "-c", code],
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE,
                       timeout=5)
    out = p.stdout.decode("utf-8", "ignore").splitlines()
    return out[0].strip() if out else ""

@pytest.mark.parametrize("expr,expected", [
    # Два целых числа
    ("2 + 3",                  "5"),
    ("6 - 2",                  "4"),
    ("2 * 3",                  "6"),
    ("2 / 5",                  "0.4"),
    ("17 // 9",                "1"),
    ("17 % 9",                 "8"),
    ("2 ** 3",                 "8"),
    ("2 < 3",                  "True"),
    ("3 < 2",                  "False"),
    ("2 <= 2",                 "True"),
    ("2 <= 1",                 "False"),
    ("2 == 2",                 "True"),
    ("2 == 3",                 "False"),
    ("2 != 1",                 "True"),
    ("2 != 2",                 "False"),
    ("2 >= 2",                 "True"),
    ("2 >= 3",                 "False"),
    ("3 > 2",                  "True"),
    ("2 > 3",                  "False"),

    # Два вещественных числа
    ("2.3 + 3.3",              "5.6"),
    ("6.3 - 2.3",              "4.0"),
    ("0.5 * 2",                "1.0"),
    ("5.0 / 0.5",              "10.0"),
    ("17.0 // 9.0",            "1.0"),
    ("17.0 % 9.0",             "8.0"),
    ("2.25 ** 0.5",            "1.5"),
    ("2.0 < 3.0",              "True"),
    ("3.0 < 2.0",              "False"),
    ("2.0 <= 2.0",             "True"),
    ("2.0 <= 1.0",             "False"),
    ("2.0 == 2.0",             "True"),
    ("2.0 == 3.0",             "False"),
    ("2.0 != 1.0",             "True"),
    ("2.0 != 2.0",             "False"),
    ("2.0 >= 2.0",             "True"),
    ("2.0 >= 3.0",             "False"),
    ("3.0 > 2.0",              "True"),
    ("2.0 > 3.0",              "False"),

    # Целое и вещественное число
    ("2 + 3.0",                "5.0"),
    ("2.0 + 3",                "5.0"),
    ("6 - 2.0",                "4.0"),
    ("6.0 - 2",                "4.0"),
    ("2 * 3.0",                "6.0"),
    ("2.0 * 3",                "6.0"),
    ("2 / 5.0",                "0.4"),
    ("17.0 / 2",               "8.5"),
    ("17 // 9.0",              "1.0"),
    ("17.0 // 9",              "1.0"),
    ("17 % 9.0",               "8.0"),
    ("17.0 % 9",               "8.0"),
    ("2 ** 3.0",               "8.0"),
    ("2.0 ** 3",               "8.0"),
    ("2 < 3.0",                "True"),
    ("2.0 < 3",                "True"),
    ("3.0 < 2",                "False"),
    ("3 < 2.0",                "False"),
    ("2 <= 2.0",               "True"),
    ("2 <= 1.0",               "False"),
    ("2.0 <= 2",               "True"),
    ("2.0 <= 1",               "False"),
    ("2 == 2.0",               "True"),
    ("2.0 == 2",               "True"),
    ("2 == 3.0",               "False"),
    ("2.0 == 3",               "False"),
    ("2 != 1.0",               "True"),
    ("2.0 != 1",               "True"),
    ("2 != 2.0",               "False"),
    ("2.0 != 2",               "False"),
    ("2 >= 2.0",               "True"),
    ("2.0 >= 2",               "True"),
    ("2 >= 3.0",               "False"),
    ("2.0 >= 3",               "False"),
    ("3 > 2.0",                "True"),
    ("3.0 > 2",                "True"),
    ("3 > 2.0",                "True"),
    ("2.0 > 3",                "False"),

    # Две строки
    ('"a" + "b"',              "'ab'"),
    ('"abc" == "abc"',         "True"),
    ('"abd" == "abc"',         "False"),
    ('"abc" != "abd"',         "True"),
    ('"abc" != "abc"',         "False"),
    ('"abc" < "abcd"',         "True"),
    ('"abcd" < "abc"',         "False"),
    ('"alpha" < "zet"',        "True"),
    ('"A" < "a"',              "True"),
    ('"theta" < "alpha"',      "False"),
    ('"abc" < "abc"',          "False"),
    ('"abc" <= "abcd"',        "True"),
    ('"abcd" <= "abc"',        "False"),
    ('"abc" <= "abc"',         "True"),
    ('"abc" <= "abd"',         "True"),
    ('"abc" >= "abc"',         "True"),
    ('"abc" >= "abd"',         "False"),
    ('"abd" >= "abcd"',        "True"),
    ('"abcd" >= "cbd"',        "False"),
    ('"abc" > "abc"',          "False"),
    ('"abd" > "abc"',          "True"),
    ('"abc" > "abd"',          "False"),
    ('"abc" > "abac"',         "True"),
    ('"abac" > "abc"',         "False"),
    ('"abcd" > "abc"',         "True"),
    ('"abc" > "abcd"',         "False"),

    # Строка и целое число
    ('"ab" * 2',               "'abab'"),
    ('2 * "ab"',               "'abab'"),
    ('"ab" * 0',               "''"),
    ('0 * "ab"',               "''"),
    ('"ab" * -1',              "''"),
    ('-1 * "ab"',              "''"),

    # Приоритеты операций
    ('2 + 3 * 4',              "14"),
    ('(2 + 3) * 4',            "20"),
    ('2 + 3 * 4 + 5',          "19"),
    ('2 + 3 * (4 + 5)',        "29"),
    ('2 + (3 * 4) + 5',        "19"),
    ('(2 + 3) * (4 + 5)',      "45"),
    ('5 ** 2 + 4',             "29"),
    ('2 ** 2 * 5',             "20"),
    ('2 ** (2 * 5)',           "1024"),
    ('2 ** 3 ** 2',            "512"),
    ('5 ** 2 * 4 + 17 == 117', "True"),
    ('2 ** 9 / 32 + 3 / 4',    "16.75"),
    ('-2**2',                  "-4"),
    ('(-2)**2',                "4"),
    ('2**-2',                  "0.25"),
    ('100 - 10 - 5',           "85"),
    ('100 // 10 // 3',         "3"),
    ('100 // (10 // 3)',       "33"),
    ('20 / 4 * 2',             "10.0"),
    ('20 // 3 % 4',            "2"),
    ('20 % 3 * 4',             "8"),
    ('1 < 2 < 3',              "True"),
    ('1 < 3 > 2',              "True"),
    ('1 == 1 < 2',             "True"),
    ('2 > 1 == 1',             "True"),
    ('1 + 2 < 5 - 1',          "True"),
    ('1 + 2 <= 3',             "True"),
])

def test_single_line_expressions(expr, expected):
    """
    Этот декоратор позволяет параметрическое выполнение тестовой функции. Тестовые 
    случаи определяются списком кортежей, где каждый кортеж состоит из `expr`
    (строки, представляющей выражение для оценки) и `expected` (строки,
    представляющей ожидаемый результат). Декоратор автоматически запускает тестовую
    функцию для каждого кортежа в списке, предоставляя значения `expr` и
    `expected` для каждого выполнения.

    :param expr: Строка, представляющая выражение, которое должно быть вычислено.
    :param expected: Строка, представляющая ожидаемый результат вычисления выражения.
    :return: None
    """
    my = run_cppython(expr)
    py = run_cpython(expr)
    assert my == expected, f"cppython: {expr!r} -> {my!r}, expected: {expected!r}"
    assert py == expected, f"CPython: {expr!r} -> {py!r}, expected: {expected!r}"
    assert my == py,     f"Mismatch: cppython={my!r} vs CPython={py!r}"

@pytest.mark.parametrize("commands,expected", [
    # if-elif-else
    (["a = 5",
      "if a == 5:",
      "    b = 6",
      "",
      "b"
      ], "6"),

    (["a = 5",
      "if a == 5:",
      "    b = 4",
      "",
      "if a > b:",
      "    c = 'greater'",
      "elif a < b:",
      "    c = 'less'",
      "else:",
      "    c = 'equal'",
      "",
      "c"
      ], "'greater'"),

    (["a = 5",
      "if a == 5:",
      "    b = 6",
      "",
      "if a > b:",
      "    c = 'greater'",
      "elif a < b:",
      "    c = 'less'",
      "else:",
      "    c = 'equal'",
      "",
      "c"
      ], "'less'"),

    (["a = 5",
      "if a == 5:",
      "    b = 5",
      "",
      "if a > b:",
      "    c = 'greater'",
      "elif a < b:",
      "    c = 'less'",
      "else:",
      "    c = 'equal'",
      "",
      "c"
      ], "'equal'"),

    # while
    (["a = 1",
      "s = 0",
      "while a < 101:",
      "    s = s + a",
      "    a = a + 1",
      "",
      "s"
    ], "5050"),

    # while-else
    (["a = 1",
      "s = 0",
      "while a < 101:",
      "    s = s + a",
      "    a = a + 1",
      "else:",
      "    s = 404",
      "",
      "s"
      ], "404"),

    # while-else-break
    (["a = 1",
      "s = 0",
      "while a < 101:",
      "    s = s + a",
      "    a = a + 1",
      "    if a == 5:",
      "        break",
      "else:",
      "    s = 404",
      "",
      "s"
      ], "10"),

    # while-continue
    (["a = 0",
      "s = 0",
      "while a < 10:",
      "    a = a + 1",
      "    if a % 2 == 0:",
      "        continue",
      "    s = s + a",
      "",
      "s"
      ], "25"),
])

def test_multiline_expressions(commands, expected):
    """
    Тестирует вычисление многострочных выражений кода,
    интерпретируемых интерпретаторами cppython и CPython.
    Тесты проверяют корректность присваивания переменных,
    конструкций управления потоком, таких как if-elif-else, циклы,
    и других выражений. Для каждого предоставленного входного случая
    гарантируется, что cppython и CPython производят одинаковые
    результаты, соответствующие ожидаемому выводу.

    :param commands: Список строк, представляющих строки многострочного
        кода Python. Эти команды выполняются последовательно.
    :type commands: List[str]
    :param expected: Ожидаемый вывод, полученный в результате выполнения кода,
        представленного в `commands`.
    :type expected: str
    :return: None
    :raises AssertionError: Если выводы cppython или CPython
        не соответствуют ожидаемому результату `expected` или не соответствуют друг другу.
    """
    my = run_cppython(commands)
    py = run_cpython(commands)
    assert my == expected, f"cppython: {commands!r} -> {my!r}, expected: {expected!r}"
    assert py == expected, f"CPython: {commands!r} -> {py!r}, expected: {expected!r}"
    assert my == py,     f"Mismatch: cppython={my!r} vs CPython={py!r}"