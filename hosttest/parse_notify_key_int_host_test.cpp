#include <cassert>
#include <charconv>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {
bool ParseNotifyInt32(const std::string &s, int32_t &out)
{
    if (s.empty()) {
        return false;
    }
    int32_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

bool ParseNotifyUint32(const std::string &s, uint32_t &out)
{
    if (s.empty()) {
        return false;
    }
    uint32_t value = 0;
    const char *first = s.data();
    const char *last = first + s.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}
} // namespace

int main()
{
    int32_t i = 0;
    uint32_t u = 0;
    assert(ParseNotifyInt32("100", i) && i == 100);
    assert(ParseNotifyInt32("-1", i) && i == -1);
    assert(!ParseNotifyInt32("", i));
    assert(!ParseNotifyInt32("abc", i));
    assert(!ParseNotifyInt32("12a", i));
    assert(!ParseNotifyInt32("2147483648", i)); // INT32_MAX+1
    assert(ParseNotifyUint32("0", u) && u == 0);
    assert(ParseNotifyUint32("4294967295", u) && u == 4294967295u);
    assert(!ParseNotifyUint32("", u));
    assert(!ParseNotifyUint32("-1", u));
    assert(!ParseNotifyUint32("4294967296", u)); // UINT32_MAX+1

    bool threw = false;
    try {
        (void)std::stoi(std::string(40, '9'));
    } catch (const std::out_of_range &) {
        threw = true;
    } catch (const std::invalid_argument &) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        (void)std::stoul(std::string(40, '9'));
    } catch (const std::out_of_range &) {
        threw = true;
    } catch (const std::invalid_argument &) {
        threw = true;
    }
    assert(threw);

    return 0;
}
