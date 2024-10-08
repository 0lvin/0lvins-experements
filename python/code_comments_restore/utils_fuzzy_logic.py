import utils_clean


def search_part_with_same_hash_head(text, hash_text, diff_len):
    stop = len(hash_text)
    # search start of magic
    start_hash = utils_clean.get_hash(text[:stop], False).strip()
    full_len = len(text) - diff_len
    while start_hash != hash_text:
        if full_len <= stop:
            return None
        last_stop = stop
        if len(hash_text) > len(start_hash):
            # try skip difference between hashes
            stop = stop + len(hash_text) - len(start_hash)
        else:
            if text[stop] not in ("/", "\n", "*"):
                for i in range(stop, len(text)):
                    if text[i] in ("/", "\n", "*"):
                        stop = i - 1
                        break
            # looks like not hashable
            stop = stop + 1
        start_hash = utils_clean.get_hash(text[:stop], False).strip()
        if start_hash[: len(hash_text)] == hash_text:
            # slowdown search
            stop = last_stop + 1
            start_hash = utils_clean.get_hash(text[:stop], False).strip()
        print(f"{100 * stop / len(text)}%\r", end="")
    return text[:stop]


def search_part_with_same_hash_tail(text, hash_text, diff_len):
    stop = len(text) - len(hash_text)
    # search start of magic
    start_hash = utils_clean.get_hash(text[stop:], False)
    while start_hash.strip() != hash_text:
        if stop <= diff_len:
            return None
        if len(hash_text) > len(start_hash):
            # try skip difference between hashes
            stop = stop - (len(hash_text) - len(start_hash))
        else:
            # looks like not hashable
            stop = stop - 1
        start_hash = utils_clean.get_hash(text[stop:], False)
        print(f"{100 * stop / len(text)}%\r", end="")
    return text[stop:]


def optimize_head(src_text, dst_text):
    src_hash = utils_clean.get_hash(src_text, True)
    dst_hash = utils_clean.get_hash(dst_text, True)
    common_hash = None
    for i in range(min(len(src_hash), len(dst_hash))):
        if src_hash[i] != dst_hash[i]:
            common_hash = src_hash[: i - 1]
            break
    if common_hash:
        common_hash = common_hash.strip()
    while True:
        if common_hash:
            print(f"common head {100 * len(common_hash) / len(dst_hash)}%\r", end="")
            common_hash_dst = None
            common_hash_src = search_part_with_same_hash_head(
                src_text, common_hash, len(src_hash) - len(common_hash)
            )
            if common_hash_src:
                if dst_text[: len(common_hash_src)] == common_hash_src:
                    common_hash_dst = common_hash_src
                else:
                    common_hash_dst = search_part_with_same_hash_head(
                        dst_text, common_hash, len(dst_hash) - len(common_hash)
                    )
            if common_hash_dst and common_hash_src:
                if len(common_hash_dst) != len(common_hash_src):
                    return common_hash_src + dst_text[len(common_hash_dst) :]
                return dst_text
            # looks as too long hash
            found_something = False
            for i in range(1, len(common_hash)):
                if common_hash[-i] == "\n":
                    common_hash = common_hash[:-i].strip()
                    found_something = True
                    break
            if not found_something:
                print("No common head\r", end="")
                return dst_text
            print(
                f"try shorter common head {100 * len(common_hash) / len(dst_hash)}%\r",
                end="",
            )
        else:
            print("no common head\r", end="")
            return dst_text


def optimize_tail(src_text, dst_text):
    src_hash = utils_clean.get_hash(src_text, True)
    dst_hash = utils_clean.get_hash(dst_text, True)
    common_hash = None
    for i in range(1, min(len(src_hash), len(dst_hash))):
        if src_hash[-i] != dst_hash[-i]:
            if i > 1:
                common_hash = src_hash[-i + 1 :]
            break
    while True:
        found_something = False
        if common_hash:
            if common_hash.find("\n") != -1:
                found_something = True
                common_hash = common_hash[common_hash.find("\n") :].strip()
        if common_hash:
            print(f"common tail {100 * len(common_hash) / len(dst_hash)}%\r", end="")
            common_hash_dst = None
            common_hash_src = search_part_with_same_hash_tail(
                src_text, common_hash, len(src_hash) - len(common_hash)
            )
            if common_hash_src:
                if dst_text[: -len(common_hash_src)] == common_hash_src:
                    common_hash_dst = common_hash_src
                else:
                    common_hash_dst = search_part_with_same_hash_tail(
                        dst_text, common_hash, len(dst_hash) - len(common_hash)
                    )
            if common_hash_dst and common_hash_src:
                if len(common_hash_dst) != len(common_hash_src):
                    return dst_text[: -len(common_hash_dst)] + common_hash_src
                return dst_text
            if not found_something:
                print("No common tail\r", end="")
                return dst_text
            # looks as too long hash
            print(
                f"try shorter common tail {100 * len(common_hash) / len(dst_hash)}%\r",
                end="",
            )
        else:
            print(f"no comomn tail\r", end="")
            return dst_text


def file_hash(name):
    with open(name, "rb") as file_desc:
        return utils_clean.get_hash(file_desc.read().decode("utf8"), True)
