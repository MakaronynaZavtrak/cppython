#
# Created by semyo on 18.07.2026.
#

# ============================================================
#  Исключения: try / except / else / finally, raise, from,
#  иерархия классов исключений
#  Запуск:  cppython 05_exceptions.py
# ============================================================

# --- базовый перехват (ArithmeticError ловит и деление на ноль) ---
try:
    x = 1 / 0
    print(">>> сюда попадать нельзя <<<")
except ArithmeticError:
    print("поймали деление на ноль")

# --- except ... as e: доступ к объекту исключения ---
def check_age(age):
    if age < 0:
        raise ValueError("возраст не может быть отрицательным")
    return age

try:
    check_age(-1)
except ValueError as e:
    print("ValueError:", e)

# --- else выполняется, только если исключения не было ---
try:
    result = 10 + 5
except Exception:
    print(">>> не должно случиться <<<")
else:
    print("else: результат =", result)

# --- finally выполняется всегда, даже при return из try ---
def read_safely(data, index):
    try:
        return data[index]
    except IndexError:
        return "нет такого индекса"
    finally:
        print("   finally отработал")

print("read_safely([1,2,3], 1) =", read_safely([1, 2, 3], 1))
print("read_safely([1,2,3], 9) =", read_safely([1, 2, 3], 9))

# --- raise ... from ... : явная цепочка причин ---
try:
    try:
        raise ValueError("исходная причина")
    except ValueError as original:
        raise RuntimeError("обёрнутая ошибка") from original
except RuntimeError as e:
    print("RuntimeError:", e)

# --- KeyError при обращении к отсутствующему ключу ---
try:
    d = {"a": 1}
    print(d["b"])
except KeyError as e:
    print("KeyError:", e)

print("done: exceptions")
