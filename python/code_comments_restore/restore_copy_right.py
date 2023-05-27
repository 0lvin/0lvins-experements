import os
import sys

COPYRIGHT_COMMENT = """
//
// Heretic II
// Copyright 1998 Raven Software
//
"""


def process(src_inode):
    if os.path.isfile(src_inode):
        process_file(src_inode)
    else:
        for dirname, _, filenames in os.walk(src_inode):
            # print path to all filenames.
            for filename in filenames:
                if (
                    filename.endswith('.c') or
                    filename.endswith('.cpp') or
                    filename.endswith('.h')
                ):
                    src_file = os.path.join(dirname, filename)
                    print (f"Process {src_file}")

                    content = ""
                    with open(src_file, "rb") as f:
                        try:
                            content = f.read().decode("utf8").strip()
                        except Exception as e:
                            print (f"Couldn't parse {e}")
                            continue

                    if (
                        "Copyright (C)" in content or
                        "LICENSE: Public domain" in content or
                        "Greg Kennedy" in content or
                        "Raven Software" in content or
                        "Public Domain" in content or
                        "GNU General Public License" in content or
                        "RAD Game Tools and Valve Software" in content or
                        "This software is in the public domain." in content or
                        "Daniel Gibson" in content or
                        "Yamagi Burmeister" in content or
                        "Rich Geldreich" in content or
                        "Id Software, Inc." in content
                    ):
                        continue

                    content = COPYRIGHT_COMMENT + content

                    content = "\n".join([
                        line.rstrip() for line in content.split("\n")
                    ])
                    content = content.strip() + "\n"

                    with open(src_file, "wb") as f:
                        f.write(content.encode("utf8"))

                    print (f"Updated with copyright {src_file}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print ("""
            usage: python restore_copy_right.py <dir>
            Restore Raven copyrights
        """)
    else:
        process(sys.argv[1])
