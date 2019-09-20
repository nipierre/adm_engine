#include <algorithm>

#include "ear/ear.hpp"
#include "bw64/bw64.hpp"
#include "adm/adm.hpp"

const unsigned int BLOCK_SIZE = 4096; // in frames

class AudioObjectRenderer {

public:
  AudioObjectRenderer(const std::vector<size_t>& trackIds, const size_t& audioPackFormat, const adm::TypeDescriptor& typeDescriptor)
    : _trackIds(trackIds)
    , _audioPackFormat(audioPackFormat)
    , _typeDescriptor(typeDescriptor)
  {
  }

  std::vector<size_t> getTrackIds() const {
    return _trackIds;
  }

  size_t getAudioPackFormat() const {
    return _audioPackFormat;
  }

  adm::TypeDescriptor getTypeDescriptor() const {
    return _typeDescriptor;
  }

private:
  const std::vector<size_t> _trackIds;
  const size_t _audioPackFormat;
  const adm::TypeDescriptor _typeDescriptor;

};

std::ostream& operator<<(std::ostream& os, const AudioObjectRenderer& renderer) {
  os << "Channels: ";
  for (size_t channel : renderer.getTrackIds()) {
     os << channel << " ";
  }
  os << " - AudioPackFormat: " << renderer.getAudioPackFormat();
  os << " - TypeDesciptor: " << adm::formatTypeDefinition(renderer.getTypeDescriptor());
  return os;
}

std::vector<float> getDirectSpeakersGains(const ear::Layout& layout, const size_t& outputNbChannels) {
  // calculate gains for direct speakers
  std::cout << std::endl << "### Calculate gains:" << std::endl;
  ear::GainCalculatorDirectSpeakers speakerGainCalculator(layout);
  ear::DirectSpeakersTypeMetadata speakersMetadata;
  std::vector<float> gains(outputNbChannels);
  speakerGainCalculator.calculate(speakersMetadata, gains);

  // print the output
  auto fmt = std::setw(10);
  std::cout << std::setprecision(4);

  std::cout << fmt << "channel"
            << fmt << "gain"  << std::endl;
  for (size_t i = 0; i < outputNbChannels; i++) {
    std::cout << fmt << layout.channels()[i].name()
              << fmt << gains[i] << std::endl;
  }
  return gains;
}

void render(const std::unique_ptr<bw64::Bw64Reader>& bw64File,
            const ear::Layout& layout,
            const std::string programmeTitle,
            const std::vector<AudioObjectRenderer> renderers) {

  const size_t outputNbChannels = layout.channels().size();
  std::map<size_t, std::vector<size_t>> channelsMapping;

  for(AudioObjectRenderer renderer: renderers) {
    std::cout << "\t- Render: " << renderer << std::endl;

    switch(renderer.getTypeDescriptor().get()) {
      case 1: // TypeDefinition::DIRECT_SPEAKERS
        break;
      case 0: // TypeDefinition::UNDEFINED
      case 2: // TypeDefinition::MATRIX
      case 3: // TypeDefinition::OBJECTS
      case 4: // TypeDefinition::HOA
      case 5: // TypeDefinition::BINAURAL
        std::cerr << "Unsupported type descriptor: " << adm::formatTypeDefinition(renderer.getTypeDescriptor()) << std::endl;
        exit(1);
    }

    switch(renderer.getAudioPackFormat()) {
      case 1: // AP_00010001 => urn:itu:bs:775:3:pack:mono_(0+1+0)
        if(renderer.getTrackIds().size() != 1) {
          std::cerr << "AudioPackFormat does not fit the number of tracks tracks: " << renderer << std::endl;
          exit(1);
        }
        for (int oc = 0; oc < outputNbChannels; ++oc) {
          channelsMapping[oc].push_back(renderer.getTrackIds().at(0));
        }
        break;

      case 2: // AP_00010002 => urn:itu:bs:2051:0:pack:stereo_(0+2+0)
        if(renderer.getTrackIds().size() != 2) {
          std::cerr << "AudioPackFormat does not fit the number of tracks tracks: " << renderer << std::endl;
          exit(1);
        }
        for (int oc = 0; oc < outputNbChannels; ++oc) {
          channelsMapping[oc].push_back(renderer.getTrackIds().at(oc));
        }
        break;

      default:
        std::cerr << "Unsupported audio pack format: " << renderer.getAudioPackFormat() << std::endl;
        exit(1);
    }
  }

  std::cout << "CHANNEL MAPPING: " << std::endl;
  for (const auto& mapping : channelsMapping) {
    std::cout << "  - Input channels: ";
    for (auto intputChannel : mapping.second) {
      std::cout << intputChannel << " ";
    }
    std::cout << " ==> output channel: " << mapping.first << std::endl;
  }

  const std::vector<float> gains = getDirectSpeakersGains(layout, outputNbChannels);

  // Render samples with gains
  std::cout << std::endl << "### Render samples with gains:" << std::endl;

  // Output file
  std::stringstream outputFileName;
  outputFileName << programmeTitle << ".wav";
  auto outputFile = bw64::writeFile(outputFileName.str(), outputNbChannels, bw64File->sampleRate(), bw64File->bitDepth());

  // Buffers
  const size_t inputNbChannels = bw64File->channels();
  std::vector<float> fileInputBuffer(BLOCK_SIZE * inputNbChannels);
  std::vector<float> fileOutputBuffer;

  // Read file, render with gains and write output file
  size_t filePosition = 0;
  while (!bw64File->eof()) {
    // Read a data block
    auto readFrames = bw64File->read(&fileInputBuffer[0], BLOCK_SIZE);
    float* ocsample = new float[outputNbChannels];
    size_t frame = 0;
    size_t sample = 0;

    while(frame < readFrames) {
      for (int oc = 0; oc < outputNbChannels; ++oc) {
        // for each output channel, apply the mapped input channels...
        ocsample[oc] = 0.0;
        for(size_t ic : channelsMapping.at(oc)) {
          ocsample[oc] += fileInputBuffer[sample + ic] * gains[oc];
        }
      }
      filePosition += inputNbChannels;
      sample += inputNbChannels;
      for (int oc = 0; oc < outputNbChannels; ++oc) {
        fileOutputBuffer.push_back(ocsample[oc]); // FIXME : double sound in output ?
      }
      frame++;
    }

    auto wroteFrames = outputFile->write(&fileOutputBuffer[0], readFrames);
    fileOutputBuffer.clear();
    delete[] ocsample;
  }
  std::cout << "Done with: " << outputFileName.str() << std::endl;
  bw64File->seek(0);
}
