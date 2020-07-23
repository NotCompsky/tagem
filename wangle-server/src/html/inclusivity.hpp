/* The following few lines are taken/adapted from https://docs.huihoo.com/doxygen/linux/kernel/3.7/compiler-gcc_8h_source.html
 * and are thus bound by the GPL-2.0 license: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/LICENSES/preferred/GPL-2.0
 * Macro names are preserved to indicate their origin.
 */
#define __gcc_header(x) #x
#define _gcc_header(x) __gcc_header(../##x.html)
#define HTML_INCLUDER(x) _gcc_header(x)
