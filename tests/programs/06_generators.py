#
# Created by semyo on 18.07.2026.
#

# ============================================================
#  Генераторы: yield, yield from, генераторные выражения,
#  next(), перебор в for, send(), close()
#  Запуск:  cppython 06_generators.py
# ============================================================

# --- простой генератор ---
def count_up_to(n):
    i = 1
    while i <= n:
        yield i
        i += 1

print("count_up_to(5):")
for value in count_up_to(5):
    print("  ", value)

# --- next() вручную ---
gen = count_up_to(3)
print("next:", next(gen))
print("next:", next(gen))
print("next:", next(gen))

# --- yield from: делегирование другому итерируемому ---
def chain(a, b):
    yield from a
    yield from b

print("chain([1, 2], [3, 4]):")
for v in chain([1, 2], [3, 4]):
    print("  ", v)

# --- бесконечный генератор + ручное ограничение ---
def naturals():
    n = 0
    while True:
        n += 1
        yield n

stream = naturals()
collected = []
count = 0
while count < 5:
    collected.append(next(stream))
    count += 1
print("первые 5 натуральных:", collected)

# --- генераторное выражение ---
squares_gen = (v * v for v in range(1, 6))
result = []
for sq in squares_gen:
    result.append(sq)
print("квадраты через genexpr:", result)

# --- send(): двусторонняя связь с генератором ---
def echo():
    received = yield "готов"
    while True:
        received = yield received

e = echo()
print("старт:      ", next(e))       # доходим до первого yield -> "готов"
print("send(42):   ", e.send(42))
print("send('hi'): ", e.send("hi"))
e.close()
print("генератор закрыт")

print("done: generators")
