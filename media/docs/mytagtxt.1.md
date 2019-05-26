% MYTAGTXT(1) MYTAGTXT User Manual
% NotCompsky
% April 2019

# NAME

mytagtxt - Utility to aid the tagging of a stream of text files

# SYNOPSIS

mytagtxt *MYSQL_CONFIG_FILE* [*OPTIONS*]

# DESCRIPTION

A Qt5 and MySQL C++ GUI program designed for rapid heirarchical tagging of many text files, file paths being provided by STDIN.

# ARGUMENTS

See **mytag**(1)

# OPTIONS

See **mytag**(1)

# EXAMPLES

In descending order of usefulness.

`find -name '*.txt' -o -name '*.md' ~/Documents  |  mytagtxt ~/.config/mysql/mytag.auth`
:   Process all files with **txt** and **md** file extensions.

`myfindtxt ~/Documents  |  mydefmt  |  mytagtxt ~/.config/mysql/mytag.auth`
:   The same as above, but using **myfindtxt**(1)

# USAGE

See **mytag**(1) for the core usage details.

Additional commands in **mytagtxt** include:

* *i* to toggle read-only mode
* *s* to save current file

# COMMON MISTAKES

See **mytag**(1)

# BUGS

**Bad behaviour, including silent failure, for incorrect usage is not considered a bug, so long as it immediately exits with an error exit code.**

## CRITICAL BUGS

None known.

## SECONDARY BUGS

None known.

## MISSING FEATURES

None known.

# ROADMAP

None known.

# CONTRIBUTING

See **mytag**(1)

# SEE ALSO
mytagimg(1), mytagvid(1), myfindtxt(1), mytag(1)
