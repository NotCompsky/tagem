#ifndef __ASCIIFY_FLAGS__
#define __ASCIIFY_FLAGS__

namespace compsky::asciify::flag {
    namespace concat {
        struct Start{};
        struct End{};
        const Start start;
        const End end;
    }
    struct ChangeBuffer{};
    const ChangeBuffer change_buffer;
    struct Escape{};
    const Escape escape;
    struct StrLen{};
    const StrLen strlen;
    struct FillWithLeadingZeros{};
    const FillWithLeadingZeros fill_with_leading_zeros;
    struct UpToFirstZero{};
    const UpToFirstZero uptofirstzero;
    namespace ensure {
        struct BetweenZeroAndOne{};
        const BetweenZeroAndOne between_zero_and_one;
        struct BetweenZeroAndOneInclusive{};
        const BetweenZeroAndOneInclusive between_zero_and_one_inclusive;
        struct BetweenZeroAndTenLeftInclusive{};
        const BetweenZeroAndTenLeftInclusive between_zero_and_ten_left_inclusive;
    }
    namespace guarantee {
        struct BetweenZeroAndOne{};
        const BetweenZeroAndOne between_zero_and_one;
        struct BetweenZeroAndOneInclusive{};
        const BetweenZeroAndOneInclusive between_zero_and_one_inclusive;
        struct BetweenZeroAndTenLeftInclusive{};
        const BetweenZeroAndTenLeftInclusive between_zero_and_ten_left_inclusive;
    }
}

#endif
