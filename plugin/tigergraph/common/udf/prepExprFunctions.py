#!/usr/bin/python3

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

import argparse
import sys
from enum import Enum


class State(Enum):
    prolog = 0
    headers_started = 1
    before_body = 2
    epilog = 3
    done = 4


def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


class LineSource:
    def __init__(self, file_name: str):
        self.infile = open(file_name, 'r')

    def generator(self):
        for line in self.infile:
            if line[-1] != '\n':
                line += '\n'
            yield line


def main():
    # Parse command line args
    arg_parser = argparse.ArgumentParser(
        description="Annotates given ExprFunctions.hpp with mergeHeaders tags.",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    arg_parser.add_argument('infile', help="the source ExprFunctions.hpp to annotate")
    args = arg_parser.parse_args()

    try:
        line_source = LineSource(args.infile).generator()
        state = State.prolog
        # print("***** prolog")
        while state != State.done:
            if state == State.prolog:
                line = line_source.__next__()
                if line.startswith('#include'):
                    # print("***** headers_started")
                    state = State.headers_started
                print(line, end='')
            elif state == State.headers_started:
                line = line_source.__next__()
                if not line.startswith('#include'):
                    # print("***** before_body")
                    state = State.before_body
                    print("""\

// mergeHeaders 1 section include start @ DO NOT REMOVE!
// mergeHeaders 1 section include end @ DO NOT REMOVE!
""")
                print(line, end='')
            elif state == State.before_body:
                line = line_source.__next__()
                if line.startswith('}'):
                    # print('***** epilog')
                    state = State.epilog
                    print("""\

// mergeHeaders 1 section body start @ DO NOT REMOVE!
// mergeHeaders 1 section body end @ DO NOT REMOVE!
""")
                print(line, end='')
            elif state == State.epilog:
                line = line_source.__next__()
                print(line, end='')
    except StopIteration:
        pass
    except OSError as ex:
        eprint("Couldn't open source file " + args.infile + " for reading: " + ex.strerror)


if __name__ == '__main__':
    main()
