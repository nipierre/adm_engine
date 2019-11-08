#include <iostream>

#include <bw64/bw64.hpp>

#include "adm_engine/renderer.hpp"
#include "adm_engine/parser.hpp"

using namespace admengine;

int dumpBw64AdmFile(const std::string& path) {
  auto bw64File = bw64::readFile(path);
  displayBw64FileInfos(bw64File);
  displayAdmDocument(getAdmDocument(parseAdmXmlChunk(bw64File)));
  displayChnaChunk(parseAdmChnaChunk(bw64File));
  return 0;
}

int renderAdmContent(const std::string& input,
                     const std::string& destination,
                     const std::map<std::string, float>& elementGains,
                     const std::string& elementIdToRender = "") {
  auto bw64File = bw64::readFile(input);
  const std::string outputDirectory(destination);
  const std::string outputLayout("0+2+0"); // TODO: get it from args
  Renderer renderer(bw64File, outputLayout, outputDirectory, elementGains, elementIdToRender);
  renderer.process();
  return 0;
}

void displayUsage(const char* application) {
  std::cout << "Usage: " << application << " INPUT [OPTIONS]" << std::endl;
  std::cout << std::endl;
  std::cout << "  INPUT                   BW64/ADM audio file path" << std::endl;
  std::cout << "  OPTIONS:" << std::endl;
  std::cout << "    -o OUTPUT            Destination directory" << std::endl;
  std::cout << "    -e ELEMENT_ID        Select the AudioProgramme or AudioObject to be renderer by ELEMENT_ID" << std::endl;
  std::cout << "    -g ELEMENT_ID=GAIN   GAIN value (in dB) to apply to ADM element defined by its ELEMENT_ID" << std::endl;
  std::cout << std::endl;
  std::cout << "  If no OUTPUT argument is specified, this program dumps the input BW64/ADM file information." << std::endl;
  std::cout << "  Otherwise, it enables ADM rendering to BW64/ADM file into destination directory." << std::endl;
  std::cout << std::endl;
  std::cout << "  Examples:" << std::endl;
  std::cout << "    - Dumping BW64/ADM file info:" << std::endl;
  std::cout << "          " << application << " /path/to/input/file.wav" << std::endl;
  std::cout << "    - Rendering ADM:" << std::endl;
  std::cout << "          " << application << " /path/to/input/file.wav -o /path/to/output/directory" << std::endl;
  std::cout << "    - Rendering specified ADM element:" << std::endl;
  std::cout << "          " << application << " /path/to/input/file.wav -e APR_1002 -o /path/to/output/directory" << std::endl;
  std::cout << "    - Rendering ADM, applying gains to elements:" << std::endl;
  std::cout << "          " << application << " /path/to/input/file.wav -o /path/to/output/directory -g AO_1001=-4.0 -g ACO_1002=5.0" << std::endl;
  std::cout << std::endl;
}

int main(int argc, char **argv) {
  if(argc < 2) {
    displayUsage(argv[0]);
    return 1;
  }

  std::string inputFilePath = argv[1];
  std::string outputDirectoryPath;
  std::string elementIdToRender;
  std::map<std::string, float> elementGains;

  std::cout << "Input file:            " << inputFilePath << std::endl;
  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];
    if(arg == "-o") {
      outputDirectoryPath = argv[++i];
      std::cout << "Output directory:      " << outputDirectoryPath << std::endl;
    } else if(arg == "-e") {
      elementIdToRender = argv[++i];
      std::cout << "ADM element to render: " << elementIdToRender << std::endl;
    } else if(arg == "-g") {
      std::string gainPair = argv[++i];
      size_t splitPos = gainPair.find("=");
      std::string elemId = gainPair.substr(0, splitPos);
      std::string gainDbStr = gainPair.substr(splitPos + 1, gainPair.size());
      elementGains[elemId] = pow(10.0, std::atof(gainDbStr.c_str()) / 20.0);
      std::cout << "Gain:                  " << elementGains[elemId] << " (" << gainDbStr << " dB) applied to " << elemId << std::endl;
    } else {
      displayUsage(argv[0]);
      return 1;
    }
  }


  if(outputDirectoryPath.empty()) {
    return dumpBw64AdmFile(inputFilePath);
  } else {
    return renderAdmContent(inputFilePath, outputDirectoryPath, elementGains, elementIdToRender);
  }
}
