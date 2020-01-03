#include "Logger.h"

#include <unordered_set>


// cria uma stream que não faz nada
class NullBuffer : public std::streambuf
{
public:
  int overflow(int c) { return c; }
};
NullBuffer null_buffer;
std::ostream null_stream(&null_buffer);

std::unordered_set<std::string> __ignorar;
std::ostream& Logger::out(const char* subsystem)
{
    if(__ignorar.count(subsystem) == 1)
        return null_stream;

    std::cout << "\n[" << subsystem << "] ";
    return std::cout;
}


void Logger::silence(const char* subsystem)
{
    __ignorar.insert(std::string(subsystem));
}
