% MYTAG(1) MYTAG User Manual
% NotCompsky
% April 2019

# NAME

tagem - Family of utilities to aid the tagging of a stream of large quantities of files

# SYNOPSIS

tagem**FILE_TYPE** [*OPTIONS*]

where **FILE_TYPE** is one of **txt**, **img**, **vid**

# DESCRIPTION

A Qt5 and MySQL C++ GUI program designed for rapid heirarchical tagging of many files, file paths being provided by STDIN.

# OPTIONS

t
:   Skip previously tagged files

# EXAMPLES

See **tagemtxt**(1), **tagemimg**(1), **tagemvid**(1) for examples.

# USAGE

The program is largely based on key and mouse events.

## KEYS

* *d* to move to next file
* *n* to read/write a note assigned to the file
* *r* to rate the current file
* *t* to tag the current file
* *q* to exit the program
* *x* to delete current file
* *SHIFT* + *NUMBER* key to both tag the current file and store the assigned tag in the number key's slot
* *NUMBER* key to assign this stored tag to the current file

## MOUSE

All but **tagemtxt** include the following:

* Click and drag, then press *i*, to create an object instance around a box
* Click on two object instances' buttons sequentially to create a master-slave object relation


# COMMON MISTAKES

`Bad config file`
:   Windows: See **USAGE** argument **MYSQL_CONFIG_FILE** for information about Windows text editors creating bad config files.

# ROADMAP

**tagempdf** is planned, to support PDF rendering, and will likely act very similarly to **tagemimg**.

# CONTRIBUTING

Repository is found at https://github.com/NotCompsky/tagems

Lines that are easiest to contribute to are often marked with `# CONTRIBUTIONS WELCOME`

# SEE ALSO
tagemtxt(1), tagemimg(1), tagemvid(1)
