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


def process_file(src_file, dst_file):
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
            print(f"check head\r", end="")
            dst_text = utils_fuzzy_logic.optimize_head(src_text, dst_text)
            print(f"check tail\r", end="")
            dst_text = utils_fuzzy_logic.optimize_tail(src_text, dst_text)
            with open(dst_file, "wb") as dst_desc:
                dst_desc.write(dst_text.encode("utf8"))
            print(f"different: {src_file} => {dst_file}")
    else:
        print(f"Look to {src_file} -> {dst_file}")


def process(src_inode, dst_inode):
    if os.path.isfile(dst_inode) and os.path.isfile(src_inode):
        process_file(src_inode, dst_inode)
    else:
        for dirname, _, filenames in os.walk(src_inode):
            # print path to all filenames.
            for filename in filenames:
                if filename.endswith(".c") or filename.endswith(".h"):
                    src_file = os.path.join(dirname, filename)
                    dst_file = src_file.replace(src_inode, dst_inode)
                    process_file(src_file, dst_file)


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
