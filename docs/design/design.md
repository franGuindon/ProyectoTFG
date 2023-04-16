## Design

@startuml

struct Buffer {
  + template <typename dtype>
  + T data
  {field} + size_t size (in dtype units)
  + size_t width
  + size_t height
}

class GroundTruthExtractor {
  + extractGroundTruth(Buffer in, Buffer out)
}

class FeatureExtractor {
  + extractFeatures(Buffer in, Buffer out)
}

class RandomDecisionForest {
  + extractFeatures(Buffer in, Buffer out)
}

class VideofileSource {
  + pullFrame(Buffer frame)
  + pushFrame(Buffer frame)
}

class VideofileSink {
  + pullFrame(Buffer frame)
  + pushFrame(Buffer frame)
}

@enduml

@startuml

participant FeatureExtractor as feat
participant RandomDecisionForest as rdf

feat -> rdf : 

@enduml
