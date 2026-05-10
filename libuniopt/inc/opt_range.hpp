#pragma once

inline bool opt_IsInRangeOne(int value, int rangeStart, int rangeEnd)
{
    return (unsigned)value - (unsigned)rangeStart <= (unsigned)rangeEnd - (unsigned)rangeStart;
}

inline bool opt_IsInRangeOne(unsigned int value, unsigned int rangeStart, unsigned int rangeEnd)
{
    return (unsigned)value - (unsigned)rangeStart <= (unsigned)rangeEnd - (unsigned)rangeStart;
}
