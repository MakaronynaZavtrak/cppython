#
# Created by semyo on 18.07.2026.
#

# ============================================================
#  Структуры данных: list, tuple, dict, set, frozenset,
#  str, bytes, bytearray + генераторы коллекций
#  Запуск:  cppython 04_data_structures.py
# ============================================================

# --- list: индексы, срезы, длина, изменение ---
nums = [10, 20, 30, 40, 50]
print("nums            =", nums)
print("nums[0]         =", nums[0])
print("nums[-1]        =", nums[-1])
print("nums[1:4]       =", nums[1:4])
print("nums[::2]       =", nums[::2])
print("len(nums)       =", len(nums))
nums.append(60)
print("после append    =", nums)

# --- list comprehension ---
squares = [v * v for v in range(1, 6)]
print("squares         =", squares)
evens = [v for v in range(10) if v % 2 == 0]
print("evens           =", evens)

# --- tuple + распаковка (в т.ч. со звёздочкой) ---
point = (3, 4)
print("point           =", point)
a, b = point
print("a, b            =", a, b)
first, *rest = [1, 2, 3, 4, 5]
print("first           =", first)
print("rest            =", rest)

# --- dict: доступ, добавление, перебор ---
person = {"name": "Олег", "group": "КН-302"}
print("person          =", person)
print("person['name']  =", person["name"])
person["year"] = 2
print("перебор словаря:")
for key in person:
    print("  ", key, "->", person[key])

# --- dict comprehension ---
cubes = {v: v * v * v for v in range(1, 5)}
print("cubes           =", cubes)

# --- set + set comprehension (дубликаты схлопываются) ---
s = {1, 2, 2, 3, 3, 3}
print("set из дублей   =", s)
mods = {v % 3 for v in range(10)}
print("set comp        =", mods)
print("2 in s          =", 2 in s)

# --- frozenset ---
fs = frozenset([1, 2, 3, 2, 1])
print("frozenset       =", fs)
print("len(fs)         =", len(fs))

# --- str: умножение, индекс, срез, длина, repr ---
text = "python"
print("text            =", text)
print("text * 3        =", text * 3)
print("text[0]         =", text[0])
print("text[1:4]       =", text[1:4])
print("len(text)       =", len(text))
print("repr(text)      =", repr(text))

# --- bytes ---
data = b"abc"
print("data            =", data)
print("data * 3        =", data * 3)
print("len(data)       =", len(data))

# --- bytearray (изменяемый) ---
buf = bytearray(b"abc")
buf.append(100)
print("bytearray       =", buf)

print("done: data structures")
