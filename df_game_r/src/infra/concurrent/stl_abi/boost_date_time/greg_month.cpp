/* Copyright (c) 2002-2005 CrystalClear Software, Inc.
 * Use, modification and distribution is subject to the
 * Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)
 * Author: Jeff Garland, Bart Garst
 * $Date: 2008-11-23 06:13:35 -0500 (Sun, 23 Nov 2008) $
 */

#ifndef BOOST_DATE_TIME_SOURCE
#define BOOST_DATE_TIME_SOURCE
#endif

#include "boost/date_time/gregorian/greg_month.hpp"
#include "boost/date_time/gregorian/greg_facet.hpp"
#include "boost/date_time/date_format_simple.hpp"
#include "boost/date_time/compiler_config.hpp"
#if defined(BOOST_DATE_TIME_INCLUDE_LIMITED_HEADERS)
#include "boost/date_time/gregorian/formatters_limited.hpp"
#else
#include "boost/date_time/gregorian/formatters.hpp"
#endif
#include "boost/date_time/date_parsing.hpp"
#include "boost/date_time/gregorian/parsers.hpp"

#include "greg_names.hpp"

namespace boost {
namespace gregorian {

// IDA_EA: 0x87DCD60
// IDA_NAME: _ZN5boost9gregorian10greg_month17get_month_map_ptrEv
// DEMANGLED: boost::gregorian::greg_month::get_month_map_ptr()
greg_month::month_map_ptr_type greg_month::get_month_map_ptr() {
    static month_map_ptr_type month_map_ptr(new greg_month::month_map_type());

    if (month_map_ptr->empty()) {
        std::string value("");
        for (unsigned short index = 1; index <= 12; ++index) {
            greg_month month(static_cast<month_enum>(index));
            value = month.as_long_string();
            value = date_time::convert_to_lower(value);
            month_map_ptr->insert(std::make_pair(value, index));
            value = month.as_short_string();
            value = date_time::convert_to_lower(value);
            month_map_ptr->insert(std::make_pair(value, index));
        }
    }
    return month_map_ptr;
}

// IDA_EA: 0x87DC970
// IDA_NAME: _ZNK5boost9gregorian10greg_month15as_short_stringEv
// DEMANGLED: boost::gregorian::greg_month::as_short_string() const
const char* greg_month::as_short_string() const {
    return short_month_names[value_ - 1];
}

// IDA_EA: 0x87DC990
// IDA_NAME: _ZNK5boost9gregorian10greg_month14as_long_stringEv
// DEMANGLED: boost::gregorian::greg_month::as_long_string() const
const char* greg_month::as_long_string() const {
    return long_month_names[value_ - 1];
}

// IDA_EA: 0x87DCC00
// IDA_NAME: _ZN5boost9gregorian25special_value_from_stringERKSs
// DEMANGLED: boost::gregorian::special_value_from_string(std::string const&)
special_values special_value_from_string(const std::string& value) {
    const short index = date_time::find_match(special_value_names,
                                              special_value_names,
                                              date_time::NumSpecialValues,
                                              value);
    if (index >= date_time::NumSpecialValues) {
        return not_special;
    }
    return static_cast<special_values>(index);
}

#ifndef BOOST_NO_STD_WSTRING
// IDA_EA: 0x87DC9B0
// IDA_NAME: _ZNK5boost9gregorian10greg_month16as_short_wstringEv
// DEMANGLED: boost::gregorian::greg_month::as_short_wstring() const
const wchar_t* greg_month::as_short_wstring() const {
    return w_short_month_names[value_ - 1];
}

// IDA_EA: 0x87DC9D0
// IDA_NAME: _ZNK5boost9gregorian10greg_month15as_long_wstringEv
// DEMANGLED: boost::gregorian::greg_month::as_long_wstring() const
const wchar_t* greg_month::as_long_wstring() const {
    return w_long_month_names[value_ - 1];
}
#endif  // BOOST_NO_STD_WSTRING

#ifndef BOOST_DATE_TIME_NO_LOCALE
// IDA_EA: 0x87DCAA0
// IDA_NAME: _ZN5boost9gregorian16create_facet_defEc
// DEMANGLED: boost::gregorian::create_facet_def(char)
BOOST_DATE_TIME_DECL
boost::date_time::all_date_names_put<greg_facet_config, char>*
create_facet_def(char type) {
    typedef boost::date_time::all_date_names_put<greg_facet_config, char>
        facet_def;
    return new facet_def(short_month_names,
                         long_month_names,
                         special_value_names,
                         short_weekday_names,
                         long_weekday_names);
}

// IDA_EA: 0x87DCB80
// IDA_NAME: _ZN5boost9gregorian15generate_localeERSt6localec
// DEMANGLED: boost::gregorian::generate_locale(std::locale&, char)
BOOST_DATE_TIME_DECL std::locale generate_locale(std::locale& locale,
                                                 char type) {
    typedef boost::date_time::all_date_names_put<greg_facet_config, char>
        facet_def;
    return std::locale(locale,
                       new facet_def(short_month_names,
                                     long_month_names,
                                     special_value_names,
                                     short_weekday_names,
                                     long_weekday_names));
}

#ifndef BOOST_NO_STD_WSTRING
// IDA_EA: 0x87DCA40
// IDA_NAME: _ZN5boost9gregorian16create_facet_defEw
// DEMANGLED: boost::gregorian::create_facet_def(wchar_t)
BOOST_DATE_TIME_DECL
boost::date_time::all_date_names_put<greg_facet_config, wchar_t>*
create_facet_def(wchar_t type) {
    typedef boost::date_time::all_date_names_put<greg_facet_config, wchar_t>
        facet_def;
    return new facet_def(w_short_month_names,
                         w_long_month_names,
                         w_special_value_names,
                         w_short_weekday_names,
                         w_long_weekday_names);
}

// IDA_EA: 0x87DCB00
// IDA_NAME: _ZN5boost9gregorian15generate_localeERSt6localew
// DEMANGLED: boost::gregorian::generate_locale(std::locale&, wchar_t)
BOOST_DATE_TIME_DECL std::locale generate_locale(std::locale& locale,
                                                 wchar_t type) {
    typedef boost::date_time::all_date_names_put<greg_facet_config, wchar_t>
        facet_def;
    return std::locale(locale,
                       new facet_def(w_short_month_names,
                                     w_long_month_names,
                                     w_special_value_names,
                                     w_short_weekday_names,
                                     w_long_weekday_names));
}
#endif  // BOOST_NO_STD_WSTRING
#endif  // BOOST_DATE_TIME_NO_LOCALE

}  // namespace gregorian
}  // namespace boost
