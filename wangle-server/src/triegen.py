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
	import os
	
	src:str = sys.argv[1]
	dst:str = sys.argv[2]
	
	if os.path.getmtime(dst) >= os.path.getmtime(src):
		exit(0)
	
	gen = BIterator([line for line in open(src).read().split("\n")])
	ls:list = []
	for line in gen:
		if line == "":
			continue
		if line.endswith(">"):
			ls.append((line[:-1], get_return_value(gen)))
	
	open(dst,"w").write(generate_list(True, ls, 1))
