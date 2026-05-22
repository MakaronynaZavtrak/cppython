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

    if len(lines) == 1:
        expr = lines[0]

        code = f"""
import sys

_result = eval({expr!r})

if _result is not None:
    sys.stdout.write(repr(_result))
"""
    else:
        code_to_exec = "\n".join(lines[:-1])
        last_expr = lines[-1].strip()

        code = f"""
import sys

_ns = {{"__builtins__": __builtins__}}

exec({code_to_exec!r}, _ns)

_result = eval({last_expr!r}, _ns)

if _result is not None:
    sys.stdout.write(repr(_result))
"""

    p = subprocess.run([PYTHON, "-c", code],
                       stdout=subprocess.PIPE,
                       stderr=subprocess.PIPE,
                       timeout=5)

    err = p.stderr.decode("utf-8", "ignore")

    if err:
        print("STDERR:")
        print(err)

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
    ("2 ** -3",                "0.125"),
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

    # огромные числа
    ("123456789123456789 + 1 ", "123456789123456790"),
    ("999999999999999999 * 2", "1999999999999999998"),
    ("10**50", "100000000000000000000000000000000000000000000000000"),
    ("10**100 + 10**100", "20000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
    ("(10**50) * (10**50)", "10000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"),
    ("1e3", "1000.0"),
    ("1e-3", "0.001"),
    ("2E2", "200.0"),
    ("-3e4", "-30000.0"),
    ("-3e4 + 15", "-29985.0"),

    # underscore в числах
    ("1_000_000", "1000000"),
    ("10_000 + 5_000", "15000"),
    ("1_2_3_4", "1234"),

    # сравнение больших чисел
    ("10 ** 50 > 10 ** 49", "True"),
    ("10 ** 50 == 10 ** 50", "True"),
    ("10 ** 50 < 10 ** 100", "True"),

    # cоздание list с помощью литералов
    ("[]", "[]"),
    ("[1, 2, 3]", "[1, 2, 3]"),

    # вложенные списки
    ("[[1, 2], [3, 4, [5, [6]]]]", "[[1, 2], [3, 4, [5, [6]]]]"),

    # игнорирование последней запятой
    ("[1, 2, 3,]", "[1, 2, 3]"),

    # полиморфность списка
    ("[1, True, 'abc']", "[1, True, 'abc']"),

    # индексация
    ("[1, 2, 3][1]", "2"),

    # отрицательная индексация
    ("[1, 2, 3, 4][-1]", "4"),
    ("[1, 2, 3, 4][-3]", "2"),

    # приведение boolean значения к int
    ("[1, 2, 3, 4][False]", "1"),
    ("[1, 2, 3, 4][True]", "2"),
    ("[1, 2, 3, 4][-True]", "4"),

    # evaluating значения по индексу
    ("[1, 2, 3, 4][False + 2] == 3", "True"),
    ("[1, 2, 3, 4][False] != 1", "False"),

    # индексация во вложенных списках
    ("[1, [2, [3, 4]], 5][1][1][1]", "4"),

    # сравнение списков
    ("[1, 2] < [2, 3]", "True"),
    ("[1, 2] < [1]", "False"),
    ("[1, 2] <= [2, 3]", "True"),
    ("[] == []", "True"),
    ("[1] == [1]", "True"),
    ("[1, 2] == [1, 2]", "True"),
    ("[1, 2] != [1, 3]", "True"),
    ("[1, [2, 3]] == [1, [2, 3]]", "True"),
    ("[1] < [2]", "True"),
    ("[1, 2] < [1, 3]", "True"),
    ("[1, 2] < [1, 2, 0]", "True"),
    ("[1, 2, 5] > [1, 2, 0]", "True"),
    ("[] < [1]", "True"),
    ("[[1]] >= [[1]]", "True"),
    ("[[1]] < [[2]]", "True"),
    ("[[1, 9]] >= [[2]]", "False"),
    ("[1, True] == [1, 1]", "True"),
    ("[1] == [True] == [1.0]", "True"),

    # dict len
    ("len({})", "0"),
    ("len({'a': 1})", "1"),
    ("len({'a': 1, 'b': 2})", "2"),

    # hash
    ("hash(1) == hash(1)", "True"),
    ("hash(1) == hash(1.0)", "True"),
    ("hash(1) == hash(True)", "True"),

    ("hash(0) == hash(False)", "True"),
    ("hash(2) == hash(2.0)", "True"),

    ("hash(2.5) == hash(2.5)", "True"),
    ("hash(2.5) != hash(3.5)", "True"),

    ("hash('abc') == hash('abc')", "True"),
    ("hash('abc') != hash('xyz')", "True"),

    ("hash((1, 2)) == hash((1, 2))", "True"),
    ("hash((1, 2)) == hash((1, 3))", "False"),

    ("hash((1, True)) == hash((1.0, 1))", "True"),

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

    # функции

    # чтение значения аргумента
    (["def f(x):",
      "    return x * 2",
      "",
      "f(5)"
      ], "10"),

    # изменение аргумента
    (["def f(x):",
      "    x = x + 1",
      "    return x",
      "",
      "f(5)"
      ], "6"),

    # нотирование типов
    (["def sum(a: int, b: int) -> int:",
      "    return a + b",
      "",
      "sum(5, 6)"
      ], "11"),

    # рекурсивный вызов
    (["def factorial(n: int) -> int:",
      "    if n == 0:",
      "        return 1",
      "    else:",
      "        return n * factorial(n - 1)",
      "",
      "factorial(5)"
      ], "120"),

    # корректность области видимости
    (["a = 5",
      "def f():",
      "    a = 6",
      "    return a",
      "",
      "a",
      ], "5"),

    (["a = 5",
      "def f():",
      "    a = 6",
      "    return a",
      "",
      "f()"
      ], "6"),

    # чтение внешней переменной
    (["a = 10",
      "def f():",
      "    return a",
      "",
      "f()"
      ], "10"),

    # несколько вызовов
    (["def f():",
      "    x = 0",
      "    x = x + 1",
      "    return x",
      "",
      "f()"
      ], "1"),

    (["def f():",
      "    x = 0",
      "    x = x + 1",
      "    return x",
      "",
      "f()",
      "f()"
      ], "1"),

    # цикл while внутри функции
    (["def fact(n):",
      "    result = 1",
      "    while n > 1:",
      "        result = result * n",
      "        n = n - 1",
      "    return result",
      "",
      "fact(5)"
      ], "120"),

    # функция внутри функции
    (["def outer():",
      "    x = 5",
      "    def inner():",
      "        return x",
      "    return inner()",
      "",
      "outer()"], "5"),

    # while + return внутри
    (["def f():",
      "    i = 0",
      "    while i < 10:",
      "        return i",
      "",
      "f()"], "0"),

    # break внутри функции
    (["def f():",
      "    i = 0",
      "    while True:",
      "        break",
      "    return 42",
      "",
      "f()"], "42"),

    # огромное число и стек вызовов
    (["def factorial(n: int) -> int:",
      "    if n == 0:",
      "        return 1",
      "    else:",
      "        return n * factorial(n - 1)",
      "",
      "factorial(500)"
      ], "1220136825991110068701238785423046926253574342803192842192413588385845373153881997605496447502203281863013616"
         "4771482035841633787220781772004807852051593292854779075719393306037729608590862704291745478824249127263443056"
         "7017327076946106280231045264421887878946575477714986349436778103764427403382736539747138647787849543848959553"
         "7537990423241061271326984327745715546309977202781014561081188373709531016356324432987029563896628911658974769"
         "5720879269288712817800702651745077684107196243903943225364226052349458501299185715012487069615681416253590566"
         "9342381300885624924689156412677565448188650659384795177536089400574523894033579847636394490531306232374906644"
         "5048824665075946735862074637925184200459369692981022263971952597190945217823331756934581508552332820762820023"
         "4026269078983424517120062077146409794561161276291459512372299133401695523638509428855920187274337951730145863"
         "5757082835578015873543276888868012039988238470215146760544540766353598417443048012893831389688163948746965881"
         "7504506926365338175055478128640000000000000000000000000000000000000000000000000000000000000000000000000000000"
         "000000000000000000000000000000000000000000000"),

    # наличие атрибута класса
    (["class C:",
      "    x = 10",
      "",
      "c = C()",
      "c.x"], "10"),

    # изменение атрибута
    (["class A:",
      "    x = 10",
      "",
      "a = A()",
      "a.x = 99",
      "a.x"], "99"),

    # нестатичность атрибута
    (["class A:",
      "    x = 10",
      "",
      "a = A()",
      "b = A()",
      "b.x = 99",
      "a.x"], "10"),

    # method binding
    (["class A:",
      "    def f(self):",
      "        return 42",
      "",
      "a = A()",
      "m = a.f",
      "m()"], "42"),

    (["class A:",
      "    def f(self):",
      "        return 42",
      "",
      "a = A()",
      "a.f()"], "42"),

    (["class A:",
      "    def f(self):",
      "        return 42",
      "",
      "a = A()",
      "A.f(a)"], "42"),

    (["class A:",
      "    def f(self):",
      "        return 42",
      "",
      "A().f()"], "42"),

    (["class A:",
      "    def f(self):",
      "        return 42",
      "",
      "A.f(777)"], "42"),

    # базовый __init__
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "",
      "a = A(10)",
      "a.x"], "10"),

    # несколько аргументов __init__
    (["class A:",
      "    def __init__(self, x, y):",
      "        self.x = x",
      "        self.y = y",
      "",
      "a = A(3, 7)",
      "a.x"], "3"),

    (["class A:",
      "    def __init__(self, x, y):",
      "        self.x = x",
      "        self.y = y",
      "",
      "a = A(3, 7)",
      "a.y"], "7"),

    # метод использует поля
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "    def get(self):",
      "        return self.x",
      "",
      "a = A(42)",
      "a.get()"], "42" ),


    # hasattr с обёрткой в двойные кавычки
    (["class C:",
      "    x = 10",
      "",
      "c = C()",
      "hasattr(c, \"x\")"], "True"),

    # hasattr с обёрткой в одинарные кавычки
    (["class C:",
      "    x = 10",
      "",
      "c = C()",
      "hasattr(c, \'x\')"], "True"),

    # несуществующий атрибут
    (["class C:",
      "    x = 10",
      "",
      "c = C()",
      "hasattr(c, \'y\')"], "False"),

    # getattr
    (["class C:",
      "    x = 10",
      "",
      "c = C()",
      "getattr(c, \'x\')"], "10"),

    # setattr на instance
    (["class D:",
      "    pass",
      "",
      "d = D()",
      "setattr(d, \"y\", 99)",
      "d.y"], "99"),

    #setattr на класс
    (["class A:",
      "    pass",
      "",
      "setattr(A, \"x\", 77)",
      "a = A()",
      "a.x"], "77"),

    # setattr перезаписывает
    (["class A:",
      "    pass",
      "",
      "a = A()",
      "setattr(a, \"x\", 1)",
      "setattr(a, \"x\", 2)",
      "a.x"], "2" ),

    # setattr + hasattr
    (["class D:",
      "    pass",
      "",
      "d = D()",
      "setattr(d, \"y\", 99)",
      "hasattr(d, \"y\")"], "True"),

    # setattr + getattr
    (["class D:",
      "    pass",
      "",
      "d = D()",
      "setattr(d, \"y\", 99)",
      "getattr(d, \"y\")"], "99"),

    # родительский атрибут
    (["class A:",
      "    x = 10",
      "",
      "class B(A):",
      "    pass",
      "",
      "b = B()",
      "b.x"], "10"),

    # изменение родительского атрибута
    (["class A:",
      "    x = 10",
      "",
      "class B(A):",
      "    x = 707",
      "",
      "b = B()",
      "b.x"], "707" ),

    # применение hasattr к родительскому атрибуту
    (["class A:",
      "    x = 10",
      "",
      "class B(A):",
      "    pass",
      "",
      "b = B()",
      "hasattr(b, \"x\")"], "True"),

    # применение getattr к родительскому атрибуту
    (["class A:",
      "    x = 10",
      "",
      "class B(A):",
      "    pass",
      "",
      "b = B()",
      "getattr(b, \"x\")"], "10"),

    # перезапись родительского метода
    (["class A:",
      "    def f(self):",
      "        return 1",
      "",
      "class B(A):",
      "    def f(self):",
      "        return 707",
      "",
      "b = B()",
      "b.f()"],"707"),

    # перезапись родительского метода + super()
    (["class A:",
      "    def f(self):",
      "        return 1",
      "",
      "class B(A):",
      "    def f(self):",
      "        return super().f() + 1",
      "",
      "b = B()",
      "b.f()"],"2"),

    (["class A:",
      "    def f(self):",
      "        return 23",
      "",
      "class B(A):",
      "    def g(self):",
      "        return super().f() * 10",
      "",
      "B().g()"], "230"),

    # chaining атрибутов
    (["class A:",
      "    pass",
      "",
      "class B:",
      "    pass",
      "",
      "a = A()",
      "b = B()",
      "a.b = b",
      "b.x = 42",
      "a.b.x"], "42"),

    # getattr + метод
    (["class A:",
      "    def f(self):",
      "        return 10",
      "",
      "a = A()",
      "m = getattr(a, \"f\")",
      "m()"], "10"),

    # getattr + class method
    (["class A:",
      "    def f(self):",
      "        return 10",
      "",
      "m = getattr(A, \"f\")",
      "m(A())"], "10"),

    # несколько уровней наследования
    (["class A:",
      "    x = 1",
      "",
      "class B(A):",
      "    pass",
      "",
      "class C(B):",
      "    pass",
      "",
      "C().x"], "1"),

    # super() chain
    (["class A:",
      "    def f(self):",
      "        return 1",
      "",
      "class B(A):",
      "    def f(self):",
      "        return super().f() + 1",
      "",
      "class C(B):",
      "    def f(self):",
      "        return super().f() + 1",
      "",
      "C().f()"], "3"),

    # shadowing метода полем
    (["class A:",
      "    def f(self):",
      "        return 1",
      "",
      "a = A()",
      "a.f = 42",
      "a.f"], "42"),

    # метод vs атрибут
    (["class A:",
      "    f = 10",
      "    def f(self):",
      "        return 1",
      "",
      "A().f()"], "1"),

    # __init__ родительского класса
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "",
      "class B(A):",
      "    pass",
      "",
      "b = B(15)",
      "b.x"], "15"),

    # Базовый super() в __init__
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "",
      "class B(A):",
      "    def __init__(self, x):",
      "        super().__init__(x)",
      "",
      "b = B(10)",
      "b.x"], "10"),

    # super() + добавление своего поля
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "",
      "class B(A):",
      "    def __init__(self, x):",
      "        super().__init__(x)",
      "        self.y = x + 1",
      "",
      "b = B(5)",
      "b.y"], "6"),

    # наличие обоих полей
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "",
      "class B(A):",
      "    def __init__(self, x):",
      "        super().__init__(x)",
      "        self.y = x + 1",
      "",
      "b = B(5)",
      "b.x"], "5"),

    # цепочка наследования
    (["class A:",
      "    def __init__(self):",
      "        self.a = 1",
      "",
      "class B(A):",
      "    def __init__(self):",
      "        super().__init__()",
      "        self.b = 2",
      "",
      "class C(B):",
      "    def __init__(self):",
      "        super().__init__()",
      "        self.c = 3",
      "",
      "c = C()",
      "c.a"], "1"),

    (["class A:",
      "    def __init__(self):",
      "        self.a = 1",
      "",
      "class B(A):",
      "    def __init__(self):",
      "        super().__init__()",
      "        self.b = 2",
      "",
      "class C(B):",
      "    def __init__(self):",
      "        super().__init__()",
      "        self.c = 3",
      "",
      "c = C()",
      "c.b"], "2"),

    (["class A:",
      "    def __init__(self):",
      "        self.a = 1",
      "",
      "class B(A):",
      "    def __init__(self):",
      "        super().__init__()",
      "        self.b = 2",
      "",
      "class C(B):",
      "    def __init__(self):",
      "        super().__init__()",
      "        self.c = 3",
      "",
      "c = C()",
      "c.c"], "3"),

    # super() с несколькими аргументами
    (["class A:",
      "    def __init__(self, x, y):",
      "        self.x = x",
      "        self.y = y",
      "",
      "class B(A):",
      "    def __init__(self, x, y):",
      "        super().__init__(x, y)",
      "",
      "b = B(3, 7)",
      "b.x"], "3"),

    (["class A:",
      "    def __init__(self, x, y):",
      "        self.x = x",
      "        self.y = y",
      "",
      "class B(A):",
      "    def __init__(self, x, y):",
      "        super().__init__(x, y)",
      "",
      "b = B(3, 7)",
      "b.y"], "7"),

    # super() + изменение аргументов
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "",
      "class B(A):",
      "    def __init__(self, x):",
      "        super().__init__(x + 1)",
      "",
      "b = B(10)",
      "b.x"], "11"),

    # вызов метода после super init
    (["class A:",
      "    def __init__(self, x):",
      "        self.x = x",
      "    def get(self):",
      "        return self.x",
      "",
      "class B(A):",
      "    def __init__(self, x):",
      "        super().__init__(x)",
      "",
      "b = B(42)",
      "b.get()"], "42"),

    # super() + переопределение
    (["class A:",
      "    def __init__(self):",
      "        self.x = 1",
      "",
      "class B(A):",
      "    def __init__(self):",
      "        super().__init__()",
      "        self.x = 2",
      "",
      "b = B()",
      "b.x"], "2"),

    # super() внутри метода, вызванного из init
    (["class A:",
      "    def __init__(self):",
      "        self.x = 1",
      "",
      "class B(A):",
      "    def __init__(self):",
      "        self.init_base()",
      "    def init_base(self):",
      "        super().__init__()",
      "",
      "b = B()",
      "b.x"], "1"),

    # property
    (["class A:",
      "    def __init__(self):",
      "        self._x = 10",
      "    def get_x(self):",
      "        return self._x",
      "    x = property(get_x)",
      "",
      "a = A()",
      "a.x"], "10"),

    # property set
    (["class A:",
      "    def __init__(self):",
      "        self._x = 0",
      "    def get_x(self):",
      "        return self._x",
      "    def set_x(self, v):",
      "        self._x = v",
      "    x = property(get_x, set_x)",
      "",
      "a = A()",
      "a.x = 42",
      "a.x"], "42"),

    # data descriptor priority
    (["class A:",
      "    def get_x(self):",
      "        return 999",
      "    def set_x(self, v):",
      "        pass",
      "    x = property(get_x, set_x)",
      "",
      "a = A()",
      "a.x = 10",
      "a.x"], "999"),

    (["class A:",
      "    def __init__(self):",
      "        self._x = 100",
      "    def get_x(self):",
      "        return self._x",
      "    def set_x(self, v):",
      "        self._x = v",
      "    x = property(get_x, set_x)",
      "",
      "a = A()",
      "a.x = 50",
      "a._x"], "50"),

    (["class A:",
      "    def __init__(self):",
      "        self._x = 100",
      "    def get_x(self):",
      "        return self._x",
      "    def set_x(self, v):",
      "        self._x = v",
      "    x = property(get_x, set_x)",
      "",
      "a = A()",
      "a.x = 50",
      "a.x"], "50"),

    # несколько объектов
    (["class A:",
      "    def __init__(self, x):",
      "        self._x = x",
      "    def get_x(self):",
      "        return self._x",
      "    def set_x(self, v):",
      "        self._x = v",
      "    x = property(get_x, set_x)",
      "",
      "a = A(1)",
      "b = A(2)",
      "a.x = 999",
      "a.x"], "999"),

    (["class A:",
      "    def __init__(self, x):",
      "        self._x = x",
      "    def get_x(self):",
      "        return self._x",
      "    def set_x(self, v):",
      "        self._x = v",
      "    x = property(get_x, set_x)",
      "",
      "a = A(1)",
      "b = A(2)",
      "a.x = 999",
      "b.x"], "2"),

    # корректный вызов методов от property
    (["class A:",
      "    def __init__(self):",
      "        self.value = 123",
      "    def get_x(self):",
      "        return self.value",
      "    x = property(get_x)",
      "",
      "a = A()",
      "a.x"], "123"),

    # property внутри наследования
    (["class A:",
      "    def __init__(self):",
      "        self._x = 10",
      "    def get_x(self):",
      "        return self._x",
      "    x = property(get_x)",
      "",
      "class B(A):",
      "    pass",
      "",
      "b = B()",
      "b.x"], "10"),

    # переопределение property в подклассе
    (["class A:",
      "    def get_x(self):",
      "        return 1",
      "    x = property(get_x)",
      "",
      "class B(A):",
      "    def get_x(self):",
      "        return 2",
      "    x = property(get_x)",
      "",
      "b = B()",
      "b.x"], "2"),

    # приоритеты дескрипторов
    # data descriptor > instance field
    (["class D:",
      "    def __get__(self, obj, owner):",
      "        return 123",
      "    def __set__(self, obj, value):",
      "        obj.real = value",
      "",
      "class A:",
      "    x = D()",
      "",
      "a = A()",
      "a.x = 999",
      "a.real"], "999"),

    (["class D:",
      "    def __get__(self, obj, owner):",
      "        return 123",
      "    def __set__(self, obj, value):",
      "        obj.real = value",
      "",
      "class A:",
      "    x = D()",
      "",
      "a = A()",
      "a.x = 999",
      "a.x"], "123"),

    # instance field > non-data descriptor
    (["class D:",
      "    def __get__(self, obj, owner):",
      "        return 555",
      "",
      "class A:",
      "    x = D()",
      "",
      "a = A()",
      "a.x = 999",
      "a.x"], "999"),

    # descriptor на классе
    (["class D:",
      "    def __get__(self, obj, owner):",
      "        return 777",
      "",
      "class A:",
      "    x = D()",
      "",
      "A.x"], "777"),

    # базовый декоратор
    (["def deco(f):",
      "    def wrapper():",
      "        return f() + 1",
      "    return wrapper",
      "",
      "@deco",
      "def x():",
      "    return 10",
      "",
      "x()"], "11"),

    # несколько декораторов
    (["def a(f):",
      "    def w():",
      "        return f() + 1",
      "    return w",
      "",
      "def b(f):",
      "    def w():",
      "        return f() * 10",
      "    return w",
      "",
      "@a",
      "@b",
      "def x():",
      "    return 3",
      "",
      "x()"], "31"),

    # порядок применения декораторов
    (["def a(f):",
      "    def w():",
      "        return \"A(\" + f() + \")\"",
      "    return w",
      "",
      "def b(f):",
      "    def w():",
      "        return \"B(\" + f() + \")\"",
      "    return w",
      "",
      "@a",
      "@b",
      "def x():",
      "    return \"X\"",
      "",
      "x()"], "\'A(B(X))\'"),

    # замыкание внутри декоратора
    (["def repeat(n):",
      "    def deco(f):",
      "        def wrapper():",
      "            result = 0",
      "            i = 0",
      "            while i < n:",
      "                result = result + f()",
      "                i = i + 1",
      "            return result",
      "        return wrapper",
      "    return deco",
      "",
      "@repeat(3)",
      "def x():",
      "    return 5",
      "",
      "x()"], "15"),

    # декоратор с аргументами функции
    (["def deco(f):",
      "    def wrapper(x):",
      "        return f(x) * 2",
      "    return wrapper",
      "",
      "@deco",
      "def sqr(x):",
      "    return x * x",
      "",
      "sqr(5)"], "50"),

    # декоратор без обёртки
    (["def deco(f):",
      "    return f",
      "",
      "@deco",
      "def x():",
      "    return 42",
      "",
      "x()"], "42"),

    # замена функции декоратором
    (["def deco(f):",
      "    def other():",
      "        return 999",
      "    return other",
      "",
      "@deco",
      "def x():",
      "    return 1",
      "",
      "x()"], "999"),

    # декоратор класса
    (["def deco(cls):",
      "    cls.value = 777",
      "    return cls",
      "",
      "@deco",
      "class A:",
      "    pass",
      "",
      "a = A()",
      "a.value"], "777"),

    # несколько декораторов класса
    (["def a(cls):",
      "    cls.a = 1",
      "    return cls",
      "",
      "def b(cls):",
      "    cls.b = 2",
      "    return cls",
      "",
      "@a",
      "@b",
      "class X:",
      "    pass",
      "",
      "x = X()",
      "x.a"], "1"),

    (["def a(cls):",
      "    cls.a = 1",
      "    return cls",
      "",
      "def b(cls):",
      "    cls.b = 2",
      "    return cls",
      "",
      "@a",
      "@b",
      "class X:",
      "    pass",
      "",
      "x = X()",
      "x.b"], "2"),

    # декорированный метод внутри класса
    (["def deco(f):",
      "    def wrapper(self):",
      "        return f(self) + 1",
      "    return wrapper",
      "",
      "class A:",
      "    @deco",
      "    def x(self):",
      "        return 10",
      "",
      "a = A()",
      "a.x()"], "11"),

    # декоратор + наследование
    (["def deco(f):",
      "    def wrapper(self):",
      "        return f(self) * 2",
      "    return wrapper",
      "",
      "class A:",
      "    @deco",
      "    def value(self):",
      "        return 5",
      "",
      "class B(A):",
      "    pass",
      "",
      "b = B()",
      "b.value()"], "10"),

    # фабрика декораторов
    (["def add(n):",
      "    def deco(f):",
      "        def wrapper():",
      "            return f() + n",
      "        return wrapper",
      "    return deco",
      "",
      "@add(7)",
      "def x():",
      "    return 5",
      "",
      "x()"], "12"),

    # рекурсивный декоратор
    (["def deco(f):",
      "    def wrapper(n):",
      "        return f(n)",
      "    return wrapper",
      "",
      "@deco",
      "def fact(n):",
      "    if n == 0:",
      "        return 1",
      "    return n * fact(n - 1)",
      "",
      "fact(5)"], "120"),

    # декоратор и глобальные переменные
    (["x = 100",
      "",
      "def deco(f):",
      "    def wrapper():",
      "        return f() + x",
      "    return wrapper",
      "",
      "@deco",
      "def y():",
      "    return 1",
      "",
      "y()"], "101"),

    # цепочка декораторов с замыканием
    (["def make(n):",
      "    def deco(f):",
      "        def wrapper():",
      "            return f() + n",
      "        return wrapper",
      "    return deco",
      "",
      "@make(1)",
      "@make(2)",
      "@make(3)",
      "def x():",
      "    return 0",
      "",
      "x()"], "6"),

    # staticmethod через класс
    (["class A:",
      "    @staticmethod",
      "    def f():",
      "        return 42",
      "",
      "A.f()"], "42"),

    # staticmethod через instance
    (["class A:",
      "    @staticmethod",
      "    def f():",
      "        return 42",
      "",
      "a = A()",
      "a.f()"], "42"),

    # staticmethod НЕ получает self
    (["class A:",
      "    @staticmethod",
      "    def f(x):",
      "        return x",
      "",
      "A.f(123)"], "123"),

    # classmethod через класс
    (["class A:",
      "    x = 10",
      "    @classmethod",
      "    def f(cls):",
      "        return cls.x",
      "",
      "A.f()"], "10"),

    # classmethod через instance
    (["class A:",
      "    x = 10",
      "    @classmethod",
      "    def f(cls):",
      "        return cls.x",
      "",
      "a = A()",
      "a.f()"], "10"),

    # наследование + classmethod
    (["class A:",
      "    x = 1",
      "    @classmethod",
      "    def f(cls):",
      "        return cls.x",
      "",
      "class B(A):",
      "    x = 2",
      "",
      "B.f()"], "2"),

    # super + classmethod
    (["class A:",
      "    @classmethod",
      "    def f(cls):",
      "        return 1",
      "",
      "class B(A):",
      "    @classmethod",
      "    def f(cls):",
      "        return super().f() + 1",
      "",
      "B.f()"], "2"),

    # смешанное наследование
    (["class A:",
      "    @staticmethod",
      "    def f():",
      "        return 1",
      "",
      "class B(A):",
      "    pass",
      "",
      "B.f()"], "1"),

    # индексация в списке через переменную
    (["a = [1, 2, 3,]",
      "a[-2]"], "2"),

    # индексация результата вызова
    (["def make():",
      "    return [1, 2, 3]",
      "",
      "make()[2]"], "3"),

    # индексация атрибута
    (["class A:",
      "    def __init__(self):",
      "        self.data = [100, 200]",
      "",
      "a = A()",
      "a.data[0]"], "100"),

    (["class A:",
      "    def __init__(self):",
      "        self.data = [100, 200]",
      "",
      "a = A()",
      "a.data[1]"], "200"),

    # прямой вызов __getitem__
    (["a = [5, 6, 7]",
      "a.__getitem__(0)"], "5"),

    (["a = [5, 6, 7]",
      "a.__getitem__(1)"], "6"),

    (["a = [5, 6, 7]",
      "a.__getitem__(-1)"], "7"),

    # chained postfix
    (["class A:",
      "    def f(self):",
      "        return [[1, 2], [3, 4]]",
      "",
      "a = A()",
      "a.f()[1][0]"], "3"),

    # индексирование проходит через __getitem__
    (["class A:",
      "    def __getitem__(self, index):",
      "        return index * 100",
      "",
      "a = A()",
      "a[1]"], "100"),

    (["class A:",
      "    def __getitem__(self, index):",
      "        return index * 100",
      "",
      "a = A()",
      "a[5]"], "500"),

    # изменение значения по индексу
    (["a = [1, 2, 3,]",
      "a[0] = 'abc' * 3",
      "a"], "['abcabcabc', 2, 3]"),

    (["a = [1, 2, 3,]",
      "a[1] = 5 ** 2 == 25",
      "a"], "[1, True, 3]"),

    (["a = [1, 2, 3,]",
      "a[-1] = 999",
      "a"], "[1, 2, 999]"),

    (["a = [1, 2, 3,]",
      "a[-1] = [2, 4, 6]",
      "a"], "[1, 2, [2, 4, 6]]"),

    (["a = [1, 2, 3]",
      "b = a",
      "b[0] = 505",
      "a"], "[505, 2, 3]"),

    # append
    (["a = [1, 2]",
      "a.append(3)",
      "a"], "[1, 2, 3]"),

    (["a = []",
      "a.append([1, 2])",
      "a[0][1]"], "2"),

    (["a = [1, 2, 3]",
      "a.append(True)",
      "a"], "[1, 2, 3, True]"),

    # pop
    (["a = [1, 2, 3]",
      "a.pop()"], "3"),

    (["a = [1, 2, 3]",
      "b = a.pop()",
      "a"], "[1, 2]"),

    (["a = [1, 2, 3]",
      "a.pop(0)"], "1"),

    (["a = [1, 2, 3]",
      "b = a.pop(0)",
      "a"], "[2, 3]"),

    (["a = [1, 2, 3]",
      "a.pop(-1)"], "3"),

    (["a = [1, 2, 3]",
      "b = a.pop(-1)",
      "a"], "[1, 2]"),

    (["a = [1, 2, 3]",
      "b = a",
      "c = b.pop()",
      "a"], "[1, 2]" ),

    (["a = [1, 2, 3]",
      "b = a",
      "c = b.pop()",
      "b"], "[1, 2]" ),

    # len
    (["a = []",
      "len(a)"], "0"),

    (["a = [1, 2, 3]",
      "len(a)"], "3"),

    (["a = [1, 2, 3]",
      "a.__len__()"], "3"),

    (["a = [[1], [2], [3]]",
      "len(a)"], "3"),

    (["a = [1, 2]",
      "a.append(3)",
      "len(a)"], "3"),

    (["a = [1, 2, 3]",
      "a.pop()",
      "len(a)"], "2"),

    # len от alias
    (["a = [1, 2, 3, 4, 5]",
      "b = a",
      "c = b.pop()",
      "len(b)"], "4"),

    # extend
    (["a = [1]",
      "b = a",
      "a.extend([2, 3])",
      "b"], "[1, 2, 3]"),

    (["a = [1, 2]",
      "a.extend([3, 4])",
      "a"], "[1, 2, 3, 4]"),

    (["a = []",
      "a.extend([1])",
      "a"], "[1]"),

    (["a = [1]",
      "b = a",
      "a.extend([2, 3])",
      "b"], "[1, 2, 3]"),

    (["a = [[1]]",
      "a.extend([[2], [3]])",
      "len(a)"], "3"),

    (["a = [[1]]",
      "a.extend([[2], [3], ['abc', True], [False]])",
      "a"], "[[1], [2], [3], ['abc', True], [False]]"),

    # insert
    (["a = [1, 2, 3]",
      "a.insert(1, 999)",
      "a"], "[1, 999, 2, 3]"),

    (["a = [1, 2, 3]",
      "a.insert(1, True)",
      "a"], "[1, True, 2, 3]"),

    # в начало
    (["a = [2, 3]",
      "a.insert(0, 1)",
      "a"], "[1, 2, 3]"),

    # в конец
    (["a = [1, 2]",
      "a.insert(2, 3)",
      "a"], "[1, 2, 3]"),

    (["a = [1, 2]",
      "a.insert(999, 3)",
      "a"], "[1, 2, 3]"),

    # отрицательный индекс
    (["a = [1, 2, 3]",
      "a.insert(-1, 999)",
      "a"], "[1, 2, 999, 3]"),

    # большой отрицательный индекс
    (["a = [2, 3]",
      "a.insert(-999, 1)",
      "a"],
     "[1, 2, 3]"),

    # общая мутабельность
    (["a = [1, 2]",
      "b = a",
      "b.insert(1, 999)",
      "a"], "[1, 999, 2]"),

    # remove
    (["a = [1, 2, 3]",
      "a.remove(2)",
      "a"], "[1, 3]"),

    (["a = [1, 2, 2, 3]",
      "a.remove(2)",
      "a"], "[1, 2, 3]"),

    (["a = ['a', 'b', 'c']",
      "a.remove('b')",
      "a"], "['a', 'c']"),

    (["a = [True, False]",
      "a.remove(True)",
      "a"], "[False]"),

    (["a = [True, False, True]",
      "a.remove(True)",
      "a"], "[False, True]"),

    (["a = [1, 2, 3]",
      "b = a",
      "b.remove(2)",
      "a"], "[1, 3]"),

    (["a = [1, 1.0, True]",
      "a.remove(True)",
      "a"], "[1.0, True]"),

    # clear
    (["a = [1, 2, 3]",
      "a.clear()",
      "a"], "[]"),

    (["a = []",
      "a.clear()",
      "a"], "[]"),

    (["a = [1, 2, 3]",
      "b = a",
      "b.clear()",
      "a"], "[]"),

    (["a = [1]",
      "a.append(2)",
      "a.clear()",
      "a"], "[]"),

    (["a = [1, 2, 3]",
      "a.clear()",
      "len(a)"], "0"),

    # count
    (["a = [1, 2, 1]",
      "a.count(1)"], "2"),

    (["a = [1, 1.0, True]",
      "a.count(1)"], "3"),

    (["a = [False, 0, 0.0]",
      "a.count(False)"], "3"),

    (["a = ['x', 'y', 'x']",
      "a.count('x')"], "2"),

    # not found
    (["a = [1, 2, 3]",
      "a.count(404)"], "0"),

    # пустой список
    (["a = []",
      "a.count(1)"], "0"),

    # index
    (["a = [1, 2, 1, 1]",
      "a.index(1, 1)"], "2"),

    (["a = [1, 2, 1, 1]",
      "a.index(1, 0, 2)"], "0"),

    (["a = [1, 2, 1, 1]",
      "a.index(1, -2)"], "2"),

    (["a = [1, 2, 1, 1]",
      "a.index(1, 0, -1)"], "0"),

    (["a = [1, 1.0, True]",
      "a.index(True, 1)"], "1"),

    # reverse
    (["a = [1, 2, 3]",
      "a.reverse()",
      "a"], "[3, 2, 1]"),

    (["a = [1, 2, 3]",
      "b = a",
      "b.reverse()",
      "a"], "[3, 2, 1]"),

    (["a = []",
      "a.reverse()",
      "a"], "[]"),

    (["a = [42]",
      "a.reverse()",
      "a"], "[42]"),

    # copy
    (["a = [1, 2]",
      "b = a.copy()",
      "b.append(3)",
      "a"], "[1, 2]"),

    (["a = [1, 2]",
      "b = a.copy()",
      "b.append(3)",
      "b"], "[1, 2, 3]"),

    (["a = [[1]]",
      "b = a.copy()",
      "b[0].append(999)",
      "b"], "[[1, 999]]"),

    (["a = []",
      "b = a.copy()",
      "b"], "[]"),

    # независимость от внешнего контейнера
    (["a = [1, 2]",
      "b = a.copy()",
      "a.append(999)",
      "b"], "[1, 2]"),

    # копирование поверхностно
    (["a = [1, [2, 3]]",
      "b = a.copy()",
      "b[1][0] = 4",
      "a"], "[1, [4, 3]]"),

    # сортировка
    (["a = [2, 1, 3]",
     "a.sort()",
     "a"], "[1, 2, 3]"),

    (["a = ['xyz', 'abc', 'aaa']",
      "a.sort()",
      "a"], "['aaa', 'abc', 'xyz']"),

    (["a = [True, 1, 0.5]",
      "a.sort()",
      "a"], "[0.5, True, 1]"),

    # сортировка в обратном порядке
    (["a = [True, 2, 3.0]",
      "a.sort(reverse = True)",
      "a"], "[3.0, 2, True]"),

    # сортировка через лямбду
    (["a = ['abc', 'abab', 'a', '']",
      "a.sort(key = lambda x: len(x))",
      "a"], "['', 'a', 'abc', 'abab']"),

    # сортировка через лямбду + в обратном порядке
    (["a = ['abc', 'abab', 'a', '']",
      "a.sort(key = lambda x: len(x), reverse = True)",
      "a"], "['abab', 'abc', 'a', '']"),

    # словарь
    (["a = {}",
      "a"], "{}"),

    (["a = {}",
      "a['x'] = 101",
      "a['x']"], "101"),

    (["a = {}",
      "a['x'] = 101",
      "a"], "{'x': 101}"),

    # хранение в порядке добавления
    (["a = {}",
      "a['x'] = 101",
      "a['y'] = 202",
      "a['z'] = 303",
      "a"], "{'x': 101, 'y': 202, 'z': 303}"),

    (["a = {}",
      "a['x'] = 101",
      "a['y'] = 202",
      "a['z'] = 303",
      "a['y'] = 999",
      "a"], "{'x': 101, 'y': 999, 'z': 303}"),

    (["a = {}",
      "a['x'] = 101",
      "a['y'] = 202",
      "a['z'] = 303",
      "a['y'] = 999",
      "a['c'] = 1",
      "a"], "{'x': 101, 'y': 999, 'z': 303, 'c': 1}"),

    (["a = {'x': 10}",
      "a['x']"], "10"),

    # метод get
    (["a = {'x': 10}",
      "a.get('x')"], "10"),

    (["a = {'x': 10}",
      "a.get('y')"], ""),

    (["a = {'x': 10}",
      "a.get('y', 404)"], "404"),

    (["a = {'x': 10}",
      "a.get('x', 404)"], "10"),

    (["a = {'x': 10, 'y': 20}",
      "a['x'] + a['y']"], "30"),

    (["a = {}",
      "b = 100",
      "a.get('x', b + 5)"], "105"),

    (["a = {'x': 1}",
      "k = 'x'",
      "a[k]"], "1"),

    (["a = {'x': 1}",
      "k = 'x'",
      "a.get(k)"], "1"),

    # alias
    (["a = {'x': 123}",
      "b = a",
      "b['x'] = 456",
      "a"], "{'x': 456}"),

    # len
    (["a = {}",
      "len(a)"], "0"),

    (["a = {'x': 1, 'y': True}",
     "len(a)"], "2"),

    # clear
    (["a = {'x': 222, 'y': 238}",
      "a.clear()",
      "a"], "{}"),

    (["a = {}",
      "a.clear()",
      "a"], "{}" ),

    # копирование спика
    (["a = {}",
      "a.copy()"], "{}"),

    (["a = {'x': 1}",
      "b = a.copy()",
      "b"], "{'x': 1}"),

    (["a = {'x': 1}",
      "b = a.copy()",
      "b['x'] = 2",
      "a"], "{'x': 1}"),

    (["a = {'x': 1}",
      "b = a.copy()",
      "b['x'] = 2",
      "b"], "{'x': 2}"),

    (["a = {'x': 1}",
      "b = a.copy()",
      "b['x'] = 2",
      "b['z'] = 4",
      "b['y'] = 3",
      "a"], "{'x': 1}"),

    (["a = {'x': 1}",
      "b = a.copy()",
      "b['x'] = 2",
      "b['z'] = 4",
      "b['y'] = 3",
      "b"], "{'x': 2, 'z': 4, 'y': 3}"),

    # проверка shallow copy
    (["a = {'x': [1]}",
      "b = a.copy()",
      "b['x'].append(2)",
      "a"], "{'x': [1, 2]}"),

    # pop
    (["a = {'x': 1}",
      "a.pop('x')"], "1"),

    (["a = {'x': 1}",
      "a.pop('x')",
      "a"], "{}"),

    # default
    (["a = {}",
      "a.pop('x', 404)"], "404"),

    (["a = {'x': 1}",
      "a.pop('x', 404)"], "1"),

    (["a = {'x': 1, 'y': 2, 'z': 3}",
      "a.pop('y')",
      "a"], "{'x': 1, 'z': 3}"),

    # базовый update
    (["a = {'x': 1}",
      "b = {'y': 2}",
      "a.update(b)",
      "a"], "{'x': 1, 'y': 2}"),

    # перезапись существующего значения
    (["a = {'x': 1}",
      "b = {'x': 999}",
      "a.update(b)",
      "a"], "{'x': 999}"),

    # существующий ключ не меняет порядок
    (["a = {'x': 1, 'y': 2}",
      "b = {'y': 999, 'z': 3}",
      "a.update(b)",
      "a"], "{'x': 1, 'y': 999, 'z': 3}"),

    # мутабельность через ссылку
    (["a = {'x': 1}",
      "b = {'y': 2}",
      "c = a",
      "a.update(b)",
      "c"], "{'x': 1, 'y': 2}"),

    # update возвращает None
    (["a = {'x': 1}",
      "b = {'y': 2}",
      "a.update(b)"], ""),

    # update пустым словарем
    (["a = {'x': 1}",
      "b = {}",
      "a.update(b)",
      "a"], "{'x': 1}"),

    # update пустого словаря
    (["a = {}",
      "b = {'x': 1}",
      "a.update(b)",
      "a"], "{'x': 1}"),

    # setdefault
    # базовая вставка
    (["a = {}",
      "a.setdefault('x', 10)"], "10"),

    # изменение словаря
    (["a = {}",
      "a.setdefault('x', 10)",
      "a"], "{'x': 10}"),

    # существующий ключ
    (["a = {'x': 1}",
      "a.setdefault('x', 999)"], "1"),

    # существующий ключ не перезаписывается
    (["a = {'x': 1}",
      "a.setdefault('x', 999)",
      "a"], "{'x': 1}"),

    # без аргумента по умолчанию
    (["a = {}",
      "a.setdefault('x')"], ""),

    # изменение словаря без аргумента по умолчанию
    (["a = {}",
      "a.setdefault('x')",
      "a"], "{'x': None}"),

    # сохранение порядка при вставке
    (["a = {'a': 1}",
      "a.setdefault('b', 2)",
      "a.setdefault('c', 3)",
      "a"], "{'a': 1, 'b': 2, 'c': 3}"),

    # переменная в качестве ключа
    (["a = {}",
      "k = 'hello'",
      "a.setdefault(k, 42)"], "42"),

    (["a = {}",
      "k = 'hello'",
      "a.setdefault(k, 42)",
      "a"], "{'hello': 42}"),

    # переменная в качестве значения
    (["a = {}",
      "v = 777",
      "a.setdefault('x', v)",
      "a"], "{'x': 777}"),

    # alias-семантика
    (["a = {}",
      "b = a",
      "b.setdefault('x', 123)",
      "a"], "{'x': 123}"),

    # наличие None
    (["a = {'x': None}",
      "a.setdefault('x', 999)",
      "a"], "{'x': None}"),

    # popitem
    (["a = {'x': 1}",
      "a.popitem()"], "('x', 1)"),

    # lifo
    (["a = {'x': 1, 'y': 2}",
      "a.popitem()"], "('y', 2)"),

    # проверка удаления
    (["a = {'x': 1, 'y': 2}",
      "a.popitem()",
      "a"], "{'x': 1}"),

    (["a = {'x': 1}",
      "a.popitem()",
      "a"], "{}"),

    # базовый keys()
    (["a = {'x': 1, 'y': 2}",
      "a.keys()"],
     "dict_keys(['x', 'y'])"),

    # пустой словарь
    (["a = {}",
      "a.keys()"],
     "dict_keys([])"),

    # dynamic view semantics
    (["a = {'x': 1}",
      "k = a.keys()",
      "a['y'] = 2",
      "k"],
     "dict_keys(['x', 'y'])"),

    # после удаления
    (["a = {'x': 1, 'y': 2}",
      "k = a.keys()",
      "c = a.pop('x')",
      "k"],
     "dict_keys(['y'])"),

    # базовый values()
    (["a = {'x': 1, 'y': 2}",
      "a.values()"],
     "dict_values([1, 2])"),

    # пустой dict
    (["a = {}",
      "a.values()"],
     "dict_values([])"),

    # dynamic view semantics
    (["a = {'x': 1}",
      "v = a.values()",
      "a['y'] = 2",
      "v"],
     "dict_values([1, 2])"),

    # после удаления
    (["a = {'x': 1, 'y': 2}",
      "v = a.values()",
      "a.pop('x')",
      "v"],
     "dict_values([2])"),

    # базовый items
    (["a = {'x': 1, 'y': 2}",
      "a.items()"], "dict_items([('x', 1), ('y', 2)])"),

    # пустой словарь
    (["a = {}",
      "a.items()"], "dict_items([])"),

    # dynamic view semantics
    (["a = {'x': 1}",
      "i = a.items()",
      "a['y'] = 2",
      "i"], "dict_items([('x', 1), ('y', 2)])"),

    # после удаления
    (["a = {'x': 1, 'y': 2}",
      "i = a.items()",
      "a.pop('x')",
      "i"], "dict_items([('y', 2)])"),

    # fromkeys
    (["{}.fromkeys(['a', 'b'])"],
     "{'a': None, 'b': None}"),

    # разные типы ключей
    (["d = {1: 'a', 'x': 42, True: 'yes'}",
      "d"], "{1: 'yes', 'x': 42}"),

    # tuple-ключи
    (["d = {(1, 2): 'tuple', (3, 4): 'xy'}",
      "d[(1, 2)]"], "'tuple'"),

    # bool/int hash collision
    (["d = {}",
      "d[1] = 'int'",
      "d[True] = 'bool'",
      "d"], "{1: 'bool'}"),

    # float/int equality
    (["d = {}",
      "d[1] = 'x'",
      "d[1.0] = 'y'",
      "d"], "{1: 'y'}"),

    # nested tuple keys
    (["d = {((1, 2), (3, 4)): 'nested'}",
      "d"], "{((1, 2), (3, 4)): 'nested'}"),

    # iteration over items
    (["d = {True: 1, '1': 2}",
      "res = []",
      "for i in d.items():",
      "    res.append(i)",
      "",
      "res"], "[(True, 1), ('1', 2)]"),

    # popitem
    (["d = {42.0: 1, 305: 2}",
      "d.popitem()"], "(305, 2)"),

    # setdefault()
    (["d = {}",
      "d.setdefault((400, 'qwer', True), 42)",
      "d"], "{(400, 'qwer', True): 42}"),

    # update()
    (["a = {5 == 10 ** 2 / (2 * 10): 1}",
      "b = {('one', 'two', 'three'): 'four'}",
      "a.update(b)",
      "a"], "{True: 1, ('one', 'two', 'three'): 'four'}"),

    (["d = {}.fromkeys([1, 2, 3, 'six', 'seven'], 67)",
      "d"], "{1: 67, 2: 67, 3: 67, 'six': 67, 'seven': 67}"),

    # сохранение порядка ключей при перезаписи
    (["d = {}",
      "d[True] = 1",
      "d[1] = 2",
      "d[(404,)] = 3",
      "d['abc'] = 4.0",
      "d"], "{True: 2, (404,): 3, 'abc': 4.0}"),

    # с default value
    (["{}.fromkeys(['a', 'b'], 0)"],
     "{'a': 0, 'b': 0}"),

    # tuple
    (["{}.fromkeys(('x', 'y'), 1)"],
     "{'x': 1, 'y': 1}"),

    # пустой iterable
    (["{}.fromkeys([])"],
     "{}"),

    # перезапись дубликатов
    (["a = {}.fromkeys(['a', 'a', 'a'], 1)",
      "a"], "{'a': 1}"),

    # пустой tuple
    (["a = ()",
      "a"], "()"),

    # singleton tuple
    (["a = (1,)",
      "a"], "(1,)"),

    # несколько элементов
    (["a = (1, 2, 3)",
      "a"], "(1, 2, 3)"),

    # разные типы
    (["a = (1, 'hello', True, None)",
      "a"], "(1, 'hello', True, None)"),

    # вложенный tuple
    (["a = ((1, 2), (3, 4))",
      "a"], "((1, 2), (3, 4))"),

    # tuple внутри list
    (["a = [(1, 2), (3, 4)]",
      "a"], "[(1, 2), (3, 4)]"),

    # tuple внутри dict
    (["a = {'x': (1, 2)}",
      "a"], "{'x': (1, 2)}"),

    # индексирование
    (["a = (10, 20, 30)",
      "a[0]"], "10"),

    (["a = (10, 20, 30)",
      "a[1]"], "20"),

    (["a = (10, 20, 30)",
      "a[2]"], "30"),

    # отрицательные индексы
    (["a = (10, 20, 30)",
      "a[-1]"], "30"),

    (["a = (10, 20, 30)",
      "a[-2]"], "20"),

    (["a = (10, 20, 30)",
      "a[-3]"], "10"),

    # tuple alias
    (["a = (1, 2, 3)",
      "b = a",
      "b"], "(1, 2, 3)"),

    # tuple в выражении
    (["a = (1, 2)",
      "a[0] + a[1]"], "3"),

    # nested indexing
    (["a = ((1, 2), (3, 4))",
      "a[1][0]"], "3"),

    # singleton tuple indexing
    (["a = (42,)",
      "a[0]"], "42"),

    # count
    (["a = (1, 2, 3, 1)",
      "a.count(1)"], "2"),

    (["a = (1, 2, 3)",
      "a.count(4)"], "0"),

    (["a = ('x', 'y', 'x')",
      "a.count('x')"], "2"),

    (["a = (True, False, True)",
      "a.count(True)"], "2"),

    (["a = ()",
      "a.count(1)"], "0"),

    (["a = (1, 1.0, True)",
      "a.count(1)"], "3"),

    (["a = ((1, 2), (1, 2))",
      "a.count((1, 2))"], "2"),

    # index
    (["a = (10, 20, 30)",
      "a.index(20)"], "1"),

    (["a = (10, 20, 30, 20)",
      "a.index(20, 2)"], "3"),

    (["a = (10, 20, 30, 20)",
      "a.index(20, 2, 4)"], "3"),

    (["a = (10, 20, 30)",
      "a.index(30)"], "2"),

    (["a = ('a', 'b', 'c')",
      "a.index('b')"], "1"),

    (["a = (1, 2, 3)",
      "a.index(1, -3)"], "0"),

    # len
    ("len((1, 2, 3))", "3"),

    ("len(())", "0"),

    (["a = (10, 20)",
      "len(a)"], "2"),

    (["a = ('x', 'y', 'z')",
      "len(a)"], "3"),

    # list iterator
    (["a = [1, 2, 3]",
      "i = iter(a)",
      "next(i)"], "1"),

    (["a = [1, 2, 3]",
      "i = iter(a)",
      "next(i)",
      "next(i)"], "2"),

    (["a = [1, 2, 3]",
      "i = iter(a)",
      "next(i)",
      "next(i)",
      "next(i)"], "3"),

    # независимость итераторов
    (["a = [1, 2]",
      "i1 = iter(a)",
      "i2 = iter(a)",
      "next(i1)",
      "next(i2)"], "1"),

    # iterator тоже iterable
    (["a = [1, 2]",
      "i = iter(a)",
      "j = iter(i)",
      "next(j)"], "1"),

    # tuple iterator
    (["a = (10, 20)",
      "i = iter(a)",
      "next(i)"], "10"),

    (["a = (10, 20)",
      "i = iter(a)",
      "next(i)",
      "next(i)"], "20"),

    # независимость итераторов
    (["a = (1, 2)",
      "i1 = iter(a)",
      "i2 = iter(a)",
      "next(i1)",
      "next(i2)"], "1"),

    # базовый for
    (["res = 0",
      "for x in [1, 2, 3]:",
      "    res = res + x",
      "",
      "res"], "6"),

    # tuple iteration
    (["res = 0",
      "for x in (1, 2, 3):",
      "    res = res + x",
      "",
      "res"], "6"),

    # пустой iterable
    (["res = 100",
      "for x in []:",
      "    res = 0",
      "",
      "res"], "100"),

    # break
    (["res = 0",
      "for x in [1, 2, 3, 4]:",
      "    if x == 3:",
      "        break",
      "    res = res + x",
      "",
      "res"], "3"),

    # continue
    (["res = 0",
      "for x in [1, 2, 3, 4]:",
      "    if x == 2:",
      "        continue",
      "    res = res + x",
      "",
      "res"], "8"),

    # nested for
    (["res = 0",
      "for x in [1, 2]:",
      "    for y in [10, 20]:",
      "        res = res + y",
      "",
      "res"], "60"),

    # iter(list)
    (["it = iter([1, 2, 3])",
      "res = 0",
      "for x in it:",
      "    res = res + x",
      "",
      "res"], "6"),

    # iter(tuple)
    (["it = iter((1, 2, 3))",
      "res = 0",
      "for x in it:",
      "    res = res + x",
      "",
      "res"], "6"),

    # keys() iteration
    (["a = {'x': 1, 'y': 2}",
      "res = []",
      "for k in a.keys():",
      "    res.append(k)",
      "",
      "res"], "['x', 'y']"),

    # values() iteration
    (["a = {'x': 1, 'y': 2}",
      "res = []",
      "for v in a.values():",
      "    res.append(v)",
      "",
      "res"], "[1, 2]"),


    # iter(keys())
    (["a = {'x': 1}",
      "i = iter(a.keys())",
      "next(i)"], "'x'"),

    # iter(values())
    (["a = {'x': 42}",
      "i = iter(a.values())",
      "next(i)"], "42"),

    # iter(items())
    (["a = {'x': 1}",
      "i = iter(a.items())",
      "next(i)"], "('x', 1)"),

    # empty dict keys
    (["a = {}",
      "for k in a.keys():",
      "    k",
      ""], ""),

    # empty dict values
    (["a = {}",
      "for v in a.values():",
      "    v",
      ""], ""),

    # empty dict items
    (["a = {}",
      "for item in a.items():",
      "    item",
      ""], ""),

    # iter(iterator)
    (["a = {'x': 1}",
      "i = iter(a.keys())",
      "j = iter(i)",
      "next(j)"], "'x'"),

    # порядок сохранения не ломается
    (["a = {}",
      "a['z'] = 1",
      "a['a'] = 2",
      "a['m'] = 3",
      "res = []",
      "for k in a.keys():",
      "    res.append(k)",
      "",
      "res"], "['z', 'a', 'm']"),

    # items order
    (["a = {}",
      "a['z'] = 1",
      "a['a'] = 2",
      "res = []",
      "for item in a.items():",
      "    res.append(item)",
      "",
      "res"], "[('z', 1), ('a', 2)]"),

    # mutation visibility
    (["a = {'x': 1}",
      "v = a.values()",
      "a['y'] = 2",
      "res = []",
      "for x in v:",
      "    res.append(x)",
      "",
      "res"], "[1, 2]"),

    # set
    (["a = {1, 2, 3}",
      "a"], "{1, 2, 3}"),

    (["a = {1}",
      "a"], "{1}"),

    (["a = {1, 2,}",
      "a"], "{1, 2}"),

    (["a = {1 + 2, 3 * 4}",
      "a"], "{3, 12}"),

    # add
    (["a = {1, 2}",
      "a.add(3)",
      "a"], "{1, 2, 3}"),

    # add duplicate
    (["a = {1, 2}",
      "a.add(2)",
      "a"], "{1, 2}"),

    # remove
    (["a = {1, 2, 3}",
      "a.remove(2)",
      "a"], "{1, 3}"),

    # discard existing
    (["a = {1, 2}",
      "a.discard(2)",
      "a"], "{1}"),

    # discard missing
    (["a = {1}",
      "a.discard(999)",
      "a"], "{1}"),

    # базовый union
    (["a = {1, 2}",
      "b = {2, 3}",
      "a.union(b)"], "{1, 2, 3}"),

    # исходное множество не меняется
    (["a = {1, 2}",
      "b = {3}",
      "c = a.union(b)",
      "a"], "{1, 2}"),

    # # union empty (пока не поддерживается)
    # (["a = {1, 2}",
    #   "a.union(set())"], "{1, 2}"),

    # устранение дублкатов
    (["a = {1, 2}",
      "b = {2, 2, 3}",
      "a.union(b)"], "{1, 2, 3}"),

    # базовое пересечение множеств
    (["a = {1, 2, 3}",
      "b = {2, 3, 4}",
      "a.intersection(b)"], "{2, 3}"),

    # # пересечение без общих элементов (пока не поддерживается)
    # (["a = {1, 2}",
    #   "b = {3, 4}",
    #   "a.intersection(b)"], "set()"),

    # пересечение множества с самим собой
    (["a = {1, 2, 3}",
      "a.intersection(a)"], "{1, 2, 3}"),

    # # пересечение с пустым множеством (пока не поддерживается)
    # (["a = {1, 2, 3}", "a.intersection(set())"], "set()"),

    # базовая разность множеств
    (["a = {1, 2, 3}",
      "b = {2}",
      "a.difference(b)"], "{1, 3}"),

    # разность без пересечения
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.difference(b)"], "{1, 2}"),

    # # разность множества с самим собой (пока не поддерживается)
    # (["a = {1, 2, 3}", "a.difference(a)"], "set()"),

    # # разность с пустым множеством (пока не поддерживается)
    # (["a = {1, 2, 3}",
    #   "a.difference(set())"], "{1, 2, 3}"),

    # # пустое множество минус непустое (пока не поддерживается)
    # (["a = set()",
    #   "b = {1, 2}",
    #   "a.difference(b)"], "set()")

    # базовая симметрическая разность
    (["a = {1, 2, 3}",
      "b = {3, 4}",
      "a.symmetric_difference(b)"], "{1, 2, 4}"),

    # # полностью одинаковые множества (пока не поддерживается)
    # (["a = {1, 2}",
    #   "b = {1, 2}",
    #   "a.symmetric_difference(b)"], "set()")

    # полностью разные множества
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.symmetric_difference(b)"], "{1, 2, 3, 4}"),

    # # симметрическая разность с пустым множеством (пока не поддерживается)
    # (["a = {1, 2, 3}",
    #   "a.symmetric_difference(set())"], "{1, 2, 3}"),

    # # пустое множество с непустым (пока не поддерживается)
    # (["a = set()",
    #   "b = {1, 2}",
    #   "a.symmetric_difference(b)"], "{1, 2}")

    # issubset
    (["a = {1, 2}",
      "b = {1, 2, 3}",
      "a.issubset(b)"], "True"),

    (["a = {1, 4}",
      "b = {1, 2, 3}",
      "a.issubset(b)"], "False"),

    # множество является подмножеством самого себя
    (["a = {1, 2, 3}",
      "a.issubset(a)"], "True"),

    # # пустое множество — подмножество любого (пока не поддерживается)
    # (["a = set()",
    #   "b = {1, 2}",
    #   "a.issubset(b)"], "True"),

    # # непустое множество не является подмножеством пустого (пока не поддерживается)
    # (["a = {1}",
    #   "b = set()",
    #   "a.issubset(b)"], "False"),

    # # пустое множество — подмножество самого себя (пока не поддерживается)
    # (["a = set()",
    #   "b = set()",
    #   "a.issubset(b)"], "True")

    # базовая проверка надмножества
    (["a = {1, 2, 3}",
      "b = {1, 2}",
      "a.issuperset(b)"], "True"),

    (["a = {1, 2}",
      "b = {1, 2, 3}",
      "a.issuperset(b)"], "False"),

    # множество является надмножеством самого себя
    (["a = {1, 2, 3}",
      "a.issuperset(a)"], "True"),

    # # любое множество — надмножество пустого (пока не поддерживается)
    # (["a = {1, 2}",
    #   "b = set()",
    #   "a.issuperset(b)"], "True"),

    # # пустое множество не является надмножеством непустого (пока не поддерживается)
    # (["a = set()",
    #   "b = {1}",
    #   "a.issuperset(b)"], "False"),

    # # пустое множество — надмножество самого себя (пока не поддерживается)
    # (["a = set()",
    #   "b = set()",
    #   "a.issuperset(b)"], "True")


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