% MYTAGTXT(1) MYTAGTXT User Manual
% NotCompsky
% April 2019

# NAME

tagemtxt - Utility to aid the tagging of a stream of text files

# SYNOPSIS

tagemtxt [*OPTIONS*]

# DESCRIPTION

A Qt5 and MySQL C++ GUI program designed for rapid heirarchical tagging of many text files, file paths being provided by STDIN.

# OPTIONS

See **tagem**(1)

# EXAMPLES

In descending order of usefulness.

`find -name '*.txt' -o -name '*.md' ~/Documents  |  tagemtxt ~/.config/mysql/tagem.auth`
:   Process all files with **txt** and **md** file extensions.

`myfindtxt ~/Documents  |  mydefmt  |  tagemtxt ~/.config/mysql/tagem.auth`
:   The same as above, but using **myfindtxt**(1)

# USAGE

See **tagem**(1) for the core usage details.

Additional commands in **tagemtxt** include:

* *i* to toggle read-only mode
* *s* to save current file

# COMMON MISTAKES

See **tagem**(1)

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

See **tagem**(1)

# SEE ALSO
tagemimg(1), tagemvid(1), myfindtxt(1), tagem(1)
