/*
 * Copyright 2017 Hans Van Moer
 * Licensed under the GNU Public Licence version 3 (https://www.gnu.org/licenses/gpl.html)
 */
#ifndef GAME_LOGGER_H
#define GAME_LOGGER_H

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>


namespace Game{

  namespace Log{

    /*
     * a type representing a unique logger ID
     */
    using Id = std::string;

    /*
     * the default logger ID
     */
    extern const Id default_id;

    /*
     * the clock used to measure the logger period and timestamp
     */
    using Clock = std::chrono::system_clock;

    /*
     * the time point type of the logger clock
     */
    using TimePoint = Clock::time_point;

    /*
     * the period type of the logger clock
     */
    using Duration = Clock::duration;
      
    /*
     * determines the priority of logging messages
     */
    enum class Priority{
      DEBUG, INFO, WARNING, ERROR
    };
    
    /*
     * defines a complete ordering on priorities
     */
    bool operator<(Priority first, Priority second);

    /*
     * prints a string representation of the priority
     */
    std::ostream &operator<<(std::ostream &output, Priority priority);
    
    
    /*
     * a singleton representing all global logging settings
     * all methods on this class are thread-safe
     */
    class LoggerSystem{
    public:

      /*
       * returns a pointer to the instance
       */
      static LoggerSystem *instance();

      /*
       * sets the period when logging messages are written
       * e.g. if you set it to 1000ms the logging thread will check for unwritten messages every 1000ms
       * changes will be applied after a call to start()
       */
      void period(Duration period);

      /*
       * returns the period logging messages are written
       */
      Duration period() const;

      /*
       * sets the number of threads used to write log messages
       * changes will be applied after a call to start()
       * throws std::invalid_argument if thread_count = 0
       */
      void thread_count(size_t thread_count);

      /*
       * returns the number of threads used to write log messages
       */
      size_t thread_count() const;

      /*
       * sets the min priority of all loggers created after this call
       */
      void min_priority(Priority min_priority);

      /*
       * returns the min priority
       */
      Priority min_priority() const;
      
      /*
       * starts the logging system
       * if it is already started the system will be restarted if any of its properties are updated
       * returns true if the system was started or restarted
       */
      bool start();

      /*
       * stops the logging system if it is started
       * returns true if the system was running and is now stopped
       * note that stop is called by the destructor
       */
      bool stop();
      
      /*
       * sets a logger output to a specified stream
       * owns_output controls whether the output stream is owned by the logger system and is destroyed after use
       */
      void set_output(Id id, std::ostream *output, bool owns_output);
      
      /*
       * sets the logger output to ignore all subsequent messages
       */
      void clear_output(Id id); 
      
    private:
      
      LoggerSystem();

      ~LoggerSystem();
    };

    /*
     * a guard type that configures, starts and stops the logger system using a RAII pattern
     */
    class LoggerSystemGuard{
    public:

      LoggerSystemGuard(Priority min_priority, Duration period, std::size_t thread_count, bool defer_start);

      bool start();
      
      ~LoggerSystemGuard();

    private:
      LoggerSystemGuard(const LoggerSystemGuard &) = delete;
      LoggerSystemGuard &operator=(const LoggerSystemGuard &) = delete;
    };

    template<typename T> struct LoggerHelper;
    
    /*
     * a basic logger
     * this type is not type-safe
     */
    class Logger{
    public:

      /*
       * creates a new logger
       */
      Logger();

      Logger(const Id &id);

      Logger(const Id &id, Priority min_priority);

      Logger(Priority priority);

      /*
       * prints the argument to the logger if the logger's priority is equal or higher than the min priority
       * if the argument is a manipulator it is always executed
       */
      template<typename T> Logger &operator<<(T &&arg){
	return LoggerHelper<T>::execute(*this, std::forward<T>(arg));
      };

      /*
       * set/get this logger's priority
       */
      
      void priority(Priority priority);

      Priority priority() const;

      /*
       * set/get this logger's min priority
       */
      
      void min_priority(Priority min_priority);
      
      Priority min_priority() const;

      /*
       * resets this logger, discarding the current logger message
       */
      void reset();

      /*
       * flushes the logger's message to the output only if the current priority is equal or higher than the min priority 
       */
      void flush();
      
    private:
      const Priority min_priority_;
      Priority priority_;
      std::ostringstream buffer_;

      template<typename T> friend struct LoggerHelper;
    };

    /*
     * end the current logger message and calls Logger::flush
     */
    Logger &end(Logger &logger);

    /*
     * calls Logger::reset
     */
    Logger &reset(Logger &logger);

    /*
     * sets the logger's priority
     */
    
    Logger &debug(Logger &logger);

    Logger &info(Logger &logger);

    Logger &warning(Logger &logger);

    Logger &error(Logger &logger);

    /*
     * the type for Logger manipulators
     */
    using Manipulator = Logger & (*)(Logger &);

    /*
     * a helper class for printing logger messages
     */
    namespace{

      template<typename T> struct LoggerHelper{
	
	static Logger &execute(Logger &logger, T && arg){
	  logger.buffer_ << std::forward<T>(arg);
	  return logger;
	};
	
      };
      
      template<> struct LoggerHelper<Manipulator>{
	
	static Logger &execute(Logger &logger, Manipulator arg){
	  return (*arg)(logger);
	};
      };

    }
    
  }
  
}

#endif
