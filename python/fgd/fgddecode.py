# Implementation of https://book.leveldesignbook.com/appendix/resources/formats/fgd
import sys
import json
import copy


ENTITY_EXAMPLE = "||1.0|1.0|1.0|general|0|0|0|0|0|0|shadow|0|0.0|0.0|0|0|0|0:0|0|0|none||.5|.5|.5"


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
                if len(values[token]) == 1:
                    values[token].insert(0, [.0, .0, .0])
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
                elif value and value[0] == '"':
                    model_path = value.replace('"', '')
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

def convert_classes_to_entity(classes, sub_lines):
    if len(sub_lines) < 27:
        sub_lines += [""] * (27 - len(sub_lines))
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
    return "|".join(sub_lines)


if len(sys.argv) < 2:
    print("fgd is not provided")
    exit(1)

classes = parse_fgd(sys.argv[1])

entities = []
with open("entity.dat", "rb") as f:
    entities = f.read().decode("utf8")
    # remove \r
    entities = entities.replace("\r", "\n").strip().split("\n")

processed_entities = []
result_entities = []
for line in entities:
    sub_lines = line.split("|")
    if len(sub_lines) < 2:
        result_entities.append(line)
    elif sub_lines[0] not in classes:
        result_entities.append(line)
    else:
        processed_entities.append(sub_lines[0])
        result_entities.append(convert_classes_to_entity(classes, sub_lines))


non_processed = list(set(classes.keys()) - set(processed_entities))
if non_processed:
    result_entities.append("")
    result_entities.append("// " + sys.argv[1].replace(".fgd", ""))
    for name in sorted(non_processed):
        if classes[name].get("description"):
            new_entity = ENTITY_EXAMPLE.split("|")
            new_entity[0] = name
            result_entities.append(convert_classes_to_entity(classes, new_entity))

# remove duplicates
strings = []
keys = []
for line in result_entities:
    if line.strip() not in strings or not line:
        # has key
        if "|" in line:
            key = line.strip().split("|")[0].lower()
            # already has such key
            if key in keys:
                continue
            # save to processed key
            keys.append(key)
        # save to resulted file
        strings.append(line.strip())

with open("entity.dat", "wb") as f:
    f.write((("\n".join(strings)).strip() + "\n").encode("utf8"))

# dump all parsed items
print(json.dumps(classes, indent=2))
