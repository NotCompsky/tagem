#!/usr/bin/env python3
# -*- coding: UTF-8 -*-


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


import libtriegen


class BIterator:
	# Iterate in both directions
	# Based on: http://docs.python.org/tutorial/classes.html#iterators
	def __init__(self, collection):
		self.data = collection
		self.index = 0
	def __iter__(self):
		return self
	def prev(self):
		if self.index == 0:
			raise IndexError
		self.index -= 1
		return self.data[self.index]
	def __next__(self):
		self.index += 1
		if self.index == len(self.data):
			raise StopIteration
		return self.data[self.index]


def get_return_value(gen:BIterator) -> str:
	s:str = ""
	for line in gen:
		if not line.startswith("\t"):
			gen.prev()
			return s[:-1]
		s += line + "\n"
	return s[:-1]


def unescape(line:str):
	new_line:str = ""
	gen = (c for c in line)
	for c in gen:
		if c == "\\":
			c = next(gen)
			if c == "0":
				c = "\0"
			elif c == "n":
				c = "\n"
			elif c == "r":
				c = "\r"
		new_line += c
	return new_line


if __name__ == '__main__':
	import sys
	import os
	
	src:str = sys.argv[1]
	dst:str = sys.argv[2]
	
	try:
		if os.path.getmtime(dst) >= max([os.path.getmtime(__file__), os.path.getmtime(libtriegen.__file__), os.path.getmtime(src)]):
			exit(0)
	except FileNotFoundError:
		pass
	
	gen = BIterator([unescape(line) for line in open(src).read().split("\n")])
	ls:list = []
	for line in gen:
		if line == "":
			continue
		if line.endswith(">"):
			ls.append((line[:-1], get_return_value(gen)))
	
	open(dst,"w").write(libtriegen.generate_list(True, ls, 1))
