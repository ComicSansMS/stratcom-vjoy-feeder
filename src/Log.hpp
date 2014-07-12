#ifndef STRATCOM_VJOY_INCLUDE_GUARD_LOG_HPP_
#define STRATCOM_VJOY_INCLUDE_GUARD_LOG_HPP_

#include <iosfwd>
#include <memory>

class Logger {
private:
    class Pimpl;
    std::unique_ptr<Pimpl> const pimpl_;
public:
    ~Logger();
    static Logger* getInstance();
    static void shutdown();
    std::ostream& getLogStream();
    void notifyMessageLogged(std::ostream& os);
private:
    Logger();
};

#define LOG(msg) \
    Logger::getInstance()->notifyMessageLogged(Logger::getInstance()->getLogStream() << msg)

class LoggerShutdownToken {
public:
    ~LoggerShutdownToken() {
        Logger::getInstance()->shutdown();
    }
};

#endif
