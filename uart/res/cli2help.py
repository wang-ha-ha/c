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
    #modlog.engine().set_filter(Cli2Help.MODLOG_FILTER, modlog.DEBUG)
    import sys
    ctx = Cli2Help()
    if ctx.args.parse_args(ctx):
        if ctx.xml.load_file(ctx.args.xml_resource_file()):
            ctx.xml.set_ns_shortcut("cli", "http://alexis.royer.free.fr/CLI")
            if ctx.clic.execute(ctx):
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
            @param ctx (Xml2Cpp) Program execution context.
            @return (bool) True for success. """
        import argparse

        _args = argparse.ArgumentParser(prog="cli2help")
        _args.add_argument("--version", action="version", version=("%(prog)s - " + clicommon.CtxUtils.version()))
        _args.add_argument("--lang", help="Language ('en', 'fr'...).")
        _toc_group = _args.add_mutually_exclusive_group()
        _toc_group.add_argument("--toc", dest="toc", help="Generate table of content.",
                                action="store_const", const="True", default="True")
        _toc_group.add_argument("--no-toc", dest="toc", help="Do not generate table of content.",
                                action="store_const", const="False", default="True")
        _title_numbers_group = _args.add_mutually_exclusive_group()
        _title_numbers_group.add_argument("--title-numbers", dest="title_numbers", help="Generate titles with their number.",
                                          action="store_const", const="True", default="True")
        _title_numbers_group.add_argument("--no-title-numbers", dest="title_numbers", help="Generate titles with no number.",
                                          action="store_const", const="False", default="True")
        _args.add_argument("--output", help="Output file (by default the result is printed out in the standard output)")
        _args.add_argument("xml-res", type=file, help="CLI XML resource file")

        self._res = _args.parse_args()
        #modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, vars(self._res))

        if vars(self._res)["output"] is None:
            ctx.out = xmldoc.OutputDocument(clicommon.Console())
        else:
            ctx.out = xmldoc.OutputDocument(clicommon.OutputFile(vars(self._res)["output"]))

        return True

    @staticmethod
    def program_name():
        """ Program name accessor.
            @return (str) Program name. """
        return "cli2help"

    def lang(self):
        """ Language property accessor.
            @return (str) Language property . """
        _lang = vars(self._res)["lang"]
        if _lang is not None:
            if _lang == "en":
                return "en"
            elif _lang == "fr":
                return "fr"
            else:
                modlog.log(Cli2Help.MODLOG_FILTER, modlog.WARNING, "Unknown language '%s'. Using default 'en'.")
        return "en"

    def toc(self):
        """ Table of content generation property accessor.
            @return (boolean) True for table of content generation, False otherwise. """
        _toc = vars(self._res)["toc"]
        _toc = (_toc == "True")
        return _toc

    def title_numbers(self):
        """ Generate titles with their number property accessor.
            @return (boolean) True for titles with their number, False otherwise. """
        _title_numbers = vars(self._res)["title_numbers"]
        _title_numbers = (_title_numbers == "True")
        return _title_numbers

    def xml_resource_file(self):
        """ Resource file name accessor.
            @return (str) XML resource file name. """
        return vars(self._res)["xml-res"].name

class Cli2Help(object):
    """ CLI XML resource file to help html transformation. """

    MODLOG_FILTER = "cli2help"

    def __init__(self):
        """ Constructor. """
        self.args = Args()
        self.xml = xmldoc.InputDocument()
        self.clic = self
        self.out = None
        self.cache = clicommon.Cache()

    @staticmethod
    def execute(ctx):
        """ XML 2 HTML transformation execution.
            @param ctx (Cli2Help) Execution context.
            @return (bool) True for success, False otherwise. """
        modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "execute()")
        try:
            ctx.cache.execute(ctx)
            ctx.Html.execute(ctx)
            return True
        except ctx.Utils.ExecutionStoppedException, _ese:
            modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, _ese)
        except RuntimeError, _re:
            modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, _re)
            modlog.log(ctx.MODLOG_FILTER, modlog.FATAL_ERROR, str(_re))
        return False

    class Html(object):
        """ Section generation routines. """

        def __init__(self):
            """ Constructor. """
            # Static class, nothing to be done
            pass

        @staticmethod
        def execute(ctx):
            """ Executes the class generation routines.
                @param ctx (Cli2Help) Execution context. """
            # Build HTML document
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating html document...")
            ctx.Html.html(ctx)
            # Output to the console
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Writing html document...")
            ctx.out.write(encoding=ctx.xml.encoding())

        @staticmethod
        def html(ctx):
            """ Generate HTML document.
                @param ctx (Cli2Help) Execution context. """
            # Auto-generation comment
            ctx.out.add_comment(None, "File auto-generated by 'cli2help.py' - %s. Do not edit!" % clicommon.CtxUtils.version())

            # Create root node
            _xml_html = ctx.out.add_root_node("html")

            # Head and body subnodes
            ctx.Html.head(ctx, _xml_html)
            ctx.Html.body(ctx, _xml_html)

        @staticmethod
        def head(ctx, xml_html):
            """ Generate head section.
                @param ctx (Cli2Help) Execution context.
                @param xml_html (XML node) html output node. """
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating head...")
            _xml_head = ctx.out.add_node(xml_html, "head")
            _xml_encoding = ctx.out.add_node(_xml_head, "meta", attrs=[["http-equiv", "Content-Type"], ["content", "text/html; charset=utf-8"]]) # pylint: disable=unused-variable
            _xml_title = ctx.out.add_node(_xml_head, "title", content=ctx.cache.page_title) # pylint: disable=unused-variable
            ctx.Html.css(ctx, _xml_head)
            ctx.Html.javascript(ctx, _xml_head)

        @staticmethod
        def javascript(ctx, xml_head):
            """ Javascript code generation.
                @param ctx (Cli2Help) Execution context.
                @param xml_head (XML node) Output XML node. """
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating javascript...")

            _js = "\n"

            _js += "function onLoad() {\n"
            if ctx.args.title_numbers():
                _js += "    numberTitles(null);\n"
            if ctx.args.toc():
                _js += "    generateToc(null);\n"
            _js += "}\n\n"

            if ctx.args.title_numbers():
                _js += """
var m_iH2, m_iH3;
function numberTitles(XML_Node) {
    function getTextNode(xml_Node) {
        if (xml_Node.nodeName == \"#text\") { return xml_Node; }
        return getTextNode(xml_Node.firstChild);
    }

    if (XML_Node == null) {
        XML_Node = document;
        m_iH2 = 0; m_iH3 = 0;
    }

    if (XML_Node.nodeName == \"H2\") {
        m_iH2 ++; m_iH3 = 0;
        var xml_TextNode = getTextNode(XML_Node);
        if (xml_TextNode != null) {
            xml_TextNode.nodeValue = (m_iH2 + \". \" + xml_TextNode.nodeValue);
        }
    }
    if (XML_Node.nodeName == \"H3\") {
        m_iH3 ++;
        var xml_TextNode = getTextNode(XML_Node);
        if (xml_TextNode != null) {
            xml_TextNode.nodeValue = (m_iH2 + \".\" + m_iH3 + \". \" + xml_TextNode.nodeValue);
        }
    }

    // Recursion
    for (var xml_Child = XML_Node.firstChild; xml_Child != null; xml_Child = xml_Child.nextSibling) {
        numberTitles(xml_Child);
    }
}
"""

            if ctx.args.toc():
                _js += """
function generateToc(XML_Node) {
    function getTableNode(XML_Node) {
        if (XML_Node == null) { XML_Node = document; }
        if (XML_Node.nodeName == \"H1\") {
            var xml_H1 = XML_Node;
            if ((xml_H1.nextSibling != null) && (xml_H1.nextSibling.nodeName == \"TABLE\")) {
                return xml_H1.nextSibling;
            } else if (xml_H1.parentNode != null) {
                var xml_TableNode = document.createElement(\"TABLE\");
                if (xml_H1.nextSibling != null) {
                    xml_H1.parentNode.insertBefore(xml_TableNode, xml_H1.nextSibling);
                } else {
                    xml_H1.parentNode.appendChild(xml_TableNode);
                }
                return xml_TableNode;
            }
        } else {
            for (var xml_Child = XML_Node.firstChild; xml_Child != null; xml_Child = xml_Child.nextSibling) {
                var xml_TableNode = getTableNode(xml_Child);
                if (xml_TableNode != null) {
                    return xml_TableNode;
                }
            }
        }
        return null;
    }
    function createTocEntry(STR_TocClass, STR_AnchorName, STR_Title) {
        var xml_TableNode = getTableNode();
        if (xml_TableNode != null) {
            var xml_TextNode = document.createTextNode(STR_Title);
            var xml_ANode = document.createElement(\"A\"); xml_ANode.href = \"#\" + STR_AnchorName; \
xml_ANode.className = STR_TocClass; xml_ANode.appendChild(xml_TextNode);
            var xml_TdNode = document.createElement(\"TD\"); xml_TdNode.appendChild(xml_ANode);
            var xml_TrNode = document.createElement(\"TR\"); xml_TrNode.appendChild(xml_TdNode);
            xml_TableNode.appendChild(xml_TrNode);
        }
    }

    function getAnchorNode(XML_Node) {
        if (XML_Node == null) { return null; }
        if (XML_Node.nodeName == \"A\") { return XML_Node; }
        return getAnchorNode(XML_Node.previousSibling);
    }
    function getTextNode(XML_Node) {
        if (XML_Node == null) { return null; }
        if (XML_Node.nodeName == \"#text\") { return XML_Node; }
        return getTextNode(XML_Node.firstChild);
    }

    if (XML_Node == null) { XML_Node = document; }
    if (XML_Node.nodeName == \"H2\") {
        var xml_AnchorNode = getAnchorNode(XML_Node);
        var xml_TextNode = getTextNode(XML_Node);
        if ((xml_AnchorNode != null) && (xml_TextNode != null)) {
            createTocEntry(\"toc-h2\", xml_AnchorNode.name, xml_TextNode.nodeValue);
        }
    }
    if (XML_Node.nodeName == \"H3\") {
        var xml_AnchorNode = getAnchorNode(XML_Node);
        var xml_TextNode = getTextNode(XML_Node);
        if ((xml_AnchorNode != null) && (xml_TextNode != null)) {
            createTocEntry(\"toc-h3\", xml_AnchorNode.name, xml_TextNode.nodeValue);
        }
    }

    // Recursion
    for (var xml_Child = XML_Node.firstChild; xml_Child != null; xml_Child = xml_Child.nextSibling) {
        generateToc(xml_Child);
    }
}
"""

            _xml_script = ctx.out.add_node(xml_head, "script", attrs=[["type", "text/javascript"]])
            ctx.out.add_comment(_xml_script, _js)

        @staticmethod
        def css(ctx, xml_head):
            """ CSS code generation.
                @param ctx (Cli2Help) Execution context.
                @param xml_head (XML node) Output XML node. """
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating css...")

            _css = ""

            _css += """
body { font-family: Arial; }
.command-section { text-decoration: underline; }
.command-synopsis { font-family: monospace; }
.option-name { font-weight: bold; }
.param-name { font-weight: bold; }
.param-type { font-style: italic; }
.default { font-style: italic; font-size: 80%; }
.file-comment { font-size: 80%; }
"""
            if ctx.args.toc():
                _css += "\n"
                _css += ".toc-h2 { margin-left: 50px; }\n"
                _css += ".toc-h3 { margin-left: 100px; }\n"

            _xml_style = ctx.out.add_node(xml_head, "style", attrs=[["type", "text/css"]])
            ctx.out.add_comment(_xml_style, _css)

        @staticmethod
        def body(ctx, xml_html):
            """ Generate body section.
                @param ctx (Cli2Help) Execution context.
                @param xml_html (XML node) html output node. """
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating body...")

            _xml_body = ctx.out.add_node(xml_html, "body", attrs=[["onload", "onLoad();"]])

            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating general section...")
            ctx.out.add_node(_xml_body, "h1", content=ctx.cache.page_title)

            # Intro / general presentation
            ctx.out.add_node(_xml_body, "a", attrs=[["name", "intro"]])
            ctx.out.add_node(_xml_body, "h2", content=("%s%s%s" % (
                                                       ctx.Utils.translate(ctx, "Command Line Interface "),     # pylint: disable=bad-continuation
                                                       ctx.cache.cli_name,                                      # pylint: disable=bad-continuation
                                                       ctx.Utils.translate(ctx, " (general presentation)"))))   # pylint: disable=bad-continuation

            # Comment line patterns
            _arxml_comments = ctx.xml.xpath_set(None, "/cli:cli/cli:comment")
            if len(_arxml_comments) > 0:
                _xml_p = ctx.out.add_node(_xml_body, "p")
                ctx.out.add_node(_xml_p, "span", content=(
                    ctx.Utils.translate(ctx, "This command line interface uses the following patterns for comments definition:")
                ))
                _xml_table = ctx.out.add_node(_xml_p, "table")
                for _xml_comment in _arxml_comments:
                    _xml_tr = ctx.out.add_node(_xml_table, "tr")
                    _xml_td1 = ctx.out.add_node(_xml_tr, "td", attrs=[["valign", "top"]])
                    ctx.out.add_node(_xml_td1, "li")
                    _xml_td2 = ctx.out.add_node(_xml_tr, "td", attrs=[["valign", "top"]], content=ctx.xml.attr_value(_xml_comment, "start"))

            # Menus
            _xml_p = ctx.out.add_node(_xml_body, "p")
            ctx.out.add_node(_xml_p, "span", content=("'%s'%s" % (ctx.cache.cli_name, ctx.Utils.translate(ctx, " is composed of the following menus:"))))

            _xml_table = ctx.out.add_node(_xml_p, "table")
            for _xml_menu in ctx.cache.menus:
                _xml_tr = ctx.out.add_node(_xml_table, "tr")
                _xml_td1 = ctx.out.add_node(_xml_tr, "td", attrs=[["valign", "top"]])
                ctx.out.add_node(_xml_td1, "li")
                _xml_td2 = ctx.out.add_node(_xml_tr, "td", attrs=[["valign", "top"]])
                _main_menu_notice = ""
                if ctx.xml.is_node(_xml_menu, "cli:cli"):
                    _main_menu_notice = ctx.Utils.translate(ctx, " (main menu)")
                ctx.out.add_node(_xml_td2, "a", attrs=[["href", "#%s" % ctx.Utils.anchor_name(ctx, _xml_menu)]],
                                                content=("%s%s" % (ctx.xml.attr_value(_xml_menu, "@name"), _main_menu_notice))) # pylint: disable=bad-continuation
                _xml_td3 = ctx.out.add_node(_xml_tr, "td") # pylint: disable=unused-variable
                _xml_td4 = ctx.out.add_node(_xml_tr, "td")
                ctx.Utils.put_node_help(ctx, _xml_td4, ctx.Utils.get_node_help(ctx, _xml_menu))

            # Describe each menu
            for _xml_menu in ctx.cache.menus:
                ctx.Menu.execute(ctx, _xml_body, _xml_menu)

            # Footer
            _xml_p = ctx.out.add_node(_xml_body, "p", attrs=[["class", "file-comment"]])
            ctx.out.add_content(_xml_p, "File auto-generated by 'cli2help.py' - CLI library %s (%s, "
                                        % (clicommon.CtxUtils.short_version(), clicommon.CtxUtils.author_name())) # pylint: disable=bad-continuation
            ctx.out.add_node(_xml_p, "a", attrs=[["target", "blank"], ["href", clicommon.CtxUtils.url()]],
                                          content="http://alexis.royer.free.fr/CLI/") # pylint: disable=bad-continuation
            ctx.out.add_content(_xml_p, ").")

    class Menu(object):
        """ Menu documentation: describe the commands of the menu. """

        def __init__(self):
            """ Constructor. """
            # Static class, nothing to be done
            pass

        @staticmethod
        def execute(ctx, xml_body, xml_menu):
            """ Executes the class generation routines.
                @param ctx (Cli2Help) Execution context.
                @param xml_body (XML node) body output node.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node. """
            modlog.log(ctx.MODLOG_FILTER, modlog.TRACE, "Generating menu section '%s'..." % ctx.xml.attr_value(xml_menu, "@name"))
            ctx.Menu.title(ctx, xml_body, xml_menu)
            ctx.Menu.command_list(ctx, xml_body, xml_menu)
            ctx.Menu.commands(ctx, xml_body, xml_menu)

        @staticmethod
        def title(ctx, xml_body, xml_menu):
            """ Title of menu.
                @param ctx (Cli2Help) Execution context.
                @param xml_body (XML node) body output node.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node. """
            ctx.out.add_node(xml_body, "a", attrs=[["name", ctx.Utils.anchor_name(ctx, xml_menu)]])
            _main_menu_notice = ""
            if ctx.xml.is_node(xml_menu, "cli:cli"):
                _main_menu_notice = ctx.Utils.translate(ctx, " (main menu)")
            ctx.out.add_node(xml_body, "h2", content=("%s%s%s" % (
                ctx.Utils.translate(ctx, "Menu "),
                ctx.xml.attr_value(xml_menu, "@name"),
                _main_menu_notice
            )))

        @staticmethod
        def command_list(ctx, xml_body, xml_menu):
            """ List of commands available in the menu.
                @param ctx (Cli2Help) Execution context.
                @param xml_body (XML node) body output node.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node. """
            # List out the commands
            _xml_p = ctx.out.add_node(xml_body, "p")
            ctx.out.add_node(_xml_p, "span", content=("'%s'%s" % (
                ctx.xml.attr_value(xml_menu, "@name"),
                ctx.Utils.translate(ctx, " is composed of the following commands:")
            )))
            _xml_table = ctx.out.add_node(_xml_p, "table")
            for _xml_endl in xml_menu.cli_Cli2xxx_commands:
                _xml_tr = ctx.out.add_node(_xml_table, "tr")
                _xml_td1 = ctx.out.add_node(_xml_tr, "td", attrs=[["valign", "top"]])
                ctx.out.add_node(_xml_td1, "li")
                _xml_td2 = ctx.out.add_node(_xml_tr, "td", attrs=[["valign", "top"]])
                _xml_a = ctx.out.add_node(_xml_td2, "a", attrs=[["href", "#%s" % ctx.Utils.anchor_name(ctx, _xml_endl)]], # pylint: disable=unused-variable
                                                         content=("%s" % (ctx.Command.name(ctx, xml_menu, _xml_endl)))) # pylint: disable=bad-continuation
                _xml_td3 = ctx.out.add_node(_xml_tr, "td") # pylint: disable=unused-variable
                _xml_td4 = ctx.out.add_node(_xml_tr, "td")
                ctx.Utils.put_node_help(ctx, _xml_td4, ctx.Command.description(ctx, xml_menu, _xml_endl))

        @staticmethod
        def commands(ctx, xml_body, xml_menu):
            """ Generation menu commands documentation.
                @param ctx (Cli2Help) Execution context.
                @param xml_body (XML node) body output node.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node. """
            for _xml_endl in xml_menu.cli_Cli2xxx_commands:
                ctx.Command.execute(ctx, xml_body, xml_menu, _xml_endl)

    class Command(object):
        """ Command documentation: describe a given command. """

        def __init__(self):
            """ Constructor. """
            # Static class, nothing to be done
            pass

        @staticmethod
        def execute(ctx, xml_body, xml_menu, xml_endl):
            """ Executes the class generation routines.
                @param ctx (Cli2Help) Execution context.
                @param xml_body (XML node) body output node.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line. """
            ctx.Command.generate(ctx, xml_body, xml_menu, xml_endl)

        @staticmethod
        def generate(ctx, xml_body, xml_menu, xml_endl):
            """ Generate command line documentation.
                @param ctx (Cli2Help) Execution context.
                @param xml_body (XML node) body output node.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line. """
            # Title
            ctx.out.add_node(xml_body, "a", attrs=[["name", ctx.Utils.anchor_name(ctx, xml_endl)]])
            ctx.out.add_node(xml_body, "h3", content=ctx.Command.name(ctx, xml_menu, xml_endl))

            # Synopsis
            _xml_p = ctx.out.add_node(xml_body, "p")
            ctx.out.add_node(_xml_p, "span", attrs=[["class", "command-section"]],
                                             content=ctx.Utils.translate(ctx, "Synopsis:")) # pylint: disable=bad-continuation
            ctx.out.add_node(_xml_p, "br")
            ctx.out.add_node(_xml_p, "span", attrs=[["class", "command-synopsis"]],
                                             content=ctx.Command.synopsis(ctx, xml_menu, xml_endl)) # pylint: disable=bad-continuation

            # Description
            _xml_p = ctx.out.add_node(xml_body, "p")
            ctx.out.add_node(_xml_p, "span", attrs=[["class", "command-section"]],
                                             content=ctx.Utils.translate(ctx, "Description:")) # pylint: disable=bad-continuation
            ctx.out.add_node(_xml_p, "br")
            ctx.Utils.put_node_help(ctx, _xml_p, ctx.Command.description(ctx, xml_menu, xml_endl))

            # Options
            if ctx.Command.has_options(ctx, xml_menu, xml_endl):
                _xml_p = ctx.out.add_node(xml_body, "p")
                ctx.out.add_node(_xml_p, "u", content=ctx.Utils.translate(ctx, "Options:"))

                _xml_table = ctx.out.add_node(_xml_p, "table")
                ctx.Command.options(ctx, _xml_table, xml_menu, xml_endl)

            # Parameters
            if ctx.Command.has_params(ctx, xml_menu, xml_endl):
                _xml_p = ctx.out.add_node(xml_body, "p")
                ctx.out.add_node(_xml_p, "u", content=ctx.Utils.translate(ctx, "Parameters:"))

                _xml_table = ctx.out.add_node(_xml_p, "table")
                ctx.Command.params(ctx, _xml_table, xml_menu, xml_endl)

            # Leading to menu
            _xml_menu = ctx.xml.xpath_node(xml_endl, "./cli:menu")
            if _xml_menu is not None:
                _menu_name = ""
                _anchor_name = ""
                if ctx.xml.is_node_with_attr(_xml_menu, "cli:menu", "@name"):
                    _menu_name = ctx.xml.attr_value(_xml_menu, "@name")
                    _anchor_name = ctx.Utils.anchor_name(ctx, _xml_menu)
                elif ctx.xml.is_node_with_attr(_xml_menu, "cli:menu", "@ref"):
                    _menu_name = ctx.xml.attr_value(_xml_menu, "@ref")
                    _xml_menu_ref = ctx.cache.node(_xml_menu).cli_Cli2xxx_menu
                    if _xml_menu_ref is not None:
                        _anchor_name = ctx.Utils.anchor_name(ctx, _xml_menu_ref)
                    else:
                        ctx.Utils.abort(ctx, _xml_menu, "unknown menu reference '%s'" % _menu_name)
                else:
                    ctx.Utils.abort(ctx, _xml_menu, "missing either @name or @ref attribute for cli:menu node")

                _xml_p = ctx.out.add_node(xml_body, "p")
                ctx.out.add_content(_xml_p, ctx.Utils.translate(ctx, "This command opens the menu "))
                ctx.out.add_node(_xml_p, "a", attrs=[["href", "#%s" % _anchor_name]], content=_menu_name)
                ctx.out.add_content(_xml_p, ctx.Utils.translate(ctx, "."))

        @staticmethod
        def name(ctx, xml_menu, xml_endl):
            """ Command name computation.
                @param ctx (Cli2Help) Execution context.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line.
                @return (str) Name for the command line. """
            if getattr(xml_endl, "cli_Cli2Help_name", None) is None:
                ctx.Command.start_recursion(ctx, xml_menu, xml_endl)
            return xml_endl.cli_Cli2Help_name

        @staticmethod
        def synopsis(ctx, xml_menu, xml_endl):
            """ Synopsis computation.
                @param ctx (Cli2Help) Execution context.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line.
                @return (str) Synopsis of the command line. """
            if getattr(xml_endl, "cli_Cli2Help_synopsis", None) is None:
                ctx.Command.start_recursion(ctx, xml_menu, xml_endl)
            return xml_endl.cli_Cli2Help_synopsis

        @staticmethod
        def description(ctx, xml_menu, xml_endl):
            """ Description computation.
                @param ctx (Cli2Help) Execution context.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line.
                @return (str) Description of the command line. """
            if getattr(xml_endl, "cli_Cli2Help_description", None) is None:
                ctx.Command.start_recursion(ctx, xml_menu, xml_endl)
            return xml_endl.cli_Cli2Help_description

        @staticmethod
        def has_options(ctx, xml_menu, xml_endl):
            """ Determines whether the command line has options or not.
                @param ctx (Cli2Help) Execution context.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line.
                @return (boolean) True if the command line has options, False otherwise. """
            if getattr(xml_endl, "cli_Cli2Help_options", None) is None:
                ctx.Command.start_recursion(ctx, xml_menu, xml_endl)
            return len(xml_endl.cli_Cli2Help_options) > 0

        @staticmethod
        def options(ctx, xml_out, xml_menu, xml_endl):
            """ Generate options documentation for the given command line.
                @param ctx (Cli2Help) Execution context.
                @param xml_out (XML node) Node which options documentation should be output to.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line. """
            if ctx.Command.has_options(ctx, xml_menu, xml_endl):
                for _option in xml_endl.cli_Cli2Help_options:
                    _xml_tr = ctx.out.add_node(xml_out, "tr")
                    _xml_td1 = ctx.out.add_node(_xml_tr, "td")
                    ctx.out.add_node(_xml_td1, "span", attrs=[["class", "option-name"]], content=_option["name"])
                    _xml_td2 = ctx.out.add_node(_xml_tr, "td")
                    ctx.Utils.put_node_help(ctx, _xml_td2, _option["description"])

        @staticmethod
        def has_params(ctx, xml_menu, xml_endl):
            """ Determines whether the command line has parameters or not.
                @param ctx (Cli2Help) Execution context.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line.
                @return (boolean) True if the command line has parameters, False otherwise. """
            if getattr(xml_endl, "cli_Cli2Help_params", None) is None:
                ctx.Command.start_recursion(ctx, xml_menu, xml_endl)
            return len(xml_endl.cli_Cli2Help_params) > 0

        @staticmethod
        def params(ctx, xml_out, xml_menu, xml_endl):
            """ Generate parameters documentation for the given command line.
                @param ctx (Cli2Help) Execution context.
                @param xml_out (XML node) Node which parameters documentation should be output to.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line. """
            if ctx.Command.has_params(ctx, xml_menu, xml_endl):
                for _param in xml_endl.cli_Cli2Help_params:
                    _xml_tr = ctx.out.add_node(xml_out, "tr")
                    _xml_td1 = ctx.out.add_node(_xml_tr, "td")
                    ctx.out.add_node(_xml_td1, "span", attrs=[["class", "param-name"]], content=_param["name"])
                    _xml_td2 = ctx.out.add_node(_xml_tr, "td")
                    ctx.out.add_node(_xml_td2, "span", attrs=[["class", "param-type"]], content=_param["type"])
                    _xml_td3 = ctx.out.add_node(_xml_tr, "td")
                    ctx.Utils.put_node_help(ctx, _xml_td3, _param["description"])

        class RecursionContext(object):
            """ Recursion context variable management. Named parameter idiom. """

            NAME = "name"
            SYNOPSIS = "synopsis"
            DESCRIPTION = "description"
            OPTIONS = "options"
            PARAMS = "params"

            def __init__(self, copy=None):
                """ Constructor.
                    @param copy (RecursionContext) Optional argument. Iterator to copy. """
                self._xml_menu = None
                self._xml_endl = None
                self._operations = None
                self._xml_start = None
                self._xml_current = None
                self._xml_end = None
                self._tag_except = None
                self._rec_count = 0
                if copy is not None:
                    self._xml_menu = copy.menu()
                    self._xml_endl = copy.endl()
                    self._operations = copy.operations()
                    self._xml_start = copy.start()
                    self._xml_current = copy.current()
                    self._xml_end = copy.end()
                    self._tag_except = copy.tag_except()
                    self._rec_count = copy.rec_count() + 1

            def menu(self):
                """ cli:cli or cli:menu accessor.
                    @return (XML node) cli:cli or cli:menu XML node. """
                return self._xml_menu
            def endl(self):
                """ cli:endl accessor.
                    @return (XML node) cli:endl XML node. """
                return self._xml_endl
            def set_command(self, ctx, xml_menu, xml_endl):
                """ Current command setter.
                    @param ctx (Cli2Help) Execution context.
                    @param xml_menu (XML node) cli:cli or cli:menu XML node.
                    @param xml_endl (XML node) cli:endl XML node.
                    @return (RecursionContext) self. """
                if ctx is None: # [W0613(unused-argument)] Unused argument 'ctx'
                    pass
                self._xml_menu = xml_menu
                self._xml_endl = xml_endl
                return self

            def operations(self):
                """ All operations being processed accessor.
                    @return (Array of str) Processed operations. """
                return self._operations

            def start(self):
                """ Recursion start node acessor.
                    @return (XML node) Recursion start node. """
                return self._xml_start
            def current(self):
                """ Current recursion node accessor.
                    @return (XML node) Current recursion node. """
                return self._xml_current
            def end(self):
                """ Recursion end node accessor.
                    @return (XML node) Recursion end node. """
                return self._xml_end
            def tag_except(self):
                """ Recursion cli:tag exception accessor.
                    @return (XML node) None or cli:tag XML node an exception should be processed for this recursion step. """
                return self._tag_except
            def set_bounds(self, ctx, xml_start, xml_end, current=None, tag_except=None):
                """ Recursion bounds positionning.
                    @param ctx (Cli2Help) Execution context.
                    @param xml_start (XML node) Recusion start node.
                    @param xml_end (XML node) Recursion end node.
                    @param current (XML node) Recursion current node.
                    @param tag_except (XML node) None or cli:tag XML node an exception should be processed for this recursion step.
                    @return (RecursionContext) self. """
                _init_cache_attributes = ((self._xml_end is None) or (xml_end != self._xml_end))
                self._xml_start = xml_start
                self._xml_end = xml_end
                self._xml_current = xml_end
                if current is not None:
                    self._xml_current = current
                self._tag_except = tag_except
                if ctx.xml.is_node(xml_end, "cli:endl"):
                    self._operations = [self.NAME, self.SYNOPSIS, self.DESCRIPTION, self.OPTIONS, self.PARAMS]
                else:
                    self._operations = [self.SYNOPSIS, self.DESCRIPTION, self.OPTIONS, self.PARAMS]
                # Initialize cache attributes
                if _init_cache_attributes:
                    self._xml_cache().cli_Cli2Help_name = ""
                    self._xml_cache().cli_Cli2Help_synopsis = ""
                    self._xml_cache().cli_Cli2Help_description = {
                        "text": "",
                        "auto": True
                    }
                    self._xml_cache().cli_Cli2Help_options = []
                    self._xml_cache().cli_Cli2Help_params = []
                return self
            def next_parent(self, ctx):
                """ Recursion iteration: move on next parent.
                    @param ctx (Cli2Help) Execution context.
                    @return (bool) True if a parent has been found, False otherwise. """
                if ctx.xml.is_node_with_attr(self._xml_current, "cli:tag", "@id"):
                    if ( # pylint: disable=bad-continuation
                        (self.tag_except() == self._xml_current)
                        and (not ctx.xml.is_node_with_attr_value(self._xml_current, "cli:tag", "@hollow", "yes"))
                    ):
                        self._xml_current = ctx.xml.parent_node(self._xml_current)
                    else:
                        self._xml_current = None
                else:
                    self._xml_current = ctx.xml.parent_node(self._xml_current)
                if self._xml_current is not None:
                    # Once we have moved to the parent node, loose any _tag_except status.
                    self._tag_except = None
                return self._xml_current is not None
            def rec_count(self):
                """ Parent recursion calls count accessor.
                    @return (int) Number of recursion calls prior to this one. """
                return self._rec_count
            def log_indent(self):
                """ Log indentation computation.
                    @return (str) Log indentation, depending on the number of recursion calls prior to this one. """
                _indent = str(self).replace("__main__.RecursionContext instance at ", "")
                _i_left = self._rec_count
                while _i_left > 0:
                    _indent += "-"
                    _i_left -= 1
                return _indent

            def _xml_cache(self):
                """ Returns the XML node that will receive cache results.
                    @return (XML node) XML node that will receive cache results. """
                return self.end()
            def name(self):
                """ Name computation accessor.
                    @return (str) Current name computation. """
                return self._xml_cache().cli_Cli2Help_name
            def more_name(self, ctx, name_part):
                """ Name computation extension.
                    @param ctx (Cli2Help) Execution context.
                    @param name_part (str) Part of name for extension. """
                if ctx is None: # [W0613(unused-argument)] Unused argument 'ctx'
                    pass
                if self._xml_cache().cli_Cli2Help_name != "":
                    self._xml_cache().cli_Cli2Help_name += " "
                self._xml_cache().cli_Cli2Help_name += name_part
            def synopsis(self):
                """ Synopsis computation accessor.
                    @return (str) Current synopsis computation. """
                return self._xml_cache().cli_Cli2Help_synopsis
            def more_synopsis(self, ctx, synopsis_part):
                """ Synopsis computation extension.
                    @param ctx (Cli2Help) Execution context.
                    @param synopsis_part (str) Part of synopsis for extension. """
                if ctx is None: # [W0613(unused-argument)] Unused argument 'ctx'
                    pass
                if (self._xml_cache().cli_Cli2Help_synopsis != "") and (synopsis_part != ""):
                    if ( # pylint: disable=bad-continuation
                        (self._xml_cache().cli_Cli2Help_synopsis.endswith("[") or self._xml_cache().cli_Cli2Help_synopsis.endswith("("))
                        and (synopsis_part != "|")
                    ):
                        pass
                    elif ((synopsis_part == "]") or (synopsis_part == ")")) and (not self._xml_cache().cli_Cli2Help_synopsis.endswith("|")):
                        pass
                    elif (synopsis_part == "*") and self._xml_cache().cli_Cli2Help_synopsis.endswith("]"):
                        pass
                    else:
                        self._xml_cache().cli_Cli2Help_synopsis += " "
                self._xml_cache().cli_Cli2Help_synopsis += synopsis_part
            def description(self):
                """ Description computation accessor.
                    @return (Tuple: see ctx.Utils.get_node_help()) Current description computation. """
                return self._xml_cache().cli_Cli2Help_description
            def set_description(self, ctx, description):
                """ Description computation setter.
                    @param ctx (Cli2Help) Execution context.
                    @param description (Tuple: see ctx.Utils.get_node_help()) Description to set.
                    @warning Description ignored if already set. """
                if self._xml_cache().cli_Cli2Help_description["text"] == "":
                    if (description is None) or (description["text"] is None) or (description["text"] == ""):
                        self._xml_cache().cli_Cli2Help_description = {
                            "text": ctx.Utils.translate(ctx, "No help available"),
                            "auto": True
                        }
                    else:
                        self._xml_cache().cli_Cli2Help_description = description
            def options(self):
                """ Options computation accessor.
                    @return ([{"name"|"description", str}] Current options computation. """
                return self._xml_cache().cli_Cli2Help_options
            def add_option(self, ctx, option_name, option_description):
                """ Options computation extension.
                    @param ctx (Cli2Help) Execution context.
                    @param option_name (str) Name of option. May be empty, option will be ignored in this case.
                    @param option_description (Tuple: see ctx.Utils.get_node_help()) Option description. """
                if ctx is None: # [W0613(unused-argument)] Unused argument 'ctx'
                    pass
                if len(option_name) > 0:
                    _option = {
                        "name": option_name,
                        "description": option_description
                    }
                    self._xml_cache().cli_Cli2Help_options.append(_option)
            def params(self):
                """ Parameters computation accessor.
                    @return ([{"name"|"type"|"description", str) Current parameters computation. """
                return self._xml_cache().cli_Cli2Help_params
            def add_param(self, ctx, param_name, param_type, param_description):
                """ Parameters computation extension.
                    @param ctx (Cli2Help) Execution context.
                    @param param_name (str) Parameter name.
                    @param param_type (str) Paremeter type.
                    @param param_description (Tuple: see ctx.Utils.get_node_help()) Parameter description. """
                if ctx is None: # [W0613(unused-argument)] Unused argument 'ctx'
                    pass
                _param = {
                    "name": param_name,
                    "type": param_type,
                    "description": param_description
                }
                self._xml_cache().cli_Cli2Help_params.append(_param)

        @staticmethod
        def start_recursion(ctx, xml_menu, xml_endl):
            """ Start recursion for the given command line.
                @param ctx (Cli2Help) Execution context.
                @param xml_menu (XML node) Current cli:cli or cli:menu[@name] input node.
                @param xml_endl (XML node) Current cli:endl node terminating a command line. """
            # Launch recursion
            _rec = ctx.Command.RecursionContext()
            _rec.set_command(ctx, xml_menu, xml_endl)
            _rec.set_bounds(ctx, xml_menu, xml_endl)
            modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, _rec.log_indent() + "Start of recursion:")
            ctx.Command.recursion(ctx, _rec)
            # Sort options
            _dict = {}
            for _option in xml_endl.cli_Cli2Help_options:
                _dict[_option["name"]] = _option
            xml_endl.cli_Cli2Help_options = []
            _keys = _dict.keys()
            _keys.sort()
            for _key in _keys:
                xml_endl.cli_Cli2Help_options.append(_dict[_key])
            # Sort parameters
            _dict = {}
            for _param in xml_endl.cli_Cli2Help_params:
                _dict[_param["name"]] = _param
            xml_endl.cli_Cli2Help_params = []
            _keys = _dict.keys()
            _keys.sort()
            for _key in _keys:
                xml_endl.cli_Cli2Help_params.append(_dict[_key])
            # Trace recursion result
            if modlog.engine().is_enabled(ctx.MODLOG_FILTER, modlog.DEBUG):
                _trace = ""
                _trace += ("name = '%s', " % _rec.name())
                _trace += ("synopsis = '%s', " % _rec.synopsis())
                _trace += ("description = '%s'/auto=%d, " % (_rec.description()["text"], _rec.description()["auto"]))
                _trace += ("%d options, " % len(_rec.options()))
                _trace += ("%d params" % len(_rec.params()))
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, "Command found: " + _trace)

        @staticmethod
        def recursion(ctx, rec):
            """ Recursion routine.
                @param ctx (Cli2Help) Execution context.
                @param rec (RecursionContext) Recursion context. """
            # --- Debug trace ---
            ctx.Command.trace_recursion(ctx, rec)

            # --- Stop conditions ---
            # General stop conditions.
            if ( # pylint: disable=bad-continuation
                (rec.current() == rec.start())
                or ctx.xml.is_node(rec.current(), "cli:cli") or ctx.xml.is_node_with_attr(rec.current(), "cli:menu", "@name")
                or (
                    ctx.xml.is_node_with_attr(rec.end(), "cli:tag", "@ref")
                    and ctx.xml.is_node_with_attr_value(rec.current(), "cli:tag", "@id", ctx.xml.attr_value(rec.end(), "@ref"))
                )
            ):
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "End of recursion")
                return

            # --- Pre-recursion node processing
            if (rec.DESCRIPTION in rec.operations()) and (rec.description()["text"] == ""):
                if ( # pylint: disable=bad-continuation
                    ctx.Utils.is_cli_or_menu_node(ctx, rec.current())
                    or ctx.Utils.is_hollow_tag_node(ctx, rec.current())
                ):
                    rec.set_description(ctx, {"text": "", "auto": True})
                if not ctx.Utils.get_node_help(ctx, rec.current())["auto"]:
                    if ( # pylint: disable=bad-continuation
                        ctx.xml.is_node(rec.current(), "cli:endl") or ctx.xml.is_node(rec.current(), "cli:keyword")
                        or ctx.xml.is_node(rec.current(), "cli:tag")
                    ):
                        rec.set_description(ctx, ctx.Utils.get_node_help(ctx, rec.current()))

            # --- Recursive call over parent nodes ---
            _rec2 = ctx.Command.RecursionContext(rec)
            if _rec2.next_parent(ctx):
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, _rec2.log_indent() + "Recursive call over parent node")
                ctx.Command.recursion(ctx, _rec2)

            # --- Post-recursion node processing ---
            if ctx.xml.is_node(rec.current(), "cli:endl"):
                # cli:endl
                pass
            elif ctx.xml.is_node(rec.current(), "cli:keyword"):
                # cli:keyword
                if rec.NAME in rec.operations():
                    rec.more_name(ctx, ctx.xml.attr_value(rec.current(), "@string"))
                if rec.SYNOPSIS in rec.operations():
                    rec.more_synopsis(ctx, ctx.xml.attr_value(rec.current(), "@string"))
            elif ctx.xml.is_node(rec.current(), "cli:param"):
                # cli:param
                if rec.NAME in rec.operations():
                    rec.more_name(ctx, "<%s>" % ctx.xml.attr_value(rec.current(), "@id"))
                if rec.SYNOPSIS in rec.operations():
                    rec.more_synopsis(ctx, "<%s>" % ctx.xml.attr_value(rec.current(), "@id"))
                if rec.PARAMS in rec.operations():
                    rec.add_param(
                        ctx,
                        param_name=ctx.xml.attr_value(rec.current(), "@id"),
                        param_type=("(%s)" % ctx.xml.attr_value(rec.current(), "@type")),
                        param_description=ctx.Utils.get_node_help(ctx, rec.current())
                    )
            elif ctx.xml.is_node_with_attr(rec.current(), "cli:tag", "@id"):
                # cli:tag[@id]
                if rec.current() != rec.tag_except(): # Do not process cli:tag nodes twice
                    modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "Tag recursion...")
                    ctx.Command.tag_recursion(ctx, rec)
            elif ctx.xml.is_node_with_attr(rec.current(), "cli:tag", "@ref"):
                # cli:tag[@ref]
                pass
            else:
                ctx.Utils.abort(ctx, rec.current(), "Command.recursion(): unknown node '%s'" % ctx.xml.node_name(rec.current()))

        @staticmethod
        def trace_recursion(ctx, rec):
            """ Trace recursion routine.
                @param ctx (Cli2Help) Execution context.
                @param rec (RecursionContext) Recursion context. """
            if modlog.engine().is_enabled(ctx.MODLOG_FILTER, modlog.DEBUG):
                _trace = rec.log_indent()
                _trace += (ctx.Utils.node_log(ctx, "start", rec.start()) + ", ")
                _trace += (ctx.Utils.node_log(ctx, "current", rec.current()) + ", ")
                _trace += (ctx.Utils.node_log(ctx, "end", rec.end()) + ", ")
                _trace += "operation = ("
                for _operation in rec.operations():
                    _trace += _operation
                    if _operation != rec.operations()[-1]:
                        _trace += ", "
                _trace += "), "
                _trace += ("%s" % ctx.Utils.node_log(ctx, "tag_except", rec.tag_except()))
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, _trace)

        @staticmethod
        def tag_recursion(ctx, rec):
            """ Tag recursion routine (current node is cli:tag[@id]).
                @param ctx (Cli2Help) Execution context.
                @param rec (RecursionContext) Recursion context.
                @return (type depends on operation parameter) Operation result. """
            # Cache tag info
            rec.xml_tag_id = ctx.cache.node(rec.current())
            rec.tag_id = ctx.xml.attr_value(rec.xml_tag_id, "@id")
            rec.xml_last_common_node = rec.xml_tag_id.cli_Cli2xxx_common_parent
            rec.xml_tag_refs = rec.xml_tag_id.cli_Cli2xxx_tag_refs
            if len(rec.xml_tag_refs) == 0:
                # Dummy tag: do nothing
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, "Dummy tag: do nothing")
            # Chech whether the cli:tag[@id] node is an ancestor of the last common node.
            elif ctx.xml.is_ancestor_or_self(rec.xml_tag_id, rec.xml_last_common_node):
                # Backward tag
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "Backward tag recursion...")
                ctx.Command.backward_tag_recursion(ctx, rec)
            else:
                # Forward tag
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "Forward tag recursion...")
                ctx.Command.forward_tag_recursion(ctx, rec)

        @staticmethod
        def backward_tag_recursion(ctx, rec):
            """ Backward tag recursion routine.
                @param ctx (Cli2Help) Execution context.
                @param rec (RecursionContext) Recursion context.
                @return (type depends on operation parameter) Operation result. """
            # General backward tag recursive call should not be done in general routines by now
            modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "General backward tag recursion:")
            _rec2 = ctx.Command.RecursionContext(rec)
            _rec2.set_bounds(ctx, rec.start(), rec.end(), current=rec.xml_last_common_node, tag_except=rec.current())
            ctx.Command.recursion(ctx, _rec2)

            if rec.NAME in rec.operations():
                rec.more_name(ctx, "[...]")
            for _xml_backward_tag_ref in rec.xml_tag_refs[1:]: # Omit the first item which is the cli:tag[@id] node itself
                if ctx.xml.attr_value(_xml_backward_tag_ref, "@max") is None:
                    ctx.Utils.warn(ctx, _xml_backward_tag_ref, "Backward cli:tag[@ref='%s'] with no @max attribute. Considering @max='unbounded'" % rec.tag_id)

                if rec.SYNOPSIS in rec.operations():
                    rec.more_synopsis(ctx, "[")

                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "Other backward tag[@ref] recursion:")
                _rec2 = ctx.Command.RecursionContext(rec)
                _rec2.set_bounds(ctx, rec.xml_tag_id, _xml_backward_tag_ref, tag_except=_xml_backward_tag_ref)
                ctx.Command.recursion(ctx, _rec2)
                if rec.OPTIONS in rec.operations():
                    rec.add_option( # pylint: disable=bad-continuation
                        ctx,
                        option_name=_rec2.synopsis(),
                        option_description=_rec2.description()
                    )
                    for _option in _rec2.options():
                        rec.add_option(ctx, _option["name"], _option["description"])
                if rec.PARAMS in rec.operations():
                    for _param in _rec2.params():
                        rec.add_param(ctx, _param["name"], _param["type"], _param["description"])
                if rec.SYNOPSIS in rec.operations():
                    rec.more_synopsis(ctx, _rec2.synopsis())

                if rec.SYNOPSIS in rec.operations():
                    rec.more_synopsis(ctx, "]")
                    _xml_max_attr = ctx.xml.attr_value(_xml_backward_tag_ref, "@max")
                    if (_xml_max_attr is None) or (_xml_max_attr == "unbounded"):
                        rec.more_synopsis(ctx, "*")

        @staticmethod
        def forward_tag_recursion(ctx, rec):
            """ Forward tag recursion routine.
                @param ctx (Cli2Help) Execution context.
                @param rec (RecursionContext) Recursion context.
                @return (type depends on operation parameter) Operation result. """
            if rec.xml_last_common_node is not None:
                modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "Forward tag common path recursion:")
                _rec2 = ctx.Command.RecursionContext(rec)
                _rec2.set_bounds(ctx, rec.start(), rec.end(), current=rec.xml_last_common_node, tag_except=rec.current())
                ctx.Command.recursion(ctx, _rec2)

                if rec.NAME in rec.operations():
                    rec.more_name(ctx, "(...)")

                if rec.SYNOPSIS in rec.operations():
                    rec.more_synopsis(ctx, "(")

                # For each cli:tag[@ref] and cli:tag[@id and not(@hollow='yes')]
                for _xml_ref in rec.xml_tag_refs:
                    modlog.log(ctx.MODLOG_FILTER, modlog.DEBUG, rec.log_indent() + "For each tag[@ref] or self forward tag recursion")
                    _rec2 = ctx.Command.RecursionContext(rec)
                    _rec2.set_bounds(ctx, rec.xml_last_common_node, _xml_ref, tag_except=_xml_ref)
                    ctx.Command.recursion(ctx, _rec2)
                    if rec.OPTIONS in rec.operations():
                        rec.add_option( # pylint: disable=bad-continuation
                            ctx,
                            option_name=_rec2.synopsis(),
                            option_description=_rec2.description()
                        )
                        for _option in _rec2.options():
                            rec.add_option(ctx, _option["name"], _option["description"])
                    if rec.PARAMS in rec.operations():
                        for _param in _rec2.params():
                            rec.add_param(ctx, _param["name"], _param["type"], _param["description"])

                    if rec.SYNOPSIS in rec.operations():
                        if _xml_ref in rec.xml_tag_refs[1:]:
                            rec.more_synopsis(ctx, "|")
                        rec.more_synopsis(ctx, _rec2.synopsis())

                if rec.SYNOPSIS in rec.operations():
                    rec.more_synopsis(ctx, ")")

    class Utils(clicommon.CtxUtils):
        """ Utils routines. """

        def __init__(self):
            """ Constructor. """
            # Static class, nothing to be done
            clicommon.CtxUtils.__init__(self)

        @staticmethod
        def is_cli_or_menu_node(ctx, xml_node):
            """ Menu node identification.
                @param ctx (Cli2Help) Execution context.
                @param xml_node (XML node) Current input XML node: self::cli:*.
                @return True if the current node is either the root cli:cli node or a cli:menu[@name] node. """
            if ctx.xml.is_node(xml_node, "cli:cli"):
                return True
            if ctx.xml.is_node_with_attr(xml_node, "cli:menu", "@name"):
                return True
            return False

        @staticmethod
        def is_hollow_tag_node(ctx, xml_node):
            """ Hollow tag node identification.
                @param ctx (Cli2Help) Execution context.
                @param xml_node (XML node) Current input XML node: self::cli:*.
                @return True if the current node is a cli:tag[@id] with @hollow='yes'. """
            if ctx.xml.is_node_with_attr(xml_node, "cli:tag", "@id"):
                if ctx.xml.attr_value(xml_node, "@hollow") == "yes":
                    return True
            return False

        @staticmethod
        def get_node_help(ctx, xml_node):
            """ Direct help accessor.
                @param ctx (Cli2Help) Execution context.
                @param xml_node (XML node) Current input XML node: self::cli:*.
                @return (Tuple of "text": (str), "auto": (bool)) "text": Node help, "auto": True if this an automatic help message. """
            _help_res = None
            if getattr(xml_node, "cli_Cli2Help_helps", None) is None:
                xml_node.cli_Cli2Help_helps = ctx.xml.xpath_set(xml_node, "./cli:help")
                # Look for the node with the corresponding language
                if len(xml_node.cli_Cli2Help_helps) > 1:
                    for _xml_help in xml_node.cli_Cli2Help_helps:
                        if ctx.xml.attr_value(_xml_help, "@lang") == ctx.args.lang():
                            # If found keep only this one.
                            xml_node.cli_Cli2Help_helps = [_xml_help]
                            break
            if len(xml_node.cli_Cli2Help_helps) > 0:
                _help_res = {
                    "text": ctx.xml.content(xml_node.cli_Cli2Help_helps[0]),
                    "auto": False
                }
            elif ctx.xml.is_node(xml_node, "cli:param"):
                _help_res = {
                    "text": ctx.Utils.translate(ctx, "%s value" % ctx.xml.attr_value(xml_node, "@type")),
                    "auto": True
                }
            else:
                _help_res = {
                    "text": ctx.Utils.translate(ctx, "No help available"),
                    "auto": True
                }
            return _help_res

        @staticmethod
        def put_node_help(ctx, xml_out, help_info):
            """ Help output routine.
                @param ctx (Cli2Help) Execution context.
                @param xml_out (XML node) Output XML node.
                @param help_info (Tuple: see ctx.Utils.get_node_help()) Help message. """
            _help_msg = None
            _auto = True
            if help_info is not None:
                if (help_info["text"] is not None) and (help_info["text"] != ""):
                    _help_msg = help_info["text"]
                    _auto = help_info["auto"]
            if _help_msg is None:
                _help_msg = ctx.Utils.translate(ctx, "No help available")
            if _auto:
                ctx.out.add_node(xml_out, "span", attrs=[["class", "default"]], content=_help_msg)
            else:
                ctx.out.add_content(xml_out, content=_help_msg)

        @staticmethod
        def anchor_name(ctx, xml_node):
            """ Anchor name computation.
                @param ctx (Cli2Help) Execution context.
                @param xml_node (XML node) Current input XML node: self::cli:*.
                @return (str) Anchor identifier. """
            xml_node = ctx.cache.node(xml_node)
            return str(xml_node.cli_Cli2xxx_global_index)

        @staticmethod
        def translate(ctx, message):
            """ Resource translation.
                @param ctx (Cli2Help) Execution context.
                @param message (str) English resource string to translate.
                @return (str) Message translated into the given language. """
            if ctx.args.lang() == "en":
                return message

            _tr = {}
            _tr[" (general presentation)"] = {
                "fr": " (présentation générale)"
            }
            _tr[" (main menu)"] = {
                "fr": " (menu principal)"
            }
            _tr["This command line interface uses the following patterns for comments definition:"] = {
                "fr": "Cette interface en ligne de commande utilise les motifs suivants pour la définition de commentaires :"
            }
            _tr[" is composed of the following menus:"] = {
                "fr": " est composé des menus suivants :"
            }
            _tr[" is composed of the following commands:"] = {
                "fr": " est composé des commandes suivantes :"
            }
            _tr["Command Line Interface "] = {
                "fr": "Interface en ligne de commande (CLI) "
            }
            _tr["Command Line Interface documentation"] = {
                "fr": "Documentation d'interface en ligne de commande (CLI)"
            }
            _tr["Description:"] = {
                "fr": "Description :"
            }
            _tr["float value"] = {
                "fr": "Valeur décimale"
            }
            _tr["host value"] = {
                "fr": "Adresse réseau"
            }
            _tr["int value"] = {
                "fr": "Valeur entière"
            }
            _tr["Menu "] = {
                "fr": "Menu "
            }
            _tr["No help available"] = {
                "fr": "Aucune aide disponible"
            }
            _tr["Options:"] = {
                "fr": "Options :"
            }
            _tr["Parameters:"] = {
                "fr": "Paramètres :"
            }
            _tr["string value"] = {
                "fr": "Chaîne de caractères"
            }
            _tr["Synopsis:"] = {
                "fr": "Synopsis :"
            }
            _tr["This command opens the menu "] = {
                "fr": "Cette commande ouvre le menu "
            }
            _tr["."] = {
                "fr": "."
            }

            try:
                if ctx.args.lang() == "fr":
                    import codecs
                    _iso = codecs.lookup("utf-8")
                    return _iso.decode(_tr[message]["fr"])[0]
            except KeyError:
                pass
            modlog.log(ctx.MODLOG_FILTER, modlog.WARNING, "No translation for '%s'" % message)
            return message

        @staticmethod
        def node_log(ctx, var_name, xml_node):
            """ XML node log builder.
                @param var_name (str) Name of parameter.
                @param xml_node (XML node) XML node.
                @return (str) XML node log string. """
            _res = ("%s = " % var_name)
            if xml_node is None:
                _res += "None"
            else:
                _res += ctx.xml.node_name(xml_node)
                if ctx.xml.attr_value(xml_node, "@name") is not None:
                    _res += ("[@name=%s]" % ctx.xml.attr_value(xml_node, "@name"))
                elif ctx.xml.attr_value(xml_node, "@string") is not None:
                    _res += ("[@string=%s]" % ctx.xml.attr_value(xml_node, "@string"))
                elif ctx.xml.attr_value(xml_node, "@id") is not None:
                    _res += ("[@id=%s]" % ctx.xml.attr_value(xml_node, "@id"))
                elif ctx.xml.attr_value(xml_node, "@ref") is not None:
                    _res += ("[@ref=%s]" % ctx.xml.attr_value(xml_node, "@ref"))
            return _res

# Main call
main()
