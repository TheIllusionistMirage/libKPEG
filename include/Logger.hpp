/**
 * @file Logger.hpp
 * @author Koushtav Chakrabarty (koustav@fleptic.eu)
 * @date 14th Oct, 2017, 07:30 PM IST 
 * @brief The Logger module provides message logging facility to K-PEG.
 * 
 * The <b>Logging</b> module consists of a singleton logging class,
 * a teestream and macros for logging messages at various levels.
 * 
 * The message levels are error, info and debug messages.
 */

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <cstring>

// #ifndef __FILENAME__
//     /**
//     * Fetch the filename which invokes the logger
//     */
//     #define __FILENAME__ __FILE__
// #endif

/**
* Fetch the filename which invokes the logger
*/
// Courtesy: StackOverflow, of course
// (http://stackoverflow.com/a/8488201)
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

/**
 * Macro to log error/info/debug messages
 */
#define LOG(level) \
    if (level > kpeg::Logger::get().getLevel()) ;        \
else kpeg::Logger::get().getStream() << kpeg::Logger::levelStr(level) \
                                     << "[ kpeg:"         \
                                     << __FILENAME__     \
                                     << ":" << std::dec  \
                                     << __LINE__ << " ] "

namespace kpeg
{
    /**
     * @brief Logger class that 
     */
    class Logger
    {
        public:
            
            /**
            * Log levels enum
            */
            enum Level
            {
                ERROR , /** <b>ERROR</b> - Log error messages only */
                INFO  , /** <b>INFO</b> - Log error and info messages only */
                DEBUG   /** <b>DEBUG</b> - Log error, info and debug messages */
            };
            
            /**
            * Convert the enum value of a level to a human readable
            * string.
            */
            static inline const std::string levelStr(Level lvl)
            {
                std::string levelMap[] = { "[ ERROR ]", "[ INFO  ]", "[ DEBUG ]" };

                auto index = static_cast<int>(lvl);

                if (index <= Level::DEBUG)
                    return levelMap[index];

                return "";
            }
            
        public:
            
            ~Logger();
            void setLogStream(std::ostream& stream);
            Logger& setLevel(Level level);
            Level getLevel();

            std::ostream& getStream();

            static Logger& get();
            
        private:
            
            Level m_logLevel;
            std::ostream* m_logStream;
            static std::unique_ptr<Logger> m_instance;
    };
    
    // Source: http://wordaligned.org/articles/cpp-streambufs#toctee-streams
    class TeeBuf : public std::streambuf
    {
        public:
            // Construct a streambuf which tees output to both input
            // streambufs.
            TeeBuf(std::streambuf* sb1, std::streambuf* sb2);
        private:
            // This tee buffer has no buffer. So every character "overflows"
            // and can be put directly into the teed buffers.
            virtual int overflow(int c);
            // Sync both teed buffers.
            virtual int sync();
        private:
            std::streambuf* m_sb1;
            std::streambuf* m_sb2;
    };

    class TeeStream : public std::ostream
    {
        public:
            // Construct an ostream which tees output to the supplied
            // ostreams.
            TeeStream(std::ostream& o1, std::ostream& o2);
        private:
            TeeBuf m_tbuf;
    };
}

#endif // LOGGER_HPP
