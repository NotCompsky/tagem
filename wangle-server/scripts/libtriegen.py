#!/usr/bin/env python3
# -*- coding: UTF-8 -*-


# TODO: Generate GOTO statements for matching similar terms, e.g. Capitalised and uncapitalised


import re


INDENT:str = "\t"
ZERO:str = "\0"
ESCH:str = "\\0"


def replace_indents(setval:str, k:int, indent:int):
	return re.sub("(^|\n)\t", "\\1"+INDENT*(indent+2*k+2), setval) + "\n" + INDENT*(indent+2*k+2)


def fdfadsfasdfsd(i:int, j:int, r:str, retval:str, indent:int, nextchar:str):
	output:str = ""
	for k in range(i+1, len(r), 1):
		output += f"\n{INDENT*(indent+2*k)}switch({nextchar}){{"
		output += f"\n{INDENT*(indent+2*k+1)}case '{r[k]}':"
	output += f"\t// {r.replace(ZERO, ESCH)}\n{replace_indents(retval,len(r)-1,indent)};"
	for k in range(j+1, len(r), 1)[::-1]:
		output += f"\n{INDENT*(indent+2*k+2)}break;"
		output += f"\n{INDENT*(indent+2*k)}}}"
	return output


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
	
	output:str = f"{INDENT*indent}switch({nextchar}){{"
	retval:str = None
	
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
		
		if (i != -1):
			output += fdfadsfasdfsd(i, j, r, retval, indent, nextchar)
		output += f"\n{INDENT*(indent+2*j+1)}case '{s[j]}':"
		
		r = s
		i = j
		j = 0
		retval = setval
	output += fdfadsfasdfsd(i, j, r, retval, indent, nextchar)
	output = output.replace("case '\0':","case 0:")
	return output
