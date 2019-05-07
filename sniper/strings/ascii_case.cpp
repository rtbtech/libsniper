// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
 * toLowerAscii8, toLowerAscii32, toLowerAscii64, to_lower_ascii
 * from Facebook Folly library
 * https://github.com/facebook/folly/blob/master/folly/String.cpp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ascii_case.h"

namespace sniper::strings {

namespace {


void toLowerAscii8(char& c) noexcept
{
    // Branchless tolower, based on the input-rotating trick described
    // at http://www.azillionmonkeys.com/qed/asmexample.html
    //
    // This algorithm depends on an observation: each uppercase
    // ASCII character can be converted to its lowercase equivalent
    // by adding 0x20.

    // Step 1: Clear the high order bit. We'll deal with it in Step 5.
    uint8_t rotated = uint8_t(c & 0x7f);
    // Currently, the value of rotated, as a function of the original c is:
    //   below 'A':   0- 64
    //   'A'-'Z':    65- 90
    //   above 'Z':  91-127

    // Step 2: Add 0x25 (37)
    rotated += 0x25;
    // Now the value of rotated, as a function of the original c is:
    //   below 'A':   37-101
    //   'A'-'Z':    102-127
    //   above 'Z':  128-164

    // Step 3: clear the high order bit
    rotated &= 0x7f;
    //   below 'A':   37-101
    //   'A'-'Z':    102-127
    //   above 'Z':    0- 36

    // Step 4: Add 0x1a (26)
    rotated += 0x1a;
    //   below 'A':   63-127
    //   'A'-'Z':    128-153
    //   above 'Z':   25- 62

    // At this point, note that only the uppercase letters have been
    // transformed into values with the high order bit set (128 and above).

    // Step 5: Shift the high order bit 2 spaces to the right: the spot
    // where the only 1 bit in 0x20 is.  But first, how we ignored the
    // high order bit of the original c in step 1?  If that bit was set,
    // we may have just gotten a false match on a value in the range
    // 128+'A' to 128+'Z'.  To correct this, need to clear the high order
    // bit of rotated if the high order bit of c is set.  Since we don't
    // care about the other bits in rotated, the easiest thing to do
    // is invert all the bits in c and bitwise-and them with rotated.
    rotated &= ~c;
    rotated >>= 2;

    // Step 6: Apply a mask to clear everything except the 0x20 bit
    // in rotated.
    rotated &= 0x20;

    // At this point, rotated is 0x20 if c is 'A'-'Z' and 0x00 otherwise

    // Step 7: Add rotated to c
    c += char(rotated);
}

void toLowerAscii32(uint32_t& c) noexcept
{
    // Besides being branchless, the algorithm in toLowerAscii8() has another
    // interesting property: None of the addition operations will cause
    // an overflow in the 8-bit value.  So we can pack four 8-bit values
    // into a uint32_t and run each operation on all four values in parallel
    // without having to use any CPU-specific SIMD instructions.
    uint32_t rotated = c & uint32_t(0x7f7f7f7fL);
    rotated += uint32_t(0x25252525L);
    rotated &= uint32_t(0x7f7f7f7fL);
    rotated += uint32_t(0x1a1a1a1aL);

    // Step 5 involves a shift, so some bits will spill over from each
    // 8-bit value into the next.  But that's okay, because they're bits
    // that will be cleared by the mask in step 6 anyway.
    rotated &= ~c;
    rotated >>= 2;
    rotated &= uint32_t(0x20202020L);
    c += rotated;
}

void toLowerAscii64(uint64_t& c) noexcept
{
    // 64-bit version of toLower32
    uint64_t rotated = c & uint64_t(0x7f7f7f7f7f7f7f7fL);
    rotated += uint64_t(0x2525252525252525L);
    rotated &= uint64_t(0x7f7f7f7f7f7f7f7fL);
    rotated += uint64_t(0x1a1a1a1a1a1a1a1aL);
    rotated &= ~c;
    rotated >>= 2;
    rotated &= uint64_t(0x2020202020202020L);
    c += rotated;
}

} // namespace

void to_lower_ascii(char* str, size_t length) noexcept
{
    static const size_t kAlignMask64 = 7;
    static const size_t kAlignMask32 = 3;

    // Convert a character at a time until we reach an address that
    // is at least 32-bit aligned
    size_t n = (size_t)str;
    n &= kAlignMask32;
    n = std::min(n, length);
    size_t offset = 0;
    if (n != 0) {
        n = std::min(4 - n, length);
        do {
            toLowerAscii8(str[offset]);
            offset++;
        } while (offset < n);
    }

    n = (size_t)(str + offset);
    n &= kAlignMask64;
    if ((n != 0) && (offset + 4 <= length)) {
        // The next address is 32-bit aligned but not 64-bit aligned.
        // Convert the next 4 bytes in order to get to the 64-bit aligned
        // part of the input.
        toLowerAscii32(*(uint32_t*)(str + offset));
        offset += 4;
    }

    // Convert 8 characters at a time
    while (offset + 8 <= length) {
        toLowerAscii64(*(uint64_t*)(str + offset));
        offset += 8;
    }

    // Convert 4 characters at a time
    while (offset + 4 <= length) {
        toLowerAscii32(*(uint32_t*)(str + offset));
        offset += 4;
    }

    // Convert any characters remaining after the last 4-byte aligned group
    while (offset < length) {
        toLowerAscii8(str[offset]);
        offset++;
    }
}

string to_lower_ascii_copy(string_view str)
{
    string out(str);
    to_lower_ascii(out);
    return out;
}

} // namespace sniper::strings
