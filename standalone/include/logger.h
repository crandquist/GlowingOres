#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>
#include <iomanip>
#include <ctime>

// Simple logging utility class with different log levels
class Logger {
public:
    enum LogLevel {
        ERROR,   // Only critical errors
        WARNING, // Important warnings that don't stop execution
        INFO,    // Important function calls and state changes
        DEBUG    // Detailed debugging information - disabled in production
    };

    static LogLevel currentLevel;

    // Log a message with a specific level
    static void log(LogLevel level, const std::string& message) {
        if (level <= currentLevel) {
            // Get current time for timestamp
            std::time_t now = std::time(nullptr);
            char timestamp[20];
            std::strftime(timestamp, sizeof(timestamp), "%H:%M:%S", std::localtime(&now));
            
            std::string prefix;
            switch (level) {
                case ERROR:
                    prefix = "[ERROR]";
                    break;
                case WARNING:
                    prefix = "[WARN] ";
                    break;
                case INFO:
                    prefix = "[INFO] ";
                    break;
                case DEBUG:
                    prefix = "[DEBUG]";
                    break;
            }
            
            std::cout << timestamp << " " << prefix << " " << message << std::endl;
        }
    }

    // Shorthand methods for different log levels
    static void error(const std::string& message) {
        log(ERROR, message);
    }
    
    static void warning(const std::string& message) {
        log(WARNING, message);
    }
    
    static void info(const std::string& message) {
        log(INFO, message);
    }
    
    static void debug(const std::string& message) {
        log(DEBUG, message);
    }
    
    // Log control information in a compact format
    static void control(const std::string& control, float value) {
        if (INFO <= currentLevel) {
            std::cout << "\r" << std::setw(15) << std::left << control 
                      << ": " << std::fixed << std::setprecision(2) << value << std::flush;
        }
    }
    
    // Log multiple control values in a compact format
    static void controls(const std::string& title, const std::vector<std::pair<std::string, float>>& controls) {
        if (INFO <= currentLevel) {
            std::cout << "\r" << title << ": ";
            for (const auto& control : controls) {
                std::cout << control.first << "=" << std::fixed << std::setprecision(2) << control.second << " ";
            }
            std::cout << std::flush;
        }
    }

    // Set the current log level
    static void setLevel(LogLevel level) {
        currentLevel = level;
    }
};

// Initialize the static member
Logger::LogLevel Logger::currentLevel = Logger::INFO;

#endif // LOGGER_H