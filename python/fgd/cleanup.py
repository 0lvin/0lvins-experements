strings = []
text = ""

with open("entity.dat", "rb") as f:
    text = f.read().decode("utf8")

for line in text.split("\n"):
    if line.strip() not in strings or not line:
        strings.append(line.strip())

with open("entity.dat", "wb") as f:
    print(f.write("\n".join(strings).encode("utf8")))
