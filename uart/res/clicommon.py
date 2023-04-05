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


""" CLI common lib. """

import sys
import codecs
import modlog


class Cache(object):
    """ Pre-computations. """

    def __init__(self):
        """ Constructor. """
        # All nodes
        self.xml_nodes = {}
        # Main information
        self.cli_name = None # Name of the CLI
        self.page_title = None # Page title
        # Menus
        self.menus = [] # Array of cli:cli or cli:menu XML nodes.

    @staticmethod
    def execute(ctx):
        """ Realizes computations.
            @param ctx (Cli2xxx) Execution context. """
        ctx.cache.menu_refs = [] # Array of cli:menu[@ref] XML nodes.
        ctx.cache.endls = [] # Array of cli:endl XML nodes.
        ctx.cache.tag_ids = [] # Array of cli:tag[@id] XML nodes.
        ctx.cache.tag_refs = [] # Array of cli:tag[@ref] XML nodes.

        ctx.cache.read_all_nodes(ctx)
        ctx.cache.compute_main_infos(ctx)
        ctx.cache.compute_menus(ctx)
        ctx.cache.compute_command_lines(ctx)
        ctx.cache.compute_tags(ctx)

        del ctx.cache.menu_refs
        del ctx.cache.endls
        del ctx.cache.tag_ids
        del ctx.cache.tag_refs

    @staticmethod
    def read_all_nodes(ctx, xml_node=None):
        """ Read all XML nodes and cache everything is worth.
            @param ctx (Cli2xxx) Execution context.
            @param xml_node (XML node) Current XML node. None for the top call. """
        if xml_node is None:
            # First of all, determine whether xml_nodes cache should be set
            setattr(ctx.xml.root_node(), "cli_Cli2xxx_cache_test", True)
            if getattr(ctx.xml.root_node(), "cli_Cli2xxx_cache_test", None) is None:
                # The XML framework does not return the same native instances for a single node.
                # Caching needs to be done for every node so that we rely on constant instances for caching.
                ctx.cache.xml_nodes = {}
            else:
                ctx.cache.xml_nodes = None

            ctx.cache.global_node_index = 0
            ctx.cache.read_all_nodes(ctx, ctx.xml.root_node())
            ctx.cache.node(ctx.xml.root_node()).cli_Cli2xxx_child_index = 0
            del ctx.cache.global_node_index
        else:
            if ctx.xml.is_node(xml_node, "cli:*"):
                if ctx.cache.xml_nodes is not None:
                    # Remember a unique instance for each node
                    ctx.cache.xml_nodes[xml_node] = xml_node
                # Compute the global node identifier
                xml_node.cli_Cli2xxx_global_index = ctx.cache.global_node_index
                ctx.cache.global_node_index += 1
                # Memorize menu nodes
                if ctx.xml.is_node(xml_node, "cli:cli") or ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@name"):
                    ctx.cache.menus.append(xml_node)
                # Reserve cli:menu[@ref] nodes
                elif ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@ref"):
                    ctx.cache.menu_refs.append(xml_node)
                # Reserve cli:endl nodes
                elif ctx.xml.is_node(xml_node, "cli:endl"):
                    ctx.cache.endls.append(xml_node)
                # Reserve cli:tag nodes
                elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                    ctx.cache.tag_ids.append(xml_node)
                elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                    ctx.cache.tag_refs.append(xml_node)
                # Recursive call
                _i_child_node_index = 0
                for _xml_child in ctx.xml.children(xml_node):
                    if ctx.xml.is_node(_xml_child, "cli:*"):
                        _xml_child.cli_Cli2xxx_child_index = _i_child_node_index
                        _i_child_node_index += 1
                        ctx.cache.read_all_nodes(ctx, _xml_child)

    @staticmethod
    def compute_main_infos(ctx):
        """ Cache main information.
            @param ctx (Cli2xxx) Execution context. """
        if ctx.__class__.__name__ == "Cli2Help":
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Caching main infos...")
            ctx.cache.cli_name = ctx.xml.attr_value(ctx.xml.root_node(), "@name")
            ctx.cache.page_title = ("%s - %s" % (ctx.cache.cli_name, ctx.Utils.translate(ctx, "Command Line Interface documentation")))

    @staticmethod
    def compute_menus(ctx):
        """ Cache cli:cli or cli:menu list.
            @param ctx (Cli2xxx) Execution context. """
        modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Caching menu list...")

        # ctx.cache.menus already computed. See read_all_nodes().
        #ctx.cache.menus.extend(ctx.xml.xpath_set(None, "/cli:cli"))
        #ctx.cache.menus.extend(ctx.xml.xpath_set(None, "//cli:menu[@name]"))

        # For each cli:menu[@ref], find out the respective menu.
        for _xml_menu_ref in ctx.cache.menu_refs:
            _menu_ref_name = ctx.xml.attr_value(_xml_menu_ref, "@ref")
            if _menu_ref_name is None:
                # If _xml_menu_ref, it must be because it has an @ref attribute!
                ctx.Utils.abort(ctx, _xml_menu_ref, "Internal error")
            for _xml_menu in ctx.cache.menus:
                _menu_name = ctx.xml.attr_value(_xml_menu, "@name")
                if _menu_name is None:
                    ctx.Utils.abort(ctx, _xml_menu, "Missing '@name' attribute for menu")
                if _menu_name == _menu_ref_name:
                    # Menu reference found
                    _xml_menu_ref.cli_Cli2xxx_menu = _xml_menu
            # Eventually check the menu reference has been resolved
            if getattr(_xml_menu_ref, "cli_Cli2xxx_menu", None) is None:
                ctx.Utils.abort(ctx, _xml_menu_ref, "No such menu '%s'" % _menu_ref_name)

    @staticmethod
    def compute_command_lines(ctx):
        """ Cache command line informations. One cli:endl by command line.
            Command lines are attached to their respective menu.
            @param ctx (Cli2xxx) Execution context. """
        modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Caching command lines...")

        # For each menu, create a command list.
        for _xml_menu in ctx.cache.menus:
            _xml_menu.cli_Cli2xxx_commands = []

        # For each cli:tag[@id], find-out the respective menu.
        for _xml_endl in ctx.cache.endls:
            _xml_menu = ctx.cache.owner_menu(ctx, _xml_endl)
            _xml_menu.cli_Cli2xxx_commands.append(_xml_endl)

    @staticmethod
    def compute_tags(ctx):
        """ Cache tag informations.
            @param ctx (Cli2xxx) Execution context. """
        modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Caching tag informations...")

        # For each menu, create tag caches.
        for _xml_menu in ctx.cache.menus:
            _xml_menu.cli_Cli2xxx_tag_ids = {}
            _xml_menu.cli_Cli2xxx_tag_refs = []

        # For each cli:tag[@id] nodes, find out the respective menu.
        for _xml_tag in ctx.cache.tag_ids:
            _xml_menu = ctx.cache.owner_menu(ctx, _xml_tag)
            _tag_id = ctx.xml.attr_value(_xml_tag, "@id")
            _xml_menu.cli_Cli2xxx_tag_ids[_tag_id] = _xml_tag
            _xml_tag.cli_Cli2xxx_tag_refs = []
            if ctx.xml.attr_value(_xml_tag, "@hollow") != "yes":
                # Self reference anticipation.
                _xml_tag.cli_Cli2xxx_tag_refs.append(_xml_tag)

        # For each cli:tag[@ref], find out the respective menu and cli:tag[@id].
        for _xml_tag in ctx.cache.tag_refs:
            _xml_menu = ctx.cache.owner_menu(ctx, _xml_tag)
            _xml_menu.cli_Cli2xxx_tag_refs.append(_xml_tag)
            _tag_id = ctx.xml.attr_value(_xml_tag, "@ref")
            if _xml_menu.cli_Cli2xxx_tag_ids.has_key(_tag_id):
                _xml_tag.cli_Cli2xxx_tag_id = _xml_menu.cli_Cli2xxx_tag_ids[_tag_id]
                _xml_tag.cli_Cli2xxx_tag_id.cli_Cli2xxx_tag_refs.append(_xml_tag)
            else:
                ctx.Utils.abort(ctx, _xml_tag, "No such tag identifier '%s'" % _tag_id)

        # For each tag identifier, find out the latest common node.
        for _xml_menu in ctx.cache.menus:
            for _tag_id in _xml_menu.cli_Cli2xxx_tag_ids.keys():
                _xml_tag_id = _xml_menu.cli_Cli2xxx_tag_ids[_tag_id]
                _xml_common_parents = None
                for _xml_tag_ref in _xml_tag_id.cli_Cli2xxx_tag_refs:
                    # Compute list of parents.
                    _xml_parents = []
                    _xml_parent = _xml_tag_ref
                    while _xml_parent is not None:
                        _xml_parents.append(_xml_parent)
                        if _xml_parent != _xml_menu:
                            _xml_parent = ctx.xml.parent_node(_xml_parent)
                        else:
                            _xml_parent = None
                    _xml_parents.reverse()
                    # Compute the intersection with common parents computed until then.
                    if _xml_common_parents is None:
                        _xml_common_parents = _xml_parents
                    else:
                        _xml_common_parents2 = []
                        for _xml_parent in _xml_common_parents:
                            if _xml_parent in _xml_parents:
                                _xml_common_parents2.append(_xml_parent)
                            else:
                                break
                        _xml_common_parents = _xml_common_parents2
                    # Determine whether this is a backward tag[@ref]
                    _xml_tag_ref.cli_Cli2xxx_is_backward = ctx.xml.is_ancestor_or_self(_xml_tag_id, _xml_tag_ref)
                # Store the latest common parent computed
                if _xml_common_parents is None:
                    modlog.log(ctx.MODLOG_FILTER, modlog.WARNING, "No tag reference for tag '%s'" % _tag_id)
                    _xml_tag_id.cli_Cli2xxx_common_parent = None
                elif len(_xml_common_parents) > 0:
                    _xml_tag_id.cli_Cli2xxx_common_parent = ctx.cache.node(_xml_common_parents[-1])

    @staticmethod
    def owner_menu(ctx, xml_node):
        """ Computes the cli:cli or cli:menu owner menu that contains the given node.
            Returns the instance used for cached data.
            @param ctx (Cli2xxx) Execution context.
            @param xml_node (XML node) Node to compute the owner menu for.
            @return (XML node) cli:cli or cli:menu owner menu. Abortion if anything wrong occurred. """
        _xml_parent = ctx.xml.parent_node(xml_node)
        while _xml_parent is not None:
            if ctx.xml.is_node(_xml_parent, "cli:cli") or ctx.xml.is_node_with_attr(_xml_parent, "cli:menu", "@name"):
                return ctx.cache.node(_xml_parent)
            _xml_parent = ctx.xml.parent_node(_xml_parent)
        ctx.Utils.abort(ctx, xml_node, "No parent menu found for node.")

    def node(self, xml_node):
        """ Returns the XML node instance that stores cached data.
            This method exists because some XML parsers (libxml2 for instance) do not always return the same python instance for a same XML node.
            @param xml_node (XML node) XML node.
            @return (XML node) XML node that stores cached data for that XML node. """
        if self.xml_nodes is None:
            # No cache
            return xml_node
        else:
            return self.xml_nodes[xml_node]

class Output(object):
    """ Generic transformation output class. """

    MODLOG_FILTER = "clicommon.Output"

    def __init__(self):
        """ Constructor. """
        #modlog.engine().set_filter(self.MODLOG_FILTER, modlog.DEBUG)
        self._codec = None
        self._output_buffer = ""

    def set_encoding(self, encoding):
        """ Automatic encoding setter.
            @param encoding (str) Encoding name. """
        try:
            self._codec = codecs.lookup(encoding)
        except LookupError:
            modlog.log(self.MODLOG_FILTER, modlog.WARNING, "Unknown encoding '%s'" % encoding)

    def put(self, text):
        """ Push text for output.
            @param text (str) Output text.
            @return (Output) Self.  """
        #modlog.log(self.MODLOG_FILTER, modlog.DEBUG, "put(text = '%s')" % text)
        if self._codec is not None:
            text = self._codec.encode(text)[0]
        self._output_buffer += text
        return self

    def endl(self):
        """ Push an end of line.
            @return (Output) Self. """
        return self.put("\n")

    def flush(self):
        """ Final writing of the prepared output.
            @return (boolean) True for success, False otherwise. """
        _res = False
        if self._open():
            self._put(self._output_buffer)
            self._output_buffer = ""
            _res = self._close()
        return _res
    def _open(self):
        """ Overridable output stream opening method.
            @return (boolean) True for success, False otherwise. """
        # Equivalent to True, but prevents pylint warnings:
        # - R0201(no-self-use): Method could be a function
        #return True
        return self is not None
    def _put(self, text):
        """ Overridable line output method.
            @param text (str) Output text.
            @return (boolean) True for success, False otherwise.  """
        # Replacement for True, prevents pylint warnings:
        # R0201(no-self-use): Method could be a function
        # W0613(unused-argument): Unused argument 'text'
        #return True
        return (self is not None) and (text is not None)
    def _close(self):
        """ Overridable output stream closure method.
            @return (boolean) True for success, False otherwise.  """
        # Equivalent to True, but prevents pylint warnings:
        # - R0201(no-self-use): Method could be a function
        #return True
        return self is not None

class Console(Output):
    """ Output result of transformation in the console. """

    def __init__(self):
        """ Constructor. """
        Output.__init__(self)

    def _put(self, text):
        """ Output text in the console.
            @param text (str) Output text.
            @return (boolean) True for success, False otherwise. """
        sys.stdout.write(text)
        return True

class OutputFile(Output):
    """ Output result of trnasformation in an output file. """

    def __init__(self, output_path):
        """ Constructor.
            @param output_path (str) Output path. """
        Output.__init__(self)
        self._output_path = output_path
        self._output_file = None

    def _open(self):
        """ Open the output file for writing.
            @return (boolean) True for success, False otherwise. """
        if self._output_file is None:
            self._output_file = open(self._output_path, "w")
            return self._output_file is not None
        return False

    def _put(self, text):
        """ Output text in the console.
            @param text (str) Output text.
            @return (boolean) True for success, False otherwise. """
        self._output_file.write(text)
        return True

    def _close(self):
        """ Close the output file.
            @return (boolean) True for success, False otherwise. """
        self._output_file.close()
        self._output_file = None
        return True

class CtxUtils(object):
    """ Common utils routines. """

    def __init__(self):
        """ Constructor. """
        # Static class, nothing to be done
        pass

    @staticmethod
    def version():
        """ Version accessor.
            @return (str) Program version. """
        return "CLI library %s (%s, %s)" % (CtxUtils.short_version(), CtxUtils.author_name(), CtxUtils.url())

    @staticmethod
    def short_version():
        """ Short version accessor.
            @return (str) Short program version. """
        return "2.9"

    @staticmethod
    def author_name():
        """ Author name accessor.
            @return (str) Author name. """
        return "Alexis Royer"

    @staticmethod
    def url():
        """ URL accessor.
            @return (str) URL. """
        return "http://alexis.royer.free.fr/CLI/"

    @staticmethod
    def node_location(ctx, xml_node):
        """ Computes a location for the given node.
            @param ctx (Cli2xxx) Execution context.
            @param xml_node (XML node) Node to compute a location for.
            @return (str) Node location. """
        if xml_node is None:
            return "(root)"
        elif ctx.xml.is_node(xml_node, "cli:cli"):
            return "cli:cli"
        elif ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@name"):
            return "cli:menu[@name='%s']" % ctx.xml.attr_value(xml_node, "@name")
        elif ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@ref"):
            return "cli:menu[@ref='%s']" % ctx.xml.attr_value(xml_node, "@ref")
        elif ctx.xml.is_node_with_attr(xml_node, "cli:keyword", "@string"):
            return "cli:keyword[@string='%s']" % ctx.xml.attr_value(xml_node, "@string")
        elif ctx.xml.is_node_with_attr(xml_node, "cli:param", "@id"):
            return "cli:param[@id='%s']" % ctx.xml.attr_value(xml_node, "@id")
        elif ctx.xml.is_node_with_attr(xml_node, "cli:param", "@ref"):
            return "cli:param[@ref='%s']" % ctx.xml.attr_value(xml_node, "@ref")
        elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
            return "cli:tag[@id='%s']" % ctx.xml.attr_value(xml_node, "@id")
        elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
            return "cli:tag[@ref='%s']" % ctx.xml.attr_value(xml_node, "@ref")
        else:
            return ctx.xml.node_name(xml_node)

    class ExecutionStoppedException(Exception):
        """ Exception raised when the transformation execution has stopped because of an error. """

    @staticmethod
    def abort(ctx, xml_node, message):
        """ Abort current execution.
            @param ctx (Cli2xxx) Execution context.
            @param xml_node (XML node) Context node.
            @param message (str) Error message.
            @throw (...) """
        modlog.log(ctx.MODLOG_FILTER, modlog.ERROR, ctx.Utils.node_location(ctx, xml_node) + ": " + message)
        raise ctx.Utils.ExecutionStoppedException()

    @staticmethod
    def warn(ctx, xml_node, message):
        """ Display warning.
            @param ctx (Cli2xxx) Execution context.
            @param xml_node (XML node) Context node.
            @param message (str) Warning message. """
        modlog.log(ctx.MODLOG_FILTER, modlog.WARNING, ctx.Utils.node_location(ctx, xml_node) + ": " + message)
