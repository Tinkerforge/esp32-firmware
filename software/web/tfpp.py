#!/usr/bin/env python3

import sys
import argparse
import pathlib
import re
from dataclasses import dataclass

@dataclass
class Define:
    value: int
    location: str

@dataclass
class If:
    value: int
    location: str

@dataclass
class Else:
    value: int
    location: str

def any_zero(ifs_elses):
    return any([if_else.value == 0 for if_else in ifs_elses])

def remove_end(line, end):
    if len(end) > 0 and line.endswith(end):
        line = line[:-len(end)]

    return line

def parse_file(input_path, defines, ifs_elses):
    output_lines = []

    with input_path.open(encoding='utf-8') as input_file:
        for i, line in enumerate(input_file.readlines()):
            line_rstripped = line.rstrip()
            m = re.match(r'^(?:[/\s]*//|\s*\{\s*/\*)\s*#\s*(.*?)(\s*(?:|\*/\s*\}))$', line_rstripped)

            if m != None:
                directive = m.group(1)
                end = m.group(2)
                m = re.match(r'^(include\s|define\s|undef\s|ifdef\s|ifndef\s|if\s|ifn\s|else|endif|region|endregion)\s*(.*?)\s*$', directive)

                if m == None:
                    raise Exception(f'Malformed directive at {input_path}:{i + 1}: {line_rstripped}')

                verb = m.group(1).strip()
                arguments = m.group(2)

                ignore_line = False

                if verb == 'include':
                    m = re.match(r'^(?:"([^"]+)"|\'([^\']+)\')$', arguments)

                    if m == None:
                        raise Exception(f'Malformed path in #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    include_path = input_path.parent / pathlib.Path(m.group(1))

                    if not any_zero(ifs_elses):
                        if not include_path.exists():
                            raise Exception(f'File in #{verb} directive at {input_path}:{i + 1} is missing: {include_path}')

                        parse_file(include_path, defines, ifs_elses)
                elif verb == 'define':
                    m = re.match(r'^([A-Za-z_-][A-Za-z0-9_-]*)\s+(0|1)$', arguments)

                    if m == None:
                        raise Exception(f'Malformed arguments in #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    if not any_zero(ifs_elses):
                        symbol = m.group(1)
                        value = int(m.group(2))
                        define = defines.get(symbol)

                        if define != None:
                            raise Exception(f'Symbol {symbol} in #{verb} directive at {input_path}:{i + 1} is already defined as {define.value} at {define.location}: {line_rstripped}')
                        else:
                            defines[symbol] = Define(value, f'{input_path}:{i + 1}')
                elif verb == 'undef':
                    m = re.match(r'^([A-Za-z_-][A-Za-z0-9_-]*)$', arguments)

                    if m == None:
                        raise Exception(f'Malformed arguments in #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    if not any_zero(ifs_elses):
                        symbol = m.group(1)
                        define = defines.get(symbol)

                        if define == None:
                            raise Exception(f'Symbol {symbol} in #{verb} directive at {input_path}:{i + 1} is not defined: {line_rstripped}')

                        line = remove_end(line_rstripped, end) + f' [defined at {define.location}]{end}\n'
                        defines.pop(symbol)
                elif verb == 'ifdef' or verb == 'ifndef':
                    m = re.match(r'^([A-Za-z_-][A-Za-z0-9_-]*)$', arguments)

                    if m == None:
                        raise Exception(f'Malformed arguments in #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    symbol = m.group(1)
                    value = None

                    if not any_zero(ifs_elses):
                        define = defines.get(symbol)

                        if define != None:
                            value = 1 if verb == 'ifdef' else 0
                            line = remove_end(line_rstripped, end) + f' [defined as {define.value} at {define.location}]{end}\n'
                        else:
                            value = 0 if verb == 'ifdef' else 1
                            line = remove_end(line_rstripped, end) + f' [not defined]{end}\n'

                    ifs_elses.append(If(value, f'{input_path}:{i + 1}'))
                elif verb == 'if' or verb == 'ifn':
                    m = re.match(r'^(1|0|[A-Za-z_-][A-Za-z0-9_-]*)$', arguments)

                    if m == None:
                        raise Exception(f'Malformed arguments in #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    value_or_symbol = m.group(1)
                    value = None

                    if not any_zero(ifs_elses):
                        if value_or_symbol in ['1', '0']:
                            value = int(value_or_symbol)
                        else:
                            symbol = value_or_symbol
                            define = defines.get(symbol)

                            if define == None:
                                raise Exception(f'Symbol {symbol} in #{verb} directive at {input_path}:{i + 1} is not defined: {line_rstripped}')

                            value = define.value
                            line = remove_end(line_rstripped, end) + f' [defined as {value} at {define.location}]{end}\n'

                        if verb == 'ifn':
                            value = 1 - value

                    ifs_elses.append(If(value, f'{input_path}:{i + 1}'))
                elif verb == 'else':
                    if len(arguments) > 0:
                        raise Exception(f'Unexpected arguments in #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    if len(ifs_elses) == 0:
                        raise Exception(f'Missing #if* directive for #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    if_else = ifs_elses.pop()

                    if isinstance(if_else, Else):
                        raise Exception(f'Duplicate #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    value = if_else.value

                    if value != None:
                        value = 1 - value

                    ifs_elses.append(Else(value, f'{input_path}:{i + 1}'))
                elif verb == 'endif':
                    if len(arguments) > 0:
                        raise Exception(f'Unexpected arguments in #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    if len(ifs_elses) == 0:
                        raise Exception(f'Missing #if* directive for #{verb} directive at {input_path}:{i + 1}: {line_rstripped}')

                    ifs_elses.pop()
                elif verb == 'region' or verb == 'endregion':
                    # //#region is a typescript feature
                    ignore_line = True

                if not ignore_line:
                    line = remove_end(line.rstrip(), end) + f' <DontUseJavaScriptLineCommentInJSX/>{end}\n'

            for if_else in ifs_elses:
                if if_else.value == 0:
                    if if_else.location != f'{input_path}:{i + 1}':
                        line = '\n'

                    break

            output_lines.append(line)

    return output_lines

def tfpp(input_path, output_path, overwrite=False):
    input_path = pathlib.Path(input_path)
    output_path = pathlib.Path(output_path)

    if not input_path.exists():
        raise Exception(f'Input file {input_path} is missing')

    if output_path.exists() and not overwrite:
        raise Exception(f'Output file {output_path} already exists')

    if input_path == output_path:
        raise Exception(f'Input file {input_path} and output file {output_path} are the same')

    defines = {}
    ifs_elses = []
    output_lines = parse_file(input_path, defines, ifs_elses)

    if len(ifs_elses) > 0:
        raise Exception(f'Missing #endif directive for #{"if" if isinstance(ifs_elses[0], If) else "else"} directive at {ifs_elses[0].location}')

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path_tmp = output_path.with_suffix('.tfpptmp')

    with output_path_tmp.open(mode='w', encoding='utf-8') as output_file:
        output_file.writelines(output_lines)

    output_path_tmp.replace(output_path)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input_path')
    parser.add_argument('output_path')
    parser.add_argument('--overwrite', action='store_true')

    args = parser.parse_args()

    tfpp(input_path=args.input_path, output_path=args.output_path, overwrite=args.overwrite)

if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print(f'Error: {e}')
        sys.exit(1)
