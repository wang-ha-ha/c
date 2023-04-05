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


""" XML document management.
    @note Uses libxml2.
    @note libxml2 uses utf-8 as the regular encoding, which is not ASCII nor unicode.
          Any strings read from libxml2 must be decoded from utf-8,
          and any string passed to libxml2 must be encoded to utf-8. """

import xml.etree.ElementTree as xml
import modlog


class InputDocument(object):
    """ XML reader class. """
    MODLOG_FILTER = "XmlInDoc"

    def __init__(self):
        """ Constructor. """
        modlog.engine().set_filter(self.MODLOG_FILTER, modlog.INFO)
        self._doc = None
        self._path = None
        self._ns = {}

    def _not_static(self):
        """ Dummy method designed for method that could be static, and we want to maintain as regular methods. """
        return self is not None

    # Reader methods

    def load_file(self, path):
        """ Load an XML file.
            @param path (str) XML file path.
            @return (boolean) True for success, False otherwise. """
        #modlog.log(self.MODLOG_FILTER, modlog.TRACE, "load_file(%s)" % path)
        try:
            self._doc = xml.parse(path)
            self._path = path
            self._doc.getroot().cli_XmlInDoc_parent = None
            for _parent in self._doc.getiterator():
                for _child in _parent:
                    _child.cli_XmlInDoc_parent = _parent

            return True
        except xml.ParseError, _pe:
            modlog.log(self.MODLOG_FILTER, modlog.ERROR, str(_pe))
            return False

    def encoding(self):
        """ Retrieves encoding of the input file.
            @warning No regular libxml2 function found for that...
            @return (str) Encoding name. Defaults to utf-8. """
        _search_step = 0
        _file = open(self._path, "r")
        for _line in _file.readlines():
            _line = _line.strip()
            if _line.startswith("<?"):
                # OK, we have found a start of meta-tag pattern, now look for the encoding meta-attribute name.
                _search_step = 1
                _line = _line[_line.find("<?")+len("<?"):]
            if _search_step == 1:
                if "encoding" in _line:
                    # OK, we have found the encoding meta-attribute name
                    _search_step = 2
                    _line = _line[_line.find("encoding")+len("encoding"):]
            if _search_step == 2:
                _line = _line.strip()
                if _line.startswith("="):
                    # OK, we have found the '=' sign.
                    _search_step = 3
                    _line = _line[1:]
            if _search_step == 3:
                _line = _line.strip()
                if _line.startswith("\""):
                    # OK, we have found the '"' sign.
                    _search_step = 4
                    _line = _line[1:]
                    if "\"" in _line:
                        _line = _line[:_line.find("\"")]
                        return _line
                    else:
                        # Default to utf-8
                        return "utf-8"
            if "?>" in _line:
                # End of meta-tag encountered. Default to utf-8.
                return "utf-8"
        return "utf-8"

    def set_ns_shortcut(self, shortcut, ns_uri):
        """ Define a XML namespace shortcut.
            @param shortcut (str) Shortcut (eg. 'cli').
            @param ns_uri (str) XML namespace URI (eg. 'http://alexis.royer.free.fr/CLI') """
        self._ns[ns_uri] = shortcut

    def _short2universal_ns(self, xpath):
        """ Convert namespace shortcuts to full expanded universal namespaces.
            @param xpath (str) XPath expression with namespace shortcuts.
            @return (str) XPath expression with full expanded namespaces. """
        _xpath = xpath
        for _uri in self._ns:
            _xpath = _xpath.replace(self._ns[_uri] + ":", "{" + _uri + "}")
        return _xpath

    def _universal2short_ns(self, xpath, with_ns=True):
        """ Convert full expanded universal namespaces to namespace shortcuts.
            @param xpath (str) XPath expression with full expanded namespaces.
            @param with_ns (bool) False if namespaces should be removed, True (default) if their shortcut should be replaced.
            @return (str) XPath expression with namespace shortcuts. """
        _xpath = xpath
        for _uri in self._ns.keys():
            _replace = ""
            if with_ns:
                _replace = self._ns[_uri] + ":"
            _xpath = _xpath.replace("{" + _uri + "}", _replace)
        return _xpath

    def root_node(self):
        """ XML root node accessor.
            @return (XML node) XML root node. """
        _xml_root_node = self._doc.getroot()
        return _xml_root_node

    def node_name(self, node, with_ns=True):
        """ Retrieves the node name of the given node, with its namespace if any.
            @param node (XML node) Node which name to retrieve.
            @return (str) Node name. Empty string node is not a valid node. """
        _ns = ""
        _node_name = ""
        if (node is not None) and (node != self._doc) and (node.tag is not None):
            _node_name = self._universal2short_ns(node.tag, with_ns=with_ns)
        return _ns + _node_name

    def attr_value(self, xml_node, attr_name):
        """ Retrieves the attribute value.
            @param xml_node (XML node) Node which attribute to retrieve.
            @param attr_name (str) Name of attribute to rerieve for the given node.
            @return (str) Attribute value if found, None otherwise. """
        _xml_node = xml_node
        if _xml_node is None:
            _xml_node = self.root_node()
        _attr_name = attr_name
        if _attr_name.startswith("@"):
            _attr_name = _attr_name[1:]
        _attr_value = _xml_node.get(_attr_name)
        #modlog.log(self.MODLOG_FILTER, modlog.TRACE, "attr_value(xml_node = '%s', attr_name = '%s') -> '%s'"
        #                                             % (self.node_name(_node), _attr_name, _attr_value))
        return _attr_value

    def content(self, xml_node):
        """ Retrieves the node content.
            @param xml_node (XML node) Node which content to retrieve.
            @return (str) Node content. """
        self._not_static()
        _content = ""
        if xml_node is not None:
            if xml_node.text is not None:
                _content += xml_node.text
            for _xml_child in xml_node:
                if _xml_child.tail is not None:
                    _content += _xml_child.tail
        #modlog.log(self.MODLOG_FILTER, modlog.TRACE, "content(xml_node = '%s') -> '%s'" % (self.node_name(xml_node), _content))
        return _content

    def parent_node(self, xml_node):
        """ Retrieves the parent node of the given node.
            @param xml_node (XML node) Node to retrieve the parent of.
            @return (XML node) Parent node, None if no parent found. """
        self._not_static()
        _xml_parent_node = getattr(xml_node, "cli_XmlInDoc_parent", None)
        return _xml_parent_node

    def is_ancestor_or_self(self, xml_ancestor, xml_child):
        """ Determines whether xml_ancestor is actually an ancestor of xml_child, or equals xml_child.
            @param xml_ancestor (XML node) Ancestor to be checked.
            @param xml_child (XML node) Child to be checked.
            @return (bool) True is xml_ancestor is actually an ancestor of or equals xml_child, False otherwise. """
        _xml_parent = xml_child
        while (_xml_parent is not None) and (xml_ancestor is not None):
            if _xml_parent == xml_ancestor:
                return True
            _xml_parent = self.parent_node(_xml_parent)
        return False

    def children(self, xml_node):
        """ Retrieves children nodes of the given node, including text nodes (whose tags ar None).
            @param xml_node (XML node) Node to retrieve the children of.
            @return ([XML node]) Children. Empty set if no children found. """
        self._not_static()
        _xml_children = []
        if xml_node is not None:
            if xml_node.text is not None:
                _xml_text = xml.Element(None)
                _xml_text.text = xml_node.text
                _xml_children.append(_xml_text)
            for _xml_child in xml_node:
                _xml_children.append(_xml_child)
                if _xml_child.tail is not None:
                    _xml_text = xml.Element(None)
                    _xml_text.text = _xml_child.tail
                    _xml_children.append(_xml_text)
        #modlog.log(self.MODLOG_FILTER, modlog.DEBUG, "children(xml_node = '%s') -> %d nodes" % (self.node_name(xml_node), len(_xml_children)))
        return _xml_children

    def is_node(self, xml_node, node_name):
        """ Determines whether the node name matches the given searched name.
            @param xml_node (XML node) Node which name to test.
            @param node_name (str) Searched node name, possibly with namespace.
            @return (bool) True if the node matches the given name, false otherwise. """
        _match = self._xpath_match_self(xml_node, self._short2universal_ns(node_name))
        return _match is not None

    def is_node_with_attr(self, xml_node, node_name, attr_name):
        """ Determines whether the node name matches the given searched name, and has an attribute of the given name.
            @param xml_node (XML node) Node which name to test.
            @param node_name (str) Searched node name, possibly with namespace.
            @param attr_name (str) Attrbute name to check the existence.
            @return (bool) True if the node matches the given name and has an attribute of the given name, false otherwise. """
        if self.is_node(xml_node, node_name):
            if self.attr_value(xml_node, attr_name) is not None:
                return True
        return False

    def is_node_with_attr_value(self, xml_node, node_name, attr_name, attr_value):
        """ Determines whether the node name matches the given searched name, and has an attribute of the given name with the given value.
            @param xml_node (XML node) Node which name to test.
            @param node_name (str) Searched node name, possibly with namespace.
            @param attr_name (str) Attribute name to check the existence.
            @param attr_value (str) Expected attribute value.
            @return (bool) True if the node matches the given name and has an attribute of the given name and the expected value, false otherwise. """
        if self.is_node(xml_node, node_name):
            _attr_value = self.attr_value(xml_node, attr_name)
            if _attr_value is not None:
                if _attr_value == attr_value:
                    return True
        return False

    def xpath_set(self, xml_focus, xpath):
        """ XML set XPath query.
            @param xml_focus (XML node) Current XML node, None for an absolute XPath expression.
            @param xpath (str) XPath expression to evaluate.
            @return ([XML node]) Set of XML nodes, empty set if nothing found. """
        # Check focus node
        if xml_focus is None:
            xml_focus = self._doc
        _search = []
        # Split on '|'
        for _xpath in xpath.split("|"):
            _xml_focus = xml_focus
            _xpath = _xpath.strip()
            # Expand namespace shortcuts by their universal names
            _xpath = self._short2universal_ns(_xpath)
            # ElementTree does not manage self:: axis
            if _xpath.startswith("self::"):
                _match = self._xpath_match_self(_xml_focus, _xpath[len("self::"):])
                if _match is None:
                    continue
                else:
                    _xpath = _xpath.replace("self::" + _match, ".")
            # ElementTree does not manage preceding-sibling:: axis
            elif _xpath.startswith("preceding-sibling::"):
                self._xpath_set_preceding_sibling(_search, _xml_focus, _xpath)
                continue
            # ElementTree does not manage preceding:: axis
            elif _xpath.startswith("preceding::"):
                self._xpath_set_preceding(_search, _xml_focus, _xpath)
                continue
            # ElementTree does not manage ancestor:: axis
            elif _xpath.startswith("ancestor::"):
                self._xpath_set_ancestor(_search, _xml_focus, _xpath)
                continue
            # ElementTree warning workaround
            elif _xpath.startswith("//"):
                if "*" in _xpath:
                    # ElementTree returns empty sets for '//' queries with wilcards
                    self._xpath_set_global_wildcard(_search, _xpath)
                    continue
                else:
                    # Start with a "." in order to avoid the following warning:
                    #   FutureWarning: This search is broken in 1.3 and earlier, and will be fixed in a future version.
                    #   If you rely on the current behaviour, change it to './/{<uri>}<tag>'
                    _xml_focus = self._doc
                    _xpath = "." + _xpath
            elif _xpath.startswith("/"):
                _xml_focus = self.root_node()
                _xpath = _xpath[1:]
                _match = self._xpath_match_self(_xml_focus, _xpath)
                if _match is None:
                    continue
                else:
                    _xpath = _xpath.replace(_match, ".")
            elif "*" in _xpath:
                self._xpath_set_child_wildcard(_search, _xml_focus, _xpath)
                continue
            # Execute query
            _search.extend(_xml_focus.findall(_xpath))
        # Output result
        return _search

    def _xpath_set_preceding_sibling(self, _search, _xml_focus, _xpath):
        """ xpath_set() for "preceding-sibling::" queries.
            @param _search ([XML Node]) xpath_set() local variable.
            @param _xml_focus (XML node) xpath_set() local variable.
            @param _xpath (str) xpath_set() local variable. """
        _iter = self.children(self.parent_node(_xml_focus))
        for _xml_node in _iter:
            if _xml_node == _xml_focus:
                break
            _match = self._xpath_match_self(_xml_node, _xpath)
            if _match is None:
                # Mismatch.
                continue
            _subpath = _xpath.replace(_match, ".")
            if _subpath == ".":
                # No need to make a recursive call
                _search.append(_xml_node)
            else:
                _search.extend(self.xpath_set(_xml_node, _subpath))

    def _xpath_set_preceding(self, _search, _xml_focus, _xpath):
        """ xpath_set() for "preceding::" queries.
            @param _search ([XML Node]) xpath_set() local variable.
            @param _xml_focus (XML node) xpath_set() local variable.
            @param _xpath (str) xpath_set() local variable. """
        _iter = self._doc.getiterator()
        for _xml_node in _iter:
            if _xml_node == _xml_focus:
                break
            # Note: preceding:: axis does not return ancestors
            _is_ancestor_or_self = self.is_ancestor_or_self(xml_ancestor=_xml_node, xml_child=_xml_focus)
            if not _is_ancestor_or_self:
                if getattr(_xml_node, "precedings_cache_self", None) is None:
                    _xml_node.precedings_cache_self = {}
                if _xml_node.precedings_cache_self.has_key(_xpath):
                    # This "preceding" query has already been processed on that node
                    pass
                else:
                    _match = self._xpath_match_self(_xml_node, _xpath)
                    if _match is None:
                        # Mismatch. Empty set.
                        _xml_node.precedings_cache_self[_xpath] = []
                    else:
                        _subpath = _xpath.replace(_match, ".")
                        if _subpath == ".":
                            # No need to make a recursive call
                            _xml_node.precedings_cache_self[_xpath] = [_xml_node]
                        else:
                            _xml_node.precedings_cache_self[_xpath] = self.xpath_set(_xml_node, _subpath)
                _search.extend(_xml_node.precedings_cache_self[_xpath])

    def _xpath_set_ancestor(self, _search, _xml_focus, _xpath):
        """ xpath_set() for "ancestor::" queries.
            @param _search ([XML Node]) xpath_set() local variable.
            @param _xml_focus (XML node) xpath_set() local variable.
            @param _xpath (str) xpath_set() local variable. """
        _xml_ancestor = self.parent_node(_xml_focus)
        while _xml_ancestor is not None:
            _match = self._xpath_match_self(_xml_ancestor, _xpath)
            if _match is not None:
                _subpath = _xpath.replace(_match, ".")
                if _subpath == ".":
                    # No need to make a recursive call
                    _search.append(_xml_ancestor)
                else:
                    _search.append(self.xpath_set(_xml_ancestor, _subpath))
            _xml_ancestor = self.parent_node(_xml_ancestor)

    def _xpath_set_global_wildcard(self, _search, _xpath):
        """ xpath_set() for '//' queries with wilcards.
            @param _search ([XML Node]) xpath_set() local variable.
            @param _xpath (str) xpath_set() local variable. """
        for _xml_node in self._doc.getiterator():
            _match = self._xpath_match_self(_xml_node, _xpath)
            if _match is not None:
                _subpath = _xpath.replace(_match, ".")
                if _subpath == ".":
                    # No need to make a recursive call
                    _search.append(_xml_node)
                else:
                    _search.append(self.xpath_set(_xml_node, _subpath))

    def _xpath_set_child_wildcard(self, _search, _xml_focus, _xpath):
        """ xpath_set() for child queries with wilcards.
            @param _search ([XML Node]) xpath_set() local variable.
            @param _xpath (str) xpath_set() local variable. """
        for _xml_node in _xml_focus:
            _match = self._xpath_match_self(_xml_node, _xpath)
            if _match is not None:
                _subpath = _xpath.replace(_match, ".")
                if _subpath == ".":
                    # No need to make a recursive call
                    _search.append(_xml_node)
                else:
                    _search.append(self.xpath_set(_xml_node, _subpath))

    def xpath_node(self, xml_focus, xpath):
        """ XML node XPath query.
            @param xml_focus (XML node) Current XML node, None for an absolute XPath expression.
            @param xpath (str) XPath expression to evaluate.
            @return (XML node) XML node if found, first one if several found, None if nothing found. """
        _xml_node = None
        _set = self.xpath_set(xml_focus, xpath)
        if len(_set) > 0:
            if len(_set) > 1:
                modlog.log(self.MODLOG_FILTER, modlog.WARNING, "xpath_node('%s'): several node found, but only one is returned" % xpath)
            _xml_node = _set[0]
        return _xml_node

    def xpath_test(self, focus, xpath):
        """ XML boolean XPath query.
            @param focus (XML node) Current XML node, None for an absolute XPath expression.
            @param xpath (str) XPath expression to evaluate.
            @return (boolean) True if the XPath evaluates to something, False otherwise. """
        return len(self.xpath_set(focus, xpath)) > 0

    # ElementTree is not fully xpath-compliant:
    #  - self:: and preceding:: axis missing
    #  - root node name match missing
    def _xpath_match_self(self, xml_focus, xpath):
        """ Checks that the given node matches the given xpath expression along the self:: axis.
            @param xml_focus (XML node) XML node to be checked.
            @param xpath (str) Self XPath expression to check on the given XML node.
            @return (str) None if the expression did not match, matching expression otherwise. """
        _xpath = xpath

        # Axis
        _axis = ""
        if _xpath.startswith("self::"):
            _axis = "self::"
            _xpath = _xpath[len(_axis):]
        elif _xpath.startswith("preceding-sibling::"):
            _axis = "preceding-sibling::"
            _xpath = _xpath[len(_axis):]
        elif _xpath.startswith("preceding::"):
            _axis = "preceding::"
            _xpath = _xpath[len(_axis):]
        elif _xpath.startswith("ancestor::"):
            _axis = "ancestor::"
            _xpath = _xpath[len(_axis):]
        elif _xpath.startswith("//"):
            _axis = "//"
            _xpath = _xpath[len(_axis):]

        # Namespace
        _ns = ""
        if _xpath.startswith("{") and ("}" in _xpath):
            _ns = _xpath[:_xpath.find("}")+1]
            _xpath = _xpath[len(_ns):]

        # Remove additional xpath expression for sub-nodes
        if "/" in _xpath:
            _xpath = _xpath[:_xpath.find("/")]

        # Predicates
        _predicates = ""
        if _xpath.endswith("]") and ("[" in _xpath):
            _predicates = _xpath[_xpath.find("["):]
            _xpath = _xpath[:-len(_predicates)]

        # Check node name
        if xml_focus.tag is None:
            # Text node
            return None
        elif xml_focus.tag == _ns + _xpath:
            pass
        elif xml_focus.tag.startswith(_ns) and (_xpath == "*"):
            pass
        else:
            return None

        # Check predicates.
        if _predicates != "":
            _set = self.xpath_set(xml_focus, "../%s%s%s" % (_ns, _xpath, _predicates))
            if xml_focus not in _set:
                return None

        # Match is OK
        _res = _axis + _ns + _xpath + _predicates
        return _res

class OutputDocument(object):
    """ XML writer class. """
    MODLOG_FILTER = "XmlOutDoc"

    def __init__(self, out):
        """ Constructor.
            @param out (clicommon.Output) Output device.
            @param output (clicommon.Output) Final output instance. """
        modlog.engine().set_filter(self.MODLOG_FILTER, modlog.TRACE)
        self._doc = None
        self._ns = {} # Not really used at present.
        self._encoding = None
        self._doc_comments = []
        self._out = out

    def _not_static(self):
        """ Dummy method designed for method that could be static, and we want to maintain as regular methods. """
        return self is not None

    def _node_name(self, node):
        """ Retrieves the node name of the given node, with its namespace if any.
            @param node (XML node) Node which name to retrieve.
            @return (str) Node name. """
        self._not_static()
        _ns = ""
        _node_name = ""
        if node is not None:
            _node_name = node.tag
        return _ns + _node_name

    def add_root_node(self, node_name):
        """ Root node creation.
            @param node_name (str) name of node.
            @return (XML node) New node. """
        _xml_root_node = xml.Element(node_name)
        self._doc = xml.ElementTree(_xml_root_node)
        return _xml_root_node

    def add_comment(self, xml_node, comment):
        """ Comment creation.
            @param xml_node (XML node) Node to create a comment for. May be None in order to create a comment at the root of the document.
            @param comment (str) Text of comment. """
        _xml_comment = xml.Comment(comment)
        if xml_node is not None:
            xml_node.append(_xml_comment)
        else:
            self._doc_comments.append(_xml_comment)

    def add_node(self, xml_focus, node_name, attrs=None, content=None):
        """ Node creation.
            @param xml_focus (XML node) Node to create a child for.
            @param node_name (str) Name of the new node.
            @param attrs ([[str,str]]) Optional. Attributes with their value.
            @param content (str) Optional. Text content.
            @return (XML node) New node. """
        #_trace = "Node '%s': new child '%s'" % (self._node_name(xml_focus), node_name)
        _xml_new_node = xml.Element(node_name)
        if attrs is not None:
            for _attr in attrs:
                #_trace += (", @%s='%s'" % (_attr[0], _attr[1]))
                _xml_new_node.set(_attr[0], _attr[1])
            # Store an 'attrs' attribute in order to memorize attribute order.
            _xml_new_node.attrs = attrs
        if content is not None:
            #_trace += (", content='%s'" % content)
            self.add_content(_xml_new_node, content)
        xml_focus.append(_xml_new_node)
        #modlog.log(self.MODLOG_FILTER, modlog.DEBUG, _trace)
        return _xml_new_node

    def add_content(self, xml_focus, content):
        """ Node content setting.
            @param xml_focus (XML node) Node which content to set.
            @param content (str) Content to set. """
        self._not_static()
        _xml_text = xml.Element(None)
        _xml_text.text = content
        xml_focus.append(_xml_text)

    def write(self, encoding="utf-8"):
        """ Document output.
            @param encoding (str) Optional. Encoding to use  for output document. """
        #modlog.log(self.MODLOG_FILTER, modlog.TRACE, "Dumping XML output with encoding '%s'" % encoding)

        # XML header
        self._out.put("<?xml version=\"1.0\" encoding=\"%s\"?>\n" % encoding)

        # Add doc comments
        for _xml_comment in self._doc_comments:
            self._out.put(xml.tostring(_xml_comment) + "\n")

        # Generate XML body
        self._write_node(encoding, self._doc.getroot(), 0)

        # Effectively flush output
        self._out.flush()

    def _write_node(self, encoding, xml_node, indent_count):
        """ XML node output. Recursive method.
            @param encoding (str) Optional. Encoding to use  for output document.
            @param xml_node (XML node) Node to output.
            @para indent_count (int) Indentation. -1 for no indentation. """
        if xml_node.tag == xml.Comment:
            # Comment node
            self._put_indentation(indent_count)
            self._put_xml_string(xml.tostring(xml_node, encoding=encoding))
            if indent_count >= 0:
                self._out.put("\n")
        elif xml_node.tag is None:
            # Text node
            self._put_xml_string(xml.tostring(xml_node, encoding=encoding))
        else:
            self._put_indentation(indent_count)
            self._out.put("<")
            self._out.put("%s" % xml_node.tag)
            # Rely on 'attrs' attribute in order to respect attribute order.
            if getattr(xml_node, "attrs", None) is not None:
                for _attr in xml_node.attrs:
                    self._out.put(" %s=\"%s\"" % (_attr[0], _attr[1]))
            if len(xml_node) == 0:
                self._out.put("/>")
                if indent_count >= 0:
                    self._out.put("\n")
            else:
                self._out.put(">")
                _indent_content = -1
                if indent_count >= 0:
                    _indent_content = indent_count + 1
                    for _xml_child in xml_node:
                        if _xml_child.tag is None:
                            # Text node
                            _indent_content = -1
                            break
                if _indent_content >= 0:
                    self._out.put("\n")
                for _xml_child in xml_node:
                    self._write_node(encoding, _xml_child, _indent_content)
                if _indent_content >= 0:
                    self._put_indentation(indent_count)
                self._out.put("</%s>" % xml_node.tag)
                if indent_count >= 0:
                    self._out.put("\n")

    def _put_indentation(self, indent_count):
        """ Indentation output.
            @param indent_count (int) Indentation. -1 for no indentation. """
        self._not_static()
        if indent_count >= 0:
            _indent_count = indent_count
            while _indent_count > 0:
                self._out.put("  ")
                _indent_count -= 1

    def _put_xml_string(self, xml_string):
        """ XML string output.
            @param xml_string (str) XML string to output. """
        self._not_static()
        _b_first_line = True
        for _line in xml_string.split("\n"):
            if not _b_first_line:
                self._out.put("\n")
            if not _line.startswith("<?xml"):
                self._out.put(_line)
                _b_first_line = False
