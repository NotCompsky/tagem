# Outline

What the browser sees:

	One HTML file - with inline SVG icons, inline CSS, and inline Javascript.

What is really written:

* A directory of C++ header files which generate the HTML
* A directory of text files containing labelled SVG icons to display
* A directory of CSS files where every line is double-quoted (this will eventually be tidied)
* A directory of (almost) entirely valid JavaScript files, with several oddities:
  * The occasional `!!!MACRO!!!SOME_NAME_HERE`, where a C++ macro is pasted later
  * An overabundance of `$` signs in global variable names (this will eventually be tidied)

# Pipeline

## JavaScript

The JavaScript passes through [minjs.py](server/scripts/minjs.py), which minimises the names beginning with `$$$`, and performs a limited number of checks on them (for instance, ensuring that every such variable that is used is defined somewhere). With that code generator, JavaScript files can be added to the directory without needing to be explicitly referenced in any build files, and they will still appear in the final JavaScript.

## CSS

The CSS is included in the HTML in basically its written form - the double quotes around every line basically just strip the whitespace.

## SVG

The text files contain a license, followed by a list of labelled SVG files subject to that license. They pass through [svggen.py](server/scripts/svggen.py), which takes all the SVGs listed, optionally attaches the license to each, optionally attaches a license (or even replaces the icon with the label), and pastes the SVGs into a C++ header file, which defines them as macros for inlined inclusion into the HTML.

## HTML

Much of the HTML is generated using simple C++ macros. It is, at this point, far more a C++ file than an HTML file, which is the reason it is not in the `static` directory - it is more a backend problem than frontend.
