#pragma once

#include "logger.h"
#include "tokenize.h"
#include "schema.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/hex.hpp>

#include <inttypes.h>
#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>

namespace swss {

template<typename T>
static inline void lexical_convert(const std::string &str, T &t)
{
    t = boost::lexical_cast<T>(str);
}

namespace lexical_convert_detail
{

    template <typename T, typename... Args>
    void lexical_convert(
        std::vector<std::string>::const_iterator begin,
        std::vector<std::string>::const_iterator end,
        T &t)
    {
        if (begin == end)
        {
            SWSS_LOG_THROW("Insufficient corpus");
        }
        auto cur_itr = begin++;
        if (begin != end)
        {
            SWSS_LOG_THROW("Too much corpus");
        }
        swss::lexical_convert(*cur_itr, t);
    }

    template <typename T, typename... Args>
    void lexical_convert(
        std::vector<std::string>::const_iterator begin,
        std::vector<std::string>::const_iterator end,
        T &t,
        Args &... args)
    {
        if (begin == end)
        {
            SWSS_LOG_THROW("Insufficient corpus");
        }
        swss::lexical_convert(*(begin++), t);
        return lexical_convert(begin, end, args...);
    }

}

template <typename T, typename... Args>
void lexical_convert(const std::vector<std::string> &strs, T &t, Args &... args)
{
    lexical_convert_detail::lexical_convert(strs.begin(), strs.end(), t, args...);
}

namespace join_detail
{

    template <typename D, typename T>
    void join(std::ostringstream &ostream, D, const T &t)
    {
        ostream << t;
    }

    template <typename D, typename T, typename... Args>
    void join(std::ostringstream &ostream, const D &delimiter, const T &t, const Args &... args)
    {
        ostream << t << delimiter;
        join(ostream, delimiter, args...);
    }

    template <typename D, typename Iterator>
    void join(std::ostringstream &ostream, const D &delimiter, Iterator begin, Iterator end)
    {
        if (begin == end)
        {
            return;
        }

        ostream << *begin;
        while(++begin != end)
        {
            ostream << delimiter << *begin;
        }
    }
}

template <typename D, typename T, typename... Args>
static inline std::string join(const D &delimiter, const T &t, const Args &... args)
{
    std::ostringstream ostream;
    join_detail::join(ostream, delimiter, t, args...);
    return ostream.str();
}

template <typename D, typename Iterator>
static inline std::string join(const D &delimiter, Iterator begin, Iterator end)
{
    std::ostringstream ostream;
    join_detail::join(ostream, delimiter, begin, end);
    return ostream.str();
}

template <typename D, typename Iterator>
static inline std::string join(const D &delimiter, char beginsym, char endsym, Iterator begin, Iterator end)
{
    std::ostringstream ostream;
    ostream << beginsym;
    join_detail::join(ostream, delimiter, begin, end);
    ostream << endsym;
    return ostream.str();
}


static inline bool hex_to_binary(const std::string &hex_str, std::uint8_t *buffer, size_t buffer_length)
{
    if (hex_str.length() != (buffer_length * 2))
    {
        SWSS_LOG_DEBUG("Buffer length isn't sufficient");
        return false;
    }

    try
    {
        boost::algorithm::unhex(hex_str, buffer);
    }
    catch(const boost::algorithm::non_hex_input &e)
    {
        SWSS_LOG_DEBUG("Invalid hex string %s", hex_str.c_str());
        return false;
    }

    return true;
}

template<typename T>
static inline void hex_to_binary(const std::string &s, T &value)
{
    return hex_to_binary(s, &value, sizeof(T));
}

static inline std::string binary_to_hex(const void *buffer, size_t length)
{
    std::string s;
    auto buf = static_cast<const std::uint8_t *>(buffer);

    boost::algorithm::hex(
        buf,
        buf + length,
        std::back_inserter<std::string>(s));

    return s;
}

static inline std::string binary_to_printable(const void *buffer, size_t length)
{
    std::string printable;
    printable.reserve(length * 4);

    auto buf = static_cast<const std::uint8_t *>(buffer);

    for (size_t i = 0; i < length; i++)
    {
        std::uint8_t c = buf[i];
        if (std::isprint(c))
        {
            if (c == '\\')
            {
                printable.push_back('\\');
                printable.push_back('\\');
            }
            else
            {
                printable.push_back(c);
            }
        }
        else if (std::isspace(c))
        {
            printable.push_back('\\');
            if (c == '\n')
            {
                printable.push_back('n');
            }
            else if (c == '\r')
            {
                printable.push_back('r');
            }
            else if (c == '\t')
            {
                printable.push_back('t');
            }
            else if (c == '\v')
            {
                printable.push_back('v');
            }
            else if (c == '\f')
            {
                printable.push_back('f');
            }
        }
        else
        {
            printable.push_back('\\');
            printable.push_back('x');
            printable.push_back("0123456789ABCDEF"[c >> 4]);
            printable.push_back("0123456789ABCDEF"[c & 0xf]);
        }
    }

    return printable;
}

}
