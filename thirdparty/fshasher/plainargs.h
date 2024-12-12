// 
// plainargs - A very plain CLI arguments parsing header-only library.
// 
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace plainargs {
    class Result {
    private:
        struct Option {
            uint32_t keyIndex;
            uint32_t valueCount;
        };

        std::string directory;
        std::vector<std::string> arguments;
        std::vector<Option> options;
        std::unordered_map<std::string, uint32_t> shortKeyMap;
        std::unordered_map<std::string, uint32_t> longKeyMap;
    public:
        // Arguments are the same as main().
        Result(int argc, char *argv[]) {
            if (argc < 1) {
                return;
            }

            directory = argv[0];

            arguments.resize(size_t(argc - 1));
            for (uint32_t i = 1; i < uint32_t(argc); i++) {
                std::string &argument = arguments[i - 1];
                argument = std::string(argv[i]);

                if (!argument.empty()) {
                    bool shortKey = (argument.size() > 1) && (argument[0] == '-');
                    bool longKey = (argument.size() > 2) && (argument[0] == '-') && (argument[1] == '-');
                    if (longKey) {
                        longKeyMap[argument.substr(2)] = uint32_t(options.size());
                        options.emplace_back(Option{ i - 1, 0 });
                    }
                    else if (shortKey) {
                        shortKeyMap[argument.substr(1)] = uint32_t(options.size());
                        options.emplace_back(Option{ i - 1, 0 });
                    }
                    else if (!options.empty()) {
                        options.back().valueCount++;
                    }
                }
            }
        }

        // Return all the values associated to the long key or the short key in order.
        std::vector<std::string> getValues(const std::string &longKey, const std::string &shortKey = "", uint32_t maxValues = 0) const {
            std::vector<std::string> values;
            auto optionIt = options.end();
            if (!longKey.empty()) {
                auto it = longKeyMap.find(longKey);
                if (it != longKeyMap.end()) {
                    optionIt = options.begin() + it->second;
                }
            }

            if ((optionIt == options.end()) && !shortKey.empty()) {
                auto it = shortKeyMap.find(shortKey);
                if (it != shortKeyMap.end()) {
                    optionIt = options.begin() + it->second;
                }
            }

            if (optionIt != options.end()) {
                uint32_t valueCount = optionIt->valueCount;
                if ((maxValues > 0) && (valueCount > maxValues)) {
                    valueCount = maxValues;
                }

                values.resize(valueCount);
                for (uint32_t i = 0; i < valueCount; i++) {
                    values[i] = arguments[optionIt->keyIndex + i + 1];
                }
            }

            return values;
        }

        std::string getValue(const std::string &longKey, const std::string &shortKey = "") const {
            std::vector<std::string> values = getValues(longKey, shortKey, 1);
            return !values.empty() ? values.front() : std::string();
        }

        // Return whether an option with the long key or short key was specified.
        bool hasOption(const std::string &longKey, const std::string &shortKey = "") const {
            if (!longKey.empty() && (longKeyMap.find(longKey) != longKeyMap.end())) {
                return true;
            }
            else if (!shortKey.empty() && (shortKeyMap.find(shortKey) != shortKeyMap.end())) {
                return true;
            }
            else {
                return false;
            }
        }

        // Corresponds to argv[0].
        const std::string &getDirectory() const {
            return directory;
        }

        // No bounds checking, must be a valid index.
        const std::string getArgument(uint32_t index) const {
            return arguments[index];
        }

        // Will be one less than argc.
        uint32_t getArgumentCount() const {
            return arguments.size();
        }
    };

    // Parse and return the arguments in a structure that can be queried easily. Does not perform any validation of the arguments.
    Result parse(int argc, char *argv[]) {
        return Result(argc, argv);
    }
};
