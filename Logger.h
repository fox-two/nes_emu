#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <iomanip>

class Logger
{
public:
    static std::ostream& out(const char* subsystem);
    static void silence(const char* subsystem);

private:
    Logger();
};

#endif // LOGGER_H
