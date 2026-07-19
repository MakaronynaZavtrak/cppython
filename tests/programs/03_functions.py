#
# Created by semyo on 18.07.2026.
#

# ============================================================
#  Функции, аргументы, замыкания, рекурсия, lambda
#  Запуск:  cppython 03_functions.py
# ============================================================

# --- значения по умолчанию и именованные аргументы ---
def greet(name, greeting="Привет"):
    return greeting + ", " + name + "!"

print(greet("Олег"))
print(greet("Олег", greeting="Здорово"))

# --- *args: произвольное число позиционных аргументов ---
def total(*nums):
    s = 0
    for n in nums:
        s += n
    return s

print("total(1, 2, 3, 4) =", total(1, 2, 3, 4))

# --- **kwargs: произвольные именованные аргументы ---
def describe(**attrs):
    for key in attrs:
        print("  ", key, "=", attrs[key])

print("describe(color=red, size=10):")
describe(color="red", size=10)

# --- рекурсия ---
def fib(n):
    if n < 2:
        return n
    return fib(n - 1) + fib(n - 2)

print("fib(10)           =", fib(10))

# --- замыкание с nonlocal ---
def make_counter():
    count = 0
    def step():
        nonlocal count
        count += 1
        return count
    return step

counter = make_counter()
print("counter:", counter(), counter(), counter())

# --- lambda ---
square = lambda v: v * v
add = lambda a, b: a + b
print("square(9)         =", square(9))
print("add(3, 4)         =", add(3, 4))

# --- функция как аргумент функции ---
def apply_twice(f, value):
    return f(f(value))

print("apply_twice(square, 3) =", apply_twice(square, 3))   # square(square(3)) = 81

print("done: functions")
