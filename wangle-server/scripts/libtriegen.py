'''
Copyright 2020 Adam Gray
This file is part of the tagem program.
The tagem program is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published by the
Free Software Foundation version 3 of the License.
The tagem program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
This copyright notice should be included in any copy or substantial copy of the tagem source code.
The absense of this copyright notices on some other files in this project does not indicate that those files do not also fall under this license, unless they have a different license written at the top of the file.
'''
#!/usr/bin/env python3
# -*- coding: UTF-8 -*-


# TODO: Generate GOTO statements for matching similar terms, e.g. Capitalised and uncapitalised


import re


INDENT = "\t"


def replace_indents(setval:str, k:int, indent:int):
	return re.sub("(^|\n)\t", "\\1"+INDENT*(indent+2*k+2), setval) + "\n" + INDENT*(indent+2*k+2)


def generate_list(startswith:bool, strings:list, indent:int) -> str:
	nextchar:str = ""
	if startswith:
		nextchar = "*(++str)"
	else:
		nextchar = "*(--str)"
		if (type(strings[0]) is str):
			strings = [x[::-1] for x in strings]
		else:
			print(strings)
			strings = [(string[::-1], return_value) for (string, return_value) in strings]
			print(strings)
	
	r:str = ""
	i:int = -1
	j:int = 0
	ZERO:str = "\0"
	ESCH:str = "\\0"
	
	output:str = f"{INDENT*indent}switch({nextchar}){{"
	
	for (s, setval) in sorted(strings):
		if s == "":
			continue
		
		while len(r)>j and s[j] == r[j]:
			# NOTE: We do not need to test len(s)>j due to sorting placing 'foo' before 'foobar'
			# Go deeper
			j += 1
		
		if (r!="" and j >= len(r)):
			# i.e.  r == s[:j]
			print(i,j, r,s)
			j = 0
			continue
		
		if (i == -1):
			output += f"\n{INDENT*(indent+2*j+1)}case '{s[j]}':"
		else:
			for k in range(i+1, len(r), 1):
				output += f"\n{INDENT*(indent+2*k)}switch({nextchar}){{"
				output += f"\n{INDENT*(indent+2*k+1)}case '{r[k]}':"
			output += f"\t// {r.replace(ZERO, ESCH)}\n{replace_indents(retval,len(r)-1,indent)};"
			for k in range(j+1, len(r), 1)[::-1]:
				output += f"\n{INDENT*(indent+2*k+2)}break;"
				output += f"\n{INDENT*(indent+2*k)}}}"
			
			output += f"\n{INDENT*(indent+2*j+1)}case '{s[j]}':"
			
		r = s
		i = j
		j = 0
		retval = setval
	for k in range(i+1, len(r), 1):
		output += f"\n{INDENT*(indent+2*k)}switch({nextchar}){{"
		output += f"\n{INDENT*(indent+2*k+1)}case '{r[k]}':"
	output += f"\t// {r.replace(ZERO, ESCH)}\n{replace_indents(retval,len(r)-1,indent)};"
	for k in range(0, len(r), 1)[::-1]:
		output += f"\n{INDENT*(indent+2*k+2)}break;"
		output += f"\n{INDENT*(indent+2*k)}}}"
	output = output.replace("case '\0':","case 0:")
	return output
