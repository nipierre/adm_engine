
#include "adm_helper.hpp"

#include "parser.hpp"

namespace admengine {

std::shared_ptr<adm::AudioObject> createAdmAudioObject(const adm::AudioObjectName& audioObjectName, const ear::Layout& outputLayout) {
  auto mixObject = adm::AudioObject::create(audioObjectName);
  auto mixPackFormat = adm::AudioPackFormat::create(adm::AudioPackFormatName(""), adm::TypeDefinition::DIRECT_SPEAKERS);
  adm::AudioPackFormatId mixPackFormatId = adm::audioPackFormatLookupTable().at(outputLayout.name());
  mixPackFormat->set(mixPackFormatId);

  mixObject->addReference(mixPackFormat);
  for (auto channel : outputLayout.channels())
  {
    auto audioTrackUid = adm::AudioTrackUid::create();
    audioTrackUid->setReference(mixPackFormat);

    adm::AudioTrackFormatId audioTrackFormatId = adm::audioTrackFormatLookupTable().at(channel.name());
    auto audioTrackFormat = adm::AudioTrackFormat::create(adm::AudioTrackFormatName(""), adm::FormatDefinition::PCM);
    audioTrackFormat->set(audioTrackFormatId);
    audioTrackUid->setReference(audioTrackFormat);

    mixObject->addReference(audioTrackUid);
  }
  return mixObject;
}

std::shared_ptr<adm::Document> createAdmDocument(const std::shared_ptr<adm::AudioProgramme>& audioProgramme, const ear::Layout& outputLayout) {
  std::shared_ptr<adm::Document> admDocument = adm::Document::create();
  auto admProgramme = adm::AudioProgramme::create(audioProgramme->get<adm::AudioProgrammeName>());
  auto mixContent = adm::AudioContent::create(adm::AudioContentName("Mix"));
  auto mixObject = createAdmAudioObject(adm::AudioObjectName("Mix"), outputLayout);
  mixContent->addReference(mixObject);
  admProgramme->addReference(mixContent);
  admDocument->add(admProgramme);
  // adm::reassignIds(admDocument);
  return admDocument;
}

std::shared_ptr<adm::Document> createAdmDocument(const std::shared_ptr<adm::AudioObject>& audioObject, const ear::Layout& outputLayout) {
  std::shared_ptr<adm::Document> admDocument = adm::Document::create();
  auto mixObject = createAdmAudioObject(audioObject->get<adm::AudioObjectName>(), outputLayout);
  admDocument->add(mixObject);
  // adm::reassignIds(admDocument);
  return admDocument;
}

std::shared_ptr<bw64::AxmlChunk> createAxmlChunk(const std::shared_ptr<adm::Document>& admDocument) {
  std::stringstream xmlStream;
  adm::writeXml(xmlStream, admDocument);
  return std::shared_ptr<bw64::AxmlChunk>(new bw64::AxmlChunk(xmlStream.str()));
}

std::shared_ptr<bw64::ChnaChunk> createChnaChunk(const std::shared_ptr<adm::Document>& admDocument) {
  std::vector<bw64::AudioId> audioIds;

  auto audioObjects = admDocument->getElements<adm::AudioObject>();
  for(auto audioObject : audioObjects) {
    for(auto audioTrackUid : audioObject->getReferences<adm::AudioTrackUid>()) {
      audioIds.push_back(bw64::AudioId(audioTrackUid->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get(),
                                       formatId(audioTrackUid->get<adm::AudioTrackUidId>()),
                                       formatId(audioTrackUid->getReference<adm::AudioTrackFormat>()->get<adm::AudioTrackFormatId>()),
                                       formatId(audioTrackUid->getReference<adm::AudioPackFormat>()->get<adm::AudioPackFormatId>())
                                      ));
    }
  }
  return std::shared_ptr<bw64::ChnaChunk>(new bw64::ChnaChunk(audioIds));
}

void copyAudioProgramme(const std::shared_ptr<adm::Document>& admDocument, const std::shared_ptr<adm::AudioProgramme>& audioProgramme) {
  std::shared_ptr<adm::AudioProgramme> audioProgrammeCopy = audioProgramme->copy();

  std::vector<std::shared_ptr<adm::AudioContent>> audioContents = getAudioContents(audioProgramme);
  for(auto audioContent : audioContents) {

    auto audioContentCopy = audioContent->copy();
    std::vector<std::shared_ptr<adm::AudioObject>> audioObjects = getAudioObjects(audioContent);
    for(auto audioObject : audioObjects) {
      auto audioObjectCopy = audioObject->copy();

      std::vector<std::shared_ptr<adm::AudioPackFormat>> audioPackFormats = getAudioPackFormats(audioObject);
      for(auto audioPackFormat : audioPackFormats) {
        auto audioPackFormatCopy = audioPackFormat->copy();

        audioObjectCopy->addReference(audioPackFormatCopy);
      }

      std::vector<std::shared_ptr<adm::AudioTrackUid>> audioTracktUids = getAudioTrackUids(audioObject);
      for(auto audioTrackUid : audioTracktUids) {
        auto audioTrackUidCopy = audioTrackUid->copy();

        std::vector<std::shared_ptr<adm::AudioPackFormat>> audioPackFormatCopys = getAudioPackFormats(audioObjectCopy);
        for(auto audioPackFormatCopy : audioPackFormatCopys) {
          audioTrackUidCopy->setReference(audioPackFormatCopy);
        }

        auto audioTrackFormat = audioTrackUid->getReference<adm::AudioTrackFormat>();
        if(audioTrackFormat) {
          auto audioTrackFormatCopy = audioTrackFormat->copy();
          audioTrackUidCopy->setReference(audioTrackFormatCopy);
        }

        audioObjectCopy->addReference(audioTrackUidCopy);
        admDocument->add(audioTrackUidCopy);
      }

      audioContentCopy->addReference(audioObjectCopy);
      admDocument->add(audioObjectCopy);
    }

    audioProgrammeCopy->addReference(audioContentCopy);
    admDocument->add(audioContentCopy);
  }
  admDocument->add(audioProgrammeCopy);
}

}
