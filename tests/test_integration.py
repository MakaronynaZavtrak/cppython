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

    # str upper
    ("'hello'.upper()", "'HELLO'"),
    ("'HeLLo'.upper()", "'HELLO'"),
    ("''.upper()", "''"),
    ("'123abc'.upper()", "'123ABC'"),

    # lower
    ("'HELLO'.lower()", "'hello'"),
    ("'HeLLo'.lower()", "'hello'"),
    ("''.lower()", "''"),
    ("'123ABC'.lower()", "'123abc'"),

    # strip whitespace
    ("'  hello  '.strip()", "'hello'"),

    # strip chars
    ("'---hello---'.strip('-')", "'hello'"),

    # strip multiple chars
    ("'abcHelloabc'.strip('abc')", "'Hello'"),

    # no changes
    ("'hello'.strip()", "'hello'"),

    # empty string
    ("''.strip()", "''"),

    # strip all chars
    ("'aaaa'.strip('a')", "''"),

    # strip only edges
    ("'abchelloabcworldabc'.strip('abc')", "'helloabcworld'"),

    # split
    ("'a b c'.split()", "['a', 'b', 'c']"),
    ("'apple#banana#cherry#orange'.split('#')", "['apple', 'banana', 'cherry', 'orange']"),
    ("'apple#banana#cherry#orange'.split('#', 1)", "['apple', 'banana#cherry#orange']"),

    # multiple spaces
    ("'  a   b  c '.split()", "['a', 'b', 'c']"),

    # separator split
    ("'a,b,c'.split(',')", "['a', 'b', 'c']"),

    # maxsplit
    ("'a,b,c'.split(',', 1)", "['a', 'b,c']"),

    # maxsplit 2
    ("'a,b,c,d'.split(',', 2)", "['a', 'b', 'c,d']"),

    # no separator found
    ("'abc'.split(',')", "['abc']"),

    # empty string with sep
    ("''.split(',')", "['']"),

    # list join
    ("','.join(['a', 'b', 'c'])", "'a,b,c'"),

    # tuple join
    ("' '.join(('hello', 'world'))", "'hello world'"),

    # single element
    ("','.join(['abc'])", "'abc'"),

    # empty iterable
    ("','.join([])" , "''"),

    # join chars
    ("'-'.join('abc')", "'a-b-c'"),

    # join + split
    ("\",\".join('a b c'.split())", "'a,b,c'"),

    # basic replace
    ("'hello'.replace('l', 'X')", "'heXXo'"),

    # replace all
    ("'aaaa'.replace('a', 'b')", "'bbbb'"),

    # replace count
    ("'aaaa'.replace('a', 'b', 2)", "'bbaa'"),

    # no matches
    ("'hello'.replace('z', 'X')", "'hello'"),

    # negative count -> replace all
    ("'aaaa'.replace('a', 'b', -1)", "'bbbb'"),

    # empty replacement
    ("'hello'.replace('l', '')", "'heo'"),

    # start only
    ("'hello'.startswith('ll', 2)", "True"),

    # неправильный аргумент start
    ("'hello'.startswith('he', 1)", "False"),

    # с параметром end
    ("'hello'.startswith('ll', 2, 4)", "True"),

    # обрезанный конец
    ("'hello'.startswith('lo', 0, 4)", "False"),

    # пустой срез
    ("'hello'.startswith('h', 3, 1)", "False"),

    # базовый endswith
    ("'hello'.endswith('lo')", "True"),

    ("'hello'.endswith('he')", "False"),

    # with start
    ("'hello'.endswith('lo', 2)", "True"),

    # с параметром end
    ("'hello'.endswith('ll', 0, 4)", "True"),

    # обрезанный конец
    ("'hello'.endswith('lo', 0, 4)", "False"),

    # пустой параметр suffix
    ("'hello'.endswith('')", "True"),

    # пустой срез
    ("'hello'.endswith('o', 3, 1)", "False"),

    # find
    ("'hello'.find('ll')", "2"),

    # not found
    ("'hello'.find('z')", "-1"),

    # с параметром start
    ("'hello'.find('l', 3)", "3"),

    # с параметром end
    ("'hello'.find('l', 0, 3)", "2"),

    # обрезанный диапазон
    ("'hello'.find('o', 0, 4)", "-1"),

    # пустая подстрока
    ("'hello'.find('')", "0"),

    # count
    ("'hello'.count('l')", "2"),

    # отсутствующий символ
    ("'hello'.count('z')", "0"),

    # повторяющийся шаблон
    ("'aaaa'.count('aa')", "2"),

    # с параметром start
    ("'hello'.count('l', 3)", "1"),

    # с параметром end
    ("'hello'.count('l', 0, 3)", "1"),

    # count от пустой строки
    ("'hello'.count('')", "6"),

    ("''.count('')", "1"),

    # index
    ("'hello'.index('e')", "1"),

    # повтор
    ("'banana'.index('na')", "2"),

    # с параметром start
    ("'banana'.index('na', 3)", "4"),

    # с параметром end
    ("'banana'.index('na', 0, 4)", "2"),

    # rfind
    ("'banana'.rfind('na')", "4"),

    # первый символ
    ("'banana'.rfind('b')", "0"),

    # отсутсвующий символ
    ("'banana'.rfind('z')", "-1"),

    # с параметром start
    ("'banana'.rfind('na', 3)", "4"),

    # с параметром end
    ("'banana'.rfind('na', 0, 4)", "2"),

    # rindex
    ("'banana'.rindex('na')", "4"),

    # первый символ
    ("'banana'.rindex('b')", "0"),

    # с параметром start
    ("'banana'.rindex('na', 3)", "4"),

    # с параметром end
    ("'banana'.rindex('na', 0, 4)", "2"),

    # capitalize
    ("'hello'.capitalize()", "'Hello'"),
    ("'hELLO'.capitalize()", "'Hello'"),
    ("''.capitalize()", "''"),

    # title
    ("'hello world'.title()", "'Hello World'"),
    ("'heLLo woRLD'.title()", "'Hello World'"),
    ("'hello-world'.title()", "'Hello-World'"),
    ("'hello_world'.title()", "'Hello_World'"),
    ("''.title()", "''"),

    # swapcase
    ("'Hello WORLD'.swapcase()", "'hELLO world'"),
    ("'abc'.swapcase()", "'ABC'"),
    ("'ABC'.swapcase()", "'abc'"),
    ("'PyThOn123'.swapcase()", "'pYtHoN123'"),
    ("'!@#'.swapcase()", "'!@#'"),

    # isaplha
    ("'hello'.isalpha()", "True"),
    ("'Hello'.isalpha()", "True"),
    ("'hello123'.isalpha()", "False"),
    ("'123'.isalpha()", "False"),
    ("'hello world'.isalpha()", "False"),
    ("''.isalpha()", "False"),
    ("'Привет'.isalpha()", "True"),

    # isdigit

    # только цифры
    ("'12345'.isdigit()", "True"),

    # цифры и буквы
    ("'123abc'.isdigit()", "False"),

    # буквы
    ("'hello'.isdigit()", "False"),

    # пробел
    ("'123 456'.isdigit()", "False"),

    # пустая строка
    ("''.isdigit()", "False"),

    # isalnum
    # буквы и цифры
    ("'abc123'.isalnum()", "True"),

    # только буквы
    ("'hello'.isalnum()", "True"),

    # только цифры
    ("'12345'.isalnum()", "True"),

    # пробел
    ("'abc 123'.isalnum()", "False"),

    # символы
    ("'abc!'.isalnum()", "False"),

    # пустая строка
    ("''.isalnum()", "False"),

    # кириллица
    ("'Привет123'.isalnum()", "True"),

    # isspace

    # только пробелы
    ("'   '.isspace()", "True"),

    # пробелы и табы
    ("'\\t\\n'.isspace()", "True"),

    # isspace с tab
    ("\"\\t\".isspace()", "True"),

    # isspace с newline
    ("\"\\n\".isspace()", "True"),

    # escape \n внутри строки
    ("\"a\\nb\"", "'a\\nb'"),

    # escape \t внутри строки
    ("\"a\\tb\"", "'a\\tb'"),

    # текст и пробелы
    ("' hello '.isspace()", "False"),

    # пустая строка
    ("''.isspace()", "False"),

    # обычный текст
    ("'hello'.isspace()", "False"),

    # repr

    # repr("") -> "''"
    ("repr(\"\")", "\"''\""),

    # repr("abc") -> "'abc'"
    ("repr(\"abc\")", "\"'abc'\""),

    # repr("'") -> '"\'"'
    ("repr(\"'\")", "'\"\\\'\"'"),

    # repr('"') -> '\'"\''
    ("repr('\"')", "'\\\'\"\\\''"),

    # repr("'\"") -> '\'\\\'"\''
    ("repr(\"'\\\"\")", "'\\\'\\\\\\\'\"\\\''"),

    # repr("a'b") -> '"a\'b"'
    ("repr(\"a'b\")", "'\"a\\\'b\"'"),

    # repr('a"b') -> '\'a"b\''
    ("repr('a\"b')", "'\\\'a\"b\\\''"),

    # repr("a'b\"c") -> '\'a\\\'b"c\''
    ("repr(\"a'b\\\"c\")", "'\\\'a\\\\\\\'b\"c\\\''"),

    # repr([1, "abc"]) -> "[1, 'abc']"
    ("repr([1, \"abc\"])", "\"[1, 'abc']\""),

    # repr(repr("abc")) -> '"\'abc\'"'
    ("repr(repr(\"abc\"))", "'\"\\\'abc\\\'\"'"),

    # print
    ("print()", ""),
    ("print(1)", "1"),
    ("print(1, 2, 3)", "1 2 3"),
    ("print(1, 2, 3, sep=\", \")", "1, 2, 3"),
    ("print([1, \"abc\"])", "[1, 'abc']"),
    ("print(repr(\"abc\"))", "'abc'"),

    # center
    ("'abc'.center(7)", "'  abc  '"),
    ("'abc'.center(8)", "'  abc   '"),
    ("'abc'.center(3)", "'abc'"),
    ("'abc'.center(2)", "'abc'"),
    ("'abc'.center(7, '-')", "'--abc--'"),
    ("'ab'.center(5, '*')", "'**ab*'"),
    ("'x'.center(5, '*')", "'**x**'"),
    ("'x'.center(6, '*')", "'**x***'"),
    ("''.center(4)", "'    '"),
    ("''.center(4, '-')", "'----'"),

    # ljust
    ("'abc'.ljust(8)", "'abc     '"),
    ("'abc'.ljust(8, '*')", "'abc*****'"),
    ("'abc'.ljust(3)", "'abc'"),
    ("'abc'.ljust(2)", "'abc'"),
    ("'abc'.ljust(4)", "'abc '"),
    ("'x'.ljust(5, '*')", "'x****'"),
    ("''.ljust(5)", "'     '"),
    ("''.ljust(3, '*')", "'***'"),

    # rjust
    ("'x'.rjust(5)", "'    x'"),
    ("'x'.rjust(5, '*')", "'****x'"),
    ("'abc'.rjust(5)", "'  abc'"),
    ("'abc'.rjust(3)", "'abc'"),
    ("'abc'.rjust(2)", "'abc'"),
    ("''.rjust(3)", "'   '"),
    ("'hello'.rjust(10, '*')", "'*****hello'"),

    # lstrip пробелом
    ("'   hello   '.lstrip()", "'hello   '"),
    ("'hello'.lstrip()", "'hello'"),
    ("''.lstrip()", "''"),

    # lstrip другими символами
    ("'---hello---'.lstrip('-')", "'hello---'"),
    ("'abcHelloabc'.lstrip('abc')", "'Helloabc'"),
    ("'aaaa'.lstrip('a')", "''"),
    ("'abcabcHello'.lstrip('abc')", "'Hello'"),
    ("'xyzHello'.lstrip('abc')", "'xyzHello'"),

    # rstrip пробелом
    ("'   hello   '.rstrip()", "'   hello'"),
    ("'hello'.rstrip()", "'hello'"),
    ("''.rstrip()", "''"),

    # rstrip другими символами
    ("'---hello---'.rstrip('-')", "'---hello'"),
    ("'abcHelloabc'.rstrip('abc')", "'abcHello'"),
    ("'aaaa'.rstrip('a')", "''"),
    ("'Helloabcabc'.rstrip('abc')", "'Hello'"),
    ("'Helloxyz'.rstrip('abc')", "'Helloxyz'"),

    # islower
    ("'hello'.islower()", "True"),
    ("'hello123'.islower()", "True"),
    ("'hello!'.islower()", "True"),
    ("'Hello'.islower()", "False"),
    ("'HELLO'.islower()", "False"),
    ("'HeLLo'.islower()", "False"),
    ("''.islower()", "False"),
    ("'123'.islower()", "False"),
    ("'!!!'.islower()", "False"),
    ("'привет'.islower()", "True"),
    ("'Привет'.islower()", "False"),
    ("'привет123'.islower()", "True"),

    # isupper
    ("'HELLO'.isupper()", "True"),
    ("'HELLO123'.isupper()", "True"),
    ("'HELLO!'.isupper()", "True"),
    ("'Hello'.isupper()", "False"),
    ("'hello'.isupper()", "False"),
    ("'HeLLo'.isupper()", "False"),
    ("''.isupper()", "False"),
    ("'123'.isupper()", "False"),
    ("'!!!'.isupper()", "False"),
    ("'ПРИВЕТ'.isupper()", "True"),
    ("'Привет'.isupper()", "False"),
    ("'ПРИВЕТ123'.isupper()", "True"),

    # isdecimal
    ("'123'.isdecimal()", "True"),
    ("'000'.isdecimal()", "True"),
    ("'123abc'.isdecimal()", "False"),
    ("'abc'.isdecimal()", "False"),
    ("'12 3'.isdecimal()", "False"),
    ("''.isdecimal()", "False"),
    ("'123.45'.isdecimal()", "False"),
    ("'-123'.isdecimal()", "False"),

    # isnumeric
    ("'123'.isnumeric()", "True"),
    ("'000'.isnumeric()", "True"),
    ("'123abc'.isnumeric()", "False"),
    ("'abc'.isnumeric()", "False"),
    ("'12 3'.isnumeric()", "False"),
    ("''.isnumeric()", "False"),
    ("'123.45'.isnumeric()", "False"),
    ("'-123'.isnumeric()", "False"),

    # istitle
    ("'Hello'.istitle()", "True"),
    ("'Hello World'.istitle()", "True"),
    ("'Hello-World'.istitle()", "True"),
    ("'hello'.istitle()", "False"),
    ("'HELLO'.istitle()", "False"),
    ("'Hello world'.istitle()", "False"),
    ("'hello World'.istitle()", "False"),
    ("''.istitle()", "False"),
    ("'A'.istitle()", "True"),
    ("'a'.istitle()", "False"),
    ("'Hello123World'.istitle()", "True"),
    ("'Hello123world'.istitle()", "False"),

    # isascii
    ("'hello'.isascii()", "True"),
    ("'HELLO'.isascii()", "True"),
    ("'123'.isascii()", "True"),
    ("'hello123'.isascii()", "True"),
    ("''.isascii()", "True"),
    ("' '.isascii()", "True"),
    ("'\\n'.isascii()", "True"),
    ("'Привет'.isascii()", "False"),
    ("'你好'.isascii()", "False"),
    ("'café'.isascii()", "False"),

    # isidentifier
    ("'abc'.isidentifier()", "True"),
    ("'_abc'.isidentifier()", "True"),
    ("'abc123'.isidentifier()", "True"),
    ("'123abc'.isidentifier()", "False"),
    ("'abc-def'.isidentifier()", "False"),
    ("'abc def'.isidentifier()", "False"),
    ("''.isidentifier()", "False"),
    ("'Привет'.isidentifier()", "True"),
    ("'переменная123'.isidentifier()", "True"),
    ("'_'.isidentifier()", "True"),
    ("'__init__'.isidentifier()", "True"),
    ("'for'.isidentifier()", "True"),
    ("'class'.isidentifier()", "True"),

    # isprintable
    ("'hello'.isprintable()", "True"),
    ("'123'.isprintable()", "True"),
    ("''.isprintable()", "True"),
    ("'Привет'.isprintable()", "True"),
    ("'hello world'.isprintable()", "True"),
    ("'\\n'.isprintable()", "False"),
    ("'\\t'.isprintable()", "False"),
    ("'\\r'.isprintable()", "False"),
    ("'hello\\n'.isprintable()", "False"),
    ("'hello\\tworld'.isprintable()", "False"),

    # partition
    ("'abc:def'.partition(':')", "('abc', ':', 'def')"),
    ("'hello world'.partition(' ')", "('hello', ' ', 'world')"),
    ("'hello'.partition(':')", "('hello', '', '')"),
    ("''.partition(':')", "('', '', '')"),
    ("':abc'.partition(':')", "('', ':', 'abc')"),
    ("'abc:'.partition(':')", "('abc', ':', '')"),
    ("'a:b:c'.partition(':')", "('a', ':', 'b:c')"),

    # rpartition
    ("'abc:def:ghi'.rpartition(':')", "('abc:def', ':', 'ghi')"),
    ("'abc:def'.rpartition(':')", "('abc', ':', 'def')"),
    ("'hello'.rpartition(':')", "('', '', 'hello')"),
    ("''.rpartition(':')", "('', '', '')"),
    ("':abc'.rpartition(':')", "('', ':', 'abc')"),
    ("'abc:'.rpartition(':')", "('abc', ':', '')"),
    ("'a:b:c:d'.rpartition(':')", "('a:b:c', ':', 'd')"),

    # splitlines
    ("'a\\nb\\nc'.splitlines()", "['a', 'b', 'c']"),
    ("'a\\nb\\nc'.splitlines(True)", "['a\\n', 'b\\n', 'c']"),
    ("'a\\nb\\nc'.splitlines(False)", "['a', 'b', 'c']"),
    ("'abc'.splitlines()", "['abc']"),
    ("''.splitlines()", "[]"),
    ("'abc\\n'.splitlines()", "['abc']"),
    ("'abc\\n'.splitlines(True)", "['abc\\n']"),
    ("'a\\r\\nb'.splitlines()", "['a', 'b']"),
    ("'a\\r\\nb'.splitlines(True)", "['a\\r\\n', 'b']"),

    # zfill
    ("'42'.zfill(5)", "'00042'"),
    ("'42'.zfill(2)", "'42'"),
    ("'42'.zfill(1)", "'42'"),
    ("'-42'.zfill(5)", "'-0042'"),
    ("'+42'.zfill(5)", "'+0042'"),
    ("''.zfill(5)", "'00000'"),
    ("'abc'.zfill(5)", "'00abc'"),
    ("'abc'.zfill(3)", "'abc'"),
    ("'abc'.zfill(0)", "'abc'"),
    ("'-abc'.zfill(8)", "'-0000abc'"),

    # expandtabs
    ("'abc\\t'.expandtabs()", "'abc     '"),
    ("'a\\tb'.expandtabs()", "'a       b'"),
    ("'a\\tb'.expandtabs(4)", "'a   b'"),
    ("'\\t'.expandtabs()", "'        '"),
    ("'\\t'.expandtabs(4)", "'    '"),
    ("''.expandtabs()", "''"),
    ("'abc'.expandtabs()", "'abc'"),
    ("'a\\tb\\nc\\td'.expandtabs(4)", "'a   b\\nc   d'"),
    ("'a\\tb'.expandtabs(1)", "'a b'"),
    ("'a\\tb'.expandtabs(0)", "'ab'"),

    # rsplit
    ("'a,b,c'.rsplit(',')", "['a', 'b', 'c']"),
    ("'a,b,c,d'.rsplit(',', 1)", "['a,b,c', 'd']"),
    ("'a,b,c,d'.rsplit(',', 2)", "['a,b', 'c', 'd']"),
    ("'abc'.rsplit(',')", "['abc']"),
    ("''.rsplit(',')", "['']"),
    ("'a b c'.rsplit()", "['a', 'b', 'c']"),
    ("'  a   b  c '.rsplit()", "['a', 'b', 'c']"),
    ("'a b c d'.rsplit(None, 1)", "['a b c', 'd']"),
    ("'a b c d'.rsplit(None, 2)", "['a b', 'c', 'd']"),

    # casefold
    ("'HELLO'.casefold()", "'hello'"),
    ("'Hello'.casefold()", "'hello'"),
    ("'straße'.casefold()", "'strasse'"),
    ("'Straße'.casefold()", "'strasse'"),
    ("''.casefold()", "''"),
    ("'ß'.casefold()", "'ss'"),
    ("'ẞ'.casefold()", "'ss'"),
    ("'İ'.casefold()", "'i̇'"),
    ("'Σ'.casefold()", "'σ'"),
    ("'ς'.casefold()", "'σ'"),

    # str
    ("str()", "''"),
    ("str('abc')", "'abc'"),
    ("str(123)", "'123'"),
    ("str(-42)", "'-42'"),
    ("str(True)", "'True'"),
    ("str(False)", "'False'"),
    ("str(None)", "'None'"),

    ("str([])", "'[]'"),
    ("str([1, 2, 3])", "'[1, 2, 3]'"),
    ("str((1, 2))", "'(1, 2)'"),
    ("str((1,))", "'(1,)'"),
    ("str({})", "'{}'"),
    ("str(str(123))", "'123'"),
    ("str(str(True))", "'True'"),
    ("str(str(None))", "'None'"),

    #maketrans
    ("'abc'.maketrans('abc', 'xyz')", "{97: 120, 98: 121, 99: 122}"),
    ("'abc'.maketrans('ab', 'xy', '!')", "{97: 120, 98: 121, 33: None}"),
    ("''.maketrans({'a': 'x', 'b': 'y'})", "{97: 'x', 98: 'y'}"),
    ("''.maketrans({97: 'x'})", "{97: 'x'}"),
    ("''.maketrans({'a': None})", "{97: None}"),

    # str.maketrans
    ("str.maketrans('abc', 'xyz')", "{97: 120, 98: 121, 99: 122}"),
    ("str.maketrans('ab', 'xy', '!')", "{97: 120, 98: 121, 33: None}"),
    ("str.maketrans({'a': 'x', 'b': 'y'})", "{97: 'x', 98: 'y'}"),
    ("str.maketrans({97: 'x'})", "{97: 'x'}"),
    ("str.maketrans({'a': None})", "{97: None}"),

    # translate
    ("'abc'.translate({97: 120})", "'xbc'"),
    ("'abc'.translate({97: 120, 98: 121})", "'xyc'"),
    ("'abc'.translate({98: None})", "'ac'"),
    ("'abc'.translate({97: None, 99: None})", "'b'"),
    ("'abc'.translate({97: 'XYZ'})", "'XYZbc'"),
    ("'abc'.translate({98: '--'})", "'a--c'"),
    ("'abc'.translate('abc'.maketrans('abc', 'xyz'))", "'xyz'"),
    ("'hello'.translate('el'.maketrans('el', 'ip'))", "'hippo'"),
    ("'a!b!c'.translate('ab'.maketrans('ab', 'xy', '!'))", "'xyc'"),
    ("'abc'.translate({120: 121})", "'abc'"),

    # format_map
    ("'{name}'.format_map({'name': 'Bob'})", "'Bob'"),
    ("'{name} is cool'.format_map({'name': 'Bob'})", "'Bob is cool'"),
    ("'{x}+{y}'.format_map({'x': 2, 'y': 3})", "'2+3'"),
    ("'{a}{b}{c}'.format_map({'a': 'x', 'b': 'y', 'c': 'z'})", "'xyz'"),
    ("'{x}'.format_map({'x': True})", "'True'"),
    ("'{x}'.format_map({'x': None})", "'None'"),
    ("''.format_map({})", "''"),
    ("'{{hello}}'.format_map({})", "'{hello}'"),
    ("'{{x}} = {x}'.format_map({'x': 42})", "'{x} = 42'"),
    ("'{x!r}'.format_map({'x': 'abc'})", "\"'abc'\""),
    ("'{x!s}'.format_map({'x': 'abc'})", "'abc'"),

    # индексирование списка
    ("'{user[0]}'.format_map({'user': [10, 20, 30]})", "'10'"),
    ("'{user[2]}'.format_map({'user': [10, 20, 30]})", "'30'"),

    # индексирование строки
    ("'{user[1]}'.format_map({'user': 'abc'})", "'b'"),

    # индексирование словаря
    ("'{user[name]}'.format_map({'user': {'name': 'Bob'}})", "'Bob'"),
    ("'{user[age]}'.format_map({'user': {'age': 21}})", "'21'"),

    # смешанные случаи
    ("'{user[profile][name]}'.format_map({'user': {'profile': {'name': 'Bob'}}})", "'Bob'"),
    ("'{users[0][name]}'.format_map({'users': [{'name': 'Bob'}]})", "'Bob'"),

    # format
    ("'{}'.format(42)", "'42'"),
    ("'{0}'.format(42)", "'42'"),
    ("'{0} {1}'.format('a', 'b')", "'a b'"),
    ("'{name}'.format(name='Bob')", "'Bob'"),
    ("'{name} is {age}'.format(name='Bob', age=21)", "'Bob is 21'"),
    ("'{{hello}}'.format()", "'{hello}'"),
    ("'{x!r}'.format(x='abc')", "\"'abc'\""),
    ("'{x!s}'.format(x='abc')", "'abc'"),
    ("'{0[1]}'.format([10,20,30])", "'20'"),
    ("'{user[name]}'.format(user={'name':'Bob'})", "'Bob'"),
    ("'{user[profile][name]}'.format(user={'profile':{'name':'Bob'}})", "'Bob'"),
    ("'{0} + {1}'.format(2,3)", "'2 + 3'"),
    ("''.format()", "''"),

    # alignment
    ("'{:>5}'.format('x')", "'    x'"),
    ("'{:<5}'.format('x')", "'x    '"),
    ("'{:^5}'.format('x')", "'  x  '"),
    ("'{x:>5}'.format(x='a')", "'    a'"),
    ("'{x:<5}'.format(x='a')", "'a    '"),
    ("'{x:^5}'.format(x='a')", "'  a  '"),

    # zero fill
    ("'{:04}'.format(7)", "'0007'"),
    ("'{:08}'.format(123)", "'00000123'"),
    ("'{x:04}'.format(x=9)", "'0009'"),

    # float precision
    ("'{:.2f}'.format(3.14159)", "'3.14'"),
    ("'{:.3f}'.format(3.14159)", "'3.142'"),
    ("'{x:.2f}'.format(x=2.71828)", "'2.72'"),

    # mixed
    ("'{x:>10}'.format(x='Bob')", "'       Bob'"),
    ("'{x:^10}'.format(x='Bob')", "'   Bob    '"),
    ("'{x:08}'.format(x=42)", "'00000042'"),
    ("'{x:.1f}'.format(x=1.99)", "'2.0'"),

    # integer formats
    ("'{:d}'.format(42)", "'42'"),

    ("'{:x}'.format(255)", "'ff'"),
    ("'{:X}'.format(255)", "'FF'"),

    ("'{:o}'.format(64)", "'100'"),

    ("'{:b}'.format(10)", "'1010'"),
    ("'{:b}'.format(255)", "'11111111'"),

    # знаки
    ("'{:+d}'.format(42)", "'+42'"),
    ("'{:+d}'.format(-42)", "'-42'"),

    ("'{: d}'.format(42)", "' 42'"),
    ("'{: d}'.format(-42)", "'-42'"),

    # плавающая точка
    ("'{:+.2f}'.format(3.14159)", "'+3.14'"),
    ("'{:+.2f}'.format(-3.14159)", "'-3.14'"),

    # проценты
    ("'{:%}'.format(0.25)", "'25.000000%'"),
    ("'{:%}'.format(1)", "'100.000000%'"),

    # разделение разрядов числа
    ("'{:,}'.format(1000)", "'1,000'"),
    ("'{:,}'.format(1000000)", "'1,000,000'"),
    ("'{:,}'.format(123456789)", "'123,456,789'"),

    # bytes
    ("b'abc'", "b'abc'"),
    ("b\"abc\" == b\"abc\"", "True"),
    ("hash(b'abc') == hash(b'abc')", "True"),
    ("b'\x41\x42\x43'", "b'ABC'"),
    ("b\"\\n\"", "b'\\n'"),
    ("b\"'\"", "b\"'\""),
    ("b\"\\x00\"", "b'\\x00'"),
    ("b\"\\n\"", "b'\\n'"),
    ("b\"\\t\"", "b'\\t'"),
    ("b\"\\\\\"", "b'\\\\'"),
    ("b\"\\xff\"", "b'\\xff'"),
    ("b\"'\"", "b\"'\""),

    # __getitem__
    ("b\"A\"[0]", "65"),
    ("b\"ABC\"[0]", "65"),
    ("b'\x41\x42\x43'[1]", "66"),
    ("b\"ABC\"[1]", "66"),
    ("b\"ABC\"[2]", "67"),
    ("b\"\\xff\"[0]", "255"),
    ("b\"\\x00\"[0]", "0"),

    # отрицательный индекс
    ("b\"ABC\"[-1]", "67"),
    ("b\"ABC\"[-2]", "66"),
    ("b\"ABC\"[-3]", "65"),
    ("b\"\\xff\"[-1]", "255"),

    # len
    ("len(b\"\")", "0"),
    ("len(b\"A\")", "1"),
    ("len(b\"ABC\")", "3"),
    ("len(b\"\\xff\")", "1"),
    ("len(b\"Hello World\")", "11"),

    # __add__
    ("b\"abc\" + b\"def\"", "b'abcdef'"),
    ("b\"\" + b\"\"", "b''"),
    ("b\"\" + b\"abc\"", "b'abc'"),
    ("b\"abc\" + b\"\"", "b'abc'"),
    ("b\"\\xff\" + b\"\\x01\"", "b'\\xff\\x01'"),
    ("b\"a\" + b\"b\" + b\"c\"", "b'abc'"),

    # __mul__
    ("b'a' * 3", "b'aaa'"),
    ("b'ab' * 3", "b'ababab'"),
    ("b'hello' * 2", "b'hellohello'"),
    ("3 * b'a'", "b'aaa'"),
    ("2 * b'ab'", "b'abab'"),
    ("b'a' * 0", "b''"),
    ("b'hello' * 0", "b''"),
    ("0 * b'abc'", "b''"),
    ("b'a' * -1", "b''"),
    ("b'hello' * -100", "b''"),
    ("-5 * b'abc'", "b''"),
    ("b'' * 10", "b''"),
    ("b'' * 0", "b''"),
    ("b'' * -5", "b''"),
    ("b'a' * 1", "b'a'"),
    ("1 * b'abc'", "b'abc'"),
    ("b'x' * 100", "b'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'"),
    ("len(b'x' * 100)", "100"),

    # __eq__
    ("b'' == b''", "True"),
    ("b'a' == b'a'", "True"),
    ("b'abc' == b'abc'", "True"),
    ("b'abc' == b'abd'", "False"),
    ("b'abc' == b''", "False"),
    ("b'' == b'abc'", "False"),
    ("b'123' == b'123'", "True"),
    ("b'123' == b'456'", "False"),
    ("b'abc' == 'abc'", "False"),
    ("b'abc' == 123", "False"),

    # __ne__
    ("b'' != b''", "False"),
    ("b'a' != b'a'", "False"),
    ("b'abc' != b'abc'", "False"),
    ("b'abc' != b'abd'", "True"),
    ("b'abc' != b''", "True"),
    ("b'' != b'abc'", "True"),
    ("b'123' != b'123'", "False"),
    ("b'123' != b'456'", "True"),
    ("b'abc' != 'abc'", "True"),
    ("b'abc' != 123", "True"),

    # __lt__
    ("b'a' < b'b'", "True"),
    ("b'b' < b'a'", "False"),
    ("b'abc' < b'abd'", "True"),
    ("b'abd' < b'abc'", "False"),
    ("b'ab' < b'abc'", "True"),
    ("b'abc' < b'ab'", "False"),
    ("b'' < b'a'", "True"),
    ("b'a' < b''", "False"),
    ("b'abc' < b'abc'", "False"),
    ("b'aaa' < b'aab'", "True"),
    ("b'aab' < b'aaa'", "False"),
    ("b'abcdef' < b'abcxyz'", "True"),
    ("b'abcxyz' < b'abcdef'", "False"),

    # __le__
    ("b'a' <= b'a'", "True"),
    ("b'a' <= b'b'", "True"),
    ("b'b' <= b'a'", "False"),
    ("b'abc' <= b'abc'", "True"),
    ("b'abc' <= b'abd'", "True"),
    ("b'abd' <= b'abc'", "False"),
    ("b'ab' <= b'abc'", "True"),
    ("b'abc' <= b'ab'", "False"),
    ("b'' <= b''", "True"),
    ("b'' <= b'a'", "True"),
    ("b'a' <= b''", "False"),

    # __gt__
    ("b'a' > b'a'", "False"),
    ("b'b' > b'a'", "True"),
    ("b'a' > b'b'", "False"),
    ("b'abd' > b'abc'", "True"),
    ("b'abc' > b'abd'", "False"),
    ("b'abc' > b'ab'", "True"),
    ("b'ab' > b'abc'", "False"),
    ("b'a' > b''", "True"),
    ("b'' > b'a'", "False"),
    ("b'' > b''", "False"),
    ("b'zzz' > b'aaa'", "True"),
    ("b'aaa' > b'zzz'", "False"),

    # __ge__
    ("b'a' >= b'a'", "True"),
    ("b'b' >= b'a'", "True"),
    ("b'a' >= b'b'", "False"),
    ("b'abc' >= b'abc'", "True"),
    ("b'abd' >= b'abc'", "True"),
    ("b'abc' >= b'abd'", "False"),
    ("b'abc' >= b'ab'", "True"),
    ("b'ab' >= b'abc'", "False"),
    ("b'a' >= b''", "True"),
    ("b'' >= b'a'", "False"),
    ("b'' >= b''", "True"),
    ("b'zzz' >= b'aaa'", "True"),
    ("b'aaa' >= b'zzz'", "False"),

    # not
    ("not True", "False"),
    ("not False", "True"),
    ("not 0", "True"),
    ("not 1", "False"),
    ("not -1", "False"),
    ("not 42", "False"),
    ("not b''", "True"),
    ("not b'a'", "False"),
    ("not b'abc'", "False"),
    ("not None", "True"),
    ("not (1 == 1)", "False"),
    ("not (1 == 2)", "True"),
    ("not not True", "True"),
    ("not not False", "False"),
    ("not not 0", "False"),
    ("not not 1", "True"),
    ("not not not 0", "True"),
    ("not not not not 0", "False"),

    # унарный +
    ("+1", "1"),
    ("+42", "42"),
    ("+0", "0"),
    ("+(-5)", "-5"),
    ("+3.14", "3.14"),
    ("+(-3.14)", "-3.14"),
    ("+True", "1"),
    ("+False", "0"),

    # унарный минус
    ("-1", "-1"),
    ("-42", "-42"),
    ("-0", "0"),
    ("-(-1)", "1"),
    ("-(-42)", "42"),
    ("-3.14", "-3.14"),
    ("-(-3.14)", "3.14"),
    ("-True", "-1"),
    ("-False", "0"),

    # bytes __contains__
    ("97 in b'abc'", "True"),
    ("98 in b'abc'", "True"),
    ("99 in b'abc'", "True"),
    ("100 in b'abc'", "False"),
    ("0 in b'abc'", "False"),
    ("97 in b''", "False"),
    ("0 in b''", "False"),
    ("255 in b'\\xff'", "True"),
    ("254 in b'\\xff'", "False"),
    ("0 in b'\\x00'", "True"),
    ("1 in b'\\x00'", "False"),
    ("0 in b'\\x00'", "True"),
    ("65 in b'ABC'", "True"),
    ("97 in b'ABC'", "False"),
    ("97 in b'aaaaa'", "True"),
    ("98 in b'aaaaa'", "False"),

    # not in
    ("97 not in b'abc'", "False"),
    ("98 not in b'abc'", "False"),
    ("99 not in b'abc'", "False"),
    ("100 not in b'abc'", "True"),
    ("0 not in b'abc'", "True"),
    ("97 not in b''", "True"),
    ("0 not in b''", "True"),
    ("255 not in b'\\xff'", "False"),
    ("254 not in b'\\xff'", "True"),
    ("0 not in b'\\x00'", "False"),
    ("1 not in b'\\x00'", "True"),

    # цепочки сравнений
    ("97 in b'abc' == True", "False"),
    ("100 in b'abc' == False", "False"),
    ("97 in b'abc' != False", "True"),
    ("100 in b'abc' != True", "False"),
    ("97 in b'abc' and True", "True"),
    ("100 in b'abc' or True", "True"),

    # предшествие отрицания
    ("not 97 in b'abc'", "False"),
    ("not 100 in b'abc'", "True"),
    ("not (97 in b'abc')", "False"),
    ("not (100 in b'abc')", "True"),
    ("not 97 not in b'abc'", "True"),
    ("not 100 not in b'abc'", "False"),

    # bytes __iter__ и __next__
    ("list(b'')", "[]"),
    ("list(b'a')", "[97]"),
    ("list(b'ab')", "[97, 98]"),
    ("list(b'abc')", "[97, 98, 99]"),

    ("list(b'\\xff')", "[255]"),
    ("list(b'\\x80')", "[128]"),
    ("list(b'\\x00')", "[0]")
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

sys.stdout.reconfigure(encoding="utf-8")

_result = eval({expr!r})

if _result is not None:
    sys.stdout.write(repr(_result))
"""
    else:
        code_to_exec = "\n".join(lines[:-1])
        last_expr = lines[-1].strip()

        code = f"""
import sys

sys.stdout.reconfigure(encoding="utf-8")

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

    # copy
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

    # list(values())
    (["d = {'x': 1, 'y': 2}",
      "list(d.values())"], "[1, 2]"),

    # list(keys())
    (["d = {'x': 1, 'y': 2}",
      "list(d.keys())"], "['x', 'y']"),

    # list(items())
    (["d = {'x': 1, 'y': 2}",
      "list(d.items())"], "[('x', 1), ('y', 2)]"),

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

    # union empty
    (["a = {1, 2}",
      "a.union(set())"], "{1, 2}"),

    # union с несколькими множествами
    (["a = {1, 2}",
      "b = {3, 4}",
      "c = {5}",
      "a.union(b, c)"], "{1, 2, 3, 4, 5}"),

    # union с пересечениями
    (["a = {1, 2}",
      "b = {2, 3}",
      "c = {3, 4}",
      "a.union(b, c)"], "{1, 2, 3, 4}"),

    # union без аргументов
    (["a = {1, 2}",
      "a.union()"], "{1, 2}"),

    # union с самим собой
    (["a = {1, 2}",
      "a.union(a)"], "{1, 2}"),

    # устранение дублкатов
    (["a = {1, 2}",
      "b = {2, 2, 3}",
      "a.union(b)"], "{1, 2, 3}"),

    # базовое пересечение множеств
    (["a = {1, 2, 3}",
      "b = {2, 3, 4}",
      "a.intersection(b)"], "{2, 3}"),

    # пересечение без общих элементов
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.intersection(b)"], "set()"),

    # пересечение множества с самим собой
    (["a = {1, 2, 3}",
      "a.intersection(a)"], "{1, 2, 3}"),

    # пересечение с пустым множеством
    (["a = {1, 2, 3}",
      "a.intersection(set())"], "set()"),

    # intersection нескольких множеств
    (["a = {1, 2, 3, 4}",
      "b = {2, 3, 4}",
      "c = {3, 4, 5}",
      "a.intersection(b, c)"], "{3, 4}"),

    # intersection без общих элементов
    (["a = {1, 2}",
      "b = {3, 4}",
      "c = {5}",
      "a.intersection(b, c)"], "set()"),

    # intersection без аргументов
    (["a = {1, 2}",
      "a.intersection()"], "{1, 2}"),

    # базовая разность множеств
    (["a = {1, 2, 3}",
      "b = {2}",
      "a.difference(b)"], "{1, 3}"),

    # разность без пересечения
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.difference(b)"], "{1, 2}"),

    # разность множества с самим собой
    (["a = {1, 2, 3}",
      "a.difference(a)"], "set()"),

    # разность с пустым множеством
    (["a = {1, 2, 3}",
      "a.difference(set())"], "{1, 2, 3}"),

    # пустое множество минус непустое
    (["a = set()",
      "b = {1, 2}",
      "a.difference(b)"], "set()"),

    # difference нескольких множеств
    (["a = {1, 2, 3, 4, 5}",
      "b = {2}",
      "c = {4, 5}",
      "a.difference(b, c)"], "{1, 3}"),

    # difference с полным удалением
    (["a = {1, 2}",
      "b = {1}",
      "c = {2}",
      "a.difference(b, c)"], "set()"),

    # difference без аргументов
    (["a = {1, 2}",
      "a.difference()"], "{1, 2}"),

    # базовая симметрическая разность
    (["a = {1, 2, 3}",
      "b = {3, 4}",
      "a.symmetric_difference(b)"], "{1, 2, 4}"),

    # полностью одинаковые множества
    (["a = {1, 2}",
      "b = {1, 2}",
      "a.symmetric_difference(b)"], "set()"),

    # полностью разные множества
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.symmetric_difference(b)"], "{1, 2, 3, 4}"),

    # симметрическая разность с пустым множеством
    (["a = {1, 2, 3}",
      "a.symmetric_difference(set())"], "{1, 2, 3}"),

    # пустое множество с непустым
    (["a = set()",
      "b = {1, 2}",
      "a.symmetric_difference(b)"], "{1, 2}"),

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

    # пустое множество — подмножество любого
    (["a = set()",
      "b = {1, 2}",
      "a.issubset(b)"], "True"),

    # непустое множество не является подмножеством пустого
    (["a = {1}",
      "b = set()",
      "a.issubset(b)"], "False"),

    # пустое множество — подмножество самого себя
    (["a = set()",
      "b = set()",
      "a.issubset(b)"], "True"),

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

    # любое множество — надмножество пустого
    (["a = {1, 2}",
      "b = set()",
      "a.issuperset(b)"], "True"),

    # пустое множество не является надмножеством непустого
    (["a = set()",
      "b = {1}",
      "a.issuperset(b)"], "False"),

    # пустое множество — надмножество самого себя
    (["a = set()",
      "b = set()",
      "a.issuperset(b)"], "True"),

    # isdisjoint
    # полностью не пересекающиеся множества
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.isdisjoint(b)"], "True"),

    # один общий элемент
    (["a = {1, 2}",
      "b = {2, 3}",
      "a.isdisjoint(b)"], "False"),

    # полностью одинаковые множества
    (["a = {1, 2}",
      "b = {1, 2}",
      "a.isdisjoint(b)"], "False"),

    # пустое множество и непустое
    (["a = set()",
      "b = {1, 2}",
      "a.isdisjoint(b)"], "True"),

    # два пустых множества
    (["a = set()",
      "b = set()",
      "a.isdisjoint(b)"], "True"),

    # один общий элемент среди многих
    (["a = {1, 2, 3, 4}",
      "b = {9, 8, 3}",
      "a.isdisjoint(b)"], "False"),

    # copy
    (["a = {1, 2, 3}",
      "b = a.copy()",
      "b"], "{1, 2, 3}"),

    # copy создает новый объект
    (["a = {1, 2}",
      "b = a.copy()",
      "b.add(3)",
      "a"], "{1, 2}"),

    # копирование пустого множества
    (["a = set()",
      "a.copy()"], "set()"),

    # clear
    (["a = {1, 2, 3}",
      "a.clear()", "a"], "set()"),

    # clear возвращает None
    (["a = {1}", "a.clear()"], ""),

    # clear для пустого множества
    (["a = set()",
      "a.clear()",
      "a"], "set()"),

    # clear не ломает set после повторного использования
    (["a = {1, 2}",
      "a.clear()",
      "a.add(5)",
      "a"], "{5}"),

    # pop удаляет элемент
    (["a = {1, 2, 3}",
      "a.pop()",
      "a"], "{2, 3}"),

    # pop возвращает элемент
    (["a = {1, 2}",
      "a.pop()"], "1"),

    # pop на множестве из одного элемента
    (["a = {42}",
      "a.pop()",
      "a"], "set()"),

    # после pop можно продолжать использовать set
    (["a = {1, 2}",
      "a.pop()",
      "a.add(5)",
      "a"], "{2, 5}"),

    # update одним множеством
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.update(b)",
      "a"], "{1, 2, 3, 4}"),

    # update несколькими множествами
    (["a = {1}",
      "b = {2}",
      "c = {3}",
      "a.update(b, c)",
      "a"], "{1, 2, 3}"),

    # update с пересечениями
    (["a = {1, 2}",
      "b = {2, 3}",
      "c = {3, 4}",
      "a.update(b, c)",
      "a"], "{1, 2, 3, 4}"),

    # update пустым множеством
    (["a = {1, 2}",
      "b = set()",
      "a.update(b)",
      "a"], "{1, 2}"),

    # update без аргументов
    (["a = {1, 2}",
      "a.update()",
      "a"], "{1, 2}"),

    # update возвращает None
    (["a = {1}",
      "b = {2}",
      "a.update(b)"],
     ""),

    # update самим собой
    (["a = {1, 2}",
      "a.update(a)",
      "a"], "{1, 2}"),

    # difference_update одним множеством
    (["a = {1, 2, 3}",
      "b = {2}",
      "a.difference_update(b)",
      "a"], "{1, 3}"),

    # difference_update несколькими множествами
    (["a = {1, 2, 3, 4, 5}",
      "b = {2}",
      "c = {4, 5}",
      "a.difference_update(b, c)",
      "a"], "{1, 3}"),

    # difference_update без пересечений
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.difference_update(b)",
      "a"], "{1, 2}"),

    # difference_update полного множества
    (["a = {1, 2}",
      "b = {1, 2}",
      "a.difference_update(b)",
      "a"], "set()"),

    # difference_update без аргументов
    (["a = {1, 2}",
      "a.difference_update()",
      "a"], "{1, 2}"),

    # difference_update возвращает None
    (["a = {1}",
      "b = {1}",
      "a.difference_update(b)"], ""),

    # difference_update самим собой (пока не поддерживается)
    (["a = {1, 2, 3}",
      "a.difference_update(a)",
      "a"], "set()"),

    # intersection_update одним множеством
    (["a = {1, 2, 3}",
      "b = {2, 3, 4}",
      "a.intersection_update(b)",
      "a"], "{2, 3}"),

    # intersection_update несколькими множествами
    (["a = {1, 2, 3, 4}",
      "b = {2, 3, 4}",
      "c = {3, 4}",
      "a.intersection_update(b, c)",
      "a"], "{3, 4}"),

    # intersection_update без пересечений (пока не поддерживается)
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.intersection_update(b)",
      "a"], "set()"),

    # intersection_update полного совпадения
    (["a = {1, 2}",
      "b = {1, 2}",
      "a.intersection_update(b)",
      "a"], "{1, 2}"),

    # intersection_update без аргументов
    (["a = {1, 2}",
      "a.intersection_update()",
      "a"], "{1, 2}"),

    # intersection_update возвращает None
    (["a = {1}",
      "b = {1}",
      "a.intersection_update(b)"], ""),

    # intersection_update самим собой
    (["a = {1, 2, 3}",
      "a.intersection_update(a)",
      "a"], "{1, 2, 3}"),

    # symmetric_difference_update базовый
    (["a = {1, 2, 3}",
      "b = {3, 4, 5}",
      "a.symmetric_difference_update(b)",
      "a"], "{1, 2, 4, 5}"),

    # symmetric_difference_update без пересечений
    (["a = {1, 2}",
      "b = {3, 4}",
      "a.symmetric_difference_update(b)",
      "a"], "{1, 2, 3, 4}"),

    # symmetric_difference_update одинаковых множеств (пока не поддерживается)
    (["a = {1, 2}",
      "b = {1, 2}",
      "a.symmetric_difference_update(b)",
      "a"], "set()"),

    # symmetric_difference_update частичного пересечения
    (["a = {1, 2, 3}",
      "b = {2, 3, 4}",
      "a.symmetric_difference_update(b)",
      "a"], "{1, 4}"),

    # symmetric_difference_update пустого множества (пока не поддерживается)
    (["a = {1, 2}",
      "b = set()",
      "a.symmetric_difference_update(b)",
      "a"], "{1, 2}"),

    # symmetric_difference_update с пустым self (пока не поддерживается)
    (["a = set()",
      "b = {1, 2}",
      "a.symmetric_difference_update(b)",
      "a"], "{1, 2}"),

    # symmetric_difference_update возвращает None
    (["a = {1}",
      "b = {2}",
      "a.symmetric_difference_update(b)"], ""),

    # symmetric_difference_update самим собой
    (["a = {1, 2, 3}",
      "a.symmetric_difference_update(a)",
      "a"], "set()"),

    # # __iter__ возвращает самого себя (пока не поддерживается)
    # (["a = {1, 2}",
    #   "it = iter(a)",
    #   "iter(it) == it"], "True"),

    # next() возвращает элементы множества
    (["a = {1}",
      "it = iter(a)",
      "next(it)"], "1"),

    # Итерация по нескольким элементам
    (["a = {1, 2}",
      "it = iter(a)",
      "x = next(it)",
      "y = next(it)",
      "{x, y}"], "{1, 2}"),

    # # Итерация по set со строками (не стабильный тест)
    # (["a = {'x', 'y'}",
    #   "it = iter(a)",
    #   "v1 = next(it)",
    #   "v2 = next(it)",
    #   "{v1, v2}"], "{'x', 'y'}"),

    # Итерация по set с tuple
    (["a = {(1, 2), (3, 4)}",
      "it = iter(a)",
      "v1 = next(it)",
      "v2 = next(it)",
      "{v1, v2}"], "{(1, 2), (3, 4)}"),

    # Повторный вызов iter()
    (["a = {1, 2}",
      "it1 = iter(a)",
      "it2 = iter(a)",
      "it1 == it2"], "False"),

    # Итерация через for
    (["a = {1, 2, 3}",
      "result = []",
      "for x in a:",
      "    result.append(x)",
      "",
      "result"], "[1, 2, 3]"),

    # пустой список
    (["a = list()",
      "a"], "[]"),

    # из списка
    (["a = list([1, 2, 3])",
      "a"], "[1, 2, 3]"),

    # из строки (итерабельность)
    (["a = list('ab')",
      "a"], "['a', 'b']"),

    # из множества
    (["a = list({1, 2, 2})",
      "a"], "[1, 2]"),

    # пустое множество
    (["a = set()",
      "a"], "set()"),

    # удаление дублей
    (["a = set([1, 2, 2, 3])",
      "a"], "{1, 2, 3}"),

    # пустой dict
    (["a = dict()",
      "a"], "{}"),

    # из пар (tuple)
    (["a = dict([(1, 'a'), (2, 'b')])",
      "a"], "{1: 'a', 2: 'b'}"),

    # из tupl'а tuple-ов
    (["a = dict(((1, 2), (3, 4)))",
      "a"], "{1: 2, 3: 4}"),

    # set не должен печататься как {}
    (["a = set()",
      "a"], "set()"),

    # dict vs set конфликт на {}
    (["a = {}",
      "a"], "{}"),

    # # list из set сохраняет уникальность (пока не поддерживается)
    # (["a = list({3, 1, 2, 2})",
    #   "a"], "[3, 1, 2]"),

    # Распаковка list через *
    (["a = [1, 2]",
      "[0, *a, 3]"], "[0, 1, 2, 3]"),

    # Распаковка tuple через *
    (["a = (1, 2)",
      "[*a]"], "[1, 2]"),

    # Распаковка set через *
    (["a = {1, 2}",
      "b = [*a]",
      "len(b)"], "2"),

    # Несколько распаковок в list
    (["a = [1, 2]",
      "b = [3, 4]",
      "[*a, *b]"], "[1, 2, 3, 4]"),

    # Распаковка пустого списка
    (["a = []",
      "[1, *a, 2]"], "[1, 2]"),

    # Распаковка строки
    (["a = 'abc'",
      "[*a]"], "['a', 'b', 'c']"),

    # Распаковка tuple в tuple
    (["a = (1, 2)",
      "(*a, 3)"], "(1, 2, 3)"),

    # Распаковка list в set
    (["a = [1, 2, 3]",
      "{*a}"], "{1, 2, 3}"),

    # Распаковка set в set
    (["a = {1, 2}",
      "{0, *a, 3}"], "{0, 1, 2, 3}"),

    # Простая распаковка dict через **
    (["a = {'x': 1}",
      "{**a}"], "{'x': 1}"),

    # Объединение dict через **
    (["a = {'x': 1}",
      "b = {'y': 2}",
      "{**a, **b}"], "{'x': 1, 'y': 2}"),

    # Перезапись ключа справа
    (["a = {'x': 1}",
      "b = {'x': 999}",
      "{**a, **b}"], "{'x': 999}"),

    # Перезапись ключа литералом после **
    (["a = {'x': 1}",
      "{**a, 'x': 42}"], "{'x': 42}"),

    # Перезапись ключа через **
    (["a = {'x': 1}",
      "{'x': 42, **a}"], "{'x': 1}"),

    # Распаковка пустого dict
    (["a = {}",
      "{**a}"], "{}"),

    # Смешанный dict literal + unpack
    (["a = {'b': 2}",
      "{'a': 1, **a, 'c': 3}"], "{'a': 1, 'b': 2, 'c': 3}"),

    # Двойная распаковка с trailing comma
    (["a = {'x': 1}",
      "{**a,}"], "{'x': 1}"),

    # Распаковка dict.copy()
    (["a = {'x': 1}",
      "{**a.copy()}"], "{'x': 1}"),

    # Распаковка результата функции
    (["def f():",
      "    return {'x': 1}",
      "",
      "{**f()}"], "{'x': 1}"),

    # Распаковка list в list с несколькими *
    (["a = [1]",
      "b = [2]",
      "c = [3]",
      "[*a, *b, *c]"], "[1, 2, 3]"),

    # Распаковка tuple в list
    (["a = (10, 20)",
      "[*a]"], "[10, 20]"),

    # Распаковка nested
    (["a = [1, 2]",
      "b = [*a]",
      "[*b, 3]"], "[1, 2, 3]"),

    # Распаковка dict keys в list
    (["a = {'x': 1, 'y': 2}",
      "[*a]"], "['x', 'y']"),

    # Распаковка generator-like iteration через tuple
    (["a = [1, 2, 3]",
      "(*a,)"], "(1, 2, 3)"),

    # str __getitem__
    (["s = 'hello'",
      "s[0]"], "'h'"),

    (["s = 'hello'",
      "s[1]"], "'e'"),

    (["s = 'hello'",
      "s[4]"], "'o'"),

    # отрицательная индексация
    (["s = 'hello'",
      "s[-1]"], "'o'"),

    (["s = 'hello'",
      "s[-2]"], "'l'"),

    # nested indexing
    (["s = 'abc'",
      "s[0][0]"], "'a'"),

    # # tuple unpacking от итератора (пока не поддерживается)
    # (["s = 'abc'",
    #   "(*s,)"], "('a', 'b', 'c')"),

    # len
    (["s = 'hello'",
      "len(s)"], "5"),

    # конкатенация
    (["s = 'abc'",
      "s[0] + s[1] + s[2]"], "'abc'"),

    # format_map
    (["class User:",
      "    pass",
      "",
      "u = User()",
      "u.name = 'Bob'",
      "'{u.name}'.format_map({'u': u})"], "'Bob'"),

    (["class Profile:",
      "    pass",
      "",
      "class User:",
      "    pass",
      "",
      "p = Profile()",
      "p.nickname = 'semyo'",
      "u = User()",
      "u.profile = p",
      "'{u.profile.nickname}'.format_map({'u': u})"], "'semyo'"),

    # атрибуты
    (["class User:",
     "    def __init__(self, name):",
     "        self.name = name",
     "",
     "'{user.name}'.format_map({'user': User('Bob')})"], "'Bob'"),

    (["it = iter(b'abc')",
      "next(it)"], "97"),

    (["it = iter(b'abc')",
      "temp = next(it)",
      "next(it)"], "98"),

    (["it = iter(b'abc')",
      "temp1 = next(it)",
      "temp2 = next(it)",
      "next(it)"], "99"),

    (["res = []",
      "for x in b'abc':",
      "    res.append(x)",
      "",
      "res"], "[97, 98, 99]")

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