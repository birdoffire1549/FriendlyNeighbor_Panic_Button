/*
  ParseUtils - A class of utility functions to aid in the parsing and deriving 
  of information from one form to another.

  Written by: Scott Griffis
  Date: 10-01-2023
  Version: 3.0.0
*/

#include "ParseUtils.h"

/**
 * Allows for information to be parsed out of a String between a Keyword and a Terminating
 * String. If the Terminator doesn't exist then this function will parse to the end of the line.
 * If character marking the end of the line then the terminator will be the end of the String.
 * This is particularly useful when parsing data from key/value pairs.
 * 
 * Example:
 * If string was equal to "This is a sentence and it may contain a lot of data", and one was to
 * set the keyword to "a " and the terminator to " and", then you would get as a result the word
 * "sentence", as a String. If the prior is true but the terminator was "$" the result would be, 
 * "sentence and it may contain a lot of data", since the "$" terminator would not be found.
 * 
 * @param string - The string from which to parse data as String.
 * @param keyword - The keyword to start parsing data just after as String.
 * @param terminator - The terminator to parse up until as String.
 * 
 * @return Returns the parsed data as String
 */
String ParseUtils::parseByKeyword(String &str, String keyword, String terminator) {
    int keyIndex = str.indexOf(keyword);
    if (keyIndex == -1) { // Keyword wasn't found...
        
        return "";
    }
    int endIndex = str.indexOf(terminator, keyIndex + keyword.length());

    // Look for Terminator if not found parse to end of line...
    if (endIndex == -1) { // Terminator not found...
        endIndex = str.indexOf("\n", keyIndex);
    }

    // If line terminator not found then to end of string...
    if (endIndex == -1) { // New-line not found...

        return str.substring(keyIndex + keyword.length());
    } 

    return str.substring(keyIndex + keyword.length(), endIndex);
}

/**
 * Allows for information to be parsed out of a string between a Keyword and a Terminating
 * string. If the Terminator doesn't exist then this function will parse to the end of the line.
 * If character marking the end of the line then the terminator will be the end of the string.
 * This is particularly useful when parsing data from key/value pairs.
 * 
 * Example:
 * If str was equal to "This is a sentence and it may contain a lot of data", and one was to
 * set the keyword to "a " and the terminator to " and", then you would get as a result the word
 * "sentence", as a string. If the prior is true but the terminator was "$" the result would be, 
 * "sentence and it may contain a lot of data", since the "$" terminator would not be found.
 * 
 * @param str - The string from which to parse data as std::string.
 * @param keyword - The keyword to start parsing data just after as std::string.
 * @param terminator - The terminator to parse up until as std::string.
 * 
 * @return Returns the parsed data as std::string
 */
std::string ParseUtils::parseByKeyword(std::string &str, std::string keyword, std::string terminator) {
    int keyIndex = str.find(keyword);
    if (keyIndex == -1) { // Keyword wasn't found...
        
        return "";
    }
    int endIndex = str.find(terminator, keyIndex + keyword.length());

    // Look for Terminator if not found parse to end of line...
    if (endIndex == -1) { // Terminator not found...
        endIndex = str.find("\n", keyIndex);
    }

    // If line terminator not found then to end of string...
    if (endIndex == -1) { // New-line not found...

        return substring(str, keyIndex + keyword.length());
    } 

    return substring(str, keyIndex + keyword.length(), endIndex);
}

/**
 * Counts the number of occurrences of a specific character within a 
 * given String.
 * 
 * @param str - The String from which the occurances of the given character will be counted.
 * @param toCnt - The character of which to count the occurrances of.
 * 
 * @return Returns the number of occurrences counted as unsigned int.
 */
unsigned int ParseUtils::occurrences(String &str, char toCnt) {
  unsigned int count = 0;
    for (unsigned int i = 0; i < str.length(); i++) { // Iterate string looking for chars...
      if (str.charAt(i) == toCnt) { // Char to count was found...
        count ++;
      }
    }

  return count;
}

/**
 * Used to tuncate a string to a specific length.
 * If the given string is less then the truncate length then the
 * given string is returned unaltered. If the given string is longer
 * than specified length then the string is trimmed to the specified
 * length.
 * 
 * @param str The given string to truncate as String.
 * @param length The length to truncate the string to as unsigned int.
 * 
 * @return Returns the truncated string as String.
 */
String ParseUtils::trunc(String &str, unsigned int length) {
  if (str.length() <= length) {

    return str;
  }
  
  return str.substring(0, (length - 1U));
}

/**
 * Counts the number of occurrences of a specific character within a 
 * given string.
 *
 * @param str - The string from which the occurances of the given character will be counted, as std::string.
 * @param toCnt - The character of which to count the occurrances of, as char.
 *
 * @return Returns the number of occurrences counted as unsigned int.
 */
unsigned int ParseUtils::occurrences(std::string &str, char toCnt) {
  unsigned int count = 0;
  for (unsigned int i = 0; i < str.length(); i++) { // Iterate string looking for chars...
    if (str.at(i) == toCnt) { // Char to count was found...
      count ++;
    }
  }

  return count;
}

/**
 * Counts the number of occurrences of a specific String within a 
 * given String.
 *
 * @param str - The String from which the occurances of the given String will be counted as String.
 * @param toCnt - The String of which to count the occurrances of as String.
 *
 * @return Returns the number of occurrences counted as unsigned int.
 */ 
unsigned int ParseUtils::occurrences(String &str, String toCnt) {
  unsigned int count = 0;
  int location = toCnt.length() * -1;
  while ((location = str.indexOf(toCnt, location + toCnt.length())) != -1) {
    count++;
  }

  return count;
}

/**
 * Counts the number of occurrences of a specific string within a 
 * given string.
 *
 * @param str - The string from which the occurances of the given string will be counted as std::string.
 * @param toCnt - The string of which to count the occurrances of as std::string.
 *
 * @return Returns the number of occurrences counted as unsigned int.
 */
unsigned int ParseUtils::occurrences(std::string &str, std::string toCnt) {
  unsigned int count = 0;
  int location = toCnt.length() * -1;
  while ((location = str.find(toCnt, location + toCnt.length())) != -1) {
    count++;
  }

  return count;
}

/**
 * Splits the given string up into multiple segments based on the given seporator. The
 * split up data segments are stored into a given String based storage array of a specific
 * size. If the storage array is too small for all of the data segments then it will be filled
 * with what it has space for, and the rest will be discarded.
 *
 * @param str - The string to perform the operation on.
 * @param separator - The character to use as a separator for the splitting process.
 * @param storage - An array of strings for storage of the results of the splitting process.
 * @param sizeOfStorage - The size of the storage array provided.
 */
void ParseUtils::split(String &str, char separator, String *storage, unsigned int sizeOfStorage) {
  unsigned int index = 0;
  for (unsigned int segmentIndex = 0U; segmentIndex < sizeOfStorage && index < str.length(); segmentIndex++) { // iterate segment storage...
    unsigned int startIndex = index;
    for (;index < str.length(); index++) { // increment index though length of the string...
      if (str.charAt(index) == separator) { // Found a separator...
        storage[segmentIndex] = str.substring(startIndex, index);
        index++;

        break; // to allow segmentIndex to increment
      } else if (index == (str.length() - 1)) { // at the end of the string...
        index++;
        storage[segmentIndex] = str.substring(startIndex, index);
      }
    }
  }
}

/*
 * Splits the given string up into multiple segments based on the given seporator. The
 * split up data segments are stored into a given string based storage array of a specific
 * size. If the storage array is too small for all of the data segments then it will be filled
 * with what it has space for, and the rest will be discarded.
 *
 * @param str - The string to perform the operation on as std::string.
 * @param separator - The character to use as a separator for the splitting process as char.
 * @param storage - An array of strings for storage of the results of the splitting process as std::string pointer.
 * @param sizeOfStorage - The size of the storage array provided as int.
 */
void ParseUtils::split(std::string &str, char separator, std::string *storage, unsigned int sizeOfStorage) {
  unsigned int index = 0U;
  for (unsigned int segmentIndex = 0U; segmentIndex < sizeOfStorage && index < str.length(); segmentIndex++) { // iterate segment storage...
    unsigned int startIndex = index;
    for (;index < str.length(); index++) { // increment index though length of the string...
      if (str.at(index) == separator) { // Found a separator...
        storage[segmentIndex] = substring(str, startIndex, index);
        index++;

        break; // to allow segmentIndex to increment
      } else if (index == (str.length() - 1)) { // at the end of the string...
        index++;
        storage[segmentIndex] = substring(str, startIndex, index);
      }
    }
  }
}

/**
 * Performs a substring type function on the given string where what is returned
 * is determined by parsing the data out inclusively from the beginIndex and 
 * exclusively up to the endIndex specified. Remember the first character in the
 * string is considered to be at index zero.
 *
 * @param str - The string to parse from as std::string.
 * @param beginIndex - The index to inclusively begin parsing from as unsigned int.
 * @param endIndex - The index to exclusively parse up to as unsigned int.
 *
 * @return Returns the parsed string as std::string.
 */
std::string ParseUtils::substring(std::string &str, unsigned int beginIndex, unsigned int endIndex) {

  return str.substr(beginIndex, endIndex - beginIndex);
}

/**
 * Performs a substring type function on the given string where what is returned
 * is determined by parsing the data out inclusively from the beginIndex and 
 * up to the end of the given string. Remember the first character in the
 * string is considered to be at index zero.
 *
 * @param str - The string to parse from as std::string.
 * @param beginIndex - The index to inclusively begin parsing from as unsigned int.
 * 
 * @return Returns the parsed string as std::string.
 */
std::string ParseUtils::substring(std::string &str, unsigned int beginIndex) {

  return str.substr(beginIndex);
}

/**
 * Trims whitespace from both ends of the given string, returning
 * the given string without any leading or trailing whitespace.
 *
 * @param str - The string to trim as std::string.
 * 
 * @return Returns the resulting string as std::string.
 */
std::string ParseUtils::trim(std::string &str) {
  std::string result = str;
  // Trim from the front of the string...
  while(!result.empty() && result.at(0) <= 32) {
    result.erase(0, 1);
  }

  // Trim from the end of the string...
  while(!result.empty() && result.at(result.length() - 1) <= 32) {
    result.erase(result.length() - 1, 1);
  }

  return result;
}

/**
 * Used to parse a float out from a string that contains a valid floating point 
 * number. If the contents of the given string are not a valid floating point number
 * then the value of 0.0 will be returned.
 *
 * @param str - The string to be parsed as a floating point number, as std::string.
 * 
 * @return Returns the parsed value as float.
 */
float ParseUtils::toFloat(std::string str) {
  if (!str.empty()) { // Something to work on...
    for (unsigned int i = 0; i < str.length(); i++) { // Verify that chars are valid for a number...
      if (!((str.at(i) >= '0' && str.at(i) <= '9') || str.at(i) == '.' || str.at(i) == '-')) { // not even close to valid number...
        
        return 0;
      }
    }

    int minus = occurrences(str, '-');
    if (minus > 1 || (minus == 1 && str.at(0) != '-')) { // too many minus signs or in wrong spot...

      return 0;
    }

    int period = occurrences(str, '.');
    if (period > 1) { // Not a valid number...

      return 0;
    }

    return std::stof(str);
  }

  return 0;
}

/**
 * Used to parse a double out from a string that contains a valid double 
 * number. If the contents of the given string are not a valid double number
 * then the value of zero will be returned.
 *
 * @param str - The string to be parsed as a double number, as std::string.
 * 
 * @return Returns the parsed value as double.
 */
double ParseUtils::toDouble(std::string str) {
  if (!str.empty()) { // Something to work on...
    for (unsigned int i = 0; i < str.length(); i++) { // Verify that chars are valid for a number...
      if (!((str.at(i) >= '0' && str.at(i) <= '9') || str.at(i) == '.' || str.at(i) == '-')) { // not even close to valid number...
        
        return 0;
      }
    }

    int minus = occurrences(str, '-');
    if (minus > 1 || (minus == 1 && str.at(0) != '-')) { // too many minus signs or in wrong spot...

      return 0;
    }

    int period = occurrences(str, '.');
    if (period > 1) { // Not a valid number...

      return 0;
    }

    return std::stod(str);
  }

  return 0;
}

/**
 * Used to parse an int out from a string that contains a valid integer 
 * number. If the contents of the given string are not a valid integer 
 * then the integer value of zero will be returned.
 *
 * @param str - The string to be parsed as an integer as std::string.
 * 
 * @return Returns the parsed value as int.
 */
int ParseUtils::toInt(std::string str) {
  if (!str.empty()) { // Something to work on...
    for (unsigned int i = 0; i < str.length(); i++) { // Verify that chars are valid for a number...
      if (!((str.at(i) >= '0' && str.at(i) <= '9') || str.at(i) == '.' || str.at(i) == '-')) { // not even close to valid number...
        
        return 0;
      }
    }

    int minus = occurrences(str, '-');
    if (minus > 1 || (minus == 1 && str.at(0) != '-')) { // too many minus signs or in wrong spot...

      return 0;
    }

    int period = occurrences(str, '.');
    if (period > 1) { // Not a valid number...

      return 0;
    }

    if (period == 1) { // Number is a float type...
      // lose percision...
      std::string temp = substring(str, 0, str.find('.'));
      if (temp.empty()) {

        return 0;
      }

      return std::stoi(temp);
    }
  }

  return std::stoi(str);
}

/**
 * This is used to convert a string representation of a hex value into the
 * equivelent unsigned int value.
 *
 * For Example:
 * The String "2B" would be converted to an unsigned int value of 43.
 * 
 * As such this function makes it rather trivial to do a compairison of 
 * the String "2B" to the actual hex value of 0x2B.
 *
 * @param hex - The hex string to perform conversion on as String.
 *
 * @return Returns the hex value as an unsigned int.
 */
unsigned int ParseUtils::hexStringToInt(String hex) {
  unsigned int result = 0;
  for (unsigned int i = 0; i < hex.length(); i++) {
    char c = hex.charAt(i);
    unsigned int temp = c - '0';
    if (temp >= 0 && temp <= 9) { // is a number digit...
      result += (temp * pow(16,(hex.length() - i - 1)));
    } else { // Could be a letter digit...
      temp = c - 'A' + 10;
      if (temp >= 0xA && temp <= 0xF) { // was a capital letter digit of proper value...
        result += (temp * pow(16,(hex.length() - i - 1)));      
      } else { // could be a lowercase letter digit...
        temp = c - 'a' + 10;
        if (temp >= 0xA && temp <= 0xF) { // was a lowercase letter digit of proper value...
          result += (temp * pow(16,(hex.length() - i - 1)));      
        } else { // Seems to not be a valid hex string...

          return 0; // because value is jacked up no matter what
        }
      }
    }
  }

  return result;
}

/**
 * This is used to convert a string representation of a hex value into the
 * equivelent unsigned int value.
 *
 * For Example:
 * The String "2B" would be converted to an unsigned int value of 43.
 * 
 * As such this function makes it rather trivial to do a compairison of 
 * the String "2B" to the actual hex value of 0x2B.
 *
 * @param hex - The hex string to perform conversion on, as std::string.
 *
 * @return Returns the hex value as an unsigned int.
 */
unsigned int ParseUtils::hexStringToInt(std::string hex) {
  unsigned int result = 0;
  for (unsigned int i = 0; i < hex.length(); i++) {
    char c = hex.at(i);
    unsigned int temp = c - '0';
    if (temp >= 0 && temp <= 9) { // is a number digit...
      result += (temp * pow(16,(hex.length() - i - 1)));
    } else { // Could be a letter digit...
      temp = c - 'A' + 10;
      if (temp >= 0xA && temp <= 0xF) { // was a capital letter digit of proper value...
        result += (temp * pow(16,(hex.length() - i - 1)));      
      } else { // could be a lowercase letter digit...
        temp = c - 'a' + 10;
        if (temp >= 0xA && temp <= 0xF) { // was a lowercase letter digit of proper value...
          result += (temp * pow(16,(hex.length() - i - 1)));      
        } else { // Seems to not be a valid hex string...

          return 0; // because value is jacked up no matter what
        }
      }
    }
  }

  return result;
}

/**
 * This function allows for an inputPattern to be applied to a given inputString, then using the given
 * desiredPattern the characters masked by the inputPattern are rearranged to match the given desiredPattern.
 * 
 * Example: 
 * String output = ParseUtils::arrangeDigitsUsingPattern("23022023", "ddmmyyyy", "mmddyyyy");
 * output will be "02232023"
 *
 * Mask patterns can be anything and hold no meaning to this function.
 * Note: I wrote this a while ago for a particular use case and as of right now am not fully aware of its full
 * abilities or faults. I hope to one day revisit it and nail-down its exact streangths and/or weaknesses.
 *
 * @param inputString - The string to rearrange as String.
 * @param inputPatter - The masking pattern for the inputString as String.
 * @param desiredPattern - The desired arrangement of the inputPattern and the characters it masks as String.
 *
 * @return Returns the rearranged string based on the shuffling of the inputString as masked by the inputPatter
 * rearranged to match the desiredPattern, as String.
 */
String ParseUtils::arrangeDigitsUsingPattern(String inputString, String inputPattern, String desiredPattern) {
  if (!inputString.isEmpty() && !inputPattern.isEmpty() && !desiredPattern.isEmpty()) { // Something to work with...
    if (inputString.length() == inputPattern.length()) { // That's good; input pattern and string are same length as required...
      char maskChars[inputPattern.length()] = {0};
      String maskedContent[inputPattern.length()] = {""};

      char prevChar = 0;
      int procIndex = -1;
      for (unsigned int i = 0; i < inputPattern.length(); i++) { // Process the inputString using inputPattern...
        if (inputPattern.charAt(i) == prevChar) { // Cur pattern char same as last...
          maskedContent[procIndex].concat(inputString.charAt(i));
        } else { // New pattern char...
          procIndex ++;
          maskChars[procIndex] = inputPattern.charAt(i);
          maskedContent[procIndex].concat(inputString.charAt(i));
        }
      }

      String result = "";
      for (unsigned int i = 0; i < desiredPattern.length(); i++) { // Build out result based on desiredPattern...
        char c = desiredPattern.charAt(i);
        int count = countConsecutiveRepeatingChars(desiredPattern, i);
        
        // Search for mask related data...
        unsigned int fetchIndex = 0;
        bool found = false;
        for (;fetchIndex < inputPattern.length(); fetchIndex++) {
          if (maskChars[fetchIndex] == c) {
            found = true;

            break;
          }
        }
        if (found) {
          String content = maskedContent[fetchIndex];

          result += content.substring(content.length() - count, content.length());
        } else { 
          found += c; // Pattern char must not be mask but literal
        }
      }

      return result;
    }
  }

  return "";
}

/**
 * This function allows for an inputPattern to be applied to a given inputString, then using the given
 * desiredPattern the characters masked by the inputPattern are rearranged to match the given desiredPattern.
 * 
 * Example: 
 * std::string output = ParseUtils::arrangeDigitsUsingPattern("23022023", "ddmmyyyy", "mmddyyyy");
 * output will be "02232023"
 *
 * Mask patterns can be anything and hold no meaning to this function.
 * Note: I wrote this a while ago for a particular use case and as of right now am not fully aware of its full
 * abilities or faults. I hope to one day revisit it and nail-down its exact streangths and/or weaknesses.
 *
 * @param inputString - The string to rearrange as std::string.
 * @param inputPatter - The masking pattern for the inputString as std::string.
 * @param desiredPattern - The desired arrangement of the inputPattern and the characters it masks as std::string.
 *
 * @return Returns the rearranged string based on the shuffling of the inputString as masked by the inputPatter
 * rearranged to match the desiredPattern, as std::string.
 */
std::string ParseUtils::arrangeDigitsUsingPattern(std::string inputString, std::string inputPattern, std::string desiredPattern) {
  if (!inputString.empty() && !inputPattern.empty() && !desiredPattern.empty()) { // Something to work with...
    if (inputString.length() == inputPattern.length()) { // That's good; input pattern and string are same length as required...
      char maskChars[inputPattern.length()] = {0};
      std::string maskedContent[inputPattern.length()] = {""};

      char prevChar = 0;
      int procIndex = -1;
      for (unsigned int i = 0; i < inputPattern.length(); i++) { // Process the inputString using inputPattern...
        if (inputPattern.at(i) == prevChar) { // Cur pattern char same as last...
          maskedContent[procIndex] += inputString.at(i);
        } else { // New pattern char...
          procIndex ++;
          maskChars[procIndex] = inputPattern.at(i);
          maskedContent[procIndex] += inputString.at(i);
        }
      }

      std::string result = "";
      for (unsigned int i = 0; i < desiredPattern.length(); i++) { // Build out result based on desiredPattern...
        char c = desiredPattern.at(i);
        int count = countConsecutiveRepeatingChars(desiredPattern, i);
        
        // Search for mask related data...
        unsigned int fetchIndex = 0;
        bool found = false;
        for (;fetchIndex < inputPattern.length(); fetchIndex++) {
          if (maskChars[fetchIndex] == c) {
            found = true;

            break;
          }
        }
        if (found) {
          std::string content = maskedContent[fetchIndex];

          result += substring(content, content.length() - count, content.length());
        } else { 
          found += c; // Pattern char must not be mask but literal
        }
      }

      return result;
    }
  }

  return "";
}

/**
 * Counts the number of repeating characters in the given string from the specified
 * beginIndex and then returns that count.
 *
 * @param str - The string to count from as String
 * @param beginIndex - The index of the first character to begin the count from as unsigned int.
 *
 * @return Returns the count as unsigned int.
 */
unsigned int ParseUtils::countConsecutiveRepeatingChars(String &str, unsigned int beginIndex) {
  unsigned int count = 1;
  char prevChar = str.charAt(beginIndex);
  for (unsigned int i = (beginIndex + 1); i < str.length(); i++) { // Iterate over string chars...
    if (prevChar == str.charAt(i)) { // Current char matches previous...
      count++;
    } else { // Current char doesn't match previous one...

      break;
    }
  }

  return count;
}

/**
 * Counts the number of repeating characters in the given string from the specified
 * beginIndex and then returns that count.
 *
 * @param str - The string to count from as std::string
 * @param beginIndex - The index of the first character to begin the count from as unsigned int.
 *
 * @return Returns the count as unsigned int.
 */
unsigned int ParseUtils::countConsecutiveRepeatingChars(std::string &str, unsigned int beginIndex) {
  unsigned int count = 1;
  char prevChar = str.at(beginIndex);
  for (unsigned int i = (beginIndex + 1); i < str.length(); i++) { // Iterate over string chars...
    if (prevChar == str.at(i)) { // Current char matches previous...
      count++;
    } else { // Current char doesn't match previous one...

      break;
    }
  }

  return count;
}

/**
 * Decodes a URL encoded string.
 *
 * @param str The string to decode as String. 
 */
String ParseUtils::decodeUrlString(String &str) {
  str.replace("%20", " ");
  str.replace("%21", "!");
  str.replace("%23", "#");
  str.replace("%24", "$");
  str.replace("%26", "&");
  str.replace("%27", "'");
  str.replace("%28", "(");
  str.replace("%29", ")");
  str.replace("%2A", "*");
  str.replace("+", " ");
  str.replace("%2B", "+");
  str.replace("%2C", ",");
  str.replace("%2F", "/");
  str.replace("%3A", ":");
  str.replace("%3B", ";");
  str.replace("%3D", "=");
  str.replace("%3F", "?");
  str.replace("%40", "@");
  str.replace("%5B", "[");
  str.replace("%2D", "]");
  str.replace("%22", "\"");
  str.replace("%25", "%");
  str.replace("%2D", "-");
  str.replace("%2E", ".");
  str.replace("%3C", "<");
  str.replace("%3E", ">");
  str.replace("%5C", "\\");
  str.replace("%5E", "^");
  str.replace("%5F", "_");
  str.replace("%60", "`");
  str.replace("%7B", "{");
  str.replace("%7C", "|");
  str.replace("%7D", "}");
  str.replace("%7E", "~");

  return str;
}

/**
 * Decodes a URL encoded string.
 *
 * @param str - The string to decode as std::string. 
 */
std::string ParseUtils::decodeUrlString(std::string &str) {
  replace(str, "%20", " ");
  replace(str, "%21", "!");
  replace(str, "%23", "#");
  replace(str, "%24", "$");
  replace(str, "%26", "&");
  replace(str, "%27", "'");
  replace(str, "%28", "(");
  replace(str, "%29", ")");
  replace(str, "%2A", "*");
  replace(str, "+", " ");
  replace(str, "%2B", "+");
  replace(str, "%2C", ",");
  replace(str, "%2F", "/");
  replace(str, "%3A", ":");
  replace(str, "%3B", ";");
  replace(str, "%3D", "=");
  replace(str, "%3F", "?");
  replace(str, "%40", "@");
  replace(str, "%5B", "[");
  replace(str, "%2D", "]");
  replace(str, "%22", "\"");
  replace(str, "%25", "%");
  replace(str, "%2D", "-");
  replace(str, "%2E", ".");
  replace(str, "%3C", "<");
  replace(str, "%3E", ">");
  replace(str, "%5C", "\\");
  replace(str, "%5E", "^");
  replace(str, "%5F", "_");
  replace(str, "%60", "`");
  replace(str, "%7B", "{");
  replace(str, "%7C", "|");
  replace(str, "%7D", "}");
  replace(str, "%7E", "~");

  return str;
}

/**
 * Used to replace a specified string of characters from within a given string, with another
 * string of characters. This supports the replaceWith string being larger than the string 
 * being replaced as specified with the 'find' string.
 *
 * @param str - The string containing the string to be replaced as std::string.
 * @param find - The string to find for replacement as std::string.
 * @param replaceWith - The string to replace the found string with as std::string.
 *
 * @return Returns the resulting string as std::string. 
 */
std::string ParseUtils::replace(std::string &str, std::string find, std::string replaceWith) {
  std::string result = "";
  int fCount = occurrences(str, find);
  int trailIndex = 0;
  int leadIndex = 0;
  for (int i = 0; i < fCount; i++) {
    leadIndex = str.find(find);
    result += substring(str, trailIndex, leadIndex) + replaceWith;
    trailIndex = leadIndex + find.length();
  }

  return result;
}

/**
 * This is used to tell if the given string is a valid Dot Notation
 * IP Address. If it is valid then true is returned otherwise false
 * as bool.
 *
 * @param str - The string to validate as String.
 *
 * @return Returns the result as bool.
 */
bool ParseUtils::validDotNotationIp(String &str) {
  if (occurrences(str, '.') != 3) { // Not valid IPv4 dot notation because not 3 separators...

    return false;
  }

  String octets[4] = {""};
  split(str, '.', octets, sizeof(octets));
  for (int i = 0; i < 4; i++) { // Iterate and verify octet values...
    int oct = octets[i].toInt();
    switch (i) {
      case 0:
        if (oct < 1 || oct > 223) {

          return false;
        }
        break;
      default:
        if (oct < 1 || oct > 255) {

          return false;
        }
        break;
    }
  }

  return true;
}