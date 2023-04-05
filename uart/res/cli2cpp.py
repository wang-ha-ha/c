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
import modlog
import xmldoc
import clicommon


# Main function
def main():
    """ Main function. """
    #modlog.engine().set_filter(Cli2Cpp.MODLOG_FILTER, modlog.DEBUG)
    import sys
    ctx = Cli2Cpp()
    if ctx.args.parse_args(ctx):
        if ctx.xml.load_file(ctx.args.xml_resource_file()):
            ctx.out.set_encoding(ctx.xml.encoding())
            ctx.xml.set_ns_shortcut("cli", "http://alexis.royer.free.fr/CLI")
            if ctx.clic.execute(ctx):
                if ctx.out.flush():
                    sys.exit(0)
    modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Execution error")
    sys.exit(-1)

# Classes

class Args(object):
    """ Program arguments management. """

    def __init__(self):
        """ Constructor. """
        self._res = None

    def parse_args(self, ctx):
        """ Parse program arguments.
            @param ctx (Cli2Cpp) Program execution context.
            @return (bool) True for success. """
        import argparse

        _args = argparse.ArgumentParser(prog="cli2cpp")
        _args.add_argument("--version", action="version", version=("%(prog)s - " + clicommon.CtxUtils.version()))
        _args.add_argument("--cli-class-name", help="Main CLI class name (default is based on CLASS_PREFIX and @name attribute).")
        _args.add_argument("--cli-class-scope", help="Main CLI class scope (default is empty).")
        _static_instance_group = _args.add_mutually_exclusive_group()
        _static_instance_group.add_argument("--static", dest="static_instance", action="store_const", const="True", default="False",
                                            help="Static instance creation")
        _static_instance_group.add_argument("--no-static", dest="static_instance", action="store_const", const="False", default="False",
                                            help="No static instance creation (default)")
        _args.add_argument("--class-prefix", help="Class prefix (default is empty).")
        _args.add_argument("--var-prefix", help="Variable prefix (default is 'm_pcli_').")
        _args.add_argument("--indent", help="Indentation pattern (default is 4 spaces).")
        _args.add_argument("--user-indent", help="User-code indentation pattern (default is '/* > */ ').")
        _args.add_argument("--output", help="Output file (by default the result is printed out in the standard output)")
        _args.add_argument("xml-res", type=file, help="CLI XML resource file")

        self._res = _args.parse_args()
        #modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, vars(self._res))

        if vars(self._res)["output"] is None:
            ctx.out = clicommon.Console()
        else:
            ctx.out = clicommon.OutputFile(vars(self._res)["output"]) # pylint: disable=redefined-variable-type

        return True

    @staticmethod
    def program_name():
        """ Program name accessor.
            @return (str) Program name. """
        return "cli2cpp"

    def cli_class_name(self, ctx):
        """ Main CLI class name.
            @param ctx (Cli2Cpp) Execution context.
            @return (str) Main CLI class name. """
        _cli_class_name = vars(self._res)["cli_class_name"]
        if _cli_class_name is None:
            _xml_cli = ctx.xml.root_node()
            _cli_class_name = self.class_prefix() + ctx.Utils.mk_cpp_name(ctx, None, ctx.xml.attr_value(_xml_cli, "@name"))
        return _cli_class_name

    def cli_class_scope(self):
        """ Main CLI class scope.
            @return (str) Class scope. """
        _class_scope = vars(self._res)["cli_class_scope"]
        if _class_scope is None:
            _class_scope = ""
        return _class_scope

    def is_static_instance(self):
        """ Static instance creation property accessor.
            @return (boolean) True for static instance creation, False otherwise. """
        _static_instance = vars(self._res)["static_instance"]
        _static_instance = (_static_instance == "True")
        return _static_instance

    def class_prefix(self):
        """ Class prefix pattern.
            @return (str) Class prefix pattern. """
        _class_prefix = vars(self._res)["class_prefix"]
        if _class_prefix is None:
            _class_prefix = ""
        return _class_prefix

    def var_prefix(self):
        """ Variable prefix pattern.
            @return (str) Variable prefix pattern. """
        _var_prefix = vars(self._res)["var_prefix"]
        if _var_prefix is None:
            _var_name = "m_pcli_"
        return _var_name

    def indent(self):
        """ Indentation pattern.
            @return (str) Indentation pattern. """
        _indent = vars(self._res)["indent"]
        if _indent is None:
            _indent = "    "
        return _indent

    def user_indent(self):
        """ User-code indentation pattern.
            @return (str) User-code indentation pattern. """
        _user_indent = vars(self._res)["user_indent"]
        if _user_indent is None:
            _user_indent = "/* > */ "
        return _user_indent

    def xml_resource_file(self):
        """ Resource file name accessor.
            @return (str) XML resource file name. """
        return vars(self._res)["xml-res"].name

class Cli2Cpp(object):
    """ CLI XML resource file to C++ transformation. """

    MODLOG_FILTER = "cli2cpp"

    def __init__(self):
        """ Constructor. """
        self.args = Args()
        self.xml = xmldoc.InputDocument()
        self.clic = self
        self.out = None
        self.cache = clicommon.Cache()

    @staticmethod
    def execute(ctx):
        """ XML 2 C++ transformation execution.
            @param ctx (Cli2Cpp) Execution context.
            @return (bool) True for success, False otherwise. """
        modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "execute()")
        try:
            ctx.cache.execute(ctx)
            ctx.Sections.execute(ctx)
            return True
        except ctx.Utils.ExecutionStoppedException, _ese:
            modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, _ese)
        except RuntimeError, _re:
            modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, _re)
            modlog.log(ctx.MODLOG_FILTER, modlog.FATAL_ERROR, str(_re))
        return False

    class Sections(object):
        """ Section generation routines. """

        def __init__(self):
            """ Constructor. """
            # Static class, nothing to be done
            pass

        @staticmethod
        def execute(ctx):
            """ Executes generation for each section.
                @param ctx (Cli2Cpp) Execution context. """
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating comment section...")
            ctx.Sections.comment_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating pre-compiled header section...")
            ctx.Sections.pch_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating head section...")
            ctx.Sections.head_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating include section...")
            ctx.Sections.include_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating type section...")
            ctx.Sections.type_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating vars section...")
            ctx.Sections.vars_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating cli section...")
            ctx.Sections.cli_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating node creation section...")
            ctx.Sections.node_creation_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating body section...")
            ctx.Sections.body_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating tail section...")
            ctx.Sections.tail_section(ctx)
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "ok")

        @staticmethod
        def comment_section(ctx):
            """ Generate comment section.
                @param ctx (Cli2Cpp) Execution context. """
            import datetime
            ctx.out.put("//! @file").endl()
            ctx.out.put("//! @author cli2cpp.py - %s" % ctx.Utils.version()).endl()
            ctx.out.put("//! @date %s" % datetime.datetime.today().isoformat()).endl()
            ctx.out.put("//! @warning File auto-generated by 'cli2cpp.py' - Do not edit!").endl()
            ctx.out.endl()

        @staticmethod
        def pch_section(ctx):
            """ Generate pre-compiled header section.
                @param ctx (Cli2Cpp) Execution context. """
            # Pre-compiled headers
            ctx.out.put("// ----- Pre-compiled headers -----").endl()
            ctx.out.put("#include \"cli/pch.h\"").endl()
            ctx.out.endl()

        @staticmethod
        def head_section(ctx):
            """ Generate head section.
                @param ctx (Cli2Cpp) Execution context. """
            ctx.Utils.extra_source(ctx, xml_node=None, option="head")
            ctx.out.endl()

        @staticmethod
        def include_section(ctx):
            """ Generate include section.
                @param ctx (Cli2Cpp) Execution context. """
            # Other includes
            ctx.out.put("// ----- Includes -----").endl()
            ctx.out.put("#include \"cli/common.h\"").endl()
            ctx.out.endl()
            ctx.Utils.extra_source(ctx, xml_node=None, option="include")
            ctx.out.endl()

        @staticmethod
        def type_section(ctx):
            """ Generate type section.
                @param ctx (Cli2Cpp) Execution context. """
            ctx.Utils.extra_source(ctx, xml_node=None, option="types")
            ctx.out.endl()

        @staticmethod
        def vars_section(ctx):
            """ Generate vars section.
                @param ctx (Cli2Cpp) Execution context. """
            ctx.Utils.extra_source(ctx, xml_node=None, option="vars")
            ctx.out.endl()

        @staticmethod
        def cli_section(ctx):
            """ CLI code generation (main section).
                @param ctx (Cli2Cpp) Execution context. """
            # Cli definition
            ctx.Menu.execute(ctx)

        @staticmethod
        def node_creation_section(ctx):
            """ Node creation code generation.
                @param ctx (Cli2Cpp) Execution context. """
            ctx.out.put("// ----- Node creation -----").endl()
            # CLI object creation
            if ctx.args.is_static_instance():
                _xml_cli = ctx.xml.root_node()
                if ctx.xml.is_node_with_attr(_xml_cli, "cli:cli", "@cpp"):
                    _var_name = ctx.Utils.node2var(ctx, _xml_cli, use_owner_cli=False)
                    ctx.out.put("cli::Cli* %s = & %s;" % (ctx.xml.attr_value(_xml_cli, "@cpp"), _var_name)).endl()
                else:
                    ctx.out.put("%s %s;" % (ctx.Utils.node2class(ctx, _xml_cli), ctx.Utils.node2var(ctx, _xml_cli, menu_with_cli_pointer=False))).endl()
            ctx.out.endl()

        @staticmethod
        def body_section(ctx):
            """ Body section code generation.
                @param ctx (Cli2Cpp) Execution context. """
            ctx.Utils.extra_source(ctx, None, option="body")
            ctx.out.endl()

        @staticmethod
        def tail_section(ctx):
            """ Generate tail section.
                @param ctx (Cli2Cpp) Proggram execution context.
                @return True for success, False otherwise. """
            ctx.Utils.extra_source(ctx, xml_node=None, option="tail")
            ctx.out.endl()
            return True

    class Menu(object):
        """ Cli (which is the main menu) or menu code generation routines. """

        def __init__(self):
            """ Constructor. """
            # Static class, nothing to be done
            pass

        @staticmethod
        def execute(ctx):
            """ Executes the class generation routines for the Cli.
                @param ctx (Cli2Cpp) Execution context. """
            ctx.Menu.menu(ctx, ctx.xml.root_node())

        @staticmethod
        def menu(ctx, xml_menu):
            """ Cli (which is the main menu) or menu code generation.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_menu (XML node) cli:cli or cli:menu node.
                @return True for success, False otherwise. """
            _class_name = ctx.Utils.node2class(ctx, xml_menu)
            _super_class = "cli::Menu"

            if ctx.xml.is_node(xml_menu, "cli:cli"):
                ctx.Utils.indent(ctx, xml_menu, 0).put("// ----- Cli class definition -----").endl()
                _super_class = "cli::Cli"

            ctx.Utils.indent(ctx, xml_menu, 0).put("class %s : public %s {" % (_class_name, _super_class)).endl()

            ctx.Menu.SubMenus.execute(ctx, xml_menu)    # Sub-menus (root-menu root only).
            ctx.Menu.Members.execute(ctx, xml_menu)     # Member declarations.
            ctx.Menu.Constructor.execute(ctx, xml_menu) # Constructor.
            ctx.Menu.Destructor.execute(ctx, xml_menu)  # Destructor.
            ctx.Menu.Populate.execute(ctx, xml_menu)    # Populating nodes
            ctx.Menu.Execute.execute(ctx, xml_menu)     # Menu implementation.
            ctx.Menu.Handlers.execute(ctx, xml_menu)    # Menu handlers.

            ctx.Utils.indent(ctx, xml_menu, 0).put("};").endl()

            if ctx.xml.is_node(xml_menu, "cli:cli"):
                ctx.out.endl()

            return True

        class SubMenus(object):
            """ Menu.SubMenus code generation routines. """

            def __init__(self):
                """ Constructor. """
                # Static class, nothing to be done
                pass

            @staticmethod
            def execute(ctx, xml_menu):
                """ Executes the Menu.SubMenus code generation routines.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Menu.SubMenus.sub_menus(ctx, xml_menu)

            @staticmethod
            def sub_menus(ctx, xml_menu):
                """ Propagate code generation over sub-menus (cli only), and set a reference to the main menu.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                # Sub menus: only for CLI object
                if ctx.xml.is_node(xml_menu, "cli:cli"):
                    _xml_cli = xml_menu
                    ctx.Utils.indent(ctx, _xml_cli, 1).put("// ----- Sub-menus -----").endl()
                    for _xml_sub_menu in ctx.cache.menus:
                        # Filter-out cli:cli XML nodes
                        if ctx.xml.is_node(_xml_sub_menu, "cli:menu"):
                            # Call code generation for the sub-menu.
                            ctx.Menu.menu(ctx, _xml_sub_menu)
                            ctx.out.endl()
                    ctx.out.endl()
                # Any menu: reference to the owner CLI object
                ctx.Utils.indent(ctx, xml_menu, 1).put("// ----- Owner CLI -----").endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("private: %s* m_pcliOwnerCli;" % ctx.args.cli_class_name(ctx)).endl()

        class Members(object):
            """ Menu.Members code generation routines. """

            def __init__(self):
                """ Constructor. """
                # Static class, nothing to be done
                pass

            @staticmethod
            def execute(ctx, xml_menu):
                """ Executes the Menu.Members code generation routines.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Menu.Members.members(ctx, xml_menu)

            @staticmethod
            def members(ctx, xml_menu):
                """ Menu members declaration: menu members (cli only), sub-nodes and extra 'member' source.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                # Menus
                if ctx.xml.is_node(xml_menu, "cli:cli"):
                    _xml_cli = xml_menu
                    ctx.Utils.indent(ctx, _xml_cli, 1).put("// ----- Menus -----").endl()
                    # Self
                    ctx.Menu.Members.declare_node(ctx, _xml_cli, _xml_cli, indent_count=1)
                    # Effective menus
                    for _xml_sub_menu in ctx.cache.menus:
                        # Filter-out cli:cli XML nodes
                        if ctx.xml.is_node(_xml_sub_menu, "cli:menu"):
                            ctx.Menu.Members.declare_node(ctx, _xml_cli, _xml_sub_menu, indent_count=1)

                # Sub-nodes
                ctx.Utils.indent(ctx, xml_menu, 1).put("// ----- Node members -----").endl()
                ctx.Menu.Members.walk(ctx, xml_menu, xml_menu, indent_count=1)

                # Members section
                ctx.Utils.extra_source(ctx, xml_node=xml_menu, option="members")

                ctx.out.endl()

            @staticmethod
            def walk(ctx, xml_menu, xml_node, indent_count=0):
                """ Recursive node declaration routine.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) Current menu being processed.
                    @param xml_node (XML node) Current cli:* node.
                    @param indent_count (int) Indentation depth. """
                for _xml_child in ctx.xml.children(xml_node):
                    if ctx.xml.is_node(_xml_child, "cli:help"):
                        # No member for help nodes.
                        pass
                    elif ctx.xml.is_node(_xml_child, "cli:comment"):
                        # No member for comment nodes.
                        pass
                    elif ctx.xml.is_node(_xml_child, "cli:cpp") or ctx.xml.is_node(_xml_child, "cli:java") or ctx.xml.is_node(_xml_child, "cli:handler"):
                        # No member for source nodes.
                        pass
                    elif ctx.xml.is_node(_xml_child, "cli:menu"):
                        # Menus are declare only in the CLI and have been declared at first. Do not declare them twice.
                        pass
                    elif ctx.xml.is_node_with_attr(_xml_child, "cli:tag", "@ref"):
                        # No member for tag references
                        pass
                    elif ctx.xml.is_node(_xml_child, "cli:*"):
                        ctx.Menu.Members.declare_node(ctx, xml_menu, _xml_child, indent_count=indent_count)
                        ctx.Menu.Members.walk(ctx, xml_menu, _xml_child, indent_count=indent_count+1)

            @staticmethod
            def declare_node(ctx, xml_menu, xml_node, indent_count=0):
                """ Member declaration for a given node.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) Current menu being processed.
                    @param xml_node (XML node) Current cli:* node.
                    @param indent_count (int) Indentation depth. """
                _class_name = ctx.Utils.node2class(ctx, xml_node)
                _var_name = ctx.Utils.node2var(ctx, xml_node, menu_with_cli_pointer=False)
                ctx.Utils.indent(ctx, xml_menu, indent_count).put("private: %s* %s;" % (_class_name, _var_name)).endl()

        class Constructor(object):
            """ Menu.Constructor code generation routines. """

            def __init__(self):
                """ Constructor. """
                # Static class, nothing to be done
                pass

            @staticmethod
            def execute(ctx, xml_menu):
                """ Executes the Menu.Constructor code generation routines.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Menu.Constructor.constructor(ctx, xml_menu)

            @staticmethod
            def constructor(ctx, xml_menu):
                """ Menu constructor code generation.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                if ctx.xml.is_node(xml_menu, "cli:cli"):
                    _str_parent_class = "cli::Cli"
                elif ctx.xml.is_node(xml_menu, "cli:menu"):
                    _str_parent_class = "cli::Menu"
                else:
                    ctx.Utils.abort(ctx, xml_menu, "no parent CLI class for element '%s'" % ctx.xml.node_name(xml_menu))

                ctx.Utils.indent(ctx, xml_menu, 1).put("// ----- Constructor -----").endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("public: explicit %s(void) :" % ctx.Utils.node2class(ctx, xml_menu)).endl()
                # Initialization
                ctx.Utils.indent(ctx, xml_menu, 2).put("%s(" % _str_parent_class)
                ctx.Menu.Constructor.params(ctx, xml_menu)
                ctx.out.put(")").endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("{").endl()

                # Populate
                if ctx.xml.is_node(xml_menu, "cli:cli"):
                    ctx.Utils.indent(ctx, xml_menu, 2).put("Populate();").endl()
                # Constructor section
                ctx.Utils.extra_source(ctx, xml_node=xml_menu, option="constructor")

                ctx.Utils.indent(ctx, xml_menu, 1).put("}").endl()
                ctx.out.endl()

            @staticmethod
            def params(ctx, xml_node):
                """ Creation parameters for a given node.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_node (XML node) Node to output creation parameters for. """
                try:
                    # tag[@id] node only need to know whether they are hollow or not.
                    if ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                        if ctx.xml.is_node_with_attr_value(xml_node, "cli:tag", "@hollow", "yes"):
                            ctx.out.put("true")
                        else:
                            ctx.out.put("false")
                        return True
                    # tag[@ref] only need to be passed their tag[@id] target
                    elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                        _xml_tag = ctx.cache.node(xml_node).cli_Cli2xxx_tag_id
                        if _xml_tag is not None:
                            ctx.out.put("*" + ctx.Utils.node2var(ctx, _xml_tag))
                            return True
                        else:
                            ctx.Utils.abort(ctx, xml_node, "unknown tag reference '%s'" % ctx.xml.attr_value(xml_node, "@ref"))
                    # menu[@ref] only need to be passed their menu[@name] target
                    elif ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@ref"):
                        _xml_menu = ctx.cache.node(xml_node).cli_Cli2xxx_menu
                        if _xml_menu is not None:
                            ctx.out.put("*" + ctx.Utils.node2var(ctx, _xml_menu))
                            return True
                        else:
                            ctx.Utils.abort(ctx, xml_node, "unknown menu reference '%s'" % ctx.xml.attr_value(xml_node, "@ref"))
                    else:
                        _keyword = None
                        if ctx.xml.is_node(xml_node, "cli:cli") or ctx.xml.is_node(xml_node, "cli:menu"):
                            _keyword = ctx.xml.attr_value(xml_node, "@name")
                        if ctx.xml.is_node(xml_node, "cli:keyword"):
                            _keyword = ctx.xml.attr_value(xml_node, "@string")
                        # Determine whether help should be set on that node.
                        _set_help = True
                        if ctx.xml.is_node(xml_node, "cli:tag") or ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@ref"):
                            _set_help = False
                        # Generate "keyword" and help parameters.
                        if _keyword is not None:
                            ctx.out.put("\"%s\"" % _keyword)
                            if _set_help:
                                ctx.out.put(", ")
                        if _set_help:
                            ctx.out.put("cli::Help()")
                            for _xml_help in ctx.xml.children(xml_node):
                                if ctx.xml.is_node(_xml_help, "cli:help"):
                                    ctx.out.put(".AddHelp(")
                                    if ctx.xml.attr_value(_xml_help, "@lang") is None:
                                        modlog.log(ctx.MODLOG_FILTER, modlog.WARNING, "Missing @lang attribute for cli:help node, using 'en' instead.")
                                        ctx.out.put("cli::Help::LANG_EN, ")
                                    elif ctx.xml.attr_value(_xml_help, "@lang") == "en":
                                        ctx.out.put("cli::Help::LANG_EN, ")
                                    elif ctx.xml.attr_value(_xml_help, "@lang") == "fr":
                                        ctx.out.put("cli::Help::LANG_FR, ")
                                    else:
                                        modlog.log(ctx.MODLOG_FILTER, modlog.WARNING, "Unknown @lang value '%s' for cli:help node, using 'en' instead.")
                                        ctx.out.put("cli::Help::LANG_EN, ")
                                    _text = ctx.xml.content(_xml_help)
                                    _text = _text.replace("\\", "\\\\")
                                    _text = _text.replace("\"", "\\\"")
                                    ctx.out.put("\"%s\"" % _text)
                                    ctx.out.put(")")
                finally:
                    pass

        class Destructor(object):
            """ Menu.Destructor code generation routines. """

            def __init__(self):
                """ Constructor. """
                # Static class, nothing to be done
                pass

            @staticmethod
            def execute(ctx, xml_menu):
                """ Executes the Menu.Destructor code generation routines.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Menu.Destructor.destructor(ctx, xml_menu)

            @staticmethod
            def destructor(ctx, xml_menu):
                """ Menu destructor code generation.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Utils.indent(ctx, xml_menu, 1).put("// ----- Destructor -----").endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("public: virtual ~%s(void) {" % ctx.Utils.node2class(ctx, xml_menu)).endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("}").endl()
                ctx.out.endl()

        class Populate(object):
            """ Menu.Populate code generation routines. """

            def __init__(self):
                """ Constructor. """
                # Static class, nothing to be done
                pass

            @staticmethod
            def execute(ctx, xml_menu):
                """ Executes the Menu.Populate code generation routines.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Menu.Populate.populate(ctx, xml_menu)

            @staticmethod
            def populate(ctx, xml_menu):
                """ Menu constructor code generation.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Utils.indent(ctx, xml_menu, 1).put("// ----- Populate -----").endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("public: void Populate(void) {").endl()

                # CLI reference
                ctx.Utils.indent(ctx, xml_menu, 2).put("// CLI reference").endl()
                ctx.Utils.indent(ctx, xml_menu, 2).put("m_pcliOwnerCli = dynamic_cast<%s*>(" % ctx.args.cli_class_name(ctx))
                ctx.out                           .put(    "const_cast<cli::Cli*>(& GetCli())") # pylint: disable=bad-whitespace
                ctx.out                           .put(");").endl()                             # pylint: disable=bad-whitespace

                # Comment line patterns
                if ctx.xml.is_node(xml_menu, "cli:cli"):
                    ctx.Utils.indent(ctx, xml_menu, 2).put("// Comment line patterns").endl()
                    for _xml_sub_node in ctx.xml.children(xml_menu):
                        if ctx.xml.is_node(_xml_sub_node, "cli:comment"):
                            ctx.Utils.indent(ctx, xml_menu, 2).put("AddCommentLinePattern(\"%s\");" % ctx.xml.attr_value(_xml_sub_node, "start")).endl()

                # Populate menus
                if ctx.xml.is_node(xml_menu, "cli:cli"):
                    ctx.Utils.indent(ctx, xml_menu, 2).put("// Create menus and populate").endl()
                    # Self CLI reference setting
                    ctx.Utils.indent(ctx, xml_menu, 2).put("%s = this;" % ctx.Utils.node2var(ctx, xml_menu)).endl()
                    # Create the menus
                    for _xml_menu in ctx.cache.menus:
                        # Filter-out cli:cli XML nodes
                        if ctx.xml.is_node(_xml_menu, "cli:menu"):
                            _var_name = ctx.Utils.node2var(ctx, _xml_menu)
                            _class_name = ctx.Utils.node2class(ctx, _xml_menu)
                            ctx.Utils.indent(ctx, xml_menu, 2).put("%s = dynamic_cast<%s*>(& AddMenu(new %s()));"
                                                                   % (_var_name, _class_name, _class_name)).endl()
                    # Populate them
                    for _xml_menu in ctx.cache.menus:
                        # Filter-out cli:cli XML nodes
                        if ctx.xml.is_node(_xml_menu, "cli:menu"):
                            ctx.Utils.indent(ctx, xml_menu, 2).put("%s->Populate();" % ctx.Utils.node2var(ctx, _xml_menu)).endl()

                # Sub-nodes
                ctx.Utils.indent(ctx, xml_menu, 2).put("// Local nodes").endl()
                ctx.Menu.Populate.walk(ctx, xml_menu, xml_menu, indent_count=2)
                # tag[@ref] items are processed at the very end
                ctx.Utils.indent(ctx, xml_menu, 2).put("// tag[@ref] -> tag[@id] connections").endl()
                for _xml_tag_ref in ctx.cache.node(xml_menu).cli_Cli2xxx_tag_refs:
                    ctx.Menu.Populate.create_node(ctx, xml_menu, _xml_tag_ref, indent_count=2)

                ctx.Utils.indent(ctx, xml_menu, 1).put("}").endl()
                ctx.out.endl()

            @staticmethod
            def walk(ctx, xml_menu, xml_node, indent_count=0):
                """ Propagate population throughout child nodes.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) Current menu.
                    @param xml_node (XML node) Current node.
                    @indent_count (int) Indentation depth. """
                for _xml_sub_node in ctx.xml.children(xml_node):
                    if ctx.xml.is_node(_xml_sub_node, "cli:help"):
                        # No population for help nodes. Help nodes generate help arguments.
                        pass
                    elif ctx.xml.is_node(_xml_sub_node, "cli:comment"):
                        if ctx.xml.is_node(xml_node, "cli:cli"):
                            # cli:comment nodes have already been processed
                            pass
                        else:
                            ctx.Utils.abort(ctx, xml_node, "misplaced cli node 'cli:comment', expected in 'cli:cli' only")
                    elif ( # pylint: disable=bad-continuation
                        ctx.xml.is_node(_xml_sub_node, "cli:cpp")
                        or ctx.xml.is_node(_xml_sub_node, "cli:java")
                        or ctx.xml.is_node(_xml_sub_node, "cli:handler")
                    ):
                        # No population for target language nodes.
                        pass
                    elif ctx.xml.is_node_with_attr(_xml_sub_node, "cli:menu", "@name") and ctx.xml.is_node(xml_node, "cli:cli"):
                        # Do not populate menus which are defined at the cli level.
                        pass
                    elif ctx.xml.is_node_with_attr(_xml_sub_node, "cli:tag", "@ref"):
                        # No population for tag[@ref] right now.
                        # Because tag[@id] could be processed later, tag[@ref] will be processed at the very end,
                        # in the main populate() routine above.
                        pass
                    elif ctx.xml.is_node(_xml_sub_node, "cli:*"):
                        ctx.Menu.Populate.create_node(ctx, xml_menu, _xml_sub_node, indent_count=indent_count)
                        if ctx.xml.is_node(_xml_sub_node, "cli:cli"):
                            # Should never occur.
                            pass
                        elif ctx.xml.is_node_with_attr(_xml_sub_node, "cli:menu", "@name"):
                            # Do not populate menus, this is done directly at the cli level.
                            pass
                        else:
                            ctx.Menu.Populate.walk(ctx, xml_menu, _xml_sub_node, indent_count=indent_count+1)

            @staticmethod
            def create_node(ctx, xml_menu, xml_node, indent_count=0, with_creation_params=True):
                """ Create the node for population.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) Current menu.
                    @param xml_node (XML node) Current node.
                    @param indent_count (int) Indentation depth.
                    @param with_creation_params (bool) Insert creation parameters or not (default is True). """
                _class_name = ctx.Utils.node2class(ctx, xml_node)
                _var_name = ctx.Utils.node2var(ctx, xml_node)
                _parent_var = ctx.Utils.node2var(ctx, ctx.xml.parent_node(xml_node))

                ctx.Utils.indent(ctx, xml_menu, indent_count)
                _do_not_cast = ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref") or ctx.xml.is_node(xml_node, "cli:menu")
                if not _do_not_cast:
                    ctx.out.put("%s = dynamic_cast<%s*>(& " % (_var_name, _class_name))
                # Attach the new object to the parent node.
                ctx.out.put("%s" % _parent_var)
                if ctx.xml.is_node(xml_node, "cli:menu"):
                    # Populate menu[@ref] item.
                    ctx.out.put("->SetMenuRef(new cli::MenuRef(")
                    _menu_ref_set = False
                    if ctx.xml.attr_value(xml_node, "@name") is not None:
                        ctx.out.put("*%s" % _var_name)
                        _menu_ref_set = True
                    elif ctx.xml.attr_value(xml_node, "@ref") is not None:
                        _xml_menu = ctx.cache.node(xml_node).cli_Cli2xxx_menu
                        if _xml_menu is not None:
                            ctx.out.put("*%s" % ctx.Utils.node2var(ctx, _xml_menu))
                            _menu_ref_set = True
                    if not _menu_ref_set:
                        ctx.Utils.abort(ctx, xml_node, "missing menu/@name or menu/@ref attribute, or invalid menu/@ref reference")
                    ctx.out.put("))")
                else:
                    # Populate other kind of item.
                    ctx.out.put("->AddElement(new %s(" % _class_name)
                    if with_creation_params:
                        ctx.Menu.Constructor.params(ctx, xml_node)
                    ctx.out.put("))")
                if not _do_not_cast:
                    ctx.out.put(")")
                # Terminate the line.
                ctx.out.put(";").endl()

        class Execute(object):
            """ Menu.Execution code generation routines. """

            def __init__(self):
                """ Constructor. """
                # Static class, nothing to be done
                pass

            @staticmethod
            def execute(ctx, xml_menu):
                """ Menu execution code generation.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Utils.indent(ctx, xml_menu, 1).put("// ----- Menu execution -----").endl()

                # Execution method
                ctx.Utils.indent(ctx, xml_menu, 1).put("public: virtual const bool Execute(const cli::CommandLine& CLI_CmdLine) const {").endl()

                ctx.Utils.indent(ctx, xml_menu, 2).put("{").endl()

                # Trace
                ctx.Utils.indent(ctx, xml_menu, 3).put("static const cli::TraceClass CLI_EXECUTION(\"CLI_EXECUTION\", cli::Help()")
                ctx.out.put(".AddHelp(cli::Help::LANG_EN, \"CLI Execution traces\")")
                ctx.out.put(u".AddHelp(cli::Help::LANG_FR, \"Traces d'exécution du CLI\")")
                ctx.out.put(");").endl()

                # Step variables
                ctx.Utils.indent(ctx, xml_menu, 3).put("cli::CommandLineIterator cli_Elements(CLI_CmdLine);").endl()

                # Call implementation on the menu object
                ctx.Menu.Execute.execute_node(ctx, xml_menu, xml_menu, indent_count=3)

                # Finishing
                ctx.Utils.indent(ctx, xml_menu, 2).put("}").endl()
                ctx.Utils.indent(ctx, xml_menu, 2).put("return false;").endl()

                ctx.Utils.indent(ctx, xml_menu, 1).put("}").endl()
                ctx.out.endl()

            @staticmethod
            def execute_node(ctx, xml_menu, xml_node, indent_count):
                """ Recursive execution code generation routine.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node.
                    @param xml_node (XML node) Current node which execution code is generated for.
                    @param indent_count (int) Number of indentation from the menu offset. """
                # Top comment
                ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("// " + ctx.Utils.node2desc(ctx, xml_node)).endl()

                # Top label
                if ctx.xml.is_node_with_attr_value(xml_node, "cli:tag", "@hollow", "yes"):
                    # cli:tag[@hollow='yes']: direct jump to end label
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("goto %s;" % ctx.Utils.node2endlbl(ctx, xml_node)).endl()
                ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("%s: ;" % ctx.Utils.node2toplbl(ctx, xml_node)).endl()

                # Start the block
                if ctx.xml.is_node(xml_node, "cli:cli") or ctx.xml.is_node(xml_node, "cli:menu") or ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("{").endl()
                elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("do {").endl()
                else:
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("if (cli_Elements == *%s) {" % ctx.Utils.node2var(ctx, xml_node)).endl()

                # Step it
                if (not ctx.xml.is_node(xml_node, "cli:tag")) and (not ctx.xml.is_node(xml_node, "cli:endl")):
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 1).put("if (! cli_Elements.StepIt()) return false;").endl()

                # Trace current keyword
                ctx.Utils.indent(ctx, xml_menu, indent_count + 1).put("cli::GetTraces().Trace(CLI_EXECUTION)")
                ctx.out.put(" << \"context = \\\"%s\\\", \"" % ctx.Utils.node2desc(ctx, xml_node))
                ctx.out.put(" << \"word = \" << (")
                ctx.out.put(    "dynamic_cast<const cli::Endl*>(*cli_Elements) ")   # pylint: disable=bad-whitespace
                ctx.out.put(    "? \"<CR>\" ")                                      # pylint: disable=bad-whitespace
                ctx.out.put(    ": (const char*) (*cli_Elements)->GetKeyword()")    # pylint: disable=bad-whitespace
                ctx.out.put(") << cli::endl;").endl()

                # Execution
                _xml_sources = []
                for _xml_source in ctx.xml.children(xml_node):
                    if ctx.xml.is_node(_xml_source, "cli:cpp"):
                        if ctx.xml.attr_value(_xml_source, "@option") is None:
                            _xml_sources.append(_xml_source)
                if len(_xml_sources) > 0:
                    ctx.out.put(ctx.args.user_indent()).endl()
                    for _xml_source in _xml_sources:
                        ctx.Utils.indent(ctx, xml_menu, indent_count + 1, with_user_indent=True)
                        ctx.Utils.source_node(ctx, _xml_source)
                        ctx.out.endl()
                    ctx.out.put(ctx.args.user_indent()).endl()

                # Sub-elements
                ctx.Menu.Execute.walk(ctx, xml_menu, xml_node, indent_count=indent_count+1)

                # Final jump
                if ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 1).put("goto %s;" % ctx.Utils.node2endlbl(ctx, xml_node)).endl()
                elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                    pass
                elif ctx.xml.is_node(xml_node, "cli:endl"):
                    pass
                else:
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 1).put("return false;").endl()

                # End the block
                if ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("} while(true);").endl()
                else:
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("}").endl()

                # End label
                ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("%s: ;" % ctx.Utils.node2endlbl(ctx, xml_node)).endl()

            @staticmethod
            def walk(ctx, xml_menu, xml_node, indent_count, xml_tag=None):
                """ Second part of recursive execution code generation routine.
                    Propagate over sub-nodes.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node.
                    @param xml_node (XML node) Current node which execution code is generated for.
                    @param xml_tag (XML node) cli:tag[@id] reference being processed: only set jumps.
                    @param indent_count (int) Number of indentation from the menu offset. """
                if ctx.xml.is_node(xml_node, "cli:endl"):
                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("return true;").endl()
                elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                    # Tag reference
                    _tag_name = ctx.xml.attr_value(xml_node, "@ref")
                    _xml_target = ctx.cache.node(xml_node).cli_Cli2xxx_tag_id
                    # Now check the target
                    if _xml_target is None:
                        ctx.Utils.abort(ctx, xml_node, "unknown tag reference '%s'" % _tag_name)
                    elif _xml_target == ctx.xml.parent_node(xml_node):
                        ctx.Utils.abort(ctx, xml_node, "tag reference '%s' directly located in the tag" % _tag_name)
                    else:
                        # Make the references
                        if xml_tag is not None:
                            ctx.Menu.Execute.walk(ctx, xml_menu, _xml_target, indent_count, xml_tag)
                        else:
                            ctx.Menu.Execute.walk(ctx, xml_menu, _xml_target, indent_count, _xml_target)
                else:
                    for _xml_sub_node in ctx.xml.children(xml_node):
                        if ( # pylint: disable=bad-continuation
                            ctx.xml.is_node(_xml_sub_node, "cli:keyword") or ctx.xml.is_node(_xml_sub_node, "cli:param")
                            or ctx.xml.is_node(_xml_sub_node, "cli:tag") or ctx.xml.is_node(_xml_sub_node, "cli:endl")
                        ):
                            if xml_tag is None:
                                ctx.Menu.Execute.execute_node(ctx, xml_menu, _xml_sub_node, indent_count)
                            else:
                                if ctx.xml.is_node_with_attr(_xml_sub_node, "cli:tag", "@ref"):
                                    ctx.Menu.Execute.walk(ctx, xml_menu, _xml_sub_node, indent_count, xml_tag)
                                elif ctx.xml.is_node_with_attr_value(_xml_sub_node, "cli:tag", "@hollow", "yes"):
                                    # Do nothing
                                    pass
                                elif ctx.xml.is_node_with_attr(_xml_sub_node, "cli:tag", "@id"):
                                    ctx.Menu.Execute.walk(ctx, xml_menu, _xml_sub_node, indent_count, xml_tag)
                                else:
                                    ctx.Utils.indent(ctx, xml_menu, indent_count + 0).put("if (cli_Elements == *%s) " % ctx.Utils.node2var(ctx, _xml_sub_node))
                                    ctx.out.put("goto %s;" % ctx.Utils.node2toplbl(ctx, xml_tag)).endl()

        # Handlers
        class Handlers(object):
            """ Menu.Handlers code generation routines. """

            def __init__(self, ):
                """ Constructor. """
                # Static class, nothing to be done
                pass

            @staticmethod
            def execute(ctx, xml_menu):
                """ Executes the Menu.Execute code generation routines.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Menu.Handlers.error_handler(ctx, xml_menu) # cli only
                ctx.Menu.Handlers.exit_handler(ctx, xml_menu)
                ctx.Menu.Handlers.prompt_handler(ctx, xml_menu)

            @staticmethod
            def error_handler(ctx, xml_menu):
                """ Error handler code generation.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                # Error handler
                if ctx.xml.is_node(xml_menu, "cli:cli"):
                    ctx.Utils.indent(ctx, xml_menu, 1).put("public: virtual const bool OnError(")           # pylint: disable=bad-whitespace
                    ctx.out                           .put(    "const cli::ResourceString& location, ")     # pylint: disable=bad-whitespace
                    ctx.out                           .put(    "const cli::ResourceString& message")        # pylint: disable=bad-whitespace
                    ctx.out                           .put(") const {").endl()                              # pylint: disable=bad-whitespace
                    _xml_extra_sources = ctx.xml.xpath_set(xml_menu, "cli:handler[@name='error']/cli:cpp")
                    if len(_xml_extra_sources) > 0:
                        ctx.out.put(ctx.args.user_indent()).endl()
                        for _xml_extra_source in _xml_extra_sources:
                            ctx.Utils.indent(ctx, xml_menu, 2, with_user_indent=True)
                            ctx.Utils.source_node(ctx, _xml_extra_source)
                            ctx.out.endl()
                        ctx.out.put(ctx.args.user_indent()).endl()
                    ctx.Utils.indent(ctx, xml_menu, 2).put("return Cli::OnError(location, message);").endl()
                    ctx.Utils.indent(ctx, xml_menu, 1).put("}").endl()
                    ctx.out.endl()

            @staticmethod
            def exit_handler(ctx, xml_menu):
                """ Exit handler code generation.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Utils.indent(ctx, xml_menu, 1).put("public: virtual void OnExit(void) const {").endl()
                _xml_extra_sources = ctx.xml.xpath_set(xml_menu, "cli:handler[@name='exit']/cli:cpp")
                if len(_xml_extra_sources) > 0:
                    ctx.out.put(ctx.args.user_indent()).endl()
                    for _xml_extra_source in _xml_extra_sources:
                        ctx.Utils.indent(ctx, xml_menu, 2, with_user_indent=True)
                        ctx.Utils.source_node(ctx, _xml_extra_source)
                        ctx.out.endl()
                    ctx.out.put(ctx.args.user_indent()).endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("}").endl()
                ctx.out.endl()

            @staticmethod
            def prompt_handler(ctx, xml_menu):
                """ Prompt handler code generation.
                    @param ctx (Cli2Cpp) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu node. """
                ctx.Utils.indent(ctx, xml_menu, 1).put("public: virtual const cli::tk::String OnPrompt(void) const {").endl()
                _xml_extra_sources = ctx.xml.xpath_set(xml_menu, "cli:handler[@name='prompt']/cli:cpp")
                if len(_xml_extra_sources) > 0:
                    ctx.out.put(ctx.args.user_indent()).endl()
                    for _xml_extra_source in _xml_extra_sources:
                        ctx.Utils.indent(ctx, xml_menu, 2, with_user_indent=True)
                        ctx.Utils.source_node(ctx, _xml_extra_source)
                        ctx.out.endl()
                    ctx.out.put(ctx.args.user_indent()).endl()
                ctx.Utils.indent(ctx, xml_menu, 2).put("return Menu::OnPrompt();").endl()
                ctx.Utils.indent(ctx, xml_menu, 1).put("}").endl()
                ctx.out.endl()

    class Utils(clicommon.CtxUtils):
        """ Utils routines. """

        def __init__(self):
            """ Constructor. """
            # Static class, nothing to be done
            clicommon.CtxUtils.__init__(self)

        @staticmethod
        def extra_source(ctx, xml_node, option):
            """ Extra source generation.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Focus node to generate extra source section for. May be None.
                @param option (str) Extra source @option attribute value.
                @return True for success, False otherwise. """
            # Determine indentation count
            _indent_count = 0
            if option == "constructor":
                _indent_count = 2
            elif option == "members":
                _indent_count = 1
            #modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, "_extra_source(): _indent_count = %d" % _indent_count)
            # Begin extra code section
            ctx.Utils.indent(ctx, xml_node, _indent_count).put("// ----- Extra cpp (option='%s') -----" % option).endl()
            # Find out extra code nodes
            _xml_extra_sources = ctx.xml.xpath_set(xml_node, "cli:cpp[@option='%s']" % option)
            if len(_xml_extra_sources) > 0:
                # Determine whether user-code should be spotted
                _show_user_indent = ((option == "members") or (option == "constructor"))
                # Generate extra code
                if _show_user_indent:
                    ctx.out.put(ctx.args.user_indent()).endl()
                for _xml_extra_source in _xml_extra_sources:
                    ctx.Utils.indent(ctx, None, _indent_count, with_user_indent=_show_user_indent)
                    ctx.Utils.source_node(ctx, _xml_extra_source)
                    ctx.out.endl()
                if _show_user_indent:
                    ctx.out.put(ctx.args.user_indent()).endl()
            # Finish
            return True

        @staticmethod
        def source_node(ctx, xml_source):
            """ Put out the code for a source node.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_source (XML node) Source node to process. """
            for _xml_child in ctx.xml.children(xml_source):
                if ctx.xml.is_node(_xml_child, "cli:out"):
                    ctx.out.put("GetOutputStream()")
                elif ctx.xml.is_node(_xml_child, "cli:err"):
                    ctx.out.put("GetErrorStream()")
                elif ctx.xml.is_node_with_attr(_xml_child, "cli:value-of", "@param"):
                    _param_id = ctx.xml.attr_value(_xml_child, "@param")
                    _xml_param = xml_source
                    while _xml_param is not None:
                        if ctx.xml.is_node_with_attr_value(_xml_param, "cli:param", "@id", _param_id):
                            break
                        _xml_param = ctx.xml.parent_node(_xml_param)
                    if _xml_param is not None:
                        ctx.out.put("(*%s)" % ctx.Utils.node2var(ctx, _xml_param))
                    else:
                        ctx.Utils.abort(ctx, _xml_child, "unknown parameter reference '%s'" % _param_id)
                else:
                    ctx.out.put(ctx.xml.content(_xml_child))

        @staticmethod
        def indent(ctx, xml_node, count, with_user_indent=False):
            """ Output indentation.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Context node.
                @param count (int) Number of indentations to output.
                @param with_user_indent (boolean) True to display user-indent indentation pattern, False otherwise.
                @return (Output) Output device. """
            # Ensure the reference of the XML node for caching
            if xml_node is None:
                xml_node = ctx.xml.root_node()
            xml_node = ctx.cache.node(xml_node)

            # Check whether the indent computation is already cached
            _cache_attr_name = "cli_Cli2Cpp_indent" + str(count) + str(with_user_indent)
            _str_indent = getattr(xml_node, _cache_attr_name, None)

            # If not, compute and cache the result
            if _str_indent is None:
                _str_indent = ""

                _count = ctx.Utils.indent_offset(ctx, xml_node) + count
                _indent_length = _count * len(ctx.args.indent())

                if with_user_indent:
                    _str_indent += ctx.args.user_indent()
                    _indent_length -= len(ctx.args.user_indent())

                while _indent_length > 0:
                    _str_indent += ctx.args.indent()
                    _indent_length -= len(ctx.args.indent())

                setattr(xml_node, _cache_attr_name, _str_indent)

            # Eventually output the indentation
            ctx.out.put(_str_indent)

            return ctx.out

        @staticmethod
        def indent_offset(ctx, xml_node):
            """ Indentation offset computation.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Context node.
                @return (int) Indentation offset. """
            _indent_offset = 0
            if xml_node is not None:
                #try:
                xml_node = ctx.cache.node(xml_node)
                _indent_offset = getattr(xml_node, "cli_Cli2Cpp_indent_offset", None)
                if _indent_offset is None:
                    if ctx.xml.is_node(xml_node, "cli:cli"):
                        _indent_offset = 0
                    elif ctx.xml.is_node(xml_node, "cli:menu"):
                        _indent_offset = 1
                    else:
                        # Default: rely on the parent's computation value.
                        _indent_offset = ctx.Utils.indent_offset(ctx, ctx.xml.parent_node(xml_node))
                    # Cache result
                    setattr(xml_node, "cli_Cli2Cpp_indent_offset", _indent_offset)
            return _indent_offset

        @staticmethod
        def node2desc(ctx, xml_node):
            """ Description computation.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Node to compute a description for.
                @return (str) Description computed. """

            _desc = ""

            if xml_node is None:
                _desc = "nil"
            else:
                xml_node = ctx.cache.node(xml_node)
                _desc = getattr(xml_node, "cli_Cli2Cpp_desc", None)
                if _desc is None:
                    _desc = ""

                    # Recursive call
                    if (not ctx.xml.is_node(xml_node, "cli:cli")) and (not ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@name")):
                        _desc += (ctx.Utils.node2desc(ctx, ctx.xml.parent_node(xml_node)) + " ")

                    # Current item description
                    if ctx.xml.is_node(xml_node, "cli:cli") or ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@name"):
                        _desc += (ctx.xml.attr_value(xml_node, "@name") + ">")
                    elif ctx.xml.is_node(xml_node, "cli:keyword"):
                        _desc += ctx.xml.attr_value(xml_node, "@string")
                    elif ctx.xml.is_node(xml_node, "cli:param"):
                        _desc += ("$" + ctx.xml.attr_value(xml_node, "@id"))
                    elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                        _desc += ("[" + ctx.xml.attr_value(xml_node, "@id") + ":]")
                    elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                        _desc += ("[-> " + ctx.xml.attr_value(xml_node, "@ref") + "]")
                    elif ctx.xml.is_node(xml_node, "cli:endl"):
                        _desc += "<CR>"
                    else:
                        _desc += "???"

                    # Cache result
                    setattr(xml_node, "cli_Cli2Cpp_desc", _desc)

            return _desc

        @staticmethod
        def node2var(ctx, xml_node, menu_with_cli_pointer=True):
            """ Determine a variable for the given node.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Node to find a variable name for.
                @param menu_with_cli_pointer (bool) Include owner cli pointer access for menus in the variable name.
                                                    Rem: all menu variables are members of the CLI object.
                @return (str) Variable name. """
            _with_cli_pointer = False
            _var_name = ""

            if xml_node is not None:
                xml_node = ctx.cache.node(xml_node)
                _with_cli_pointer = getattr(xml_node, "cli_Cli2Cpp_cli_pointer", None)
                _var_name = getattr(xml_node, "cli_Cli2Cpp_var_name", None)
                if (_with_cli_pointer is None) or (_var_name is None):
                    _with_cli_pointer = False
                    _var_name = ""

                    # Menu: access through the m_pcliOwnerCli member, unless menu_with_cli_pointer is disabled.
                    if ctx.xml.is_node(xml_node, "cli:cli") or ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@name"):
                        _with_cli_pointer = True

                    # Variable prefix
                    _var_name += ctx.args.var_prefix()

                    # Element type
                    if ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                        _var_name += "tagref"
                    elif ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@ref"):
                        _var_name += "menuref"
                    else:
                        _var_name += ctx.Utils.mk_cpp_name(ctx, xml_node, ctx.xml.node_name(xml_node, with_ns=False))

                    # Element name
                    if ctx.xml.is_node(xml_node, "cli:keyword"):
                        _var_name += "_" + ctx.Utils.mk_cpp_name(ctx, xml_node, ctx.xml.attr_value(xml_node, "@string"))
                    elif ctx.xml.is_node(xml_node, "cli:param"):
                        _var_name += "_" + ctx.Utils.mk_cpp_name(ctx, xml_node, ctx.xml.attr_value(xml_node, "@id"))

                    # Id
                    _var_name += "_" + ctx.Utils.node2id(ctx, xml_node)

                    # Cache result
                    setattr(xml_node, "cli_Cli2Cpp_cli_pointer", _with_cli_pointer)
                    setattr(xml_node, "cli_Cli2Cpp_var_name", _var_name)

            if menu_with_cli_pointer and _with_cli_pointer:
                _var_name = ("m_pcliOwnerCli->" + _var_name)
            return _var_name

        @staticmethod
        def node2id(ctx, xml_node, separator=""):
            """ Determine an identifier for the given node.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Node to find a variable name for.
                @param separator (str) Separator pattern.
                @return (str) Node identifier. """
            # Ensure the reference of the XML node for caching
            if xml_node is None:
                xml_node = ctx.xml.root_node()
            xml_node = ctx.cache.node(xml_node)

            _id = getattr(xml_node, "cli_Cli2Cpp_id", None)
            if _id is None:
                _preceding_count = xml_node.cli_Cli2xxx_child_index

                # Recursive call.
                _parent_id = ""
                _xml_parent = ctx.xml.parent_node(xml_node)
                if _xml_parent is not None:
                    if ctx.xml.is_node(_xml_parent, "cli:*"):
                        if not ctx.xml.is_node(_xml_parent, "cli:cli"):
                            _separator = ""
                            if _preceding_count >= 26+26:
                                _separator = "_"
                            _parent_id = ctx.Utils.node2id(ctx, _xml_parent, separator=_separator)
                # Generate id part based on the position of the node regarding its preceding-sibling.
                # Start with a,b,c...x,y,z,A,B,C...X,Y,Z, then numbers from 0 seperated with '_' if numbers follow each others.
                _this_id = ""
                if _preceding_count < 26:
                    _this_id = chr(97 + _preceding_count)
                elif _preceding_count < 26+26:
                    _this_id = chr(65 + _preceding_count - 26)
                else:
                    _this_id = str(_preceding_count-26-26) + "_"

                # Cache result.
                _id = _parent_id + _this_id
                setattr(xml_node, "cli_Cli2Cpp_id", _parent_id + _this_id)

            if (separator == "") and _id.endswith("_"):
                _id = _id[:-1]
            return _id

        @staticmethod
        def node2class(ctx, xml_node):
            """ Determine a class name for the given node.
                @param ctx (Cli2Cpp) Execution context.
                @param (XML node) Node to find a class name for.
                @return (str) Class name. """
            _class_name = ""
            # Root node <=> Main CLI class
            if ctx.xml.is_node(xml_node, "cli:cli"):
                _class_name = ctx.args.cli_class_name(ctx)
            # Menu and menu references
            elif ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@name"):
                _class_name = ctx.args.class_prefix() + ctx.Utils.mk_cpp_name(ctx, xml_node, ctx.xml.attr_value(xml_node, "@name"))
            elif ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@ref"):
                _class_name = "cli::MenuRef"
            elif ctx.xml.is_node(xml_node, "cli:menu"):
                ctx.Utils.abort(ctx, xml_node, "menu node should either have @name or @ref attribute.")
            # Keywords
            elif ctx.xml.is_node(xml_node, "cli:keyword"):
                _class_name = "cli::Keyword"
            # Parameters
            elif ctx.xml.is_node_with_attr(xml_node, "cli:param", "@type"):
                _type = ctx.xml.attr_value(xml_node, "@type")
                if _type == "string":
                    _class_name = "cli::ParamString"
                elif _type == "int":
                    _class_name = "cli::ParamInt"
                elif _type == "float":
                    _class_name = "cli::ParamFloat"
                elif _type == "host":
                    _class_name = "cli::ParamHost"
                else:
                    ctx.Utils.abort(ctx, xml_node, "unknown param type '%s'" % _type)
            elif ctx.xml.is_node(xml_node, "cli:param"):
                ctx.Utils.abort(ctx, xml_node, "missing @type attribute")
            # End of line
            elif ctx.xml.is_node(xml_node, "cli:endl"):
                _class_name = "cli::Endl"
            # Tags
            elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                _class_name = "cli::SyntaxTag"
            elif ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@ref"):
                _class_name = "cli::SyntaxRef"
            elif ctx.xml.is_node(xml_node, "cli:tag"):
                ctx.Utils.abort(ctx, xml_node, "tag node should either have @id or @ref attribute.")
            # Fallback
            else:
                ctx.Utils.abort(ctx, xml_node, "unknown cli node '%s'" % ctx.xml.node_name(xml_node))

            return _class_name

        @staticmethod
        def node2toplbl(ctx, xml_node):
            """ Top label name computation.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Node to compute a label name for.
                @return (str) Top label name. """
            return ctx.Utils.node2var(ctx, xml_node, menu_with_cli_pointer=False) + "_top_lbl"

        @staticmethod
        def node2endlbl(ctx, xml_node):
            """ End label name computation.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Node to compute a label name for.
                @return (str) End label name. """
            return ctx.Utils.node2var(ctx, xml_node, menu_with_cli_pointer=False) + "_end_lbl"

        @staticmethod
        def mk_cpp_name(ctx, xml_node, symbol):
            """ Build a regular C++ symbol from a given resource.
                @param ctx (Cli2Cpp) Execution context.
                @param xml_node (XML node) Current node being processed.
                @param symbol (str) Text resource to build a regular C++ symbol from.
                @return (str) Regular C++ symbol. """
            #_filter = ctx.MODLOG_FILTER + ".Utils.mk_cpp_name"
            if (symbol is None) or (symbol == ""):
                ctx.Utils.abort(ctx, xml_node, "Cannot make a C++ symbol from an empty string.")
            _cpp_name = symbol
            # Replace every special character with an underscore.
            for _special in " ,:<>*|+-()#.":
                _cpp_name = _cpp_name.replace(_special, "_")
            # Replace every accented character with its non-accented equivalent.
            _tr = {}
            _tr["a"] = ["á", "à", "â", "ä"]
            _tr["c"] = ["ç"]
            _tr["e"] = ["é", "è", "ê", "ë"]
            _tr["i"] = ["í", "ì", "î", "ï"]
            _tr["o"] = ["ó", "ò", "ô", "ö"]
            _tr["u"] = ["ú", "ù", "û", "ü"]
            for _no_accent in _tr.keys():
                for _accent in _tr[_no_accent]:
                    _cpp_name = _cpp_name.replace(_accent, _no_accent)

            #modlog.log(_filter, modlog.DEBUG, "mk_cpp_name('%s') -> '%s'" % (symbol, _cpp_name))
            return _cpp_name

# Main call
main()
