#!/usr/bin/env python3

'''
This preprocesses a series of JavaScript function definitions, const var definitions, and var declarations, into a single C++ header file; the JavaScript as a minimised string accessible as MINIMISED_JS_[FIRST_FILE_NAME], and the names of all its contained functions/vars accessible as MINIMISED_JS_DECL_[FUnCTION_NAME]
'''


import re


human2minimised:dict = {} # Maps human names of functions/vars to [type, minimised_name, parameters, value]


valid_var_names:str = "[$A-Za-z_][$A-Za-z0-9_]*"
cmnt_noncapture:str = "(?:[\s]+//[^\"']+)?"
mangle_name:str = "(\$\$\$)?"
MANGLE_NAMES:bool = False


def set_human2minimised(human_name:str, val:list,  fp:str,  line:str):
	x:list = human2minimised.get(human_name)
	if x is not None:
		raise MySyntaxError(fp, 0, line, f"variable redefined: {human_name}\nOriginal value: {x}\nNew value: {val}")
	human2minimised[human_name] = val


def get_human2minimised(human_name:str):
	try:
		return human2minimised[human_name][1]
	except KeyError:
		print(f"No definition available for: $$${human_name}")
		raise


def process_fn_line(line:str):
	# Replace var/fn names
	line = re.sub(f"\$\$\$({valid_var_names})", lambda x: get_human2minimised(x.group(1)), line)
	
	# Strip trailing comments
	m = re.search("^((?:[^/]|/[^/])*)//(.*)$", line)
	if m is not None:
		# Includes something resembling a trailing comment
		if (re.search("['\"`]", m.group(1)) is None) or (re.search("['\"`]", m.group(2)) is None):
			# There is no quotation mark in one side - meaning the comment cannot be part of a string
			line = m.group(1)
	line = re.sub("/\*(?:[^*]|\*[^/])*\*/", "", line)
	
	# Strip indent
	line = re.sub("^\t+", "", line)
	if line.startswith("else"):
		line = " " + line
	
	# Escape escapes
	line = line.replace("\\", "\\\\")
	
	# Escape quotes
	line = line.replace('"', '\\"')
	
	return line + "\\n"


def get_next_minimised_name(orig_name:str):
	if not MANGLE_NAMES:
		return "$$$" + orig_name
	i = get_next_minimised_name.i
	name:str = get_next_minimised_name.alphabet_1[i % len(get_next_minimised_name.alphabet_1)]
	i //= len(get_next_minimised_name.alphabet_1)
	while i != 0:
		name += get_next_minimised_name.alphabet_2[i % len(get_next_minimised_name.alphabet_2)]
		i //= len(get_next_minimised_name.alphabet_2)
	get_next_minimised_name.i += 1
	if name in get_next_minimised_name.reserved_names:
		return get_next_minimised_name(orig_name)
	return name
get_next_minimised_name.i = 0
get_next_minimised_name.alphabet_1 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$_"
get_next_minimised_name.alphabet_2 = get_next_minimised_name.alphabet_1 + "0123456789"
get_next_minimised_name.reserved_names = ['$']


class MySyntaxError(SyntaxError):
	def __init__(self, fp:str, line:int, content:str, descr:str):
		super().__init__(f"{fp}:{line}: {content}\n{descr}")


def is_empty_line(line:str):
	return (re.search("^(?:[\t ]*//|$)", line) is not None)


def run(fps:list, is_populating_reserved_names:bool):
	for fp in fps:
		gen = (line for line in open(fp).read().split("\n"))
		for line in gen:
			if is_empty_line(line):
				continue
			if line.startswith("const "):
				m = re.search(f"^const {mangle_name}({valid_var_names}) *= *([^;]+);{cmnt_noncapture}$", line)
				# I know JS allows MANY more characters than this - even many kawaii faces are valid variable names - but it's unreasonable
				if m is None:
					raise MySyntaxError(fp, 0, line, "Bad const syntax")
				dont_mangle_name:bool = (m.group(1) is None)
				if is_populating_reserved_names:
					if dont_mangle_name:
						get_next_minimised_name.reserved_names.append(m.group(2))
					continue
				set_human2minimised(m.group(2),  ["const", m.group(2) if dont_mangle_name else get_next_minimised_name(m.group(2)), None, m.group(3)],  fp,  line)
				continue
			if line.startswith("var "):
				m = re.search(f"^var {mangle_name}({valid_var_names})(?: *= *([^;]+))?;{cmnt_noncapture}$", line)
				if m is None:
					raise MySyntaxError(fp, 0, line, "Bad var syntax")
				dont_mangle_name:bool = (m.group(1) is None)
				if is_populating_reserved_names:
					if dont_mangle_name:
						get_next_minimised_name.reserved_names.append(m.group(2))
					continue
				set_human2minimised(m.group(2),  ["var", m.group(2) if dont_mangle_name else get_next_minimised_name(m.group(2)), None, m.group(3)],  fp,  line)
				continue
			if line.startswith("function ") or line.startswith("async function "):
				m = re.search(f"^(async |)function {mangle_name}({valid_var_names})\(([^)]*)\)\{{", line)
				if m is None:
					raise MySyntaxError(fp, 0, line, "Bad function syntax")
				fn_contents:str = ""
				while True:
					# Get the content. Note that not all variable/const/function names have been mapped yet, so the contents cannot be parsed yet - only recorded.
					line = next(gen)
					if is_empty_line(line):
						continue
					if line == "}":
						break
					fn_contents += line + "\n"
				if is_populating_reserved_names:
					continue
				dont_mangle_name:bool = (m.group(2) is None)
				set_human2minimised(m.group(3),  [m.group(1)+"function", m.group(3) if dont_mangle_name else get_next_minimised_name(m.group(3)), m.group(4), fn_contents],  fp,  line)
				continue
			raise MySyntaxError(fp, 0, line, "Not a blank line, comment, var, const or function definition")


if __name__ == "__main__":
	import argparse
	import os
	
	parser = argparse.ArgumentParser()
	parser.add_argument("macro_name", help="Name of the C++ macro containing the minimised JS")
	parser.add_argument("dst", help="C++ header file destination path for minimised JS")
	parser.add_argument("srcs", nargs="+", help="JavaScript source files to minimise and merge")
	parser.add_argument("--mangle", default=False, action="store_true")
	args = parser.parse_args()
	
	MANGLE_NAMES = args.mangle
	
	fps:list = []
	for path in args.srcs:
		if os.path.isdir(path):
			for fp in os.listdir(path):
				if fp.endswith(".js"):
					fps.append(os.path.join(path, fp))
		else:
			fps.append(path)
	if os.path.isfile(args.dst) and os.path.getmtime(args.dst) > max([os.path.getmtime(fp) for fp in fps]):
		exit(0)
	
	run(fps, True)
	run(fps, False)
	
	# Parse function contents
	for human_name, (var_name, minimised_name, parameters, value) in human2minimised.items():
		if value is not None:
			# const vars and functions may rely on other functions/variables
			gen = (line for line in value.split("\n"))
			fn_contents:str = ""
			for line in gen:
				fn_contents += process_fn_line(line)
			human2minimised[human_name][3] = fn_contents
	
	with open(args.dst, "w") as f:
		f.write("#pragma once\n")
		for human_name, (var_type, minimised_name, parameters, value) in human2minimised.items():
			f.write(f"#define MINIMISED_JS_DECL_{human_name} \"{minimised_name}\"\n")
		f.write(f"#define MINIMISED_JS_{args.macro_name} \"")
		for human_name, (var_type, minimised_name, parameters, value) in human2minimised.items():
			f.write(f"{var_type} {minimised_name}")
			if parameters is not None:
				f.write(f"({parameters}){{{value}}}")
			elif value is not None:
				f.write(f"={value};")
			else:
				f.write(";")
		f.write("\"")
