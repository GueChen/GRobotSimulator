#ifndef GCOMPONENT_STRINGPROCESSOR_H
#define GCOMPONENT_STRINGPROCESSOR_H

#include <vector>
#include <string>

namespace GComponent {

using std::vector;
using std::string;

class StringProcessor
{
public:
    StringProcessor() = delete;
    static std::vector<std::string> Split(const std::string& s, char sparse = ' ')
    {
        std::vector<std::string> rval;

        size_t cur = 0, last = 0;
        while (s[last++] == ' ');
        --last;
        while ((cur = s.find(sparse, last)) != -1)
        {
            rval.push_back(s.substr(last, cur - last));
            last = cur + 1;
        }
        rval.push_back(s.substr(last, s.length() - cur));

        return rval;
    }
};

} // namespace GComponent

#endif // GCOMPONENT_STRINGPROCESSOR_H
