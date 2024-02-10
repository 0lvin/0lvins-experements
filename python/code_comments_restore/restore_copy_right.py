import os
import sys

COPYRIGHT_COMMENT = """
/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (c) ZeniMax Media Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

"""

REPLACE_LICENSE = {
"""
/*
 * Copyright (c) ZeniMax Media Inc.
 * Licensed under the GNU General Public License 2.0.
 */

/* =======================================================================
""":
"""
/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 * Copyright (c) ZeniMax Media Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 *
 * =======================================================================
"""
}

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

                    for k, v in REPLACE_LICENSE.items():
                        if k.strip() in content:
                            content = content.replace(k.strip(), v.strip())

                            content = "\n".join([
                                line.rstrip() for line in content.split("\n")
                            ])
                            content = content.strip() + "\n"

                            with open(src_file, "wb") as f:
                                f.write(content.encode("utf8"))

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
                        "Copyright (c) ZeniMax Media Inc." in content or
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
