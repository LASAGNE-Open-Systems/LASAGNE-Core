# LASAGNE-Core
Layered Approach to Service Architectures for a Global Network Environment (LASAGNE)

## About LASAGNE
LASAGNE is a pattern-based framework for parallel and distributed computing
* Multi-threaded distributing computing platform in C++
* Adopts service-oriented principles and behaviour supported via metaprogramming techniques
* Extension mechanisms supported on a loadable, configurable infrastructure
* Provides hardware and operating system abstraction for native code execution
  * No explicit need for virtualisation
  * Uses the concept of a container (gestalt) for hosting components to applications
* "Don't need LASAGNE to talk to LASAGNE"
  * Core inclusions in LASAGNE are only permitted if they are based on openly published material
  * Uses open standard middleware technology (doesn't lock components into using them though)
* Domain agnostic
  * Core of LASAGNE does not mandate a data model representation

## Status

master:
[![Build Status](https://travis-ci.org/LASAGNE-Open-Systems/LASAGNE-Core.svg?branch=master)](https://travis-ci.org/LASAGNE-Open-Systems/LASAGNE-Core)
[![Build status](https://ci.appveyor.com/api/projects/status/1gwwcaraqoku38i8/branch/master?svg=true)](https://ci.appveyor.com/project/LASAGNE-Open-Systems/lasagne-core/branch/master)

develop:
[![Build Status](https://travis-ci.org/LASAGNE-Open-Systems/LASAGNE-Core.svg?branch=develop)](https://travis-ci.org/LASAGNE-Open-Systems/LASAGNE-Core)
[![Build status](https://ci.appveyor.com/api/projects/status/1gwwcaraqoku38i8/branch/develop?svg=true)](https://ci.appveyor.com/project/LASAGNE-Open-Systems/lasagne-core/branch/develop)

Travis CI is building using:
* g++ 4.8.1
* g++ 6.2

AppVeyor is building using:
* Visual Studio 2015
* Visual Studio 2017

## Getting Started

### Prerequisites
* Required
  * Perl
  * ACE-TAO 6.4.3  [Vanderbilt](http://download.dre.vanderbilt.edu) | [GitHub](https://github.com/DOCGroup/ACE_TAO)
  * MPC [GitHub](https://github.com/DOCGroup/MPC) (if ACE-TAO is retrieved from GitHub)
* Optional
  * OpenDDS 3.11      [OpenDDS](http://opendds.org/downloads.html) | [GitHub](https://github.com/objectcomputing/OpenDDS)
  * RTI DDS 5.2 (plus optional CORBA Compatibility Kit)
  * CoreDX 3.6.47

### Installation
See the instructions in [docs/html](docs/html/index.html) for the appropriate platform

## Contributing
Contributions can be made following the common [Integration-Manager Workflow](https://git-scm.com/book/en/v2/Distributed-Git-Distributed-Workflows#_integration_manager). The LASAGNE Core Team acts in the role of the Integration Manager.
Submissions will only be considered for review and possible inclusion if they meet the following criteria:
* Successfully pass all automated tests and checks
* Comply with the license agreement
* Approach is based upon openly available and published material
The intent is to follow a branching and release strategy along the lines of the established [GitFlow](http://nvie.com/posts/a-successful-git-branching-model/).

### Important Note on Workflow ###
Please make sure to target 'develop' as the default target for pull requests to fit in with this approach.
There may be times where bug fixes are applied to staged release branches but the majority of the time, 'develop' is the target.

## License
The LASAGNE Core is licensed under the terms of the LGPL-3.0 license. See the [LICENSE](LICENSE) file for details.

### Exceptions
* LDBC/SQLite/sqlite3 is provided copyright free from [SQLite.org](http://sqlite.org/)
