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

#### Deduplication

Deduplicate code where possible. Compile-time deduplication (preferably through metaprogramming, although short macros are also usually acceptable) is preferred when there is a risk of significant runtime performance degredation.

In particular, if you need to extract information from a string in a particular way, for instance counting the number of instances of a character in that string, prefer to create a separate function for that in a utils header file, and call that function where it is needed.

In the commit history you'll notice a lot of instances of "Tidy: Deduplicate code", where there is no net loss in lines of code. In fact, some of these appear to make the code *more complex*. Many of these commits are specifically deduplicating SQL code, introducing a slight overhead (in terms of compile-time and reading comprehension) of C++ templates in order to increase the maintainability of the SQL code (which is more important, as it has by far the biggest performance implications and is not checked by the compiler).

#### Dependencies

Minimise the dependencies added to the project (without sacrificing features), although dependencies added to the developer pipeline probably are't a big deal.

For instance, do not use the `{fmt}` aka `std::fmt` library, use [libcompsky](https://github.com/NotCompsky/libcompsky)::asciify, unless the latter cannot do what is desired (in which case you should file a feature request under that project).

#### Readability

Try to use readable JavaScript, not complex one-liners. This is partly because the code generator parser has a limited range of syntax it recognises.

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

JavaScript files pass through a code generator, that extracts function names as macros, pastes in macro values, and places everything on a single line in a C++ header file. The parser for this was written by me, so naturally it is not a full JavaScript parser, it has a very limited idea of what is acceptable syntax.

# Legal Stuff

I'm not a lawyer. Contributors are expected to have the rights to the code they add, in which case the newly added code will be licensed under 
