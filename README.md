# Cherish - Tool for Cultural Heritage Data Organization by using Image Manipulation and Sketching

## Introduction

**Cherish** is a tool that aids archaeologists and designers to represent historical, urban and archaeological structures and sites in 3D by means of image manipulation and sketching. Cherish creation was inspired by a [Yale's Dura Europos dataset](http://media.artgallery.yale.edu/duraeuropos/) - a large image dataset of an ancient site which is located in modern Syria. One of the main characteristic of the data set is its spacial sparsity and featureless natures of photographs which does not allow us to utilize the modern computer vision in order to obtain a 3D representation of the site. 

The main idea behind **Cherish** is to use the knowledge and expertise of a specialist in order to perform semi-manual data organization. Therefore, given a problem of disparate data types such photographs, illustrations, diagrams, text annotations, point cloud representation, etc., **Cherish** aims to provide a tool to combine all the above data into a comprehensive 3D space, to make 3D sense of the data. 

Note: **Cherish** is currently under development and does not have a comprehensive manual and user guide

## Binary download

If you are using Windows machine (version>=7), the fastest way is to download the pre-compiled binary and start testing the software right away. For that, go to the [release section](https://github.com/vicrucann/cherish/releases) and download the latest version. The download link will have the form of `cherish-Version-qtX.YmsvcYEAR.zip`.

## Building from source

Continuous integration:

* Linux 64 bits/GCC (build + tests running): 
[![Build Status](https://travis-ci.org/vicrucann/cherish.svg?branch=master)](https://travis-ci.org/vicrucann/cherish)
* Coverity scan for defect detection: 
[![Coverity Status](https://scan.coverity.com/projects/9322/badge.svg)](https://scan.coverity.com/projects/vicrucann-cherish)

Documentation:
* Development documentation: [![Documentation](https://codedocs.xyz/vicrucann/cherish.svg)](https://codedocs.xyz/vicrucann/cherish/)

**Cherish** can be built on a range of platforms, e.g., Windows, Linux, Mac. This is the list of libraries and tools that are used for the system compilation and build:

* [CMake](https://cmake.org/) (>=2.8.11)
* [Qt](http://www.qt.io/) (>=5.4; in case if you are planning to use Wacom tablet, the recommended Qt version is 5.4 or 5.5 due to a [tablet bug](https://bugreports.qt.io/browse/QTBUG-53628) in the stable versions older than 5.5.1)
* [OpenSceneGraph](http://www.openscenegraph.org/) (>=3.4.0)
* [Doxygen](www.doxygen.org/) (not mandatory; only used for documentation build). Note: the development documentation of a master branch can be [viewed online](https://codedocs.xyz/vicrucann/cherish/).
* [Git](https://git-scm.com/) (not mandatory; only used for development)
* C++ compiler with C++11 support, e.g., `gcc` (>=4.8.1), Visual Studio (>=2013), clang (>=3.0).
* Graphics card supporting OpenGL (>=3.3), otherwise the shaders will not work
* [Eigen library](http://eigen.tuxfamily.org) (when not using `git` for source download; see *Eigen Note* for more details).

All tools, but OpenSceneGraph (OSG), have binaries and are easy to install. There are no official binaries for OSG library and it is recommended to install it from source. Use the [official guide](http://trac.openscenegraph.org/projects/osg//wiki/Support/GettingStarted) for detailed steps. Note: the used version of OSG only requires OpenGL and CMake; there is no need to install additional plugins; the minimal install is enough.

*Eigen Note*: [Eigen library](http://eigen.tuxfamily.org) is also used within **Cherish**, however, if the source is downloaded by means of `git` (see below for instructions), no additional *Eigen* installation is required since the library is included as a third party dependency via git submodules.

### Getting the sources

You can download the latest version as an archive from [releases](https://github.com/vicrucann/cherish/releases) or use git to clone the repository:

```
$ git clone --recursive https://github.com/vicrucann/cherish.git
$ cd cherish
$ git submodule init
$ git submodule update
```

### General information for cmake options

* Build type is defined by `-DCMAKE_BUILD_TYPE` flag: `Debug` (default) or `Release`.
* Building of unit tests is defined by `-DCherish_BUILD_TEST` flag: `OFF` (default) or `ON`.
* Build the development documentation (requires doxygen) is defined by flag `-DCherish_BUILD_DOC`: `OFF` by default or `ON`.

### Command line compilation

After getting the source and all the submodules:

```
cherish/ $ mkdir cherish_build
cherish/ $ cd cherish_build
build/ $ cmake -DCMAKE_BUILD_TYPE=Release ../src/

build/ $ make
```

### Compilation using Qt Creator

Assuming the project is checked out from git repository and yout Qt Creator is configured to be used with an appropriate compiler and CMake, use Qt Creator to open the project by opening the file `cherish/src/CMakeLists.txt`.

Follow the steps of Qt Creator to finish the build using CMake. After the build is complete, compile the project by running `<Ctrl>+B`, or run from the menu: "Build" -> "Build All".

### Compilation using Visual Studio

Here we will only mention some specifics to know when performing the compilation using Visual Studio tools. 

Generally speaking, the compilation process can be split into two main parts: 

* CMake generation (for simplicity use CMake-gui)
* Compilation itself using Visual Studio of the generated project

When doing the second step, make sure to include the necessary Qt directories to the `PATH` environmental variable. Also, make sure to include certain Qt plugin folder: `plugins/imageformats`, otherwise the icons will not be displayed within the application.

## License

See the corresponding [LICENSE](https://github.com/vicrucann/cherish/blob/master/LICENSE) file.

## Contact and authors info

The software is being developed at [Yale Graphics Lab](http://graphics.cs.yale.edu/site/). The main development is performed by [Victoria Rudakova](https://github.com/vicrucann) under the guidance of [prof. Holly Rushmeier](http://graphics.cs.yale.edu/site/people/holly-rushmeier) and [prof. Julie Dorsey](http://graphics.cs.yale.edu/site/people/julie-dorsey).  

For any questions and requests, feel free to contact Victoria Rudakova - victoria.rudakova(at)yale.edu. Or use [Issues](https://github.com/vicrucann/cherish/issues) for feature requests and bug submission.

Other contributors:

* Bhavani Ananthabhotla: user feedback and research
* [Ying Yang](http://graphics.cs.yale.edu/site/people/ying-yang): sub-parts functionality and research
* [Goze Akoglu](http://ipch.yale.edu/people/kiraz-goze-akogluhttp://ipch.yale.edu/people/kiraz-goze-akoglu): user testing and feedback
* [Eleni Kotoula](http://ipch.yale.edu/people/eleni-kotoula): user testing and feedback
* [Wendy Chen](http://www.wendywchen.com/): user testing and feedback 
* [Natallia Trayan](https://www.behance.net/trayan): icon design
* Yumo Rong: user interface functionality

## Acknowledgements

Grant info will be placed here.

## References

The list of references will be placed here.
