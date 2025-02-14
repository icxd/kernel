#include "RTC.hpp"
#include "CMOS.hpp"

namespace RTC {

  static u32 s_bootTime;

  void initialize() {
    u8 cmosMode = CMOS::read(0x0b);
    cmosMode |= 2; // 24 hour mode
    cmosMode |= 4; // No BCD mode
    CMOS::write(0x0b, cmosMode);

    s_bootTime = now();
  }

  u32 boot_time() { return s_bootTime; }

  static bool update_in_progress() { return CMOS::read(0x0a) & 0x80; }

  inline bool is_leap_year(unsigned year) {
    return ((year % 4 == 0) && ((year % 100 != 0) || (year % 400) == 0));
  }

  static unsigned days_in_months_since_start_of_year(unsigned month,
                                                     unsigned year) {
    switch (month) {
    case 11:
      return 30;
    case 10:
      return 31;
    case 9:
      return 30;
    case 8:
      return 31;
    case 7:
      return 31;
    case 6:
      return 30;
    case 5:
      return 31;
    case 4:
      return 30;
    case 3:
      return 31;
    case 2:
      if (is_leap_year(year))
        return 29;
      return 28;
    case 1:
      return 31;
    default:
      return 0;
    }
  }

  static unsigned days_in_years_since_epoch(unsigned year) {
    unsigned days = 0;
    while (year > 1969) {
      days += 365;
      if (is_leap_year(year))
        ++days;
      --year;
    }
    return days;
  }

  u32 now() {
    while (update_in_progress())
      ;

    unsigned year = (CMOS::read(0x32) * 100) + CMOS::read(0x09);
    unsigned month = CMOS::read(0x08);
    unsigned day = CMOS::read(0x07);
    unsigned hour = CMOS::read(0x04);
    unsigned minute = CMOS::read(0x02);
    unsigned second = CMOS::read(0x00);

    return days_in_years_since_epoch(year - 1) * 86400 +
           days_in_months_since_start_of_year(month - 1, year) * 86400 +
           day * 86400 + hour * 3600 + minute * 60 + second;
  }

} // namespace RTC
