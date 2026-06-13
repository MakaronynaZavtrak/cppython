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
    ("list(b'\\x00')", "[0]"),

    # find
    ("b'abc'.find(b'a')", "0"),
    ("b'abc'.find(b'b')", "1"),
    ("b'abc'.find(b'c')", "2"),
    ("b'abc'.find(b'd')", "-1"),
    ("b'abcabc'.find(b'abc', 1)", "3"),
    ("b'abcabc'.find(b'abc', 1, 5)", "-1"),
    ("b'abcabc'.find(b'bc', 0, 3)", "1"),
    ("b''.find(b'a')", "-1"),
    ("b'abc'.find(b'')", "0"),
    ("b'abc'.find(b'', 1, 2)", "1"),
    ("b'ababa'.find(b'aba')", "0"),
    ("b'ababa'.find(b'aba', 1)", "2"),
    ("b'\\x00\\x01\\x02'.find(b'\\x01')", "1"),
    ("b'\\xff\\xfe\\xfd'.find(b'\\xfe')", "1"),

    # rfind
    ("b'abc'.rfind(b'a')", "0"),
    ("b'abc'.rfind(b'b')", "1"),
    ("b'abc'.rfind(b'c')", "2"),
    ("b'abc'.rfind(b'd')", "-1"),

    ("b'abcabc'.rfind(b'abc')", "3"),
    ("b'abcabc'.rfind(b'bc')", "4"),
    ("b'ababa'.rfind(b'aba')", "2"),

    ("b'abcabc'.rfind(b'abc', 1)", "3"),
    ("b'abcabc'.rfind(b'abc', 4)", "-1"),

    ("b'abcabc'.rfind(b'bc', 0, 4)", "1"),
    ("b'abcabc'.rfind(b'abc', 0, 5)", "0"),

    ("b'abc'.rfind(b'')", "3"),
    ("b'abc'.rfind(b'', 0, 2)", "2"),
    ("b''.rfind(b'')", "0"),

    ("b''.rfind(b'a')", "-1"),

    ("b'\\x00\\x01\\x02\\x01'.rfind(b'\\x01')", "3"),
    ("b'\\xff\\xfe\\xfd\\xfe'.rfind(b'\\xfe')", "3"),

    # index
    ("b'abc'.index(b'a')", "0"),
    ("b'abc'.index(b'b')", "1"),
    ("b'abc'.index(b'c')", "2"),
    ("b'abcabc'.index(b'abc')", "0"),
    ("b'abcabc'.index(b'abc', 1)", "3"),
    ("b'ababa'.index(b'aba')", "0"),
    ("b'ababa'.index(b'aba', 1)", "2"),
    ("b'abc'.index(b'')", "0"),
    ("b'abc'.index(b'', 1)", "1"),
    ("b'abc'.index(b'', 1, 2)", "1"),
    ("b''.index(b'')", "0"),
    ("b'abcabc'.index(b'bc', 0, 3)", "1"),
    ("b'abcabc'.index(b'bc', 2)", "4"),
    ("b'aaaa'.index(b'aa', 1)", "1"),
    ("b'\\x00\\x01\\x02'.index(b'\\x01')", "1"),
    ("b'\\xff\\xfe\\xfd'.index(b'\\xfe')", "1"),
    ("b'\\x00\\x00\\x01'.index(b'\\x00\\x01')", "1"),

    # rindex
    ("b'abc'.rindex(b'a')", "0"),
    ("b'abc'.rindex(b'b')", "1"),
    ("b'abc'.rindex(b'c')", "2"),

    ("b'abcabc'.rindex(b'abc')", "3"),
    ("b'abcabc'.rindex(b'bc')", "4"),
    ("b'ababa'.rindex(b'aba')", "2"),

    ("b'abcabc'.rindex(b'abc', 1)", "3"),

    ("b'abcabc'.rindex(b'bc', 0, 4)", "1"),

    ("b'\\x00\\x01\\x02\\x01'.rindex(b'\\x01')", "3"),
    ("b'\\xff\\xfe\\xfd\\xfe'.rindex(b'\\xfe')", "3"),

    # count
    ("b'abc'.count(b'a')", "1"),
    ("b'abc'.count(b'b')", "1"),
    ("b'abc'.count(b'c')", "1"),
    ("b'abc'.count(b'd')", "0"),

    ("b'abcabc'.count(b'abc')", "2"),
    ("b'abcabcabc'.count(b'abc')", "3"),

    ("b'aaaa'.count(b'a')", "4"),
    ("b'aaaa'.count(b'aa')", "2"),
    ("b'aaaa'.count(b'aaa')", "1"),

    ("b'ababa'.count(b'aba')", "1"),
    ("b'ababa'.count(b'ba')", "2"),

    ("b''.count(b'a')", "0"),
    ("b''.count(b'')", "1"),

    ("b'abc'.count(b'')", "4"),
    ("b'abc'.count(b'', 1)", "3"),
    ("b'abc'.count(b'', 1, 2)", "2"),

    ("b'abcabc'.count(b'abc', 1)", "1"),
    ("b'abcabc'.count(b'abc', 3)", "1"),
    ("b'abcabc'.count(b'abc', 4)", "0"),

    ("b'abcabc'.count(b'bc', 0, 3)", "1"),
    ("b'abcabc'.count(b'bc', 2, 6)", "1"),

    ("b'\\x00\\x01\\x00\\x01'.count(b'\\x01')", "2"),
    ("b'\\xff\\xff\\xfe'.count(b'\\xff')", "2"),
    ("b'\\xff\\xff\\xfe'.count(b'\\xff\\xff')", "1"),

    # startswith
    ("b'abc'.startswith(b'a')", "True"),
    ("b'abc'.startswith(b'ab')", "True"),
    ("b'abc'.startswith(b'abc')", "True"),

    ("b'abc'.startswith(b'b')", "False"),
    ("b'abc'.startswith(b'ac')", "False"),
    ("b'abc'.startswith(b'abcd')", "False"),

    ("b''.startswith(b'')", "True"),
    ("b''.startswith(b'a')", "False"),

    ("b'abcabc'.startswith(b'abc')", "True"),
    ("b'abcabc'.startswith(b'abc', 3)", "True"),
    ("b'abcabc'.startswith(b'abc', 1)", "False"),

    ("b'abcdef'.startswith(b'cd', 2)", "True"),
    ("b'abcdef'.startswith(b'cd', 3)", "False"),

    ("b'abcdef'.startswith(b'cd', 2, 4)", "True"),
    ("b'abcdef'.startswith(b'cd', 2, 3)", "False"),

    ("b'abcdef'.startswith(b'', 0)", "True"),
    ("b'abcdef'.startswith(b'', 3)", "True"),
    ("b'abcdef'.startswith(b'', 6)", "True"),

    ("b'\\x00\\x01\\x02'.startswith(b'\\x00')", "True"),
    ("b'\\x00\\x01\\x02'.startswith(b'\\x01')", "False"),

    ("b'aaaa'.startswith(b'aa')", "True"),
    ("b'aaaa'.startswith(b'aaa')", "True"),
    ("b'aaaa'.startswith(b'aaaa')", "True"),
    ("b'aaaa'.startswith(b'aaaaa')", "False"),

    # endswith
    ("b'abc'.endswith(b'c')", "True"),
    ("b'abc'.endswith(b'b')", "False"),
    ("b'abc'.endswith(b'bc')", "True"),
    ("b'abc'.endswith(b'abc')", "True"),
    ("b'abc'.endswith(b'abcd')", "False"),

    ("b'abcabc'.endswith(b'abc')", "True"),
    ("b'abcabc'.endswith(b'abc', 0, 3)", "True"),
    ("b'abcabc'.endswith(b'abc', 0, 6)", "True"),
    ("b'abcabc'.endswith(b'abc', 1)", "True"),

    ("b'abc'.endswith(b'')", "True"),
    ("b''.endswith(b'')", "True"),
    ("b''.endswith(b'a')", "False"),

    ("b'abc'.endswith(b'c', 0, 2)", "False"),
    ("b'abc'.endswith(b'b', 0, 2)", "True"),
    ("b'abc'.endswith(b'ab', 0, 2)", "True"),

    ("b'\\x00\\x01\\x02'.endswith(b'\\x02')", "True"),
    ("b'\\xff\\xfe\\xfd'.endswith(b'\\xfd')", "True"),
    ("b'\\xff\\xfe\\xfd'.endswith(b'\\xfe')", "False"),

    # split
    ("b'abc'.split()", "[b'abc']"),
    ("b''.split()", "[]"),

    ("b'a b c'.split()", "[b'a', b'b', b'c']"),
    ("b'a  b   c'.split()", "[b'a', b'b', b'c']"),

    ("b'a,b,c'.split(b',')", "[b'a', b'b', b'c']"),
    ("b'a,b,c'.split(b',', 1)", "[b'a', b'b,c']"),

    ("b'a,,b'.split(b',')", "[b'a', b'', b'b']"),
    ("b',a,'.split(b',')", "[b'', b'a', b'']"),

    ("b'abc'.split(b'x')", "[b'abc']"),
    ("b''.split(b',')", "[b'']"),

    ("b'abc'.split(b'abc')", "[b'', b'']"),

    ("b'one two three'.split(None, 1)", "[b'one', b'two three']"),

    # rsplit
    ("b'abc'.rsplit()", "[b'abc']"),
    ("b''.rsplit()", "[]"),

    ("b'a b c'.rsplit()", "[b'a', b'b', b'c']"),
    ("b'a  b   c'.rsplit()", "[b'a', b'b', b'c']"),

    ("b'a,b,c'.rsplit(b',')", "[b'a', b'b', b'c']"),

    ("b'a,b,c'.rsplit(b',', 1)", "[b'a,b', b'c']"),
    ("b'a,b,c'.rsplit(b',', 2)", "[b'a', b'b', b'c']"),

    ("b'a,,b'.rsplit(b',')", "[b'a', b'', b'b']"),
    ("b',a,'.rsplit(b',')", "[b'', b'a', b'']"),

    ("b'abc'.rsplit(b'x')", "[b'abc']"),

    ("b''.rsplit(b',')", "[b'']"),

    ("b'one two three'.rsplit(None, 1)", "[b'one two', b'three']"),

    ("b'\\x00,\\x01,\\x02'.rsplit(b',', 1)", "[b'\\x00,\\x01', b'\\x02']"),

    # join
    ("b','.join([b'a', b'b', b'c'])", "b'a,b,c'"),
    ("b''.join([b'a', b'b', b'c'])", "b'abc'"),

    ("b'-'.join([b'abc'])", "b'abc'"),
    ("b'-'.join([])", "b''"),

    ("b' '.join([b'hello', b'world'])", "b'hello world'"),
    ("b'--'.join([b'a', b'b', b'c'])", "b'a--b--c'"),

    ("b'abc'.join([b'', b''])", "b'abc'"),

    ("b'|'.join([b'a', b'', b'c'])", "b'a||c'"),

    ("b'\\x00'.join([b'a', b'b'])", "b'a\\x00b'"),

    ("b','.join((b'a', b'b', b'c'))", "b'a,b,c'"),

    ("b','.join(set([b'a']))", "b'a'"),

    ("b''.join([b''])", "b''"),
    ("b'-'.join([b'', b''])", "b'-'"),

    # replace
    ("b'abc'.replace(b'a', b'x')", "b'xbc'"),
    ("b'abcabc'.replace(b'a', b'x')", "b'xbcxbc'"),
    ("b'aaaa'.replace(b'aa', b'b')", "b'bb'"),
    ("b'aaaa'.replace(b'aa', b'b', 1)", "b'baa'"),
    ("b'aaaa'.replace(b'aa', b'b', 2)", "b'bb'"),
    ("b'aaaa'.replace(b'aa', b'b', 0)", "b'aaaa'"),
    ("b'abc'.replace(b'z', b'x')", "b'abc'"),
    ("b''.replace(b'a', b'b')", "b''"),
    ("b'abc'.replace(b'', b'-')", "b'-a-b-c-'"),
    ("b'abc'.replace(b'', b'-', 2)", "b'-a-bc'"),
    ("b'abc'.replace(b'', b'-', 0)", "b'abc'"),
    ("b'abc'.replace(b'b', b'')", "b'ac'"),
    ("b'aaaa'.replace(b'a', b'xyz', 2)", "b'xyzxyzaa'"),
    ("b'\\x00\\x01\\x00'.replace(b'\\x00', b'X')", "b'X\\x01X'"),
    ("b'abcabc'.replace(b'abc', b'X', 1)", "b'Xabc'"),
    ("b'abcabc'.replace(b'abc', b'X', 2)", "b'XX'"),
    ("b'abcabc'.replace(b'abc', b'X', 10)", "b'XX'"),

    # isascii
    ("b''.isascii()", "True"),

    ("b'abc'.isascii()", "True"),
    ("b'ABC'.isascii()", "True"),
    ("b'123'.isascii()", "True"),

    ("b'hello world'.isascii()", "True"),
    ("b'\\n\\t\\r'.isascii()", "True"),

    ("b'\\x00'.isascii()", "True"),
    ("b'\\x7f'.isascii()", "True"),

    ("b'\\x80'.isascii()", "False"),
    ("b'\\xff'.isascii()", "False"),

    ("b'abc\\x80'.isascii()", "False"),
    ("b'\\x80abc'.isascii()", "False"),

    ("b'\\x7f\\x7e\\x7d'.isascii()", "True"),
    ("b'\\x7f\\x80'.isascii()", "False"),

    # isalpha
    ("b''.isalpha()", "False"),

    ("b'a'.isalpha()", "True"),
    ("b'abc'.isalpha()", "True"),
    ("b'ABC'.isalpha()", "True"),
    ("b'aBcDeF'.isalpha()", "True"),

    ("b'abc123'.isalpha()", "False"),
    ("b'123abc'.isalpha()", "False"),

    ("b'123'.isalpha()", "False"),

    ("b'abc!'.isalpha()", "False"),
    ("b'!abc'.isalpha()", "False"),

    ("b'abc def'.isalpha()", "False"),

    ("b'\\n'.isalpha()", "False"),
    ("b'\\t'.isalpha()", "False"),

    ("b'\\x00'.isalpha()", "False"),
    ("b'\\x7f'.isalpha()", "False"),

    ("b'\\xff'.isalpha()", "False"),
    ("b'abc\\xff'.isalpha()", "False"),

    ("b'Z'.isalpha()", "True"),
    ("b'z'.isalpha()", "True"),

    # isdigit
    ("b''.isdigit()", "False"),

    ("b'0'.isdigit()", "True"),
    ("b'1'.isdigit()", "True"),
    ("b'9'.isdigit()", "True"),

    ("b'123'.isdigit()", "True"),
    ("b'000'.isdigit()", "True"),

    ("b'1234567890'.isdigit()", "True"),

    ("b'12a'.isdigit()", "False"),
    ("b'a12'.isdigit()", "False"),
    ("b'1a2'.isdigit()", "False"),

    ("b' '.isdigit()", "False"),
    ("b'1 2'.isdigit()", "False"),

    ("b'\\n'.isdigit()", "False"),
    ("b'\\t'.isdigit()", "False"),

    ("b'abc'.isdigit()", "False"),

    ("b'\\x00'.isdigit()", "False"),
    ("b'\\x7f'.isdigit()", "False"),
    ("b'\\xff'.isdigit()", "False"),

    ("b'123\\xff'.isdigit()", "False"),

    # isalnum
    ("b''.isalnum()", "False"),

    ("b'a'.isalnum()", "True"),
    ("b'Z'.isalnum()", "True"),

    ("b'0'.isalnum()", "True"),
    ("b'9'.isalnum()", "True"),

    ("b'abc'.isalnum()", "True"),
    ("b'ABC'.isalnum()", "True"),

    ("b'123'.isalnum()", "True"),

    ("b'abc123'.isalnum()", "True"),
    ("b'123abc'.isalnum()", "True"),
    ("b'a1b2c3'.isalnum()", "True"),

    ("b'abc!'.isalnum()", "False"),
    ("b'!abc'.isalnum()", "False"),

    ("b'abc 123'.isalnum()", "False"),
    ("b'abc\\t123'.isalnum()", "False"),
    ("b'abc\\n123'.isalnum()", "False"),

    ("b'_'.isalnum()", "False"),
    ("b'abc_def'.isalnum()", "False"),

    ("b'\\x00'.isalnum()", "False"),
    ("b'\\x7f'.isalnum()", "False"),
    ("b'\\xff'.isalnum()", "False"),

    ("b'abc\\xff'.isalnum()", "False"),
    ("b'123\\xff'.isalnum()", "False"),

    # isspace
    ("b''.isspace()", "False"),

    ("b' '.isspace()", "True"),

    ("b'\\t'.isspace()", "True"),
    ("b'\\n'.isspace()", "True"),
    ("b'\\r'.isspace()", "True"),
    ("b'\\v'.isspace()", "True"),
    ("b'\\f'.isspace()", "True"),

    ("b'   '.isspace()", "True"),
    ("b'\\t\\n\\r'.isspace()", "True"),
    ("b' \\t\\n\\r\\v\\f '.isspace()", "True"),

    ("b'a'.isspace()", "False"),
    ("b'1'.isspace()", "False"),

    ("b' a '.isspace()", "False"),
    ("b'\\tabc\\t'.isspace()", "False"),

    ("b'\\x00'.isspace()", "False"),
    ("b'\\x7f'.isspace()", "False"),
    ("b'\\xff'.isspace()", "False"),

    ("b' \\xff '.isspace()", "False"),

    # islower
    ("b'abc'.islower()", "True"),
    ("b'hello'.islower()", "True"),

    ("b'abc123'.islower()", "True"),
    ("b'abc!@#'.islower()", "True"),
    ("b'abc xyz'.islower()", "True"),

    ("b'ABC'.islower()", "False"),
    ("b'Abc'.islower()", "False"),
    ("b'aBc'.islower()", "False"),

    ("b'123'.islower()", "False"),
    ("b'!@#'.islower()", "False"),
    ("b'' .islower()", "False"),

    ("b'abc123XYZ'.islower()", "False"),

    ("b'\\xff'.islower()", "False"),
    ("b'abc\\xff'.islower()", "True"),
    ("b'ABC\\xff'.islower()", "False"),

    # isupper
    ("b'ABC'.isupper()", "True"),
    ("b'HELLO'.isupper()", "True"),

    ("b'ABC123'.isupper()", "True"),
    ("b'ABC!@#'.isupper()", "True"),
    ("b'ABC XYZ'.isupper()", "True"),

    ("b'abc'.isupper()", "False"),
    ("b'Abc'.isupper()", "False"),
    ("b'ABc'.isupper()", "False"),

    ("b'123'.isupper()", "False"),
    ("b'!@#'.isupper()", "False"),
    ("b''.isupper()", "False"),

    ("b'ABC123xyz'.isupper()", "False"),

    ("b'\\xff'.isupper()", "False"),
    ("b'ABC\\xff'.isupper()", "True"),
    ("b'abc\\xff'.isupper()", "False"),

    # lower
    ("b'ABC'.lower()", "b'abc'"),
    ("b'HELLO'.lower()", "b'hello'"),

    ("b'AbC'.lower()", "b'abc'"),
    ("b'AbC123'.lower()", "b'abc123'"),

    ("b'abc'.lower()", "b'abc'"),
    ("b'abc123'.lower()", "b'abc123'"),

    ("b''.lower()", "b''"),

    ("b'123'.lower()", "b'123'"),
    ("b'!@#'.lower()", "b'!@#'"),

    ("b'ABC xyz'.lower()", "b'abc xyz'"),

    ("b'\\xffABC'.lower()", "b'\\xffabc'"),
    ("b'\\xff\\xfeABC'.lower()", "b'\\xff\\xfeabc'"),

    ("b'A'.lower()", "b'a'"),
    ("b'Z'.lower()", "b'z'"),

    # upper
    ("b'abc'.upper()", "b'ABC'"),
    ("b'hello'.upper()", "b'HELLO'"),

    ("b'AbC'.upper()", "b'ABC'"),
    ("b'AbC123'.upper()", "b'ABC123'"),

    ("b'ABC'.upper()", "b'ABC'"),
    ("b'ABC123'.upper()", "b'ABC123'"),

    ("b''.upper()", "b''"),

    ("b'123'.upper()", "b'123'"),
    ("b'!@#'.upper()", "b'!@#'"),

    ("b'abc XYZ'.upper()", "b'ABC XYZ'"),

    ("b'\\xffabc'.upper()", "b'\\xffABC'"),
    ("b'\\xff\\xfeabc'.upper()", "b'\\xff\\xfeABC'"),

    ("b'a'.upper()", "b'A'"),
    ("b'z'.upper()", "b'Z'"),

    # swapcase
    ("b'abc'.swapcase()", "b'ABC'"),
    ("b'ABC'.swapcase()", "b'abc'"),

    ("b'AbC'.swapcase()", "b'aBc'"),
    ("b'Hello World'.swapcase()", "b'hELLO wORLD'"),

    ("b'abc123'.swapcase()", "b'ABC123'"),
    ("b'ABC123'.swapcase()", "b'abc123'"),

    ("b'123'.swapcase()", "b'123'"),
    ("b'!@#'.swapcase()", "b'!@#'"),

    ("b''.swapcase()", "b''"),

    ("b'a'.swapcase()", "b'A'"),
    ("b'Z'.swapcase()", "b'z'"),

    ("b'Python3.14'.swapcase()", "b'pYTHON3.14'"),

    ("b'\\xffabcABC'.swapcase()", "b'\\xffABCabc'"),
    ("b'\\xff\\xfeABC'.swapcase()", "b'\\xff\\xfeabc'"),

    ("b'AbCdEfGhIjK'.swapcase()", "b'aBcDeFgHiJk'"),

    # capitalize
    ("b''.capitalize()", "b''"),

    ("b'hello'.capitalize()", "b'Hello'"),
    ("b'Hello'.capitalize()", "b'Hello'"),
    ("b'HELLO'.capitalize()", "b'Hello'"),

    ("b'hElLo'.capitalize()", "b'Hello'"),

    ("b'abc123'.capitalize()", "b'Abc123'"),
    ("b'ABC123'.capitalize()", "b'Abc123'"),

    ("b'123abc'.capitalize()", "b'123abc'"),
    ("b'123ABC'.capitalize()", "b'123abc'"),

    ("b'a'.capitalize()", "b'A'"),
    ("b'A'.capitalize()", "b'A'"),

    ("b'python rocks'.capitalize()", "b'Python rocks'"),

    ("b'PYTHON ROCKS'.capitalize()", "b'Python rocks'"),

    ("b'\\xffABC'.capitalize()", "b'\\xffabc'"),
    ("b'\\xffabc'.capitalize()", "b'\\xffabc'"),

    # title
    ("b''.title()", "b''"),

    ("b'hello'.title()", "b'Hello'"),
    ("b'HELLO'.title()", "b'Hello'"),
    ("b'HeLlO'.title()", "b'Hello'"),

    ("b'hello world'.title()", "b'Hello World'"),
    ("b'HELLO WORLD'.title()", "b'Hello World'"),

    ("b'hello-world'.title()", "b'Hello-World'"),
    ("b'hello_world'.title()", "b'Hello_World'"),

    ("b'abc123def'.title()", "b'Abc123Def'"),
    ("b'123abc'.title()", "b'123Abc'"),

    ("b'a b c'.title()", "b'A B C'"),

    ("b'python\\trocks'.title()", "b'Python\\tRocks'"),
    ("b'python\\nrocks'.title()", "b'Python\\nRocks'"),

    ("b'foo.bar'.title()", "b'Foo.Bar'"),
    ("b'foo,bar'.title()", "b'Foo,Bar'"),

    ("b'fooBARbaz'.title()", "b'Foobarbaz'"),

    # istitle
    ("b'Hello'.istitle()", "True"),

    ("b'Hello World'.istitle()", "True"),

    ("b'Abc123Def'.istitle()", "True"),
    ("b'Foo-Bar'.istitle()", "True"),
    ("b'Foo_Bar'.istitle()", "True"),

    ("b'hello'.istitle()", "False"),
    ("b'HELLO'.istitle()", "False"),

    ("b'Hello world'.istitle()", "False"),
    ("b'hello World'.istitle()", "False"),

    ("b'Foo BAR'.istitle()", "False"),
    ("b'FOO Bar'.istitle()", "False"),

    ("b''.istitle()", "False"),
    ("b'123'.istitle()", "False"),
    ("b'!@#'.istitle()", "False"),

    ("b'123Abc'.istitle()", "True"),
    ("b'123abc'.istitle()", "False"),

    ("b'A'.istitle()", "True"),
    ("b'a'.istitle()", "False"),

    ("b'Hello\\tWorld'.istitle()", "True"),
    ("b'Hello\\nWorld'.istitle()", "True"),
    ("b'Hello,World'.istitle()", "True"),

    # lstrip
    ("b'abc'.lstrip()", "b'abc'"),
    ("b''.lstrip()", "b''"),

    ("b'   abc'.lstrip()", "b'abc'"),
    ("b'\\t\\n abc'.lstrip()", "b'abc'"),
    ("b'\\r\\v\\fabc'.lstrip()", "b'abc'"),

    ("b'abc   '.lstrip()", "b'abc   '"),

    ("b'xxxabc'.lstrip(b'x')", "b'abc'"),
    ("b'abc'.lstrip(b'x')", "b'abc'"),

    ("b'aaabbbccc'.lstrip(b'ab')", "b'ccc'"),

    ("b'www.python.org'.lstrip(b'cmowz.')", "b'python.org'"),

    ("b'123abc'.lstrip(b'123')", "b'abc'"),

    ("b'aaaa'.lstrip(b'a')", "b''"),

    ("b'abcabc'.lstrip(b'ab')", "b'cabc'"),

    ("b'\\xff\\xffabc'.lstrip(b'\\xff')", "b'abc'"),

    ("b'abc'.lstrip(b'abc')", "b''"),

    # rstrip
    ("b'abc'.rstrip()", "b'abc'"),
    ("b''.rstrip()", "b''"),

    ("b'abc   '.rstrip()", "b'abc'"),
    ("b'abc\\t\\n '.rstrip()", "b'abc'"),
    ("b'abc\\r\\v\\f'.rstrip()", "b'abc'"),

    ("b'   abc'.rstrip()", "b'   abc'"),

    ("b'abcxxx'.rstrip(b'x')", "b'abc'"),
    ("b'abc'.rstrip(b'x')", "b'abc'"),

    ("b'aaabbbccc'.rstrip(b'bc')", "b'aaa'"),

    ("b'python.orgwww'.rstrip(b'cmowz.')", "b'python.org'"),

    ("b'abc123'.rstrip(b'123')", "b'abc'"),

    ("b'aaaa'.rstrip(b'a')", "b''"),

    ("b'abcabc'.rstrip(b'bc')", "b'abca'"),

    ("b'abc\\xff\\xff'.rstrip(b'\\xff')", "b'abc'"),

    ("b'abc'.rstrip(b'abc')", "b''"),

    # strip
    ("b''.strip()", "b''"),

    ("b'abc'.strip()", "b'abc'"),

    ("b'   abc'.strip()", "b'abc'"),
    ("b'abc   '.strip()", "b'abc'"),
    ("b'   abc   '.strip()", "b'abc'"),

    ("b'\\t\\n abc \\r\\v\\f'.strip()", "b'abc'"),

    ("b'xxxabcxxx'.strip(b'x')", "b'abc'"),

    ("b'aaabbbccc'.strip(b'ac')", "b'bbb'"),

    ("b'www.python.org'.strip(b'cmowz.')", "b'python.org'"),

    ("b'123abc123'.strip(b'123')", "b'abc'"),

    ("b'aaaa'.strip(b'a')", "b''"),

    ("b'abcabc'.strip(b'abc')", "b''"),

    ("b'abcxxx'.strip(b'x')", "b'abc'"),

    ("b'xxxabc'.strip(b'x')", "b'abc'"),

    ("b'\\xffabc\\xff'.strip(b'\\xff')", "b'abc'"),

    ("b'\\xff\\xffabc\\xff\\xff'.strip(b'\\xff')", "b'abc'"),

    ("b'abc'.strip(b'xyz')", "b'abc'"),

    # center
    ("b'abc'.center(3)", "b'abc'"),
    ("b'abc'.center(2)", "b'abc'"),
    ("b'abc'.center(1)", "b'abc'"),

    ("b'abc'.center(4)", "b'abc '"),
    ("b'abc'.center(5)", "b' abc '"),
    ("b'abc'.center(6)", "b' abc  '"),
    ("b'abc'.center(7)", "b'  abc  '"),
    ("b'abc'.center(8)", "b'  abc   '"),

    ("b''.center(3)", "b'   '"),

    ("b'abc'.center(7, b'-')", "b'--abc--'"),
    ("b'abc'.center(8, b'-')", "b'--abc---'"),

    ("b'abc'.center(9, b'*')", "b'***abc***'"),

    ("b'abc'.center(4, b'x')", "b'abcx'"),

    ("b'\\xff'.center(5)", "b'  \\xff  '"),

    # ljust
    ("b'abc'.ljust(3)", "b'abc'"),
    ("b'abc'.ljust(2)", "b'abc'"),
    ("b'abc'.ljust(1)", "b'abc'"),

    ("b'abc'.ljust(4)", "b'abc '"),
    ("b'abc'.ljust(5)", "b'abc  '"),
    ("b'abc'.ljust(7)", "b'abc    '"),

    ("b''.ljust(3)", "b'   '"),

    ("b'abc'.ljust(4, b'-')", "b'abc-'"),
    ("b'abc'.ljust(5, b'-')", "b'abc--'"),
    ("b'abc'.ljust(7, b'-')", "b'abc----'"),

    ("b'abc'.ljust(10, b'*')", "b'abc*******'"),

    ("b'\\xff'.ljust(3)", "b'\\xff  '"),
    ("b'\\xff'.ljust(5, b'-')", "b'\\xff----'"),

    # rjust
    ("b'abc'.rjust(3)", "b'abc'"),
    ("b'abc'.rjust(2)", "b'abc'"),
    ("b'abc'.rjust(1)", "b'abc'"),

    ("b'abc'.rjust(4)", "b' abc'"),
    ("b'abc'.rjust(5)", "b'  abc'"),
    ("b'abc'.rjust(7)", "b'    abc'"),

    ("b''.rjust(3)", "b'   '"),

    ("b'abc'.rjust(4, b'-')", "b'-abc'"),
    ("b'abc'.rjust(5, b'-')", "b'--abc'"),
    ("b'abc'.rjust(7, b'-')", "b'----abc'"),

    ("b'abc'.rjust(10, b'*')", "b'*******abc'"),

    ("b'\\xff'.rjust(3)", "b'  \\xff'"),
    ("b'\\xff'.rjust(5, b'-')", "b'----\\xff'"),

    # zfill
    ("b''.zfill(0)", "b''"),
    ("b''.zfill(3)", "b'000'"),

    ("b'42'.zfill(2)", "b'42'"),
    ("b'42'.zfill(1)", "b'42'"),
    ("b'42'.zfill(0)", "b'42'"),

    ("b'42'.zfill(3)", "b'042'"),
    ("b'42'.zfill(4)", "b'0042'"),
    ("b'42'.zfill(5)", "b'00042'"),

    ("b'abc'.zfill(5)", "b'00abc'"),

    ("b'+42'.zfill(5)", "b'+0042'"),
    ("b'+42'.zfill(6)", "b'+00042'"),

    ("b'-42'.zfill(5)", "b'-0042'"),
    ("b'-42'.zfill(6)", "b'-00042'"),

    ("b'+'.zfill(3)", "b'+00'"),
    ("b'-'.zfill(3)", "b'-00'"),

    ("b'0'.zfill(5)", "b'00000'"),
    ("b'00042'.zfill(7)", "b'0000042'"),

    ("b'\\xff'.zfill(3)", "b'00\\xff'"),

    # removeprefix
    ("b'foobar'.removeprefix(b'foo')", "b'bar'"),
    ("b'foobar'.removeprefix(b'foobar')", "b''"),
    ("b'foobar'.removeprefix(b'')", "b'foobar'"),

    ("b'foobar'.removeprefix(b'bar')", "b'foobar'"),
    ("b'foobar'.removeprefix(b'foox')", "b'foobar'"),

    ("b''.removeprefix(b'foo')", "b''"),
    ("b''.removeprefix(b'')", "b''"),

    ("b'abcabc'.removeprefix(b'abc')", "b'abc'"),
    ("b'abcabc'.removeprefix(b'ab')", "b'cabc'"),

    ("b'aaaa'.removeprefix(b'a')", "b'aaa'"),
    ("b'aaaa'.removeprefix(b'aa')", "b'aa'"),

    ("b'\\xff\\x00\\x01'.removeprefix(b'\\xff')", "b'\\x00\\x01'"),

    ("b'\\xff\\x00\\x01'.removeprefix(b'\\x00')", "b'\\xff\\x00\\x01'"),

    # removesuffix
    ("b'foobar'.removesuffix(b'bar')", "b'foo'"),
    ("b'foobar'.removesuffix(b'foobar')", "b''"),

    ("b'foobar'.removesuffix(b'baz')", "b'foobar'"),
    ("b'foobar'.removesuffix(b'')", "b'foobar'"),

    ("b'abcabc'.removesuffix(b'abc')", "b'abc'"),
    ("b'abcabc'.removesuffix(b'bc')", "b'abca'"),

    ("b''.removesuffix(b'abc')", "b''"),
    ("b''.removesuffix(b'')", "b''"),

    ("b'abc'.removesuffix(b'abcabc')", "b'abc'"),

    ("b'\\x00\\x01\\x02'.removesuffix(b'\\x02')",
     "b'\\x00\\x01'"),

    ("b'hello'.removesuffix(b'o')", "b'hell'"),
    ("b'hello'.removesuffix(b'hello')", "b''"),

    # partition
    ("b'abc=123'.partition(b'=')", "(b'abc', b'=', b'123')"),

    ("b'abc'.partition(b'=')", "(b'abc', b'', b'')"),

    ("b'=abc'.partition(b'=')", "(b'', b'=', b'abc')"),
    ("b'abc='.partition(b'=')", "(b'abc', b'=', b'')"),

    ("b'abc=123=456'.partition(b'=')", "(b'abc', b'=', b'123=456')"),

    ("b'aaaa'.partition(b'aa')", "(b'', b'aa', b'aa')"),
    ("b'abc'.partition(b'abc')", "(b'', b'abc', b'')"),
    ("b'abc'.partition(b'x')", "(b'abc', b'', b'')"),

    ("b''.partition(b'x')", "(b'', b'', b'')"),

    ("b'\\x00\\x01\\x02'.partition(b'\\x01')", "(b'\\x00', b'\\x01', b'\\x02')"),

    # rpartition
    ("b'abc=123'.rpartition(b'=')", "(b'abc', b'=', b'123')"),
    ("b'abc=123=456'.rpartition(b'=')", "(b'abc=123', b'=', b'456')"),

    ("b'abc'.rpartition(b'=')", "(b'', b'', b'abc')"),
    ("b'=abc'.rpartition(b'=')", "(b'', b'=', b'abc')"),

    ("b'abc='.rpartition(b'=')", "(b'abc', b'=', b'')"),
    ("b'aaaa'.rpartition(b'aa')", "(b'aa', b'aa', b'')"),
    ("b'abc'.rpartition(b'abc')", "(b'', b'abc', b'')"),

    ("b''.rpartition(b'x')", "(b'', b'', b'')"),
    ("b'abc'.rpartition(b'x')", "(b'', b'', b'abc')"),

    ("b'\\x00\\x01\\x02\\x01'.rpartition(b'\\x01')", "(b'\\x00\\x01\\x02', b'\\x01', b'')"),

    # splitlines
    ("b''.splitlines()", "[]"),
    ("b'abc'.splitlines()", "[b'abc']"),

    ("b'abc\\ndef'.splitlines()", "[b'abc', b'def']"),
    ("b'abc\\ndef'.splitlines(True)", "[b'abc\\n', b'def']"),

    ("b'abc\\rdef'.splitlines()", "[b'abc', b'def']"),
    ("b'abc\\rdef'.splitlines(True)", "[b'abc\\r', b'def']"),

    ("b'abc\\r\\ndef'.splitlines()", "[b'abc', b'def']"),
    ("b'abc\\r\\ndef'.splitlines(True)", "[b'abc\\r\\n', b'def']"),

    ("b'abc\\vdef'.splitlines()", "[b'abc\\x0bdef']"),
    ("b'abc\\vdef'.splitlines(True)", "[b'abc\\x0bdef']"),

    ("b'abc\\fdef'.splitlines()", "[b'abc\\x0cdef']"),
    ("b'abc\\fdef'.splitlines(True)", "[b'abc\\x0cdef']"),

    ("b'\\n'.splitlines()", "[b'']"),
    ("b'\\n'.splitlines(True)", "[b'\\n']"),
    ("b'\\r\\n'.splitlines()", "[b'']"),
    ("b'\\r\\n'.splitlines(True)", "[b'\\r\\n']"),

    ("b'a\\nb\\nc'.splitlines()", "[b'a', b'b', b'c']"),
    ("b'a\\nb\\nc'.splitlines(True)", "[b'a\\n', b'b\\n', b'c']"),

    # expandtabs
    ("b''.expandtabs()", "b''"),
    ("b'\\t'.expandtabs()", "b'        '"),
    ("b'a\\t'.expandtabs()", "b'a       '"),
    ("b'ab\\t'.expandtabs()", "b'ab      '"),
    ("b'abc\\t'.expandtabs()", "b'abc     '"),
    ("b'abcd\\t'.expandtabs()", "b'abcd    '"),
    ("b'abcdefgh\\t'.expandtabs()", "b'abcdefgh        '"),

    ("b'a\\tb'.expandtabs()", "b'a       b'"),
    ("b'a\\tb'.expandtabs(4)", "b'a   b'"),
    ("b'ab\\tb'.expandtabs(4)", "b'ab  b'"),
    ("b'abc\\tb'.expandtabs(4)", "b'abc b'"),
    ("b'abcd\\tb'.expandtabs(4)", "b'abcd    b'"),

    ("b'a\\t\\nb\\t'.expandtabs()", "b'a       \\nb       '"),
    ("b'1\\t2\\t3'.expandtabs(4)", "b'1   2   3'"),

    ("b'1234\\t5'.expandtabs(4)", "b'1234    5'"),

    # hex
    ("b''.hex()", "''"),

    ("b'abc'.hex()", "'616263'"),
    ("b'ABC'.hex()", "'414243'"),
    ("b'123'.hex()", "'313233'"),

    ("b'\\x00'.hex()", "'00'"),
    ("b'\\x00\\x01\\x02'.hex()", "'000102'"),

    ("b'\\xff'.hex()", "'ff'"),
    ("b'\\xff\\xfe\\xfd'.hex()", "'fffefd'"),

    ("b'hello'.hex()", "'68656c6c6f'"),

    ("b'\\x10\\x20\\x30'.hex()", "'102030'"),

    # fromhex
    ("bytes.fromhex('')", "b''"),
    ("bytes.fromhex('61')", "b'a'"),
    ("bytes.fromhex('6162')", "b'ab'"),
    ("bytes.fromhex('616263')", "b'abc'"),
    ("bytes.fromhex('41')", "b'A'"),
    ("bytes.fromhex('414243')", "b'ABC'"),
    ("bytes.fromhex('68 65 6c 6c 6f')", "b'hello'"),

    ("bytes.fromhex('00')", "b'\\x00'"),
    ("bytes.fromhex('000102')", "b'\\x00\\x01\\x02'"),

    ("bytes.fromhex('ff')", "b'\\xff'"),
    ("bytes.fromhex('FF')", "b'\\xff'"),
    ("bytes.fromhex('ff fe fd')", "b'\\xff\\xfe\\xfd'"),

    # bytes constructor
    ("bytes()", "b''"),
    ("bytes(b'abc')", "b'abc'"),

    ("bytes('abc', 'utf-8')", "b'abc'"),
    ("bytes('hello', 'utf8')", "b'hello'"),

    ("bytes('abc', encoding='utf-8')", "b'abc'"),
    ("bytes('hello', encoding='utf8')", "b'hello'"),

    ("bytes('Привет', encoding='utf-8')",
     "b'\\xd0\\x9f\\xd1\\x80\\xd0\\xb8\\xd0\\xb2\\xd0\\xb5\\xd1\\x82'"),

    # decode
    ("b'abc'.decode()", "'abc'"),
    ("b'abc'.decode('utf-8')", "'abc'"),

    ("b'hello'.decode('ascii')", "'hello'"),

    ("b'\\x41\\x42\\x43'.decode('ascii')", "'ABC'"),

    ("b'\\xff'.decode('latin-1')", "'ÿ'"),

    ("b''.decode()", "''"),

    ("b'abc'.decode('utf8')", "'abc'"),
    ("b'abc'.decode('latin1')", "'abc'"),

    # maketrans создаёт таблицу длиной 256
    ("len(bytes.maketrans(b'abc', b'xyz'))", "256"),

    # замена отдельных символов
    ("bytes.maketrans(b'a', b'x')[97]", "120"),
    ("bytes.maketrans(b'b', b'y')[98]", "121"),
    ("bytes.maketrans(b'c', b'z')[99]", "122"),

    # остальные байты не меняются
    ("bytes.maketrans(b'a', b'x')[100]", "100"),
    ("bytes.maketrans(b'a', b'x')[0]", "0"),
    ("bytes.maketrans(b'a', b'x')[255]", "255"),

    # несколько замен
    ("bytes.maketrans(b'abc', b'xyz')[97]", "120"),
    ("bytes.maketrans(b'abc', b'xyz')[98]", "121"),
    ("bytes.maketrans(b'abc', b'xyz')[99]", "122"),

    # пустые аргументы
    ("len(bytes.maketrans(b'', b''))", "256"),

    # полное отображение одного байта
    ("bytes.maketrans(b'\\x00', b'\\xff')[0]", "255"),
    ("bytes.maketrans(b'\\xff', b'\\x00')[255]", "0"),

    # translate
    ("b'abc'.translate(bytes.maketrans(b'', b''))", "b'abc'"),

    ("b'abc'.translate(bytes.maketrans(b'a', b'x'))", "b'xbc'"),
    ("b'abc'.translate(bytes.maketrans(b'abc', b'xyz'))", "b'xyz'"),

    ("b'aaaa'.translate(bytes.maketrans(b'a', b'b'))", "b'bbbb'"),

    ("b''.translate(bytes.maketrans(b'a', b'b'))", "b''"),

    ("b'abc'.translate(bytes.maketrans(b'', b''), b'b')", "b'ac'"),
    ("b'abcabc'.translate(bytes.maketrans(b'', b''), b'bc')", "b'aa'"),

    ("b'abc'.translate(bytes.maketrans(b'a', b'x'), b'c')", "b'xb'"),

    ("b'\\x00\\x01\\x02'.translate(bytes.maketrans(b'\\x01', b'\\xff'))", "b'\\x00\\xff\\x02'"),

    ("b'hello'.translate(bytes.maketrans(b'helo', b'HELO'))", "b'HELLO'"),

    # форматирование %, __mod__

    # %s
    ("b'%s' % b'abc'", "b'abc'"),
    ("b'hello %s' % b'world'", "b'hello world'"),

    # %b
    ("b'%b' % b'abc'", "b'abc'"),

    # %d
    ("b'%d' % 123", "b'123'"),
    ("b'%d' % -5", "b'-5'"),

    # %i
    ("b'%i' % 42", "b'42'"),

    # %x
    ("b'%x' % 255", "b'ff'"),

    # %X
    ("b'%X' % 255", "b'FF'"),

    # %o
    ("b'%o' % 8", "b'10'"),

    # %c
    ("b'%c' % 65", "b'A'"),

    # tuple
    ("b'%s %d' % (b'abc', 42)", "b'abc 42'"),

    # escaped percent
    ("b'100%%' % ()", "b'100%'"),

    # выравнивание
    ("b'%5d' % 42", "b'   42'"),
    ("b'%3d' % 42", "b' 42'"),
    ("b'%2d' % 42", "b'42'"),

    ("b'%5s' % b'ab'", "b'   ab'"),
    ("b'%3s' % b'ab'", "b' ab'"),
    ("b'%2s' % b'ab'", "b'ab'"),

    ("b'%-5s' % b'ab'", "b'ab   '"),
    ("b'%-5d' % 42", "b'42   '"),

    ("b'%5d %5d' % (1, 2)", "b'    1     2'"),

    ("b'%5x' % 255", "b'   ff'"),
    ("b'%5X' % 255", "b'   FF'"),

    # zero padding
    ("b'%05d' % 42", "b'00042'"),
    ("b'%04d' % 7", "b'0007'"),
    ("b'%03d' % 123", "b'123'"),

    ("b'%05d' % -42", "b'-0042'"),

    ("b'%05x' % 255", "b'000ff'"),
    ("b'%05X' % 255", "b'000FF'"),

    ("b'%05o' % 8", "b'00010'"),

    # plus sign
    ("b'%+d' % 42", "b'+42'"),
    ("b'%+d' % -42", "b'-42'"),

    ("b'%+i' % 7", "b'+7'"),
    ("b'%+i' % -7", "b'-7'"),

    ("b'%+5d' % 42", "b'  +42'"),
    ("b'%+05d' % 42", "b'+0042'"),

    ("b'%+05d' % -42", "b'-0042'"),

    ("b'%+3d' % 123", "b'+123'"),

    # alternate form
    ("b'%#x' % 255", "b'0xff'"),
    ("b'%#X' % 255", "b'0XFF'"),
    ("b'%#o' % 8", "b'0o10'"),

    ("b'%#5x' % 255", "b' 0xff'"),
    ("b'%#5o' % 8", "b' 0o10'"),

    ("b'%#08x' % 255", "b'0x0000ff'"),
    ("b'%#08X' % 255", "b'0X0000FF'"),
    ("b'%#08o' % 8", "b'0o000010'"),

    ("b'%#+x' % 255", "b'+0xff'"),
    ("b'%#+o' % 8", "b'+0o10'"),

    # space sign
    ("b'% d' % 42", "b' 42'"),
    ("b'% d' % -42", "b'-42'"),

    ("b'% 5d' % 42", "b'   42'"),
    ("b'% 5d' % -42", "b'  -42'"),

    ("b'% 05d' % 42", "b' 0042'"),
    ("b'% 05d' % -42", "b'-0042'"),

    ("b'% x' % 255", "b' ff'"),
    ("b'% X' % 255", "b' FF'"),
    ("b'% o' % 8", "b' 10'"),

    ("b'% #x' % 255", "b' 0xff'"),
    ("b'% #o' % 8", "b' 0o10'"),

    # precision strings
    ("b'%.3s' % b'abcdef'", "b'abc'"),
    ("b'%.5b' % b'hello world'", "b'hello'"),

    # precision int
    ("b'%.5d' % 42", "b'00042'"),
    ("b'%.5i' % 42", "b'00042'"),

    ("b'%.4x' % 255", "b'00ff'"),
    ("b'%.4X' % 255", "b'00FF'"),

    ("b'%.5o' % 8", "b'00010'"),

    # width + precision
    ("b'%8.5d' % 42", "b'   00042'"),
    ("b'%10.4x' % 255", "b'      00ff'"),

    # precision + sign
    ("b'%+.5d' % 42", "b'+00042'"),
    ("b'% .5d' % 42", "b' 00042'"),

    ("b'%08.5d' % 42", "b'00000042'"),

    # %r
    ("b'%r' % b'abc'", "b\"b'abc'\""),
    ("b'%r' % 42", "b'42'"),
    ("b'%r' % 'abc'", "b\"'abc'\""),
    ("b'%.5r' % b'abcdef'", "b\"b'abc\""),
    ("b'%10r' % 42", "b'        42'"),
    ("b'%-10r' % 42", "b'42        '"),

    # %a
    ("b'%a' % 'abc'", "b\"'abc'\""),
    ("b'%a' % 42", "b'42'"),
    ("b'%a' % b'abc'", "b\"b'abc'\""),
    ("b'%.5a' % b'abcdef'", "b\"b'abc\""),
    ("b'%10a' % 42", "b'        42'"),
    ("b'%-10a' % 42", "b'42        '"),
    ("b'%a' % 'привет'", "b\"'\\\\u043f\\\\u0440\\\\u0438\\\\u0432\\\\u0435\\\\u0442'\""),

    # %f
    ("b'%f' % 3.14", "b'3.140000'"),
    ("b'%.2f' % 3.14159", "b'3.14'"),
    ("b'%8.2f' % 3.14159", "b'    3.14'"),
    ("b'%08.2f' % 3.14159", "b'00003.14'"),
    ("b'%+8.2f' % 3.14159", "b'   +3.14'"),
    ("b'% .2f' % 3.14159", "b' 3.14'"),
    ("b'%f' % -3.14", "b'-3.140000'"),
    ("b'%08.2f' % -3.14", "b'-0003.14'"),

    # %e
    ("b'%e' % 1234.5", "b'1.234500e+03'"),
    ("b'%.2e' % 1234.5", "b'1.23e+03'"),
    ("b'%+e' % 1.0", "b'+1.000000e+00'"),
    ("b'% e' % 1.0", "b' 1.000000e+00'"),
    ("b'%12.2e' % 1234.5", "b'    1.23e+03'"),
    ("b'%012.2e' % 1234.5", "b'00001.23e+03'"),
    ("b'%e' % -1234.5", "b'-1.234500e+03'"),

    # %E
    ("b'%E' % 1234.5", "b'1.234500E+03'"),
    ("b'%.2E' % 1234.5", "b'1.23E+03'"),
    ("b'%E' % 0.0", "b'0.000000E+00'"),
    ("b'%E' % 1.0", "b'1.000000E+00'"),
    ("b'%E' % -1.0", "b'-1.000000E+00'"),
    ("b'%.1E' % 1234.5", "b'1.2E+03'"),
    ("b'%.3E' % 1234.5", "b'1.234E+03'"),

    ("b'%12.2E' % 1234.5", "b'    1.23E+03'"),
    ("b'%012.2E' % 1234.5", "b'00001.23E+03'"),

    ("b'%+E' % 1.0", "b'+1.000000E+00'"),
    ("b'% E' % 1.0", "b' 1.000000E+00'"),

    ("b'%E' % 0.001", "b'1.000000E-03'"),
    ("b'%E' % 1000000.0", "b'1.000000E+06'"),

    # %g
    ("b'%g' % 1234.0", "b'1234'"),
    ("b'%g' % 1234.5", "b'1234.5'"),

    ("b'%g' % 1234567.0", "b'1.23457e+06'"),

    ("b'%g' % 0.000123", "b'0.000123'"),
    ("b'%g' % 0.0000123", "b'1.23e-05'"),

    ("b'%.3g' % 1234.5", "b'1.23e+03'"),
    ("b'%.5g' % 1234.5", "b'1234.5'"),

    ("b'%g' % 1.0", "b'1'"),
    ("b'%g' % 1.50000", "b'1.5'"),
    ("b'%+g' % 42.0", "b'+42'"),
    ("b'% g' % 42.0", "b' 42'"),

    # %G
    ("b'%G' % 1234567.0", "b'1.23457E+06'"),

    ("b'%G' % 1234.0", "b'1234'"),
    ("b'%G' % 1234.5", "b'1234.5'"),

    ("b'%G' % 1234567.0", "b'1.23457E+06'"),
    ("b'%G' % 123456789.0", "b'1.23457E+08'"),

    ("b'%G' % 0.000123", "b'0.000123'"),
    ("b'%G' % 0.0000123", "b'1.23E-05'"),
    ("b'%G' % 0.0000001", "b'1E-07'"),

    ("b'%.3G' % 1234.5", "b'1.23E+03'"),
    ("b'%.5G' % 1234.5", "b'1234.5'"),

    ("b'%.2G' % 1234567.0", "b'1.2E+06'"),
    ("b'%.4G' % 1234567.0", "b'1.235E+06'"),

    ("b'%G' % 1.0", "b'1'"),
    ("b'%G' % 1.50000", "b'1.5'"),
    ("b'%G' % 10.0000", "b'10'"),

    ("b'%+G' % 42.0", "b'+42'"),
    ("b'% G' % 42.0", "b' 42'"),

    ("b'%10G' % 42.0", "b'        42'"),
    ("b'%-10G' % 42.0", "b'42        '"),

    ("b'%12.3G' % 1234567.0", "b'    1.23E+06'"),
    ("b'%-12.3G' % 1234567.0", "b'1.23E+06    '"),

    # %g vs %G
    ("b'%g' % 1234567.0", "b'1.23457e+06'"),
    ("b'%G' % 1234567.0", "b'1.23457E+06'"),

    ("b'%.3g' % 0.0000123", "b'1.23e-05'"),
    ("b'%.3G' % 0.0000123", "b'1.23E-05'"),

    # %u
    ("b'%u' % 42", "b'42'"),
    ("b'%u' % -42", "b'-42'"),

    ("b'%5u' % 42", "b'   42'"),
    ("b'%-5u' % 42", "b'42   '"),

    ("b'%05u' % 42", "b'00042'"),

    ("b'%+u' % 42", "b'+42'"),
    ("b'% u' % 42", "b' 42'"),

    ("b'%.5u' % 42", "b'00042'"),
    ("b'%8.5u' % 42", "b'   00042'"),

    ("b'%u %u' % (1, 2)", "b'1 2'"),

    # %F
    ("b'%F' % 3.14", "b'3.140000'"),
    ("b'%.2F' % 3.14159", "b'3.14'"),


    ("b'%+F' % 42.0", "b'+42.000000'"),
    ("b'% F' % 42.0", "b' 42.000000'"),

    ("b'%12F' % 3.14", "b'    3.140000'"),
    ("b'%-12F' % 3.14", "b'3.140000    '"),

    ("b'%012.2F' % 3.14", "b'000000003.14'"),

    ("b'%F' % -3.14", "b'-3.140000'"),
    ("b'%.3F' % -3.14", "b'-3.140'"),

    ("b'%F %f' % (3.14, 3.14)", "b'3.140000 3.140000'"),

    # *
    # width через *
    ("b'%*d' % (5, 42)", "b'   42'"),
    ("b'%*d' % (-5, 42)", "b'42   '"),

    ("b'%*s' % (5, b'ab')", "b'   ab'"),
    ("b'%*s' % (-5, b'ab')", "b'ab   '"),

    ("b'%*b' % (5, b'ab')", "b'   ab'"),
    ("b'%*b' % (-5, b'ab')", "b'ab   '"),

    # precision через *
    ("b'%.*f' % (2, 3.14159)", "b'3.14'"),
    ("b'%.*F' % (2, 3.14159)", "b'3.14'"),

    ("b'%.*s' % (3, b'abcdef')", "b'abc'"),
    ("b'%.*b' % (3, b'abcdef')", "b'abc'"),

    ("b'%.*r' % (5, b'abcdef')", "b\"b'abc\""),
    ("b'%.*a' % (5, b'abcdef')", "b\"b'abc\""),

    ("b'%.*d' % (5, 42)", "b'00042'"),
    ("b'%.*i' % (5, 42)", "b'00042'"),
    ("b'%.*u' % (5, 42)", "b'00042'"),

    ("b'%.*x' % (4, 255)", "b'00ff'"),
    ("b'%.*X' % (4, 255)", "b'00FF'"),

    ("b'%.*o' % (5, 8)", "b'00010'"),

    # width + precision через *
    ("b'%*.*f' % (8, 2, 3.14159)", "b'    3.14'"),
    ("b'%*.*F' % (8, 2, 3.14159)", "b'    3.14'"),

    ("b'%*.*d' % (8, 5, 42)", "b'   00042'"),
    ("b'%*.*i' % (8, 5, 42)", "b'   00042'"),

    ("b'%*.*x' % (8, 4, 255)", "b'    00ff'"),
    ("b'%*.*X' % (8, 4, 255)", "b'    00FF'"),

    ("b'%*.*o' % (8, 5, 8)", "b'   00010'"),

    ("b'%.*f' % (-1, 3.14)", "b'3'"),

    # отрицательный width
    ("b'%*.*d' % (-8, 5, 42)", "b'00042   '"),
    ("b'%*.*f' % (-8, 2, 3.14159)", "b'3.14    '"),

    # mapping format
    ("b'%(name)s' % {b'name': b'Alex'}", "b'Alex'"),

    ("b'%(age)d' % {b'age': 42}", "b'42'"),

    ("b'%(x)d %(y)d' % {b'x': 1, b'y': 2}", "b'1 2'"),

    ("b'%(v)f' % {b'v': 3.14}", "b'3.140000'"),

    ("b'%(v).2f' % {b'v': 3.14}", "b'3.14'"),

    ("b'%(name)5s' % {b'name': b'ab'}", "b'   ab'"),

    ("b'%(name)-5s' % {b'name': b'ab'}", "b'ab   '"),

    # precision = 0
    ("b'%.0d' % 42", "b'42'"),
    ("b'%.0i' % 42", "b'42'"),
    ("b'%.0u' % 42", "b'42'"),

    ("b'%.0f' % 3.14", "b'3'"),
    ("b'%.0F' % 3.14", "b'3'"),

    ("b'%.0e' % 3.14", "b'3e+00'"),
    ("b'%.0E' % 3.14", "b'3E+00'"),

    ("b'%.0g' % 3.14", "b'3'"),
    ("b'%.0G' % 3.14", "b'3'"),

    ("b'%.0s' % b'abcdef'", "b''"),
    ("b'%.0b' % b'abcdef'", "b''"),

    ("b'%.0r' % b'abcdef'", "b''"),
    ("b'%.0a' % b'abcdef'", "b''"),

    # огромные числа
    ("b'%d' % 1234567890123456789012345678901234567890", "b'1234567890123456789012345678901234567890'"),
    ("b'%x' % 1234567890123456789012345678901234567890", "b'3a0c92075c0dbf3b8acbc5f96ce3f0ad2'"),
    ("b'%o' % 1234567890123456789012345678901234567890", "b'16406222016560155763561262742771331617605322'"),
    ("b'%x' % -1234567890123456789012345678901234567890", "b'-3a0c92075c0dbf3b8acbc5f96ce3f0ad2'"),

    ("b'%100d' % 42", "b'                                                                                                  42'"),
    ("b'%0100d' % 42", "b'0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000042'"),
    ("b'%-100d' % 42", "b'42                                                                                                  '"),

    ("b'%20.5d' % 42", "b'               00042'"),
    ("b'%-20.5d' % 42", "b'00042               '"),
    ("b'%020.5d' % 42", "b'00000000000000000042'"),

    ("b'%.*f' % (-1, 3.14)", "b'3'"),
    ("b'%.*e' % (-1, 3.14)", "b'3e+00'"),
    ("b'%.*E' % (-1, 3.14)", "b'3E+00'"),
    ("b'%.*g' % (-1, 3.14)", "b'3'"),

    # пустой bytes
    ("bytes()", "b''"),

    # bytes -> bytes
    ("bytes(b'abc')", "b'abc'"),

    # строка + encoding
    ("bytes('abc', 'utf-8')", "b'abc'"),

    ("b'abc'.__bytes__()", "b'abc'"),
    ("bytes(b'abc').__bytes__()", "b'abc'"),

    # пустой bytes
    ("bytes()", "b''"),

    # bytes -> bytes
    ("bytes(b'abc')", "b'abc'"),
    ("bytes(b'')", "b''"),

    # строка + encoding
    ("bytes('abc', 'utf-8')", "b'abc'"),
    ("bytes('', 'utf-8')", "b''"),
    ("bytes('Привет', 'utf-8')", "b'\\xd0\\x9f\\xd1\\x80\\xd0\\xb8\\xd0\\xb2\\xd0\\xb5\\xd1\\x82'"),

    # int -> N нулевых байтов
    ("bytes(0)", "b''"),
    ("bytes(1)", "b'\\x00'"),
    ("bytes(2)", "b'\\x00\\x00'"),
    ("bytes(5)", "b'\\x00\\x00\\x00\\x00\\x00'"),

    # bool
    ("bytes(True)", "b'\\x00'"),
    ("bytes(False)", "b''"),

    # list
    ("bytes([65, 66, 67])", "b'ABC'"),
    ("bytes([97, 98, 99])", "b'abc'"),
    ("bytes([])", "b''"),
    ("bytes([0, 255])", "b'\\x00\\xff'"),

    # tuple
    ("bytes((65, 66, 67))", "b'ABC'"),
    ("bytes(())", "b''"),

    # set
    ("bytes({65})", "b'A'"),
    ("bytes(set())", "b''"),

    # dict (итерируется по ключам)
    ("bytes({65: 'a'})", "b'A'"),
    ("bytes({97: 1})", "b'a'"),

    # str slicing
    # базовые слайсы
    ("\"abcdef\"[:]", "'abcdef'"),
    ("\"abcdef\"[1:4]", "'bcd'"),
    ("\"abcdef\"[:3]", "'abc'"),
    ("\"abcdef\"[3:]", "'def'"),
    ("\"abcdef\"[0:6]", "'abcdef'"),

    # отрицательные индексы
    ("\"abcdef\"[-1:]", "'f'"),
    ("\"abcdef\"[:-1]", "'abcde'"),
    ("\"abcdef\"[-3:-1]", "'de'"),
    ("\"abcdef\"[-6:-2]", "'abcd'"),

    # шаг > 1
    ("\"abcdef\"[::2]", "'ace'"),
    ("\"abcdef\"[1::2]", "'bdf'"),
    ("\"abcdef\"[:5:2]", "'ace'"),
    ("\"abcdef\"[1:5:2]", "'bd'"),

    # отрицательный шаг
    ("\"abcdef\"[::-1]", "'fedcba'"),
    ("\"abcdef\"[::-2]", "'fdb'"),
    ("\"abcdef\"[5:1:-1]", "'fedc'"),
    ("\"abcdef\"[4:0:-1]", "'edcb'"),
    ("\"abcdef\"[3::-1]", "'dcba'"),

    # большие индексы
    ("\"abcdef\"[:100]", "'abcdef'"),
    ("\"abcdef\"[100:]", "''"),
    ("\"abcdef\"[-100:]", "'abcdef'"),
    ("\"abcdef\"[:-100]", "''"),

    # пустые результаты
    ("\"abcdef\"[1:1]", "''"),
    ("\"abcdef\"[4:2]", "''"),
    ("\"abcdef\"[-1:-3]", "''"),
    ("\"abcdef\"[0:0]", "''"),

    # шаги
    ("\"abcdef\"[::3]", "'ad'"),
    ("\"abcdef\"[1::3]", "'be'"),
    ("\"abcdef\"[5::-2]", "'fdb'"),
    ("\"abcdef\"[4::-2]", "'eca'"),

    # одиночный символ через индекс
    ("\"abcdef\"[0]", "'a'"),
    ("\"abcdef\"[-1]", "'f'"),
    ("\"abcdef\"[-3]", "'d'"),

    # пустая строка
    ("\"\"[:]", "''"),
    ("\"\"[::-1]", "''"),
    ("\"\"[::2]", "''"),

    ("\"abcdef\"[::-1]", "'fedcba'"),
    ("\"abcdef\"[::-2]", "'fdb'"),
    ("\"abcdef\"[5:1:-1]", "'fedc'"),
    ("\"abcdef\"[3::-1]", "'dcba'"),
    ("\"abcdef\"[:-1]", "'abcde'"),
    ("\"abcdef\"[-1:]", "'f'"),

    # list slicing
    # базовые слайсы
    ("[1,2,3,4,5][:]", "[1, 2, 3, 4, 5]"),
    ("[1,2,3,4,5][1:4]", "[2, 3, 4]"),
    ("[1,2,3,4,5][:3]", "[1, 2, 3]"),
    ("[1,2,3,4,5][2:]", "[3, 4, 5]"),

    # отрицательные индексы
    ("[1,2,3,4,5][-1:]", "[5]"),
    ("[1,2,3,4,5][:-1]", "[1, 2, 3, 4]"),
    ("[1,2,3,4,5][-3:-1]", "[3, 4]"),
    ("[1,2,3,4,5][-5:-2]", "[1, 2, 3]"),

    # шаг > 1
    ("[1,2,3,4,5][::2]", "[1, 3, 5]"),
    ("[1,2,3,4,5][1::2]", "[2, 4]"),
    ("[1,2,3,4,5][:4:2]", "[1, 3]"),
    ("[1,2,3,4,5][1:5:2]", "[2, 4]"),

    # пустой результат
    ("[1,2,3,4,5][1:1]", "[]"),
    ("[1,2,3,4,5][4:2]", "[]"),
    ("[1,2,3,4,5][-1:-3]", "[]"),
    ("[1,2,3,4,5][0:0]", "[]"),

    # большие индексы
    ("[1,2,3,4,5][:100]", "[1, 2, 3, 4, 5]"),
    ("[1,2,3,4,5][100:]", "[]"),
    ("[1,2,3,4,5][-100:]", "[1, 2, 3, 4, 5]"),
    ("[1,2,3,4,5][:-100]", "[]"),

    ("[1,2,3,4,5][0]", "1"),
    ("[1,2,3,4,5][-1]", "5"),
    ("[1,2,3,4,5][-3]", "3"),

    # tuple slicing
    ("(1,2,3,4,5)[1:4]", "(2, 3, 4)"),
    ("(1,2,3,4,5)[:3]", "(1, 2, 3)"),
    ("(1,2,3,4,5)[2:]", "(3, 4, 5)"),

    ("(1,2,3)[0]", "1"),
    ("(1,2,3)[1]", "2"),
    ("(1,2,3)[-1]", "3"),
    ("(1,2,3)[-2]", "2"),

    ("(1,2,3,4,5)[::2]", "(1, 3, 5)"),
    ("(1,2,3,4,5)[1::2]", "(2, 4)"),

    ("(1,2,3,4,5)[::-1]", "(5, 4, 3, 2, 1)"),
    ("(1,2,3,4,5)[4:1:-1]", "(5, 4, 3)"),

    ("(1,2,3,4,5)[-3:]", "(3, 4, 5)"),
    ("(1,2,3,4,5)[:-2]", "(1, 2, 3)"),
    ("(1,2,3,4,5)[-4:-1]", "(2, 3, 4)"),

    ("(1,2,3,4,5)[100:200]", "()"),
    ("(1,2,3,4,5)[5:5]", "()"),
    ("(1,2,3,4,5)[3:1]", "()"),

    ("(1,2,3,4,5)[::10]", "(1,)"),
    ("(1,2,3,4,5)[::-10]", "(5,)"),

    # bytes slicing
    ("b'abcdef'[1:4]", "b'bcd'"),
    ("b'abcdef'[:3]", "b'abc'"),
    ("b'abcdef'[3:]", "b'def'"),

    ("b'abcdef'[::2]", "b'ace'"),
    ("b'abcdef'[1::2]", "b'bdf'"),

    ("b'abcdef'[::-1]", "b'fedcba'"),
    ("b'abcdef'[5:1:-1]", "b'fedc'"),

    ("b'abcdef'[-3:]", "b'def'"),
    ("b'abcdef'[:-2]", "b'abcd'"),
    ("b'abcdef'[-4:-1]", "b'cde'"),

    ("b'abcdef'[100:200]", "b''"),
    ("b'abcdef'[5:5]", "b''"),
    ("b'abcdef'[3:1]", "b''"),

    ("b'abcdef'[-100:2]", "b'ab'"),
    ("b'abcdef'[2:100]", "b'cdef'"),

    ("b'abcdef'[::10]", "b'a'"),
    ("b'abcdef'[::-10]", "b'f'"),

    ("b'abcdef'[1:5:2]", "b'bd'"),
    ("b'abcdef'[4:0:-2]", "b'ec'"),

    ("b'abcdef'[2:3]", "b'c'"),
    ("b'abcdef'[2:2]", "b''"),

    ("b'abcdef'[:]", "b'abcdef'"),
    ("b'abcdef'[::]", "b'abcdef'"),

    # reverse copy
    ("b'abcdef'[::-1]", "b'fedcba'"),

    ("b'abcdef'[-1:-6:-1]", "b'fedcb'"),
    ("b'abcdef'[-2::-1]", "b'edcba'"),
    ("b'abcdef'[:-7:-1]", "b'fedcba'"),

    # empty bytes
    ("b''[:]", "b''"),
    ("b''[::-1]", "b''"),
    ("b''[::2]", "b''"),

    # bytearray() constructor
    ("bytearray()", "bytearray(b'')"),

    ("bytearray(0)", "bytearray(b'')"),
    ("bytearray(3)", "bytearray(b'\\x00\\x00\\x00')"),
    ("bytearray(5)", "bytearray(b'\\x00\\x00\\x00\\x00\\x00')"),

    ("bytearray(b'abc')", "bytearray(b'abc')"),
    ("bytearray(bytearray(b'abc'))", "bytearray(b'abc')"),

    ("bytearray('abc', 'utf8')", "bytearray(b'abc')"),
    ("bytearray('привет', 'utf8')", "bytearray(b'\\xd0\\xbf\\xd1\\x80\\xd0\\xb8\\xd0\\xb2\\xd0\\xb5\\xd1\\x82')"),

    ("bytearray([97, 98, 99])", "bytearray(b'abc')"),
    ("bytearray((97, 98, 99))", "bytearray(b'abc')"),
    ("bytearray({97, 98, 99})", "bytearray(b'abc')"),

    # пока не поддерживается
    # ("bytearray(range(5))", "bytearray(b'\\x00\\x01\\x02\\x03\\x04')"),

    ("bytearray([0])", "bytearray(b'\\x00')"),
    ("bytearray([255])", "bytearray(b'\\xff')"),

    ("len(bytearray())", "0"),
    ("len(bytearray(10))", "10"),
    ("len(bytearray([97, 98, 99]))", "3"),
    ("len(bytearray(b'abc'))", "3"),
    ("len(bytearray('abcde', 'utf8'))", "5"),

    ("bytearray(b'abcdef')[0]", "97"),
    ("bytearray(b'abcdef')[1]", "98"),
    ("bytearray(b'abcdef')[-1]", "102"),
    ("bytearray(b'abcdef')[-2]", "101"),

    # slicing
    ("bytearray(b'abcdef')[1:4]", "bytearray(b'bcd')"),
    ("bytearray(b'abcdef')[:3]", "bytearray(b'abc')"),
    ("bytearray(b'abcdef')[3:]", "bytearray(b'def')"),

    ("bytearray(b'abcdef')[::2]", "bytearray(b'ace')"),
    ("bytearray(b'abcdef')[1::2]", "bytearray(b'bdf')"),

    ("bytearray(b'abcdef')[::-1]", "bytearray(b'fedcba')"),
    ("bytearray(b'abcdef')[5:1:-1]", "bytearray(b'fedc')"),

    ("bytearray(b'abcdef')[-3:]", "bytearray(b'def')"),
    ("bytearray(b'abcdef')[:-2]", "bytearray(b'abcd')"),
    ("bytearray(b'abcdef')[-4:-1]", "bytearray(b'cde')"),

    ("bytearray(b'abcdef')[100:200]", "bytearray(b'')"),
    ("bytearray(b'abcdef')[5:5]", "bytearray(b'')"),
    ("bytearray(b'abcdef')[3:1]", "bytearray(b'')"),

    ("bytearray(b'abcdef')[::10]", "bytearray(b'a')"),
    ("bytearray(b'abcdef')[::-10]", "bytearray(b'f')"),

    # отрицательные индексы + reverse slicing
    ("bytearray(b'abcdef')[-1:-6:-1]", "bytearray(b'fedcb')"),
    ("bytearray(b'abcdef')[-2::-1]", "bytearray(b'edcba')"),
    ("bytearray(b'abcdef')[:-7:-1]", "bytearray(b'fedcba')"),

    # bytearray __add__
    ("bytearray(b'abc') + bytearray(b'def')", "bytearray(b'abcdef')"),
    ("bytearray(b'') + bytearray(b'abc')", "bytearray(b'abc')"),
    ("bytearray(b'abc') + bytearray(b'')", "bytearray(b'abc')"),
    ("bytearray(b'a') + bytearray(b'b')", "bytearray(b'ab')"),
    ("bytearray([97,98]) + bytearray([99,100])", "bytearray(b'abcd')"),

    # bytearray + bytes
    ("bytearray(b'abc') + b'def'", "bytearray(b'abcdef')"),
    ("bytearray(b'') + b'xyz'", "bytearray(b'xyz')"),
    ("bytearray(b'xyz') + b''", "bytearray(b'xyz')"),

    # chaining
    ("bytearray(b'a') + bytearray(b'b') + bytearray(b'c')", "bytearray(b'abc')"),

    # bytearray __mul__
    ("bytearray(b'abc') * 0", "bytearray(b'')"),
    ("bytearray(b'abc') * 1", "bytearray(b'abc')"),
    ("bytearray(b'abc') * 2", "bytearray(b'abcabc')"),
    ("bytearray(b'abc') * 3", "bytearray(b'abcabcabc')"),
    ("bytearray(b'') * 100", "bytearray(b'')"),
    ("bytearray([97]) * 5", "bytearray(b'aaaaa')"),
    ("bytearray([97,98]) * 4", "bytearray(b'abababab')"),

    # отрицательные множители
    ("bytearray(b'abc') * -1", "bytearray(b'')"),
    ("bytearray(b'abc') * -100", "bytearray(b'')"),

    # bool как int
    ("bytearray(b'abc') * True", "bytearray(b'abc')"),
    ("bytearray(b'abc') * False", "bytearray(b'')"),

    # chaining
    ("(bytearray(b'ab') * 2) * 2", "bytearray(b'abababab')"),

    # bytearray __contains__
    ("97 in bytearray(b'abc')", "True"),
    ("98 in bytearray(b'abc')", "True"),
    ("99 in bytearray(b'abc')", "True"),
    ("100 in bytearray(b'abc')", "False"),

    ("True in bytearray([0,1,2])", "True"),
    ("False in bytearray([0,1,2])", "True"),

    ("0 in bytearray([0,255])", "True"),
    ("255 in bytearray([0,255])", "True"),
    ("254 in bytearray([0,255])", "False"),

    ("b'a' in bytearray(b'abc')", "True"),
    ("b'ab' in bytearray(b'abc')", "True"),
    ("b'bc' in bytearray(b'abc')", "True"),
    ("b'abc' in bytearray(b'abc')", "True"),
    ("b'ac' in bytearray(b'abc')", "False"),
    ("b'' in bytearray(b'abc')", "True"),

    ("bytearray(b'a') in bytearray(b'abc')", "True"),
    ("bytearray(b'ab') in bytearray(b'abc')", "True"),
    ("bytearray(b'bc') in bytearray(b'abc')", "True"),
    ("bytearray(b'ac') in bytearray(b'abc')", "False"),
    ("bytearray(b'') in bytearray(b'abc')", "True"),

    ("97 in bytearray()", "False"),
    ("b'a' in bytearray()", "False"),
    ("bytearray(b'a') in bytearray()", "False"),
    ("b'' in bytearray()", "True"),

    # bytearray __eq__
    ("bytearray(b'abc') == bytearray(b'abc')", "True"),
    ("bytearray(b'abc') == bytearray(b'abc')", "True"),
    ("bytearray(b'abc') == bytearray(b'abd')", "False"),

    ("bytearray(b'abc') == b'abc'", "True"),
    ("bytearray(b'abc') == b'abd'", "False"),

    ("bytearray() == bytearray()", "True"),
    ("bytearray() == b''", "True"),

    ("bytearray(b'abc') == bytearray(b'ab')", "False"),
    ("bytearray(b'ab') == bytearray(b'abc')", "False"),

    ("bytearray([0,1,2]) == bytearray([0,1,2])", "True"),
    ("bytearray([0,1,2]) == bytearray([0,1,3])", "False"),

    ("bytearray(b'abc') == 'abc'", "False"),
    ("bytearray(b'abc') == [97,98,99]", "False"),
    ("bytearray(b'abc') == 123", "False"),

    # bytearray __ne__
    ("bytearray(b'abc') != bytearray(b'abc')", "False"),
    ("bytearray(b'abc') != bytearray(b'abd')", "True"),

    ("bytearray(b'abc') != b'abc'", "False"),
    ("bytearray(b'abc') != b'abd'", "True"),

    ("bytearray() != bytearray()", "False"),
    ("bytearray() != b''", "False"),

    ("bytearray(b'abc') != bytearray(b'ab')", "True"),
    ("bytearray(b'ab') != bytearray(b'abc')", "True"),

    ("bytearray([0,1,2]) != bytearray([0,1,2])", "False"),
    ("bytearray([0,1,2]) != bytearray([0,1,3])", "True"),

    ("bytearray(b'abc') != 'abc'", "True"),
    ("bytearray(b'abc') != [97,98,99]", "True"),
    ("bytearray(b'abc') != 123", "True"),

    # bytearray __lt__
    ("bytearray(b'abc') < bytearray(b'abd')", "True"),
    ("bytearray(b'abd') < bytearray(b'abc')", "False"),

    ("bytearray(b'abc') < bytearray(b'abc')", "False"),

    ("bytearray(b'ab') < bytearray(b'abc')", "True"),
    ("bytearray(b'abc') < bytearray(b'ab')", "False"),

    ("bytearray(b'') < bytearray(b'a')", "True"),
    ("bytearray(b'a') < bytearray(b'')", "False"),

    ("bytearray(b'abc') < b'abd'", "True"),
    ("bytearray(b'abd') < b'abc'", "False"),

    ("bytearray(b'abc') < b'abc'", "False"),

    ("bytearray([0,1,2]) < bytearray([0,1,3])", "True"),
    ("bytearray([0,1,3]) < bytearray([0,1,2])", "False"),

    ("bytearray([0,1]) < bytearray([0,1,0])", "True"),
    ("bytearray([0,1,0]) < bytearray([0,1])", "False"),

    # bytearray __le__
    ("bytearray(b'abc') <= bytearray(b'abd')", "True"),
    ("bytearray(b'abd') <= bytearray(b'abc')", "False"),

    ("bytearray(b'abc') <= bytearray(b'abc')", "True"),

    ("bytearray(b'ab') <= bytearray(b'abc')", "True"),
    ("bytearray(b'abc') <= bytearray(b'ab')", "False"),

    ("bytearray(b'') <= bytearray(b'a')", "True"),
    ("bytearray(b'a') <= bytearray(b'')", "False"),

    ("bytearray(b'abc') <= b'abd'", "True"),
    ("bytearray(b'abc') <= b'abc'", "True"),
    ("bytearray(b'abd') <= b'abc'", "False"),

    ("bytearray([0,1,2]) <= bytearray([0,1,3])", "True"),
    ("bytearray([0,1,2]) <= bytearray([0,1,2])", "True"),
    ("bytearray([0,1,3]) <= bytearray([0,1,2])", "False"),

    ("bytearray([0,1]) <= bytearray([0,1,0])", "True"),
    ("bytearray([0,1,0]) <= bytearray([0,1])", "False"),

    # bytearray __gt__
    ("bytearray(b'abd') > bytearray(b'abc')", "True"),
    ("bytearray(b'abc') > bytearray(b'abd')", "False"),

    ("bytearray(b'abc') > bytearray(b'abc')", "False"),

    ("bytearray(b'abc') > bytearray(b'ab')", "True"),
    ("bytearray(b'ab') > bytearray(b'abc')", "False"),

    ("bytearray(b'a') > bytearray(b'')", "True"),
    ("bytearray(b'') > bytearray(b'a')", "False"),

    ("bytearray(b'abd') > b'abc'", "True"),
    ("bytearray(b'abc') > b'abd'", "False"),

    ("bytearray(b'abc') > b'abc'", "False"),

    ("bytearray([0,1,3]) > bytearray([0,1,2])", "True"),
    ("bytearray([0,1,2]) > bytearray([0,1,3])", "False"),

    ("bytearray([0,1,0]) > bytearray([0,1])", "True"),
    ("bytearray([0,1]) > bytearray([0,1,0])", "False"),

    # bytearray __ge__
    ("bytearray(b'abd') >= bytearray(b'abc')", "True"),
    ("bytearray(b'abc') >= bytearray(b'abd')", "False"),

    ("bytearray(b'abc') >= bytearray(b'abc')", "True"),

    ("bytearray(b'abc') >= bytearray(b'ab')", "True"),
    ("bytearray(b'ab') >= bytearray(b'abc')", "False"),

    ("bytearray(b'a') >= bytearray(b'')", "True"),
    ("bytearray(b'') >= bytearray(b'a')", "False"),

    ("bytearray(b'abd') >= b'abc'", "True"),
    ("bytearray(b'abc') >= b'abc'", "True"),
    ("bytearray(b'abc') >= b'abd'", "False"),

    ("bytearray([0,1,3]) >= bytearray([0,1,2])", "True"),
    ("bytearray([0,1,2]) >= bytearray([0,1,2])", "True"),
    ("bytearray([0,1,2]) >= bytearray([0,1,3])", "False"),

    ("bytearray([0,1,0]) >= bytearray([0,1])", "True"),
    ("bytearray([0,1]) >= bytearray([0,1])", "True"),
    ("bytearray([0,1]) >= bytearray([0,1,0])", "False"),

    # bytearray.find()
    ("bytearray(b'abcdef').find(b'cd')", "2"),
    ("bytearray(b'abcdef').find(bytearray(b'cd'))", "2"),

    ("bytearray(b'abcdef').find(b'xx')", "-1"),

    ("bytearray(b'abcdef').find(b'a')", "0"),
    ("bytearray(b'abcdef').find(b'f')", "5"),

    ("bytearray(b'abcdef').find(99)", "2"),
    ("bytearray(b'abcdef').find(102)", "5"),

    ("bytearray(b'abcdef').find(b'cd', 3)", "-1"),
    ("bytearray(b'abcdef').find(b'cd', 2)", "2"),

    ("bytearray(b'abcdef').find(b'de', 0, 5)", "3"),
    ("bytearray(b'abcdef').find(b'de', 0, 4)", "-1"),

    ("bytearray(b'aaaaaa').find(b'aa')", "0"),
    ("bytearray(b'aaaaaa').find(b'aa', 1)", "1"),

    ("bytearray(b'').find(b'a')", "-1"),
    ("bytearray(b'').find(b'')", "0"),

    ("bytearray(b'abc').find(b'')", "0"),
    ("bytearray(b'abc').find(b'', 2)", "2"),

    # bytearray.rfind()
    ("bytearray(b'abcdef').rfind(b'cd')", "2"),
    ("bytearray(b'abcdef').rfind(bytearray(b'cd'))", "2"),

    ("bytearray(b'abcdef').rfind(b'xx')", "-1"),

    ("bytearray(b'abcdef').rfind(b'a')", "0"),
    ("bytearray(b'abcdef').rfind(b'f')", "5"),

    ("bytearray(b'abcdef').rfind(99)", "2"),
    ("bytearray(b'abcdef').rfind(102)", "5"),

    ("bytearray(b'aaaaaa').rfind(b'aa')", "4"),
    ("bytearray(b'aaaaaa').rfind(b'aa', 0, 5)", "3"),

    ("bytearray(b'abcabcabc').rfind(b'abc')", "6"),
    ("bytearray(b'abcabcabc').rfind(b'abc', 0, 8)", "3"),

    ("bytearray(b'abcdef').rfind(b'cd', 3)", "-1"),
    ("bytearray(b'abcdef').rfind(b'cd', 2)", "2"),

    ("bytearray(b'').rfind(b'a')", "-1"),
    ("bytearray(b'').rfind(b'')", "0"),

    ("bytearray(b'abc').rfind(b'')", "3"),
    ("bytearray(b'abc').rfind(b'', 0, 2)", "2"),

    # bytearray.index()
    ("bytearray(b'abcdef').index(b'cd')", "2"),
    ("bytearray(b'abcdef').index(bytearray(b'cd'))", "2"),

    ("bytearray(b'abcdef').index(b'a')", "0"),
    ("bytearray(b'abcdef').index(b'f')", "5"),

    ("bytearray(b'abcdef').index(99)", "2"),
    ("bytearray(b'abcdef').index(102)", "5"),

    ("bytearray(b'aaaaaa').index(b'aa')", "0"),
    ("bytearray(b'aaaaaa').index(b'aa', 1)", "1"),

    ("bytearray(b'abcabcabc').index(b'abc')", "0"),
    ("bytearray(b'abcabcabc').index(b'abc', 1)", "3"),

    ("bytearray(b'abcdef').index(b'cd', 2)", "2"),
    ("bytearray(b'abcdef').index(b'de', 0, 5)", "3"),

    # bytearray.count()
    ("bytearray(b'abcdef').count(b'a')", "1"),
    ("bytearray(b'abcdef').count(b'f')", "1"),
    ("bytearray(b'abcdef').count(b'x')", "0"),

    ("bytearray(b'aaaaaa').count(b'a')", "6"),
    ("bytearray(b'aaaaaa').count(b'aa')", "3"),
    ("bytearray(b'aaaaaa').count(b'aaa')", "2"),

    ("bytearray(b'abababa').count(b'aba')", "2"),

    ("bytearray(b'abcabcabc').count(b'abc')", "3"),
    ("bytearray(b'abcabcabc').count(b'bc')", "3"),

    ("bytearray(b'abcdef').count(99)", "1"),
    ("bytearray(b'abcdef').count(120)", "0"),

    ("bytearray(b'abcdef').count(b'cd', 2)", "1"),
    ("bytearray(b'abcdef').count(b'cd', 3)", "0"),

    ("bytearray(b'abcabcabc').count(b'abc', 1)", "2"),
    ("bytearray(b'abcabcabc').count(b'abc', 1, 7)", "1"),

    ("bytearray(b'').count(b'a')", "0"),
    ("bytearray(b'').count(b'')", "1"),

    ("bytearray(b'abc').count(b'')", "4"),
    ("bytearray(b'abc').count(b'', 1)", "3"),
    ("bytearray(b'abc').count(b'', 1, 2)", "2"),

    # bytearray.startswith()
    ("bytearray(b'abcdef').startswith(b'abc')", "True"),
    ("bytearray(b'abcdef').startswith(b'ab')", "True"),
    ("bytearray(b'abcdef').startswith(b'abcdef')", "True"),

    ("bytearray(b'abcdef').startswith(b'bc')", "False"),
    ("bytearray(b'abcdef').startswith(b'def')", "False"),

    ("bytearray(b'abcdef').startswith(bytearray(b'abc'))", "True"),

    ("bytearray(b'abcdef').startswith(b'cd', 2)", "True"),
    ("bytearray(b'abcdef').startswith(b'cd', 3)", "False"),

    ("bytearray(b'abcdef').startswith(b'cd', 2, 4)", "True"),
    ("bytearray(b'abcdef').startswith(b'cd', 2, 3)", "False"),

    ("bytearray(b'abcdef').startswith(b'ef', -2)", "True"),
    ("bytearray(b'abcdef').startswith(b'de', -2)", "False"),

    ("bytearray(b'').startswith(b'')", "True"),
    ("bytearray(b'').startswith(b'a')", "False"),

    ("bytearray(b'abc').startswith(b'')", "True"),
    ("bytearray(b'abc').startswith(bytearray(b''))", "True"),

    # bytearray.endswith()
    ("bytearray(b'abcdef').endswith(b'def')", "True"),
    ("bytearray(b'abcdef').endswith(b'ef')", "True"),
    ("bytearray(b'abcdef').endswith(b'abcdef')", "True"),

    ("bytearray(b'abcdef').endswith(b'abc')", "False"),
    ("bytearray(b'abcdef').endswith(b'cd')", "False"),

    ("bytearray(b'abcdef').endswith(bytearray(b'def'))", "True"),

    ("bytearray(b'abcdef').endswith(b'de', 0, 5)", "True"),
    ("bytearray(b'abcdef').endswith(b'def', 0, 5)", "False"),

    ("bytearray(b'abcdef').endswith(b'cd', 0, 4)", "True"),
    ("bytearray(b'abcdef').endswith(b'cd', 0, 3)", "False"),

    ("bytearray(b'abcdef').endswith(b'ef', -2)", "True"),
    ("bytearray(b'abcdef').endswith(b'de', -2)", "False"),

    ("bytearray(b'').endswith(b'')", "True"),
    ("bytearray(b'').endswith(b'a')", "False"),

    ("bytearray(b'abc').endswith(b'')", "True"),
    ("bytearray(b'abc').endswith(bytearray(b''))", "True"),

    # bytearray.lstrip()
    ("bytearray(b'   abc').lstrip()", "bytearray(b'abc')"),
    ("bytearray(b'\\t\\n abc').lstrip()", "bytearray(b'abc')"),

    ("bytearray(b'abc').lstrip()", "bytearray(b'abc')"),
    ("bytearray(b'').lstrip()", "bytearray(b'')"),

    ("bytearray(b'xxxabc').lstrip(b'x')", "bytearray(b'abc')"),
    ("bytearray(b'xxxxabc').lstrip(b'x')", "bytearray(b'abc')"),

    ("bytearray(b'abc').lstrip(b'ab')", "bytearray(b'c')"),
    ("bytearray(b'aabbcc').lstrip(b'ab')", "bytearray(b'cc')"),

    ("bytearray(b'abc').lstrip(b'')", "bytearray(b'abc')"),

    ("bytearray(b'111222').lstrip(b'1')", "bytearray(b'222')"),
    ("bytearray(b'111222').lstrip(b'12')", "bytearray(b'')"),

    ("bytearray(b'xyzabc').lstrip(bytearray(b'xyz'))", "bytearray(b'abc')"),

    ("bytearray(b'aaaa').lstrip(b'a')", "bytearray(b'')"),
    ("bytearray(b'aaaa').lstrip(b'b')", "bytearray(b'aaaa')"),

    ("bytearray(b'abcabc').lstrip(b'abc')", "bytearray(b'')"),

    # bytearray.rstrip()
    ("bytearray(b'abc   ').rstrip()", "bytearray(b'abc')"),
    ("bytearray(b'abc\\t\\n').rstrip()", "bytearray(b'abc')"),

    ("bytearray(b'abc').rstrip()", "bytearray(b'abc')"),
    ("bytearray(b'').rstrip()", "bytearray(b'')"),

    ("bytearray(b'abcxxx').rstrip(b'x')", "bytearray(b'abc')"),
    ("bytearray(b'abcxxxx').rstrip(b'x')", "bytearray(b'abc')"),

    ("bytearray(b'abc').rstrip(b'bc')", "bytearray(b'a')"),
    ("bytearray(b'aabbcc').rstrip(b'bc')", "bytearray(b'aa')"),

    ("bytearray(b'abc').rstrip(b'')", "bytearray(b'abc')"),

    ("bytearray(b'222111').rstrip(b'1')", "bytearray(b'222')"),
    ("bytearray(b'222111').rstrip(b'12')", "bytearray(b'')"),

    ("bytearray(b'abcxyz').rstrip(bytearray(b'xyz'))", "bytearray(b'abc')"),

    ("bytearray(b'aaaa').rstrip(b'a')", "bytearray(b'')"),
    ("bytearray(b'aaaa').rstrip(b'b')", "bytearray(b'aaaa')"),

    ("bytearray(b'abcabc').rstrip(b'abc')", "bytearray(b'')"),

    # bytearray.strip()
    ("bytearray(b'   abc   ').strip()", "bytearray(b'abc')"),
    ("bytearray(b'\\t\\nabc\\r\\n').strip()", "bytearray(b'abc')"),

    ("bytearray(b'abc').strip()", "bytearray(b'abc')"),
    ("bytearray(b'').strip()", "bytearray(b'')"),

    ("bytearray(b'xxxabcxxx').strip(b'x')", "bytearray(b'abc')"),
    ("bytearray(b'xxxxabcxxxx').strip(b'x')", "bytearray(b'abc')"),

    ("bytearray(b'abc').strip(b'abc')", "bytearray(b'')"),
    ("bytearray(b'aaabcaa').strip(b'a')", "bytearray(b'bc')"),

    ("bytearray(b'abc').strip(b'')", "bytearray(b'abc')"),

    ("bytearray(b'111abc222').strip(b'12')", "bytearray(b'abc')"),
    ("bytearray(b'222111').strip(b'12')", "bytearray(b'')"),

    ("bytearray(b'xyzabcxyz').strip(bytearray(b'xyz'))", "bytearray(b'abc')"),

    ("bytearray(b'aaaa').strip(b'a')", "bytearray(b'')"),
    ("bytearray(b'aaaa').strip(b'b')", "bytearray(b'aaaa')"),

    ("bytearray(b'abcabc').strip(b'abc')", "bytearray(b'')"),
    ("bytearray(b'cabXYZbac').strip(b'abc')", "bytearray(b'XYZ')"),

    # removeprefix
    ("bytearray(b'foobar').removeprefix(b'foo')", "bytearray(b'bar')"),
    ("bytearray(b'foobar').removeprefix(b'foobar')", "bytearray(b'')"),
    ("bytearray(b'foobar').removeprefix(b'')", "bytearray(b'foobar')"),
    ("bytearray(b'foobar').removeprefix(b'xxx')", "bytearray(b'foobar')"),
    ("bytearray(b'foobar').removeprefix(b'f')", "bytearray(b'oobar')"),
    ("bytearray(b'foobar').removeprefix(b'foobarbaz')", "bytearray(b'foobar')"),
    ("bytearray(b'aaaaaa').removeprefix(b'aaa')", "bytearray(b'aaa')"),
    ("bytearray(b'abcabc').removeprefix(b'abc')", "bytearray(b'abc')"),
    ("bytearray(b'abcabc').removeprefix(b'abcabc')", "bytearray(b'')"),
    ("bytearray(b'').removeprefix(b'abc')", "bytearray(b'')"),

    # removesuffix
    ("bytearray(b'foobar').removesuffix(b'bar')", "bytearray(b'foo')"),
    ("bytearray(b'foobar').removesuffix(b'foobar')", "bytearray(b'')"),
    ("bytearray(b'foobar').removesuffix(b'')", "bytearray(b'foobar')"),
    ("bytearray(b'foobar').removesuffix(b'xxx')", "bytearray(b'foobar')"),
    ("bytearray(b'foobar').removesuffix(b'r')", "bytearray(b'fooba')"),
    ("bytearray(b'foobar').removesuffix(b'zzfoobar')", "bytearray(b'foobar')"),
    ("bytearray(b'aaaaaa').removesuffix(b'aaa')", "bytearray(b'aaa')"),
    ("bytearray(b'abcabc').removesuffix(b'abc')", "bytearray(b'abc')"),
    ("bytearray(b'abcabc').removesuffix(b'abcabc')", "bytearray(b'')"),
    ("bytearray(b'').removesuffix(b'abc')", "bytearray(b'')"),

    # replace
    ("bytearray(b'abcabc').replace(b'a', b'x')", "bytearray(b'xbcxbc')"),
    ("bytearray(b'abcabc').replace(b'abc', b'X')", "bytearray(b'XX')"),
    ("bytearray(b'abcabc').replace(b'abc', b'X', 1)", "bytearray(b'Xabc')"),
    ("bytearray(b'abcabc').replace(b'abc', b'X', 2)", "bytearray(b'XX')"),
    ("bytearray(b'abcabc').replace(b'abc', b'X', 0)", "bytearray(b'abcabc')"),
    ("bytearray(b'aaaa').replace(b'aa', b'b')", "bytearray(b'bb')"),
    ("bytearray(b'aaaa').replace(b'aa', b'b', 1)", "bytearray(b'baa')"),
    ("bytearray(b'abcdef').replace(b'xyz', b'123')", "bytearray(b'abcdef')"),
    ("bytearray(b'').replace(b'a', b'b')", "bytearray(b'')"),
    ("bytearray(b'abc').replace(bytearray(b'a'), bytearray(b'z'))", "bytearray(b'zbc')"),

    # replace with empty pattern
    ("bytearray(b'abc').replace(b'', b'-')", "bytearray(b'-a-b-c-')"),
    ("bytearray(b'').replace(b'', b'-')", "bytearray(b'-')"),
    ("bytearray(b'abc').replace(b'', b'-', 0)", "bytearray(b'abc')"),
    ("bytearray(b'abc').replace(b'', b'-', 1)", "bytearray(b'-abc')"),
    ("bytearray(b'abc').replace(b'', b'-', 2)", "bytearray(b'-a-bc')"),
    ("bytearray(b'abc').replace(b'', b'-', 3)", "bytearray(b'-a-b-c')"),
    ("bytearray(b'abc').replace(b'', b'-', 4)", "bytearray(b'-a-b-c-')"),
    ("bytearray(b'abc').replace(b'', b'-', 100)", "bytearray(b'-a-b-c-')"),

    # split
    ("bytearray(b'a,b,c').split(b',')", "[bytearray(b'a'), bytearray(b'b'), bytearray(b'c')]"),
    ("bytearray(b'a,b,c').split(b',', 1)", "[bytearray(b'a'), bytearray(b'b,c')]"),
    ("bytearray(b'a,b,c').split(b',', 2)", "[bytearray(b'a'), bytearray(b'b'), bytearray(b'c')]"),
    ("bytearray(b'a,b,c').split(b',', 0)", "[bytearray(b'a,b,c')]"),
    ("bytearray(b'abc').split(b',')", "[bytearray(b'abc')]"),
    ("bytearray(b'').split(b',')", "[bytearray(b'')]"),
    ("bytearray(b',a,b,').split(b',')", "[bytearray(b''), bytearray(b'a'), bytearray(b'b'), bytearray(b'')]"),
    ("bytearray(b'aaaa').split(b'aa')", "[bytearray(b''), bytearray(b''), bytearray(b'')]"),
    ("bytearray(b'one--two--three').split(b'--')", "[bytearray(b'one'), bytearray(b'two'), bytearray(b'three')]"),
    ("bytearray(b'one--two--three').split(b'--', 1)", "[bytearray(b'one'), bytearray(b'two--three')]"),

    # rsplit
    ("bytearray(b'a,b,c').rsplit(b',')", "[bytearray(b'a'), bytearray(b'b'), bytearray(b'c')]"),
    ("bytearray(b'a,b,c').rsplit(b',', 1)", "[bytearray(b'a,b'), bytearray(b'c')]"),
    ("bytearray(b'a,b,c').rsplit(b',', 2)", "[bytearray(b'a'), bytearray(b'b'), bytearray(b'c')]"),
    ("bytearray(b'a,b,c').rsplit(b',', 0)", "[bytearray(b'a,b,c')]"),
    ("bytearray(b'abc').rsplit(b',')", "[bytearray(b'abc')]"),
    ("bytearray(b'').rsplit(b',')", "[bytearray(b'')]"),
    ("bytearray(b',a,b,').rsplit(b',')", "[bytearray(b''), bytearray(b'a'), bytearray(b'b'), bytearray(b'')]"),
    ("bytearray(b'aaaa').rsplit(b'aa')", "[bytearray(b''), bytearray(b''), bytearray(b'')]"),
    ("bytearray(b'one--two--three').rsplit(b'--')", "[bytearray(b'one'), bytearray(b'two'), bytearray(b'three')]"),
    ("bytearray(b'one--two--three').rsplit(b'--', 1)", "[bytearray(b'one--two'), bytearray(b'three')]"),
    ("bytearray(b'one--two--three').rsplit(b'--', 2)", "[bytearray(b'one'), bytearray(b'two'), bytearray(b'three')]"),

    # partition
    ("bytearray(b'abc=123').partition(b'=')", "(bytearray(b'abc'), bytearray(b'='), bytearray(b'123'))"),
    ("bytearray(b'abc').partition(b'=')", "(bytearray(b'abc'), bytearray(b''), bytearray(b''))"),
    ("bytearray(b'=abc').partition(b'=')", "(bytearray(b''), bytearray(b'='), bytearray(b'abc'))"),
    ("bytearray(b'abc=').partition(b'=')", "(bytearray(b'abc'), bytearray(b'='), bytearray(b''))"),
    ("bytearray(b'a=b=c').partition(b'=')", "(bytearray(b'a'), bytearray(b'='), bytearray(b'b=c'))"),
    ("bytearray(b'aaaa').partition(b'aa')", "(bytearray(b''), bytearray(b'aa'), bytearray(b'aa'))"),
    ("bytearray(b'xyz').partition(b'xyz')", "(bytearray(b''), bytearray(b'xyz'), bytearray(b''))"),

    # rpartition
    ("bytearray(b'abc=123').rpartition(b'=')", "(bytearray(b'abc'), bytearray(b'='), bytearray(b'123'))"),
    ("bytearray(b'abc').rpartition(b'=')", "(bytearray(b''), bytearray(b''), bytearray(b'abc'))"),
    ("bytearray(b'=abc').rpartition(b'=')", "(bytearray(b''), bytearray(b'='), bytearray(b'abc'))"),
    ("bytearray(b'abc=').rpartition(b'=')", "(bytearray(b'abc'), bytearray(b'='), bytearray(b''))"),
    ("bytearray(b'a=b=c').rpartition(b'=')", "(bytearray(b'a=b'), bytearray(b'='), bytearray(b'c'))"),
    ("bytearray(b'aaaa').rpartition(b'aa')", "(bytearray(b'aa'), bytearray(b'aa'), bytearray(b''))"),
    ("bytearray(b'xyz').rpartition(b'xyz')", "(bytearray(b''), bytearray(b'xyz'), bytearray(b''))"),
    ("bytearray(b'one--two--three').rpartition(b'--')", "(bytearray(b'one--two'), bytearray(b'--'), bytearray(b'three'))"),

    # center
    ("bytearray(b'abc').center(3)", "bytearray(b'abc')"),
    ("bytearray(b'abc').center(5)", "bytearray(b' abc ')"),
    ("bytearray(b'abc').center(6)", "bytearray(b' abc  ')"),
    ("bytearray(b'abc').center(7)", "bytearray(b'  abc  ')"),
    ("bytearray(b'abc').center(8)", "bytearray(b'  abc   ')"),
    ("bytearray(b'abc').center(5, b'-')", "bytearray(b'-abc-')"),
    ("bytearray(b'abc').center(6, b'-')", "bytearray(b'-abc--')"),
    ("bytearray(b'abc').center(7, b'-')", "bytearray(b'--abc--')"),
    ("bytearray(b'').center(4)", "bytearray(b'    ')"),
    ("bytearray(b'').center(4, b'*')", "bytearray(b'****')"),

    # ljust
    ("bytearray(b'abc').ljust(3)", "bytearray(b'abc')"),
    ("bytearray(b'abc').ljust(5)", "bytearray(b'abc  ')"),
    ("bytearray(b'abc').ljust(6)", "bytearray(b'abc   ')"),
    ("bytearray(b'abc').ljust(7)", "bytearray(b'abc    ')"),
    ("bytearray(b'abc').ljust(5, b'-')", "bytearray(b'abc--')"),
    ("bytearray(b'abc').ljust(6, b'-')", "bytearray(b'abc---')"),
    ("bytearray(b'abc').ljust(7, b'*')", "bytearray(b'abc****')"),
    ("bytearray(b'').ljust(4)", "bytearray(b'    ')"),
    ("bytearray(b'').ljust(4, b'*')", "bytearray(b'****')"),

    # rjust
    ("bytearray(b'abc').rjust(3)", "bytearray(b'abc')"),
    ("bytearray(b'abc').rjust(5)", "bytearray(b'  abc')"),
    ("bytearray(b'abc').rjust(6)", "bytearray(b'   abc')"),
    ("bytearray(b'abc').rjust(7)", "bytearray(b'    abc')"),
    ("bytearray(b'abc').rjust(5, b'-')", "bytearray(b'--abc')"),
    ("bytearray(b'abc').rjust(6, b'-')", "bytearray(b'---abc')"),
    ("bytearray(b'abc').rjust(7, b'*')", "bytearray(b'****abc')"),
    ("bytearray(b'').rjust(4)", "bytearray(b'    ')"),
    ("bytearray(b'').rjust(4, b'*')", "bytearray(b'****')"),

    # zfill
    ("bytearray(b'42').zfill(5)", "bytearray(b'00042')"),
    ("bytearray(b'42').zfill(2)", "bytearray(b'42')"),
    ("bytearray(b'42').zfill(1)", "bytearray(b'42')"),
    ("bytearray(b'42').zfill(0)", "bytearray(b'42')"),
    ("bytearray(b'-42').zfill(5)", "bytearray(b'-0042')"),
    ("bytearray(b'+42').zfill(5)", "bytearray(b'+0042')"),
    ("bytearray(b'-42').zfill(6)", "bytearray(b'-00042')"),
    ("bytearray(b'+42').zfill(6)", "bytearray(b'+00042')"),
    ("bytearray(b'').zfill(4)", "bytearray(b'0000')"),
    ("bytearray(b'abc').zfill(6)", "bytearray(b'000abc')"),
    ("bytearray(b'-abc').zfill(7)", "bytearray(b'-000abc')"),

    # lower
    ("bytearray(b'ABC').lower()", "bytearray(b'abc')"),
    ("bytearray(b'HELLO').lower()", "bytearray(b'hello')"),
    ("bytearray(b'abc').lower()", "bytearray(b'abc')"),
    ("bytearray(b'AbCdEf').lower()", "bytearray(b'abcdef')"),
    ("bytearray(b'ABC123').lower()", "bytearray(b'abc123')"),
    ("bytearray(b'ABC!?').lower()", "bytearray(b'abc!?')"),
    ("bytearray(b'').lower()", "bytearray(b'')"),
    ("bytearray(b'HELLO WORLD').lower()", "bytearray(b'hello world')"),
    ("bytearray([255, 65, 66]).lower()", "bytearray(b'\\xffab')"),
    ("bytearray(b'A').lower()", "bytearray(b'a')"),

    # upper
    ("bytearray(b'abc').upper()", "bytearray(b'ABC')"),
    ("bytearray(b'hello world').upper()", "bytearray(b'HELLO WORLD')"),
    ("bytearray(b'ABC').upper()", "bytearray(b'ABC')"),
    ("bytearray(b'HELLO').upper()", "bytearray(b'HELLO')"),
    ("bytearray(b'AbCdEf').upper()", "bytearray(b'ABCDEF')"),
    ("bytearray(b'abc123').upper()", "bytearray(b'ABC123')"),
    ("bytearray(b'12345').upper()", "bytearray(b'12345')"),
    ("bytearray(b'abc!@#').upper()", "bytearray(b'ABC!@#')"),
    ("bytearray().upper()", "bytearray(b'')"),
    ("bytearray(b'hello world').upper()", "bytearray(b'HELLO WORLD')"),
    ("bytearray(b'ABC').upper().upper()", "bytearray(b'ABC')"),
    ("bytearray(b'ABC').lower()", "bytearray(b'abc')"),
    ("bytearray(b'abc').upper()", "bytearray(b'ABC')"),

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
      "res"], "[97, 98, 99]"),

    # пользовательский __bytes__
    (["class X:",
      "    def __bytes__(self):",
      "        return b'hello'",
      "",
      "bytes(X())"], "b'hello'"),

    # __bytes__ может возвращать существующий bytes
    (["class X:",
      "    def __bytes__(self):",
      "        return b''",
      "",
      "bytes(X())"], "b''"),

    # __bytes__ вызывается напрямую
    (["class X:",
      "    def __bytes__(self):",
      "        return b'world'",
      "",
      "X().__bytes__()"], "b'world'"),

    (["class X:",
      "    def __bytes__(self):",
      "        return b'custom'",
      "",
      "bytes(X())"], "b'custom'"),

    # dict.keys()
    (["d = {65: 'x', 66: 'y'}",
      "bytes(d.keys())"], "b'AB'"),

    # dict.values()
    (["d = {'a': 65, 'b': 66}",
      "bytes(d.values())"], "b'AB'"),

    # iterable через пользовательский класс
    (["class X:",
      "    def __iter__(self):",
      "        return [65, 66, 67].__iter__()",
      "",
      "bytes(X())"], "b'ABC'"),

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