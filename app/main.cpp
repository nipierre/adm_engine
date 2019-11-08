#include <iostream>

#include <bw64/bw64.hpp>

#include "adm_engine/renderer.hpp"
#include "adm_engine/parser.hpp"

using namespace admengine;

int dumpBw64AdmFile(const char* path) {
  auto bw64File = bw64::readFile(path);
  displayBw64FileInfos(bw64File);
  displayAdmDocument(getAdmDocument(parseAdmXmlChunk(bw64File)));
  displayChnaChunk(parseAdmChnaChunk(bw64File));
  return 0;
}

int renderAdmContent(const char* input, const char* destination, const std::map<std::string, float> elementGains) {
  auto bw64File = bw64::readFile(input);
  const std::string outputDirectory(destination);
  const std::string outputLayout("0+2+0"); // TODO: get it from args
  Renderer renderer(bw64File, outputLayout, outputDirectory, elementGains);
  renderer.process();
  return 0;
}

void displayUsage(const char* application) {
  std::cout << "Usage: " << application << " INPUT [OUTPUT] [ELEMENT_ID=GAIN]" << std::endl;
  std::cout << "   with INPUT              BW64/ADM audio file" << std::endl;
  std::cout << "        OUTPUT             Destination directory (optional)" << std::endl;
  std::cout << "                             - if specified, enable ADM rendering to BW64/ADM file" << std::endl;
  std::cout << "                             - otherwise, dump input BW64/ADM file information" << std::endl;
  std::cout << "        ELEMENT_ID=GAIN    GAIN value (in dB) to apply to ADM element defined by its ELEMENT_ID (optional)" << std::endl;
}

std::map<std::string, float> getElementGains(int argc, char **argv) {
  std::map<std::string, float> elementGains;
  for (int i = 3; i < argc; ++i) {
    std::string gainPair(argv[i]);
    size_t splitPos = gainPair.find("=");
    std::string elemId = gainPair.substr(0, splitPos);
    std::string gainDbStr = gainPair.substr(splitPos + 1, gainPair.size());
    elementGains[elemId] = pow(10.0, std::atof(gainDbStr.c_str()) / 20.0);
    std::cout << "Gain: " << elementGains[elemId] << " (" << gainDbStr << " dB) applied to " << elemId << std::endl;
  }
  return elementGains;
}

int main(int argc, char **argv) {
  // Read input BWF64/ADM file
  if(argc == 2) {
    return dumpBw64AdmFile(argv[1]);
  } else if(argc == 3) {
    return renderAdmContent(argv[1], argv[2], {});
  } else if(argc > 3) {
    return renderAdmContent(argv[1], argv[2], getElementGains(argc, argv));
  } else {
    displayUsage(argv[0]);
    return 1;
  }
}
