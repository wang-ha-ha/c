#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2006-2018, Alexis Royer, http://alexis.royer.free.fr/CLI
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation
#       and/or other materials provided with the distribution.
#     * Neither the name of the CLI library project nor the names of its contributors may be used to endorse or promote products derived from this software
#       without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


""" CLI to C++ compiler. """

# Imports
import sys
import os
import re
import modlog


MODLOG_FILTER = "clean_outlog"
#modlog.engine().set_filter(MODLOG_FILTER, modlog.DEBUG)


# Print usage
def print_usage():
    """ Print usage. """
    print "clean_outlog.py log_file"

# Checking arguments
if len(sys.argv) != 2:
    print_usage()
    sys.exit(1)
CLI_LOG = sys.argv[1]
if not os.path.isfile(CLI_LOG):
    print "ERROR: No such file '%s'" % (CLI_LOG)
    print_usage()
    sys.exit(1)

def debug_line(i_line, str_line):
    """ Print out the given line for debug. """
    modlog.log(
        MODLOG_FILTER, modlog.DEBUG,
        "  line[%d] = '%s'" % (
            i_line,
            str_line.replace("\b", "\\b").replace("\r", "\\r").replace("\n", "\\n")
        )
    )

def main():
    """ Main function. """

    # Get the file content
    input_file = open(CLI_LOG, "rb")
    arstr_lines = input_file.readlines()
    input_file.close()

    # Analyze each line
    i_line = 0
    while i_line < len(arstr_lines):
        modlog.log(MODLOG_FILTER, modlog.DEBUG, "Line %d:" % (i_line))
        str_line = arstr_lines[i_line]
        debug_line(i_line, str_line)

        # Remove end of lines
        modlog.log(MODLOG_FILTER, modlog.DEBUG, " Removing end of line:")
        arstr_lines[i_line] = str_line = str_line.replace("\r", "").replace("\n", "")
        debug_line(i_line, str_line)

        # Telnet special characters
        modlog.log(MODLOG_FILTER, modlog.DEBUG, " Processing telnet special characters:")
        arstr_lines[i_line] = str_line = str_line.replace("\xa7", "ç").replace("\xc3ç", "ç")
        arstr_lines[i_line] = str_line = str_line.replace("\xa9", "é").replace("\xc3é", "é")
        debug_line(i_line, str_line)

        # Remove traces menu (debug mode)
        b_remove_line = False
        if re.search(r"traces          Traces menu", str_line):
            b_remove_line = True
        if re.search(r"traces          Menu de configuration de traces", str_line):
            b_remove_line = True
        # Remove check menu (debug mode)
        if re.search(r"check           Check CLI stuff", str_line):
            b_remove_line = True
        if re.search(r"check           Vérifications du CLI", str_line):
            b_remove_line = True
        if b_remove_line:
            modlog.log(MODLOG_FILTER, modlog.DEBUG, " Removing debug line!")
            del arstr_lines[i_line]
            continue

        # Backspaces
        while re.search(r".[\b][\b]*  *[\b][\b]*", str_line):
            modlog.log(MODLOG_FILTER, modlog.DEBUG, " Backspaces found!")
            arstr_lines[i_line] = str_line = re.sub(r"(.*)[^\b][\b]([\b]*) ( *)[\b]([\b]*)(.*)", r"\1\2\3\4\5", str_line)
            debug_line(i_line, str_line)

        i_line += 1

    # Eventually write the file over
    output_file = open(CLI_LOG, "wb")
    for str_line in arstr_lines:
        modlog.log(MODLOG_FILTER, modlog.DEBUG, "Writing line")
        debug_line(-1, str_line)
        output_file.write(str_line)
        output_file.write("\n")
    output_file.close()

main()
