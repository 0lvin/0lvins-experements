import os
import sys
import utils_clean
import utils_fuzzy_logic


def process_copy(src_file, dst_file):
    with open(src_file, "rb") as src_desc:
        src_text = src_desc.read().decode("utf8")
    if not src_text:
        return
    with open(dst_file, "rb") as dst_desc:
        dst_text = utils_clean.cleanup(dst_desc.read().decode("utf8"))
    if not dst_text:
        return
    dst_text = utils_clean.cleanup(dst_text)
    if len(dst_text) < len(src_text):
        with open(dst_file, "wb") as dst_desc:
            dst_desc.write(src_text.encode("utf8"))
        print(f"update done: {src_file} => {dst_file}")
    else:
        print(f"update unnecessary: {src_file} => {dst_file}")


def gen_short_hash(long_hash):
    for k, v in (
        ("%\n", "%"),
        ("\n%", "%"),
        ("/\n", "/"),
        ("\n/", "/"),
        ("*\n", "*"),
        ("\n*", "*"),
        ("+\n", "+"),
        ("\n+", "+"),
        ("-\n", "-"),
        ("\n-", "-"),
        ("=\n", "="),
        ("\n=", "="),
        ("[\n", "["),
        ("\n[", "["),
        ("(\n", "("),
        ("\n(", "("),
        (")\n", ")"),
        ("\n)", ")"),
        ("\n}", ""),
        ("{\n", ""),
        ("\n\n", "\n"),
    ):
        while k in long_hash:
            long_hash = long_hash.replace(k, v)
    return long_hash


def process_file(src_file, dst_file, long_blocks, short_blocks):
    if os.access(src_file, os.R_OK) and os.access(dst_file, os.R_OK):
        src_file_hash = utils_fuzzy_logic.file_hash(src_file)
        dst_file_hash = utils_fuzzy_logic.file_hash(dst_file)
        if dst_file_hash == src_file_hash:
            statinfo_src = os.stat(src_file)
            statinfo_dst = os.stat(dst_file)
            if statinfo_dst.st_size != statinfo_src.st_size:
                process_copy(src_file, dst_file)
            else:
                print(f"same {src_file} => {dst_file}")
        else:
            with open(src_file, "rb") as src_desc:
                src_text = src_desc.read().decode("utf8")
            if not src_text:
                return
            with open(dst_file, "rb") as dst_desc:
                dst_text = dst_desc.read().decode("utf8")
            if not dst_text:
                return
            src_blocks = utils_clean.split_blocks(src_text)
            dst_blocks = utils_clean.split_blocks(dst_text)
            for i in range(len(dst_blocks)):
                print(f"Check blocks: {dst_file} #{i}\r", end="")
                if not dst_blocks[i].strip():
                    continue
                dst_block_hash = utils_clean.get_hash(dst_blocks[i], True)
                if not dst_block_hash.strip():
                    continue
                dst_short_hash = gen_short_hash(dst_block_hash)
                # reuse source file
                for block in src_blocks:
                    if not block.strip():
                        continue
                    src_block_hash = utils_clean.get_hash(block, True)
                    if not src_block_hash.strip():
                        continue
                    if dst_block_hash == src_block_hash:
                        dst_blocks[i] = block
                        break
                    else:
                        src_block_hash = gen_short_hash(src_block_hash)
                        if not dst_short_hash.strip():
                            continue
                        if not src_block_hash.strip():
                            continue
                        if dst_short_hash == src_block_hash:
                            dst_blocks[i] = block
                            break
                # reuse code base
                if dst_block_hash in long_blocks:
                    dst_blocks[i] = long_blocks[dst_block_hash]
                elif dst_short_hash in short_blocks:
                    dst_blocks[i] = short_blocks[dst_short_hash]
            dst_text = "".join(dst_blocks)
            print(f"check head\r", end="")
            dst_text = utils_fuzzy_logic.optimize_head(src_text, dst_text)
            print(f"check tail\r", end="")
            dst_text = utils_fuzzy_logic.optimize_tail(src_text, dst_text)
            with open(dst_file, "wb") as dst_desc:
                dst_desc.write(dst_text.encode("utf8"))
            print(f"different: {src_file} => {dst_file}")


def recreate_file(src_file, dst_file, long_blocks, short_blocks):
    if not os.access(src_file, os.R_OK) and os.access(dst_file, os.R_OK):
        with open(dst_file, "rb") as dst_desc:
            dst_text = dst_desc.read().decode("utf8")
        if not dst_text:
            return
        dst_blocks = utils_clean.split_blocks(dst_text)
        for i in range(len(dst_blocks)):
            print(f"Check blocks: {dst_file} #{i}\r", end="")
            if not dst_blocks[i].strip():
                continue
            dst_block_hash = utils_clean.get_hash(dst_blocks[i], True)
            dst_short_hash = gen_short_hash(dst_block_hash)
            if dst_block_hash in long_blocks:
                dst_blocks[i] = long_blocks[dst_block_hash]
            elif dst_short_hash in short_blocks:
                dst_blocks[i] = short_blocks[dst_short_hash]
        dst_text = "".join(dst_blocks)
        with open(dst_file, "wb") as dst_desc:
            dst_desc.write(dst_text.encode("utf8"))
        print(f"recreated: {src_file} => {dst_file}")


def process(src_inode, dst_inode):
    if os.path.isfile(dst_inode) and os.path.isfile(src_inode):
        process_file(src_inode, dst_inode)
    else:
        blocks_count = 0
        block_long_dumps = {}
        block_short_dumps = {}
        # gen dumps
        for dirname, _, filenames in os.walk(src_inode):
            # print path to all filenames.
            for filename in filenames:
                if filename.endswith(".c"):
                    src_file = os.path.join(dirname, filename)
                    with open(src_file, "rb") as src_desc:
                        src_text = src_desc.read().decode("utf8")
                    if not src_text:
                        return
                    src_blocks = utils_clean.split_blocks(src_text)
                    for block in src_blocks:
                        if not block.strip():
                            continue
                        blocks_count += 1
                        long_hash = utils_clean.get_hash(block, True)
                        if not long_hash.strip():
                            continue
                        block_long_dumps[long_hash] = block
                        short_hash = gen_short_hash(long_hash)
                        if not short_hash.strip():
                            continue
                        block_short_dumps[short_hash] = block
        print(f"Found {blocks_count} block samples")
        # compare files
        for dirname, _, filenames in os.walk(src_inode):
            # print path to all filenames.
            for filename in filenames:
                if filename.endswith(".c") or filename.endswith(".h"):
                    src_file = os.path.join(dirname, filename)
                    dst_file = src_file.replace(src_inode, dst_inode)
                    process_file(
                        src_file, dst_file, block_long_dumps, block_short_dumps
                    )

        # recreate files
        for dirname, _, filenames in os.walk(dst_inode):
            # print path to all filenames.
            for filename in filenames:
                if filename.endswith(".c"):
                    dst_file = os.path.join(dirname, filename)
                    src_file = dst_file.replace(dst_inode, src_inode)
                    try:
                        recreate_file(
                            src_file, dst_file, block_long_dumps, block_short_dumps
                        )
                    except Exception as e:
                        print(f"Process of {dst_file} from {src_file} failed with {e}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(
            """
            usage: python restore.py linux-3.4 android_kernel_htc_z4u
            restore comments in other dir by code from origin dir
        """
        )
    else:
        process(sys.argv[1], sys.argv[2])
