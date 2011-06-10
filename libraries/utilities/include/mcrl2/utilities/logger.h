// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file logger.h

#ifndef MCRL2_UTILITIES_LOGGER_H
#define MCRL2_UTILITIES_LOGGER_H

#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <string>
#include <sstream>
#include <map>
#include <iostream>

#include "mcrl2/core/text_utility.h"

/// \brief Log levels that are supported
/// \note log_debugi with i>=1 automatically indent 2*i spaces.
enum mcrl2_log_level_t
{
  log_quiet, // No log message should ever be printed to this log level!
  log_error,
  log_warning,
  log_info,
  log_verbose,
  log_debug,
  log_debug1,
  log_debug2,
  log_debug3,
  log_debug4,
  log_debug5
};

/// \brief Convert log level to string
/// This string is used to prefix messages in the logging output.
inline
std::string log_level_to_string(const mcrl2_log_level_t level)
{
  static const char* const buffer[] = {"quiet", "error", "warning", "info", "verbose", "debug", "debug1", "debug2", "debug3", "debug4", "debug5"};
  return buffer[level];
}

/// \brief Convert string to log level
inline
mcrl2_log_level_t log_level_from_string(const std::string& s)
{
  if (s == "quiet")
  {
    return log_quiet;
  }
  else if (s == "error")
  {
    return log_error;
  }
  else if (s == "warning")
  {
    return log_warning;
  }
  else if (s == "info")
  {
    return log_info;
  }
  else if (s == "verbose")
  {
    return log_verbose;
  }
  else if (s == "debug")
  {
    return log_debug;
  }
  else if (s == "debug1")
  {
    return log_debug1;
  }
  else if (s == "debug2")
  {
    return log_debug2;
  }
  else if (s == "debug3")
  {
    return log_debug3;
  }
  else if (s == "debug4")
  {
    return log_debug4;
  }
  else if (s == "debug5")
  {
    return log_debug5;
  }
  else
  {
    throw std::runtime_error("Unknown log-level " + s + " provided.");
  }
}

/// \brief Type for message distinction (by purpose).
/// Should only be used for custom message handlers.
enum mcrl2_message_t
{
  mcrl2_notice,
  mcrl2_warning,
  mcrl2_error
};

/// \prototype
std::string now_time();

/// \brief Type for function pointer for a custom message printing routine
/// \deprecated
/// provided for backward compatibility with gs*Msg
typedef void (*custom_message_handler_t)(mcrl2_message_t, const char*);
static custom_message_handler_t mcrl2_custom_message_handler = 0; //< Do not access directly

/// \brief Class for logging messages
///
/// Based on a description in the article "Logging In C++", Petru Marginean
/// Dr. Dobb's Journal, September 5, 2007
/// (url: http://drdobbs.com/cpp/201804215)
/// Requires that OutputPolicy is a class which as a static member output(const std::string&)
template <typename OutputPolicy>
class logger
{
  public:
    typedef OutputPolicy output_policy_t;

  // Prevent copying loggers
  private:
    logger(const logger&)
    {};

    logger& operator =(const logger&)
    { return *this; };

  protected:
    /// \brief Stream that is printed to internally
    /// Collects the full debug message that we are currently printing.
    std::ostringstream m_os;
    
    /// \brief The loglevel of the current message
    mcrl2_log_level_t m_level;

    /// \brief The message hint of the current message
    std::string m_hint;

    static
    bool& last_message_ended_with_newline()
    {
      static bool m_last_message_ended_with_newline = true;
      return m_last_message_ended_with_newline;
    }

    /// \brief Mapping of message hint to loglevel. This allows a finegrained
    /// control of log messages to log levels. It can e.g. be set that for some
    /// message hint all messages up to debug level are printed, whereas for other
    /// message hints no messages are printed at all.
    static
    std::map<std::string, mcrl2_log_level_t>& hint_to_level()
    {
      static std::map<std::string, mcrl2_log_level_t> m_hint_to_level;
      return m_hint_to_level;
    }

    /// \brief The amount of indentation that has to be added to a line.
    static
    unsigned int& indentation()
    {
      static unsigned int indentation = 0;
      return indentation;
    }

    mcrl2_message_t to_message_type(const mcrl2_log_level_t level) const
    {
      if (level <= log_error)
      {
        return mcrl2_error;
      }
      else if (level <= log_warning)
      {
        return mcrl2_warning;
      }
      else
      {
        return mcrl2_notice;
      }
    }

    /// \brief The default log level that is used if no specific log level has
    /// been set.
    static
    mcrl2_log_level_t default_reporting_level()
    {
      std::map<std::string, mcrl2_log_level_t>::const_iterator i = hint_to_level().find(OutputPolicy::default_hint());
      if(i != hint_to_level().end())
      {
        return i->second;
      }
      else
      {
        return log_info;
      }
    }

    /// \brief Prefix each line in s with some extra information.
    /// The things that are added are:
    /// - current time
    /// - hint
    /// - log level
    /// - indentation
    std::string process(const std::string& s)
    {
      std::string start_of_line =
        "["
      + now_time()
      + " " + m_hint + (m_hint == std::string()?"":"::") + log_level_to_string(m_level)
      + "]"
      + std::string(8 - log_level_to_string(m_level).size(), ' ')
      + std::string(2*indentation(), ' ');

      bool s_ends_with_newline = (s[s.size()-1] == '\n');

      std::string result = s;
      // Avoid adding spurious start of line after the last line in the log.
      if(s_ends_with_newline)
      {
        result.erase(result.end()-1);
      }

      // Prepend if a newline was added
      if (last_message_ended_with_newline())
      {
        result = start_of_line + result;
      }

      result = mcrl2::core::regex_replace("\n", "\n" + start_of_line, result);

      if(s_ends_with_newline)
      {
        result += "\n";
      }

      last_message_ended_with_newline() = s_ends_with_newline;

      return result;
    }

  public:
    /// \brief Default constructor
    logger()
    {}

    /// \brief Destructor; flushes output.
    /// Flushing during destruction is important to confer thread safety to the
    /// logging mechanism. Requires that output performs output in an atomic way.
    ~logger()
    {
      // With custom message handler, still log to the default location
      OutputPolicy::output(process(m_os.str()), m_hint);
      if(mcrl2_custom_message_handler != 0)
      {
        (*(mcrl2_custom_message_handler))(to_message_type(m_level), process(m_os.str()).c_str());
      }
    }

    /// \brief Set reporting level
    /// \param[in] level Log level
    /// \param[in] hint The hint for which to set log level
    static
    void set_reporting_level(const mcrl2_log_level_t level, const std::string& hint = OutputPolicy::default_hint())
    {
      hint_to_level()[hint] = level;
    }

    /// \brief Get reporting level
    /// \param[in] hint The hint for which to get log level
    static
    mcrl2_log_level_t get_reporting_level(const std::string& hint = OutputPolicy::default_hint())
    {
      std::map<std::string, mcrl2_log_level_t>::const_iterator i = hint_to_level().find(hint);
      if(i != hint_to_level().end())
      {
        return i->second;
      }
      else
      {
        return default_reporting_level();
      }
    }

    /// \brief Clear reporting level
    /// \param hint Reset the log level for hint
    static
    void clear_reporting_level(const std::string& hint)
    {
      hint_to_level().erase(hint);
    }

    /// \brief Increase the indentation level
    static
    void indent()
    {
      ++indentation();
    }

    /// \brief Decrease the indentation level
    static
    void unindent()
    {
      if(indentation() > 0)
      {
        --indentation();
      }
    }

    /// Get access to the stream provided by the logger.
    /// \param[in] l Log level for the stream
    /// \param[in] hint The hint for which the stream has to be provided.
    std::ostringstream& get(const mcrl2_log_level_t l, const std::string& hint = OutputPolicy::default_hint())
    {
      m_level = l;
      m_hint = hint;
      return m_os;
    }

    static
    void set_custom_message_handler(custom_message_handler_t handler)
    {
      mcrl2_custom_message_handler = handler;
    }
};

/// \brief Interface class for output policy.
///
/// Separates the exact way of doing output from the logger class.
class output_policy_interface
{
  public:
    /// \brief Output message.
    /// \param[in] msg Message that is written to output.
    /// \param[in] hint Hint for the stream to which the output is written.
    ///  \details Any implementation must assure that output is written using an atomic action, to prevent
    /// mixing of different lines into one line in the output.
    static void output(const std::string& msg, const std::string& hint);

    /// \brief Hint for the default stream.
    static std::string default_hint();
};

/// \brief File output class.
///
/// Provides facilities to output to a file. By default output is sent to stderr.
class file_output: output_policy_interface
{
  protected:
    /// \brief Map hints to streams
    /// This allows messages with different hints to be written to different output
    /// streams.
    static
    std::map<std::string, FILE*>& hint_to_stream()
    {
      static std::map<std::string, FILE*> m_hint_to_stream;
      return m_hint_to_stream;
    }

    /// \brief Gets a stream handle for hint
    /// \param[in] hint Hint for which to provide a stream handle.
    static
    FILE* get_stream(const std::string& hint)
    {
      std::map<std::string, FILE*>::iterator i = hint_to_stream().find(hint);
      if(i == hint_to_stream().end())
      {
        i = hint_to_stream().find(default_hint());
      }
      if (i == hint_to_stream().end())
      {
        return stderr;
      }
      else
      {
        return i->second;
      }
    }

  public:
    /// \brief Default hint (empty)
    static std::string default_hint()
    {
      static std::string default_hint;
      return default_hint;
    }

    /// \brief Set stream handle for a hint
    /// \param[in] stream A file handle
    /// \param[in] hint The hint for which to set the handle to stream.
    static
    void set_stream(FILE* stream, const std::string& hint = default_hint())
    {
      hint_to_stream()[hint] = stream;
    }

    /// \overload
    /// Output message to stream.
    /// \param[in] msg The message to be printed
    /// \param[in] hint The hint of the stream to which we print.
    /// This uses fprintf (and not e.g. <<) because fprintf is guaranteed to be
    /// atomic.
    static
    void output(const std::string& msg, const std::string& hint = default_hint())
    {
      FILE* p_stream = get_stream(hint);
      if (!p_stream)
      {
        return;
      }

      fprintf(p_stream, "%s", msg.c_str());
      fflush(p_stream);
    }
};

/// \brief Default logger that we use
typedef logger<file_output> mcrl2_logger;

/// Unless otherwise specified, we compile away all debug messages that have
/// a log level greater than MCRL2_MAX_LOG_LEVEL.
#ifndef MCRL2_MAX_LOG_LEVEL
#define MCRL2_MAX_LOG_LEVEL log_debug
#endif

/// mCRL2log(level) provides the function used to log. It performs two
/// optimisations:
/// - the first comparison (level > MCRL2_MAX_LOG_LEVEL), compares two constants
///   during compile time. The compiler will not create any code if (level > MCRl2_MAX_LOG_LEVEL).
/// - the second comparison compares two constants at runtime. This check makes
///   sure that the arguments to mCRL2log(level) will not be evaluated if level > file_logger::reporting_level().
/// In all other cases this macro provides a stream that can be printed to.
// Note that the macro uses the dirty preprocessor token concatenation. For a
// description, see e.g. http://en.wikipedia.org/wiki/C_preprocessor#Token_concatenation
// (accessed 7/4/2011)
// We also use the facilities to provide a variable number of arguments to a macro, in order
// to allow mCRL2log(level) as well as mCRL2log(level, "hint")
#define mCRL2log(level, ...) \
if (log_##level > MCRL2_MAX_LOG_LEVEL) ; \
else if (log_##level > mcrl2_logger::get_reporting_level(__VA_ARGS__)) ; \
else mcrl2_logger().get(log_##level, ##__VA_ARGS__)

#define mCRL2logEnabled(level, ...) \
((log_##level <= MCRL2_MAX_LOG_LEVEL) && log_##level <= mcrl2_logger::get_reporting_level(__VA_ARGS__))

// \deprecated. For backwards compatibility with gsMessage
#define mCRL2logGS(level) \
if (level > MCRL2_MAX_LOG_LEVEL) ; \
else if (level > mcrl2_logger::get_reporting_level()) ; \
else mcrl2_logger().get(level)

#endif /* MCRL_UTILITIES_LOGGER_H */
