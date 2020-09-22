#pragma once

#ifdef TESTS
# define STATIC_ASSERT(b) static_assert(b)
#else
# define STATIC_ASSERT
#endif
