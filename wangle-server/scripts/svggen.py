#!/usr/bin/env python3


import re


def parse_lines(text_only:bool, text_too:bool, display_licences:bool, ls:list):
	lines:iter = (line for line in ls)
	license:str = ""
	icons:dict = {}
	for line in lines:
		if line == "" or line.startswith("//"):
			continue
		m = re.search("^([A-Z0-9_]+):([^\n]+)$", line)
		if m is not None:
			icon_name:str = m.group(1)
			icon:str = ""
			while True:
				line = next(lines)
				if line == "":
					break
				if not text_only:
					icon += re.sub(' (?:xmlns|class|width|height)="[^"]*"','',re.sub("^[\s]*", "", line)).replace(" />", "/>")
			if text_only or text_too:
				icon += m.group(2)
			icon = re.sub("<svg ", '<svg xmlns="http://www.w3.org/2000/svg" ', icon) # Necessary to render as img.src
			icons[icon_name] = (license if display_licences else "") + icon
			continue
		if license == "":
			license = "<!--The following icon is subject to the following license:"
			while True:
				line = next(lines)
				if line == "":
					break
				license += "\n" + line
			license += "-->"
			continue
	return icons


if __name__ == "__main__":
	import argparse
	import os
	from urllib.parse import quote as urlescape
	
	parser = argparse.ArgumentParser()
	parser.add_argument("dst", help="C++ header file destination path")
	parser.add_argument("srcs", nargs="+", help="Source file")
	parser.add_argument("--text-only", default=False, action="store_true", help="Use text labels only")
	parser.add_argument("--text-too", default=False, action="store_true", help="Place text alongside labels")
	parser.add_argument("--display-licences", default=False, action="store_true")
	args = parser.parse_args()
	
	if os.path.isfile(args.dst) and os.path.getmtime(args.dst) > max([os.path.getmtime(__file__)]+[os.path.getmtime(fp) for fp in args.srcs]):
		# Already generated
		exit(0)
	
	with open(args.dst, "w") as f:
		for fp in args.srcs:
			for key, value in parse_lines(args.text_only, args.text_too, args.display_licences, open(fp).read().split("\n")).items():
				escaped_value:str = value.replace("\\","\\\\").replace('"','\\"').replace("\n","\\n")
				f.write(f"""#define SVG_{key} "{escaped_value}"\n""")
				f.write(f"""#define SVG_{key}__URI_ESCAPED "{urlescape(value)}"\n""")
