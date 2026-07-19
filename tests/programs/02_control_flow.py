#
# Created by semyo on 18.07.2026.
#

# ============================================================
#  Управление потоком: if / elif / else, while, for, range
#  Запуск:  cppython 02_control_flow.py
# ============================================================

# --- if / elif / else ---
def classify(n):
    if n < 0:
        return "negative"
    elif n == 0:
        return "zero"
    else:
        return "positive"

print("classify(-3)  =", classify(-3))
print("classify(0)   =", classify(0))
print("classify(7)   =", classify(7))

# --- while с break и continue ---
print("нечётные меньше 10:")
i = 0
while True:
    i += 1
    if i >= 10:
        break            # выходим из цикла
    if i % 2 == 0:
        continue         # чётные пропускаем
    print("  ", i)

# --- for по range(start, stop, step) ---
print("range(2, 12, 3):")
for k in range(2, 12, 3):
    print("  ", k)

# --- for по списку ---
for word in ["раз", "два", "три"]:
    print("слово:", word)

# --- вложенные циклы: таблица умножения 3x3 ---
print("таблица умножения 3x3:")
for a in range(1, 4):
    row = ""
    for b in range(1, 4):
        row += str(a * b) + "\t"
    print("  ", row)

# --- membership: у range __contains__ работает за O(1) ---
print("100 in range(0, 1000000000, 2) ->", 100 in range(0, 1000000000, 2))
print("101 in range(0, 1000000000, 2) ->", 101 in range(0, 1000000000, 2))

# if False: тело не выполнится — проверка, что ветка действительно пропускается
if False:
    print(">>> НЕ ДОЛЖНО ПОЯВИТЬСЯ (ветка if False) <<<")
else:
    print("ветка else отработала корректно")

print("done: control flow")
