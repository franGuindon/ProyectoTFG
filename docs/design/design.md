## Design

``` plantuml
@startuml

struct Frame {
  + template <typename T>
  + T data
  {field} + size_t size (in bytes? or in T units?)
  + size_t width
  + size_t height
}

class GroundTruthExtractor {
  + extractGroundTruth(Frame in, Frame out)
}

class FeatureExtractor {
  + extractFeatures(Frame in, Frame out)
}

class RandomDecisionForest {
  + extractFeatures(Frame in, Frame out)
}

class VideofileSource {
  + pullFrame(Frame frame)
  + pushFrame(Frame frame)
}

class VideofileSink {
  + pullFrame(Frame frame)
  + pushFrame(Frame frame)
}

@enduml
```

``` plantuml
@startuml

participant FeatureExtractor as feat
participant RandomDecisionForest as rdf

feat -> rdf : 

@enduml
```