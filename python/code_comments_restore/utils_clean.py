import re

white_space = re.compile(r"(\t| )+\n")

pattern = re.compile(
    r'//.*?$|/\*.*?\*/|\'(?:\\.|[^\\\'])*\'|"(?:\\.|[^\\"])*"', re.DOTALL | re.MULTILINE
)

pattern_white_space = re.compile(r"\s+")


def removeCCppComment(text):

    def replacer(match):
        s = match.group(0)
        # Matched string is
        # //...EOL or /*...*/  ==> Blot out all non-newline chars
        if s.startswith("/"):
            return " "
        # Matched string is '...' or "..."  ==> Keep unchanged
        else:
            return s

    return re.sub(pattern, replacer, text)


def get_hash(text, strip):
    cleaned_up_text = removeCCppComment(text)
    # drop whitespaces
    cleaned_up_text = re.sub(pattern_white_space, "\n", cleaned_up_text)
    if strip:
        # for compare case
        return cleaned_up_text.strip()
    else:
        # for search patern case
        return cleaned_up_text


def replace_space_at_start(text):
    splited = text.split("\n")
    for i in range(len(splited)):
        pos = 0
        while pos < len(splited[i]) and splited[i][pos] == " ":
            pos = pos + 1
        if pos > 0:
            tabs = int(pos / 8)
            splited[i] = ("\t" * tabs) + splited[i][tabs * 8 :]
    return "\n".join(splited)


def cleanup(text):
    text = re.sub(white_space, "\n", text)
    return replace_space_at_start(text)


def select_open_close(buf):
    brakets = {
        "[": 0,
        "{": 0,
        "(": 0,
    }
    pos = 0
    str_len = len(buf)
    have_func_bracket = False
    while pos < str_len:
        if buf[pos] == "{":
            have_func_bracket = True
        if buf[pos] in brakets:
            brakets[buf[pos]] += 1
            pos += 1
        elif buf[pos] == '"':
            end_pos = buf.find('"', pos + 1)
            if end_pos == -1:
                return buf, ""
            else:
                pos = end_pos
            pos += 1
        elif buf[pos] == "}":
            brakets["{"] -= 1
            pos += 1
        elif buf[pos] == "]":
            brakets["["] -= 1
            pos += 1
        elif buf[pos] == ")":
            brakets["("] -= 1
            pos += 1
        elif buf[pos : pos + 2] == "//":
            end_pos = buf.find("\n", pos + 2)
            if end_pos == -1:
                return buf, ""
            else:
                pos = end_pos + 1
        elif buf[pos] == "#":
            end_pos = buf.find("\n", pos + 1)
            if end_pos == -1:
                return buf, ""
            else:
                pos = end_pos + 1
        elif buf[pos : pos + 2] == "/*":
            end_pos = buf.find("*/", pos + 2)
            if end_pos == -1:
                return buf, ""
            else:
                pos = end_pos + 2
        else:
            pos += 1

        all_closed = have_func_bracket
        for ch in brakets:
            if brakets[ch] > 0:
                all_closed = False

        if all_closed:
            break

    return buf[:pos], buf[pos:]


def split_blocks(buf):
    result = []
    while buf:
        block, buf = select_open_close(buf)
        result.append(block)
    return result
