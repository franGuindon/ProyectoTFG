# Trabajo Final de Graduación

## Introduction

This git repo tracks all work I wish to publicly share about the development of my project.

This mainly consists of:
- Thesis' sources and build (and READMEs to help you construct it in no time).
- Anteproyecto's sources and build (and READMEs).
- Bitácoras sources and build (and READMEs).
- Tools I built along the way.
- I want to share the dataset as well, but I want to see if I can set up my francis.guindon.cr page using my laptop as server.

* [Design](docs/design/design.md)

## Dependencies

## Building

```
meson setup build # May include additional build options
ninja -C build
```

| Build option   | Description
| :-             | :--
| --prefix       | Controls project installation path
| --optimization | Level of optimization to use when compiling
| -Denable-docs  | Controls build of documentation
| -Denable-test  | Controls build of tests

## Tools

These tools are meant to interactively pipe with each other or provide small libraries with useful APIs.

### Ranger

FIXME: Add Ranger description

### Ranger Extension

This extension creates a shared library that may be used as dependency. It opens up the ranger API to personal tools. It aditionally extends the ranger functionality for TFG purposes.

### Feature Generator

This tool is capable of parsing and loading the dataset files into an array of features.

### Trainer

This tool is capable of taking features and ground truth labels and training a RDF model using Rangerx.

### Analyzer

This tool is capable of printing information on Ranger and Rangerx models that might be useful for further analysis.
