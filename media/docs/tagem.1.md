% MYTAG(1) MYTAG User Manual
% NotCompsky
% April 2019

# NAME

tagem - Family of utilities to aid the tagging of a stream of large quantities of files

# SYNOPSIS

tagem**FILE_TYPE** *MYSQL_CONFIG_FILE* [*OPTIONS*]

where **FILE_TYPE** is one of **txt**, **img**, **vid**

# DESCRIPTION

A Qt5 and MySQL C++ GUI program designed for rapid heirarchical tagging of many files, file paths being provided by STDIN.

# ARGUMENTS

*MYSQL_CONFIG_FILE*
:   File path of MySQL config file
    The first line must be the **URL**
    
    The second line must be the **USERNAME**
    
    The third line must be the **PASSWORD**
    
    **WARNING**: Lines must be delineated only by a single newline (\\n) character. Some older Windows text editors might insert an additional character (a carriage return, \\r), which this program does not account for.

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
tagemtxt(1), tagemimg(1), tagemvid(1), myfind(1)
