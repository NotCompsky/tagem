#!/usr/bin/env python3
# -*- coding: UTF-8 -*-


import re
from libtriegen import generate_list


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


if __name__ == '__main__':
	import sys
	
	src:str = open(sys.argv[1]).read()
	
	gen = BIterator([line for line in src.split("\n")])
	ls:list = []
	for line in gen:
		if line == "":
			continue
		if line.endswith(">"):
			ls.append((line[:-1], get_return_value(gen)))
	
	open(sys.argv[2],"w").write(generate_list(True, ls, 1))
