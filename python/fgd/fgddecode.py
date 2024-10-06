import sys
import json
import copy


def get_next_token(content):
    content = content.strip()
    pos = 1
    while pos < len(content) and content[pos] not in (
        " ",
        "\t",
        "\n",
        ":",
        "(",
        ")",
        "[",
        "]",
        "{",
        "}",
    ):
        pos += 1
    # print(">>>", content[:pos], content[pos:].strip()[:10])
    return content[:pos], content[pos:].strip(" \t")


def parse_value(content):
    brakets = {"[": 0, "{": 0, "(": 0}
    # skip open close brakets
    pos = 0
    while pos < len(content):
        if content[pos] == "]":
            brakets["["] -= 1
        elif content[pos] == "[":
            brakets["["] += 1
        elif content[pos] == "}":
            brakets["{"] -= 1
        elif content[pos] == "{":
            brakets["{"] += 1
        elif content[pos] == ")":
            brakets["("] -= 1
        elif content[pos] == "(":
            brakets["("] += 1
        elif content[pos] == '"':
            # skip quote
            pos += 1
            # search end of quote
            pos += content[pos:].index('"')
        pos += 1

        for key in brakets.keys():
            if brakets[key]:
                break
        else:
            break
    # looks as end of properties
    value = (content[1 : pos - 1]).strip()
    content = content[pos:]

    return content, value


def parse_section(entity_type, content, structure):
    values = {}
    while content:
        # search possible token
        token, content = get_next_token(content)
        if token.startswith("//"):
            pos = content.index("\n")
            content = content[pos:].strip()
        elif token == "=":
            name, content = get_next_token(content)
            values["name"] = name
        elif token == ":":
            # search possible token
            token, content = get_next_token(content)
            if token[0] == '"':
                content = token[1:] + " " + content
                pos = content.index('"')
                description = content[:pos].strip()
                content = content[pos + 1 :].strip()
            else:
                description = token
            values["description"] = description.split("\n")[0]
        elif token[0] == "[":
            content, properties = parse_value(token + " " + content)
            values["properties"] = properties
            if entity_type not in structure:
                structure[entity_type] = []
            structure[entity_type].append(values)
            return content
        elif content[0] == "(":
            content, value = parse_value(content)
            if token == "studio":
                if value[0] == '"' and value[-1] == '"':
                    value = value[1:-1]
                if "\n" not in value:
                    values["model_path"] = value.replace("\\", "/")
            elif token == "size":
                values[token] = [
                    [float(y) for y in x.strip().split(" ")] for x in value.split(",")
                ]
            elif token == "base":
                values[token] = [x.strip() for x in value.split(",")]
            elif token == "color":
                values[token] = [float(x) for x in value.split(" ")]
                if any(x > 1 for x in values[token]):
                    values[token] = [round(x / 255, 2) for x in values[token]]
            elif token == "model":
                if value.startswith("{") and not value.startswith("{{"):
                    value = json.loads(value.replace("\\", "/"))
                    if "path" in value:
                        model_path = value.get("path")
                        if model_path.startswith(":"):
                            model_path = model_path[1:]
                        values["model_path"] = model_path
                values[token] = value
            else:
                if token not in values:
                    values[token] = []
                values[token].append(value)
        else:
            print("displaced subtoken >", token, "<")
            exit(1)


def parse_fgd(filename):

    fgd_content = None
    structure = {}

    with open(filename, "rb") as f:
        fgd_content = f.read().decode("utf8")
        # remove \r
        fgd_content = fgd_content.replace("\r", "\n").strip()

    while fgd_content:
        # search possible token
        token, fgd_content = get_next_token(fgd_content)
        if token.startswith("//"):
            pos = fgd_content.index("\n")
            fgd_content = fgd_content[pos:].strip()
        elif token.lower() in ("@solidclass", "@baseclass", "@pointclass"):
            fgd_content = parse_section(token.lower(), fgd_content, structure)
        elif token.lower() == "@include":
            fgd_content, value = parse_value(fgd_content)
            with open(value, "rb") as f:
                add_content = f.read().decode("utf8")
                add_content = add_content.replace("\r", "\n").strip()
            fgd_content = add_content + " " + fgd_content
        else:
            print("displaced >", token, "<")
            exit(1)

    classes = {}

    for key in ("@baseclass", "@solidclass", "@pointclass"):
        for value in structure.get(key, []):
            new_value = {}
            for base in value.get("base", []):
                if base in classes:
                    new_value.update(copy.deepcopy(classes[base]))
            new_value.update(copy.deepcopy(value))
            classes[value.get("name")] = new_value
    return classes


if len(sys.argv) < 2:
    print("fgd is not provided")
    exit(1)

classes = parse_fgd(sys.argv[1])

entities = []
with open("entity.dat", "rb") as f:
    entities = f.read().decode("utf8")
    # remove \r
    entities = entities.replace("\r", "\n").strip().split("\n")

result_entities = []
for line in entities:
    sub_lines = line.split("|")
    if len(sub_lines) < 2:
        result_entities.append(line)
    elif sub_lines[0] not in classes:
        result_entities.append(line)
    else:
        values = classes[sub_lines[0]]
        if values.get("model_path"):
            sub_lines[1] = values["model_path"]
        if values.get("description"):
            sub_lines[23] = values["description"]
        if values.get("size"):
            sub_lines[6] = str(values["size"][0][0])
            sub_lines[7] = str(values["size"][0][1])
            sub_lines[8] = str(values["size"][0][2])
            sub_lines[9] = str(values["size"][1][0])
            sub_lines[10] = str(values["size"][1][1])
            sub_lines[11] = str(values["size"][1][2])
        if values.get("color"):
            sub_lines[24] = str(values["color"][0])
            sub_lines[25] = str(values["color"][1])
            sub_lines[26] = str(values["color"][2])
        result_entities.append("|".join(sub_lines))

with open("entity.dat", "wb") as f:
    f.write("\n".join(result_entities).encode("utf8"))

print(json.dumps(classes, indent=2))
