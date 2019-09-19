#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "adm/common_definitions.hpp"
#include "adm/parse.hpp"
#include "adm/write.hpp"

#include "bw64/bw64.hpp"

#include "ear/ear.hpp"
#include "ear/dsp/dsp.hpp"

#include "parser.hpp"
#include "renderer.hpp"

using namespace ear;
using namespace adm;

int main(int argc, char **argv) {

  // Read input BWF64/ADM file
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " [INFILE]" << std::endl;
    exit(1);
  }

  auto bw64File = bw64::readFile(argv[1]);
  auto admDocument = parseAdmXml(bw64File);

  const std::string outputLayout = "0+2+0"; // defined into libear/resources/2051_layouts.yaml
  const Layout layout = getLayout(outputLayout);
  const bool directRendering = true;

  // Extract info: which audio channels to mix to get which programme
  auto programmes = admDocument->getElements<AudioProgramme>();
  for (int i = 0; i < programmes.size(); ++i)
  {
    // parse audio programmes to get audio contents
    auto audioProgramme = programmes[i];
    std::cout << "AudioProgramme: " << audioProgramme->get<AudioProgrammeId>().get<AudioProgrammeIdValue>() << ": "
                                    << audioProgramme->get<AudioProgrammeName>();
    if(audioProgramme->has<AudioProgrammeLanguage>()) {
      std::cout << "(" << audioProgramme->get<AudioProgrammeLanguage>() << ")";
    }
    std::cout << std::endl;

    std::string programmeTitle = audioProgramme->get<AudioProgrammeName>().get();
    std::vector<AudioObjectRenderer> renderers;

    auto contents = audioProgramme->getReferences<AudioContent>();
    for (int i = 0; i < contents.size(); ++i)
    {
      // parse audio content to get audio objects
      auto audioContent = contents[i];
      std::cout << "\tAudioContent: " << audioContent->get<AudioContentId>().get<AudioContentIdValue>() << ": "
                                    << audioContent->get<AudioContentName>();
      if(audioContent->has<AudioContentLanguage>()) {
        std::cout << "(" << audioContent->get<AudioContentLanguage>() << ")";
      }
      std::cout << std::endl;

      auto objects = audioContent->getReferences<AudioObject>();
      for (int i = 0; i < objects.size(); ++i)
      {
        // parse audio objects to get audio packs and tracks
        auto audioObject = objects[i];
        std::cout << "\t\tAudioObject: " << audioObject->get<AudioObjectId>().get<AudioObjectIdValue>() << ": "
                                         << audioObject->get<AudioObjectName>() << std::endl;

        auto audioPackFormatRefs = audioObject->getReferences<AudioPackFormat>();

        std::vector<size_t> trackIds;
        size_t audioPackFormatId;
        TypeDescriptor typeDescriptor = TypeDefinition::UNDEFINED;

        // WARNING: we do not support multi-AudioPackFormat reference by object for now!
        if(audioPackFormatRefs.size()) {
          auto audioPackFormat = audioPackFormatRefs[0];
          audioPackFormatId = audioPackFormat->get<AudioPackFormatId>().get<AudioPackFormatIdValue>().get();
          std::cout << "\t\t\tAudioPackFormat ref: " << audioPackFormat->get<AudioPackFormatId>().get<AudioPackFormatIdValue>() << " ";
          audioPackFormat->get<AudioPackFormatId>().print(std::cout);
          std::cout << " => " << audioPackFormat->get<AudioPackFormatName>()
                    << " " << audioPackFormat->get<TypeDescriptor>() << std::endl;
          typeDescriptor = audioPackFormat->get<TypeDescriptor>();
        }

        auto audioTrackUidRefs = audioObject->getReferences<AudioTrackUid>();
        for (int i = 0; i < audioTrackUidRefs.size(); ++i) {
          auto audioTrackUid = audioTrackUidRefs[i];
          std::cout << "\t\t\tAudioTrackUid ref: " << audioTrackUid->get<AudioTrackUidId>().get<AudioTrackUidIdValue>() << " ";
          audioTrackUid->get<AudioTrackUidId>().print(std::cout);
          std::cout << std::endl;

          // TODO: should we compare the AudioTrackUID with the content of the BW64 "chna" chunk to get the channel ID?

          // Set audio track as input channels
          trackIds.push_back(audioTrackUid->get<AudioTrackUidId>().get<AudioTrackUidIdValue>().get());
        }

        renderers.push_back(AudioObjectRenderer(trackIds, audioPackFormatId, typeDescriptor));
      }
    }

    std::cout << "RENDER PROGRAMME:" << std::endl;
    render(bw64File, layout, programmeTitle, renderers);
  }
  return 0;
}
