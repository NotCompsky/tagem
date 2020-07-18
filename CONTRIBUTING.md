# Issues

## Logs

Plain text logs are preferred to screenshots.

# Development

## Overviews

### Server

#### Pipeline

* Python scripts generate C++ header files from `.txt` files describing inline SVGs, server endpoitns, and `qry` tokens.
* Python script generates a single JavaScript C++ header file from all the non-3rdparty JavaScript files. This JavaScript is optionally minimised. Consequently, the names of each non-3rdparty JavaScript file is unimportant.
  This header file contains the final names of each JavaScript function, many of which are used in `profile.html`
* `qry` is built
* The server itself is built

## General Guidelines

### Maintainability

Deduplicate code where possible. Compile-time deduplication (preferably through metaprogramming, although short macros are also usually acceptable) is preferred when there is a risk of significant runtime performance degredation.

In particular, if you need to extract information from a string in a particular way, for instance counting the number of instances of a character in that string, prefer to create a separate function for that in a utils header file, and call that function where it is needed.

Minimise the dependencies added to the project (without sacrificing features), although dependencies added to the developer pipeline probably are't a big deal.

For instance, do not use the `{fmt}` aka `std::fmt` library, use [libcompsky](https://github.com/NotCompsky/libcompsky)::asciify, unless the latter cannot do what is desired (in which case you should file a feature request under that project).

### Code Style

It's hard to describe hard-and-fast rules for code linting, just try to copy that of the files you are modifying. If adding new files, copy the coding style in [server.cpp](wangle-server/src/server.cpp).

In particular:
* Use tabs for left indentaiton, and spaces for right indentation.
* There is no line length limitation. Do not split lines just to limit the line lengths.
* Use `Type* foo` rather than `Type *foo`
* Avoid `goto`, `new`, `delete`, `malloc` etc. where possible (although use is permitted if the alternative is messier).
* Class and struct names should be in CamelCase, while function and variable names should be in snake_case. Exceptions are for compatibility with external libraries.
* Source code and scripts should be named in lower case separated by hyphens.
* Names should be descriptive. Yes I study maths, no I don't think that names should all be single characters. If something takes hundreds of characters to describe, so be it.
* C++ source code should have the `cpp` extension.
* C++ header files should have the `hpp` extension.

# Legal Stuff

I'm not a lawyer. Contributors are expected to have the rights to the code they add, in which case the newly added code will be licensed under 
