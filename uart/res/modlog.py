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


""" Module logger package. """

class LogLevel(object):
    """ Log level object. """

    def __init__(self, string, depth):
        """ Constructor.
            @param string (str) String representation of the log class.
            @param depth (int) Visibility depth: the lowest the most visible. """
        self._string = string
        self._depth = depth

    def string(self):
        """ String representation accessor.
            @return (str) String representation. """
        return self._string

    def depth(self):
        """ Visibility depth accessor: the lowest the most visible.
            @return (int) Visibility depth. """
        return self._depth

# Regular log levels
FATAL_ERROR = LogLevel("FATAL ERROR",   10)         # pylint: disable=bad-whitespace
ERROR       = LogLevel("ERROR",         100)        # pylint: disable=bad-whitespace
WARNING     = LogLevel("WARNING",       1000)       # pylint: disable=bad-whitespace
INFO        = LogLevel("INFO",          10000)      # pylint: disable=bad-whitespace
TRACE       = LogLevel("TRACE",         100000)     # pylint: disable=bad-whitespace
DEBUG       = LogLevel("DEBUG",         1000000)    # pylint: disable=bad-whitespace

class Engine(object):
    """ Log engine: receives logs and filter them out depending on a filter. """

    def __init__(self):
        """ Constructor. """
        self._filter = {}

    def set_filter(self, module, level):
        """ Configure the filter for the current module.
            @param module (str) Log module name.
            @param level (LogLevel) Maximum log level to display. """
        self._filter[module] = level

    def remove_filter(self, module):
        """ Filter removal.
            @param module (str) Log module name. """
        del self._filter[module]

    def is_enabled(self, module, level):
        """ Checks whether logging is enabled for the given module and level.
            @param module (str) Log module name.
            @param level (LogLevel) Log level being tested.
            @return (bool) True if logging is enabled, False otherwise. """
        return self._filter.has_key(module) and (level.depth() <= self._filter[module].depth())

    def log(self, module, level, message):
        """ Prints out a log message. If the level if an error, a warning or a regular info, it is directly printed out.
            @param module (str) Log class.
            @param level (LogLevel) Log level: FATAL_ERROR, ERROR, WARNING, INFO, TRACE, DEBUG
            @param message (str|Exception) Log message or exception. """
        if isinstance(message, Exception):
            # In case message is actually an exception, print out the stack trace
            _exception = message
            if self.is_enabled(module, level):
                import traceback
                traceback.print_exc(_exception)
        else:
            # Unicode processing before output : unicode strings when output through file redirection raise UnicodeEncodeError exceptions.
            if isinstance(message, unicode):
                # Automatically encode in utf-8
                import codecs
                message = codecs.lookup("utf-8").encode(message)[0]
            # Direct printing
            import sys
            if level.depth() < ERROR.depth():
                sys.stderr.write("Fatal error! %s\n" % message)
            elif level.depth() < WARNING.depth():
                sys.stderr.write("Error! %s\n" % message)
            elif level.depth() < INFO.depth():
                sys.stderr.write("Warning! %s\n" % message)
            elif level.depth() < TRACE.depth():
                sys.stderr.write("Info! %s\n" % message)
            # Log filtering
            if self.is_enabled(module, level):
                if level.depth() > INFO.depth():
                    sys.stderr.write("<%s|%s> %s\n" % (module, level.string(), message))

_ENGINE = Engine()
def engine():
    """ Log engine singleton.
        @return (Engine) Log engine instance. """
    return _ENGINE

def log(module, level, message):
    """ Prints out a log message.
        @param module (str) Log class.
        @param level (LogLevel) Log level: FATAL_ERROR, ERROR, WARNING, INFO, TRACE, DEBUG
        @param message (str) Log message. """
    engine().log(module, level, message)

