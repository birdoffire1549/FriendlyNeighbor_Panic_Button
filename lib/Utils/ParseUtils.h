/*
  ParseUtils - A class of utility functions to aid in the parsing and deriving
  of information from one form to another.

  Written by: Scott Griffis
  Date: 10-01-2023
  Version: 3.0.0
*/

#ifndef ParseUtils_h
#define ParseUtils_h

#include <WString.h>
#include <string>
#include <math.h>

class ParseUtils {
    private:
        ParseUtils();

    public:
        static String arrangeDigitsUsingPattern(String inputString, String inputPattern, String desiredPattern);
        static std::string arrangeDigitsUsingPattern(std::string inputString, std::string inputPattern, std::string desiredPattern);

        static unsigned int countConsecutiveRepeatingChars(String &str, unsigned int beginIndex);
        static unsigned int countConsecutiveRepeatingChars(std::string &str, unsigned int beginIndex);

        static String decodeUrlString(String &str);
        static std::string decodeUrlString(std::string &str);
        
        static unsigned int hexStringToInt(String hex);
        static unsigned int hexStringToInt(std::string hex);

        static unsigned int occurrences(String &str, char toCnt);
        static unsigned int occurrences(std::string &str, char toCnt);
        static unsigned int occurrences(std::string &str, std::string toCnt);
        static unsigned int occurrences(String &str, String toCnt);

        static String parseByKeyword(String &str, String keyword, String terminator);
        static std::string parseByKeyword(std::string &str, std::string keyword, std::string terminator);

        static std::string replace(std::string &str, std::string find, std::string replaceWith);

        static void split(String &str, char separator, String *storage, unsigned int sizeOfStorage);
        static void split(std::string &str, char separator, std::string *storage, unsigned int sizeOfStorage);
        static std::string substring(std::string &str, unsigned int beginIndex, unsigned int endIndex);
        static std::string substring(std::string &str, unsigned int beginIndex);

        static int toInt(std::string str);
        static float toFloat(std::string str);
        static double toDouble(std::string str);
        static std::string trim(std::string &str);
        static String trunc(String &str, unsigned int length);
        
        static bool validDotNotationIp(String &str);
};
#endif