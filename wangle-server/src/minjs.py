#!/usr/bin/env python3

'''
This preprocesses a series of JavaScript function definitions, const var definitions, and var declarations, into a single C++ header file; the JavaScript as a minimised string accessible as MINIMISED_JS_[FIRST_FILE_NAME], and the names of all its contained functions/vars accessible as MINIMISED_JS_DECL_[FUnCTION_NAME]
'''


import re


human2minimised:dict = {} # Maps human names of functions/vars to [minimised_name, parameters, value]


valid_var_names:str = "[$A-Za-z_][$A-Za-z0-9_]*"


def get_human2minimised(human_name:str):
	try:
		return human2minimised[human_name][0]
	except KeyError:
		print(f"No definition available for: $$${human_name}")
		raise


def process_fn_line(line:str):
	# Replace var/fn names
	line = re.sub(f"\$\$\$({valid_var_names})", lambda x: get_human2minimised(x.group(1)), line)
	
	# Strip indent
	line = re.sub("^\t+", "", line)
	if line.startswith("else"):
		line = " " + line
	
	# Escape quotes
	line = line.replace('"', '\\"')
	
	return line


def get_next_minimised_name():
	i = get_next_minimised_name.i
	name:str = get_next_minimised_name.alphabet_1[i % len(get_next_minimised_name.alphabet_1)]
	i //= len(get_next_minimised_name.alphabet_1)
	while i != 0:
		name += get_next_minimised_name.alphabet_2[i % len(get_next_minimised_name.alphabet_2)]
		i //= len(get_next_minimised_name.alphabet_2)
	get_next_minimised_name.i += 1
	return name
get_next_minimised_name.i = 0
get_next_minimised_name.alphabet_1 = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$_"
get_next_minimised_name.alphabet_2 = get_next_minimised_name.alphabet_1 + "0123456789"


if __name__ == "__main__":
	import argparse
	
	parser = argparse.ArgumentParser()
	parser.add_argument("dst", help="C++ header file destination path for minimised JS")
	parser.add_argument("srcs", nargs="+", help="JavaScript source files to minimise and merge")
	args = parser.parse_args()
	
	macro_name:str = re.sub("[^A-Za-z0-9_]", "_", args.srcs[0].upper())
	
	for fp in args.srcs:
		gen = (line for line in open(fp).read().split("\n"))
		for line in gen:
			if line == "":
				continue
			if line.startswith("const "):
				m = re.search(f"^const ({valid_var_names}) = ([^;]+);$", line)
				# I know JS allows MANY more characters than this - even many kawaii faces are valid variable names - but it's unreasonable
				if m is None:
					raise SyntaxError()
				human2minimised[m.group(1)] = [get_next_minimised_name(), None, m.group(2)]
				continue
			if line.startswith("var "):
				m = re.search(f"^var ({valid_var_names});$", line)
				if m is None:
					raise SyntaxError()
				human2minimised[m.group(1)] = [get_next_minimised_name(), None, None]
				continue
			if line.startswith("function "):
				m = re.search(f"^function ({valid_var_names})\(([^)]*)\)\{{", line)
				if m is None:
					raise SyntaxError()
				fn_contents:str = ""
				while True:
					# Get the content. Note that not all variable/const/function names have been mapped yet, so the contents cannot be parsed yet - only recorded.
					line = next(gen)
					fn_contents += line + "\n"
					if line == "}":
						break
				human2minimised[m.group(1)] = [get_next_minimised_name(), m.group(2), fn_contents]
	
	# Parse function contents
	for human_name, (minimised_name, parameters, value) in human2minimised.items():
		if value is not None:
			# const vars and functions may rely on other functions/variables
			gen = (line for line in value.split("\n"))
			fn_contents:str = ""
			for line in gen:
				fn_contents += process_fn_line(line)
			human2minimised[human_name][2] = fn_contents
	
	with open(args.dst, "w") as f:
		for human_name, (minimised_name, parameters, value) in human2minimised.items():
			f.write(f"#define MINIMISED_JS_DECL_{human_name} \"{minimised_name}\"\n")
		f.write(f"#define MINIMISED_JS_{macro_name} \"")
		for human_name, (minimised_name, parameters, value) in human2minimised.items():
			var_type:str = "function" if parameters is not None else "const" if value is not None else "var"
			f.write(f"{var_type} {minimised_name}")
			if parameters is not None:
				f.write(f"({parameters}){{{value}}}")
			elif value is not None:
				f.write(f"={value};")
			else:
				f.write(";")
		f.write("\"")
