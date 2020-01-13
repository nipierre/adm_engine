#pragma once

#include <regex>

namespace admengine {

const std::map<std::string, std::string> specCharMap = {
  { " ", "_" },
  { "é", "e" },
  { "è", "e" },
  { "ê", "e" },
  { "ë", "e" },
  { "à", "a" },
  { "â", "a" },
  { "ä", "a" },
  { "ù", "u" },
  { "û", "u" },
  { "ü", "u" }
  // TODO: complete this list...
};

const std::regex specCharRegex("[^a-zA-Z0-9_-]");

std::string replaceSpecialCharacters(std::string& text) {
  // map some special characters
  std::map<std::string, std::string>::const_iterator it;
  for (it = specCharMap.begin(); it != specCharMap.end(); it++) {
    size_t position = 0;
    while((position = text.find(it->first, position)) != std::string::npos) {
      text.replace(position, it->first.size(), it->second);
      position += it->second.size();
    }
  }

  // remove remaining special characters
  text = std::regex_replace(text, specCharRegex, "");
  return text;
}

}
