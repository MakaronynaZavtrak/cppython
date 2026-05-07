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
    ("10 ** 50 < 10 ** 100", "True")
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