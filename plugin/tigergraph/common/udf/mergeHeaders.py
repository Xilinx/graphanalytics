#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright 2021 Xilinx, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import re
import argparse
from typing import List, Set
import sys


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class MergeException(Exception):
    def __init__(self, message, file_name, line_num):
        self.message = message
        self.file_name = file_name
        self.line_num = line_num
        self.strError = self.make_error_string(message, file_name, line_num)

    @staticmethod
    def make_error_string(message, file_name, line_num):
        return file_name + ':' + str(line_num) + ':' + ' error: ' + message


class VarProcessor:
    def __init__(self, def_strings: List[str]) -> None:
        self.var_map = {}
        for def_string in def_strings:
            equal_pos = def_string.find('=')
            if equal_pos < 0:
                continue
            var_name = def_string[0 : equal_pos]
            if not var_name:
                continue
            self.var_map[var_name] = def_string[equal_pos + 1 :]

    def _start_line(self, line: str) -> None:
        self._cur_line = line
        self._index = 0
        self._mark = 0

    def _get_char(self) -> str:
        if self._index >= len(self._cur_line):
            return None
        ret = self._cur_line[self._index]
        self._index += 1
        return ret

    def _push_back(self, c) -> None:
        if c:
            self._index -= 1

    def _mark_pos(self) -> None:
        self._mark = self._index

    def _get_marked_text(self) -> str:
        return self._cur_line[self._mark : self._index]

    def expand(self, line: str) -> str:
        self._start_line(line)
        ret = ''
        c = self._get_char()
        while c:
            if c == '$':
                # set up failed var string in case var doesn't match
                failed_var = '$'
                self._mark_pos()

                # check for (.  If not found, put back the original text and bail
                open_paren = self._get_char()
                if open_paren != '(':
                    failed_var += self._get_marked_text()
                    ret += failed_var
                    c = self._get_char()
                    continue

                # read var name
                var_name = ''
                c = self._get_char()
                while c and c != ')' and c != '=':
                    var_name += c
                    c = self._get_char()

                # if = found, read default value
                default_value = ''
                has_default_value = False
                if c == '=':
                    has_default_value = True
                    c = self._get_char()
                    while c and c != ')':
                        default_value += c
                        c = self._get_char()

                # if the variable doesn't end correctly, put back the original text and bail
                if c != ')':
                    failed_var += self._get_marked_text()
                    ret += failed_var
                    c = self._get_char()
                    continue

                # if var is in the dictionary, substitute its value
                val = self.var_map.get(var_name)
                if val:
                    ret += val

                # no such variable: either substitute the default value or put back the original text
                else:
                    if has_default_value:
                        ret += default_value
                    else:
                        failed_var += self._get_marked_text()
                        ret += failed_var

            # something other than $: just emit the character
            else:
                ret += c
            c = self._get_char()
        return ret


class Tag:
    def __init__(self, src_file_name='', src_line_num=0, tag_str=''):
        self.tag_type = ''  # file, section, etc.
        self.section_type = ''  # include, header, body, footer, etc.
        self.start_or_end = 'start'
        self.file_name = ''
        if not tag_str:
            self.is_valid = True
            return

        self.is_valid = False
        tokens = tag_str.split()
        version, tokens = self.get_token(tokens, src_file_name, src_line_num)
        if version != '1':
            raise MergeException('mergeHeaders version "' + version + '" is not supported', src_file_name, src_line_num)
        self.tag_type, tokens = self.get_token(tokens, src_file_name, src_line_num)

        # logical name tag
        if self.tag_type == 'name':
            self.file_name, tokens = self.get_token(tokens, src_file_name, src_line_num)

        # section tag
        elif self.tag_type == 'section':
            self.section_type, tokens = self.get_token(tokens, src_file_name, src_line_num)
            self.start_or_end, tokens = self.get_token(tokens, src_file_name, src_line_num)
            if self.start_or_end != 'start' and self.start_or_end != 'end':
                raise MergeException('start or end expected', src_file_name, src_line_num)
            self.file_name, tokens = self.get_token(tokens, src_file_name, src_line_num)

        else:
            raise MergeException('Unrecognized tag type ' + self.tag_type, src_file_name, src_line_num)
        self.is_valid = True

    @staticmethod
    def get_token(tokens, src_file_name, src_line_num, is_required=True):
        if len(tokens) < 1:
            if is_required:
                raise MergeException('Missing tag token', src_file_name, src_line_num)
            return '', tokens
        else:
            return tokens[0], tokens[1:]

    def is_name(self):
        return self.tag_type == 'name'

    def is_start(self):
        return self.tag_type == 'section' and self.start_or_end == 'start'

    def is_end(self):
        return self.tag_type == 'section' and self.start_or_end == 'end'

    def is_group(self):
        return self.file_name == '@'


class Section:
    def __init__(self, tag=None, is_root=False):
        self.tag = tag
        self.items = []  # holds lines (strings) and Sections
        self.start_tag_line = None
        self.end_tag_line = None
        self.is_emitted = False  # if true, this section has been emitted to the merged file
        self.is_root = is_root
        self.subsections = {}  # key: [section type, file_name], value: Section object

    def add_item(self, line):
        self.items.append(line)

    def set_start_tag_line(self, line):
        self.start_tag_line = line

    def set_end_tag_line(self, line):
        self.end_tag_line = line

    def is_untagged(self):
        return not self.is_root and self.tag is None

    def is_group(self):
        if self.is_root:
            return True
        if self.is_untagged():
            return False
        return self.tag.is_group()

    def get_section_type(self):
        return self.get_section_type_from_tag(self.tag)

    def get_file_name(self):
        return self.get_file_name_from_tag(self.tag)

    @staticmethod
    def get_section_type_from_tag(tag):
        return '' if tag is None else tag.section_type

    @staticmethod
    def get_file_name_from_tag(tag):
        return '' if tag is None else tag.file_name

    def matches_tag(self, tag):
        return self.get_section_type_from_tag(tag) == self.get_section_type_from_tag(self.tag)

    def get_subsection(self, section_type, file_name) -> 'Section':
        if (section_type, file_name) in self.subsections:
            return self.subsections[section_type, file_name]
        return None

    def get_subsection_by_tag(self, tag) -> 'Section':
        return self.get_subsection(self.get_section_type_from_tag(tag), self.get_file_name_from_tag(tag))

    def add_subsection(self, section):
        if (section.get_section_type(), section.get_file_name()) in self.subsections:
            return
        self.items.append(section)
        self.subsections[section.get_section_type(), section.get_file_name()] = section


class MergeFile:
    def __init__(self, file_name: str, varProcessor: VarProcessor, is_template=False):
        self.is_valid = True
        self.contents = Section(is_root=True)
        self.file_name = file_name
        self.logical_name = file_name  # set according to 'name' tag
        self.has_logical_name = False  # true if name tag present in file
        self.line_num = 0
        self.is_template = is_template

        tagRe = re.compile('^\s*//\s*mergeHeaders')
        section_stack = [self.contents]  # stack starts with empty root section
        cur_section = section_stack[-1]
        try:
            template_file = open(file_name, 'r')
            for line in template_file:
                # Newline-terminate any non-terminated lines and do variable substitution
                if line[-1] != '\n':
                    line += '\n'
                line = varProcessor.expand(line)
                self.line_num += 1

                # Check to see if the current line is a tag.  If so, process the tag
                match = tagRe.match(line)
                if match:
                    try:
                        tag = Tag(self.file_name, self.line_num, line[match.end():])
                        # If tag is a name tag, set the logical name of this file to that of the tag
                        if tag.is_name():
                            if self.has_logical_name:
                                self.emit_error('File already has a name tag')
                            else:
                                self.logical_name = tag.file_name
                                self.has_logical_name = True

                        # Not a name tag: make sure any other kind of tag has a matching name, and process that tag
                        else:
                            if not is_template and tag.file_name != self.logical_name:
                                self.emit_error("Section file name " + tag.file_name
                                        + " does not match file's logical name " + self.logical_name)

                            # If start tag found, try to start a new section
                            if tag.is_start():
                                # If there is an existing non-group open section, close it
                                if not cur_section.is_group():
                                    # If it's an untagged section, it's now finished
                                    if cur_section.is_untagged():
                                        section_stack[-2].add_subsection(cur_section)
                                    # Not an untagged section: we already have a tagged section open, so starting
                                    # another one is an error
                                    else:
                                        self.emit_error('Two section starts found with no intervening section end')
                                    cur_section = section_stack.pop()

                                # Append to existing tagged section or start a new tagged section
                                cur_section = cur_section.get_subsection_by_tag(tag)
                                if cur_section is None:
                                    cur_section = Section(tag)
                                section_stack.append(cur_section)
                                cur_section.set_start_tag_line(line)

                            # End tag: close the current tagged section
                            elif tag.is_end():
                                if len(section_stack) == 1:
                                    self.emit_error('Section end found with no corresponding section start')
                                elif not cur_section.matches_tag(tag):
                                    self.emit_error('Section end does not match section start')  # leave section open
                                else:
                                    cur_section.set_end_tag_line(line)
                                    section_stack.pop()
                                    section_stack[-1].add_subsection(cur_section)
                                    cur_section = section_stack[-1]
                    except MergeException as ex:
                        self.emit_error(ex.message)

                # Regular (non-tag) line: add the line to the current section
                else:
                    cur_section.add_item(line)

            # If there is an open section, close it
            while section_stack:
                cur_section = section_stack.pop()
                if not cur_section.is_root:
                    self.emit_error('End of file reached in open section')
                if section_stack:
                    section_stack[-1].add_subsection(cur_section)

            template_file.close()
        except OSError as ex:
            eprint("Couldn't open file " + file_name + " for reading: " + ex.strerror)
            self.is_valid = False

    def emit_error(self, message):
        eprint(MergeException.make_error_string(message, self.file_name, self.line_num))
        self.is_valid = False


class MergeFileSet:
    def __init__(self):
        self.file_list = []
        self.file_dict = {}

    def add(self, mf: MergeFile):
        if mf.logical_name in self.file_dict:
            return
        self.file_list.append(mf)
        self.file_dict[mf.logical_name] = mf


def emit_text(text):
    print(text, end='')


def emit_section(section: Section, emit_func):
    if section.start_tag_line:
        emit_func(section.start_tag_line)
    for item in section.items:
        if isinstance(item, Section):
            emit_section(item, emit_func)
        else:
            emit_func(item)
    if section.end_tag_line:
        emit_func(section.end_tag_line)


def install(template: MergeFile, files: MergeFileSet, emit_func) -> bool:
    # Loop through all items of the template file, making substitutions from the given files
    is_valid = True
    for item in template.contents.items:
        # If section, make sure that it is a section group (has @ as file name), and merge contents
        if isinstance(item, Section):
            # If the section is not a group, emit an error but continue anyway
            if not item.is_group():
                eprint("Non-group section found at top level in template file!")
                is_valid = False

            # Emit the group's start tag
            if item.start_tag_line:
                emit_func(item.start_tag_line)

            # Emit the group's contents
            for group_item in item.items:
                # If the group has a subsection, try to find a matching section from a merging file.  If found,
                # reinstall the merging file by substituting the merging file's section in place of the template file's
                # section.  Otherwise, just emit the template file's section
                if isinstance(group_item, Section):
                    if group_item.get_file_name() in files.file_dict:
                        mf = files.file_dict[group_item.get_file_name()]
                        replacement_section = mf.contents.get_subsection_by_tag(group_item.tag)
                        if replacement_section:
                            emit_section(replacement_section, emit_func)
                            replacement_section.is_emitted = True
                        else:
                            emit_section(group_item, emit_func)
                    else:
                        emit_section(group_item, emit_func)
                else:
                    emit_func(group_item)

            # For any merging file sections yet to be added to this group, install (for the first time) the files
            # by emitting the sections in file order
            for file in files.file_list:
                section = file.contents.get_subsection(item.get_section_type(), file.logical_name)
                if section and not section.is_emitted:
                    emit_section(section, emit_func)

            # Emit the group's end tag
            if item.end_tag_line:
                emit_func(item.end_tag_line)
        else:
            emit_func(item)  # plain text line
    return is_valid


def uninstall(template: MergeFile, file_names: Set[str], emit_func):
    # Loop through all items of the template file, removing sections whose logical names are given in file_names
    is_valid = True
    for item in template.contents.items:
        # If section, make sure that it is a section group (has @ as file name), and process contents
        if isinstance(item, Section):
            # If the section is not a group, emit an error but continue anyway
            if not item.is_group():
                eprint("Non-group section found at top level in template file!")
                is_valid = False

            # Emit the group's start tag
            if item.start_tag_line:
                emit_func(item.start_tag_line)

            # Emit the group's contents, minus the files to remove
            for group_item in item.items:
                # If the group has a subsection, try to find its name in the set of names to remove.  If found,
                # uninstall the section by omitting it.  Otherwise, just emit it
                if isinstance(group_item, Section):
                    if group_item.get_file_name() not in file_names:
                        emit_section(group_item, emit_func)
                else:
                    emit_func(group_item)

            # Emit the group's end tag
            if item.end_tag_line:
                emit_func(item.end_tag_line)
        else:
            emit_func(item)  # plain text line
    return is_valid


def main():
    # Parse command line args
    argParser = argparse.ArgumentParser(description="Installs/uninstalls C++ header files into a template header file.",
        formatter_class=argparse.RawDescriptionHelpFormatter, epilog="""\
This utility installs C++ header fragments into a C++ header template.

TEMPLATE FILE
-------------

The header template needs to be annotated with 'group tags' of the form:

// mergeHeaders <version> section <section type> (start | end) @

where:
    <version> is the mergeHeaders version number (currently 1)
    <section type> is a label for a category of section, such as 'include'
        for the includes area of the header, or 'body' for the header body.

For example:

// mergeHeaders 1 section include start @

Each 'start' tag must be paired with a corresponding 'end' tag, with any
amount of lines of text in between, forming a 'group'.  Groups do not nest;
however, after the merge is complete, the merged file will contain 'sections'
nested within those groups originating from the header fragment files.

HEADER FRAGMENT FILES
---------------------

A header fragment file consists of 'sections' with intervening text.  Only
sections get merged into the final output; intervening text is discarded.

Similar to a group, a 'section' consists of a start tag, text lines, and
an end tag, but the tags are slightly different from those of the template:

// mergeHeaders <version> section <section type> (start | end) <file name>

where:
    <file name> is the name of the enclosing header fragment, without path
    
Within a header fragment, <file name> can be changed with a 'name tag':

// mergeHeaders <version> name <file name>

All start and end section tags must use the same file name as the name tag.
It is an error for the file to contain more than one name tag.

OPERATING MODES
---------------

mergeHeaders has three operating modes: install, reinstall, and uninstall.
The output of all modes is sent to stdout.

For install mode, you supply a template file and one or more header fragment
files.  The sections defined within the header fragments are installed into
the template's groups in the same order as given on the command line.
The merged output is sent to stdout.

For reinstall mode, you supply an already-merged header file as the template
file and one or more header fragment files to reinstall.  Any section in
the header file with a corresponding section in the fragment file is replaced
by the fragment file section, preserving the location of the section in the
header file.

For uninstall mode, you supply an already-merged header file as the template
file and one or more NAMES of header fragments to uninstall.  Note that these
names are not the physical file names as known to the OS, but rather are
the logical names used as the <file name> field of the section tags.
Therefore, if you have used a name tag to change the name of a header
fragment, it is that name that you should supply on the command line.

mergeHeaders operates in install or reinstall mode by default.  The two modes
are actually the same: if a header fragment section is present in the
template file, it is replaced; otherwise, it is inserted at the end of the
group.  To operate in uninstall mode, pass the -u or --uninstall flag.

VARIABLE SUBSTITUTION
---------------------

mergeHeaders also does variable substitution for variables found in the
template file or header fragment files.  Variable substitution is performed
before tags are inspected.

Variables are of the form:

$(<variable name>[=<default value>])

where:

    <variable name> is any text other than newline, =, or )
    <default value> is any text other than ) or a newline
    
For example:

$(MY_VAR)
$(MY VAR = default value)

Note that any whitespace becomes part of the variable name or default value.
Variable values are specified on the command line with an argument of the form

<variable name>=<value>

The value may be left empty to supply an empty string.

During substitution, variables that have no default value and are absent from
the command line are left AS IS.  Syntax errors in variables are also left
as is.  Variables absent from the command line but which have default values
are replaced with the default value.

For example, suppose we have the following text:

#include "$(PATH1)/header1.hpp"
#include "$(PATH1=/opt/xilinx/apps/cosinesim)/header2.hpp"
#include "$(PATH2=/opt/xilinx/xrt/include)/xrt.h"
#include "$(PATH3)/header3.hpp"

and the command line:

mergeHeaders template.hpp frag.hpp PATH1=../dev

The resulting file will be:

#include "../dev/header1.hpp"
#include "../dev/header2.hpp"
#include "/opt/xilinx/xrt/include/xrt.h"
#include "$(PATH3)/header3.hpp"
""")
    argParser.add_argument('template', help="the template header or previous result of merging")
    argParser.add_argument('args', nargs='+', help="source file names and variable definitions (of the form var=value)")
    argParser.add_argument('-u', '--uninstall', action='store_true')
    args = argParser.parse_args()

    # Separate file names from variable definitions
    file_names = []
    var_defs = []
    for item in args.args:
        if item.find('=') >= 0:
            var_defs.append(item)
        else:
            file_names.append(item)

    # Parse the template file
    var_processor = VarProcessor(var_defs)
    template_mf = MergeFile(args.template, var_processor, True)
    if not template_mf.is_valid:
        eprint('Fatal error: Template file has errors.')
        exit(1)

    # If uninstall mode, treat file names arguments as logical names and do uninstall
    if args.uninstall:
        uninstall(template_mf, set(file_names), emit_text)

    # Install mode: read files specified by file names, parsing them and doing install
    else:
        src_mfs = MergeFileSet()
        is_valid = True
        for arg in file_names:
            src_mf = MergeFile(arg, var_processor)
            if src_mf.is_valid:
                src_mfs.add(src_mf)
            else:
                is_valid = False
        if not is_valid:
            eprint('Fatal error: One or more source files has errors.')
            exit(1)

        install(template_mf, src_mfs, emit_text)

    exit(0)


if __name__ == '__main__':
    main()
