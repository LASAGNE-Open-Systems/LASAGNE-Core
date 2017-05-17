# LASAGNE-Core
Layered Approach to Service Architectures for a Global Network Environment (LASAGNE)

## About LASAGNE
LASAGNE is a pattern-based framework for parallel and distributed computing
* Multi-threaded distributing computing platform in C++
* Extension mechanisms supported on a loadable, configurable infrastructure
* Provides hardware and operating system abstraction for native code execution
 * No explicit need for virtualisation
 * Uses the concept of a container for hosting components to applications
* "Don't need LASAGNE to talk to LASAGNE"
 * Core inclusions in LASAGNE are only permitted if they are based on openly published material
 * Uses open standard middleware technology (doesn't lock components into using them though)
* Domain agnostic
 * Core of LASAGNE does not mandate a data model representation

## Getting Started

### Prerequisites
* Required
 * Perl
 * ACE-TAO 6.4.2+  [Vanderbilt](http://download.dre.vanderbilt.edu) | [GitHub](https://github.com/DOCGroup/ACE_TAO)
* Optional
 * OpenDDS 3.10+      [OpenDDS](http://opendds.org/downloads.html) | [GitHub](https://github.com/objectcomputing/OpenDDS)

### Installation
See the instructions in [docs/html](docs/html/index.html) for the appropriate platform

## Contributing
Contributions can be made following the common [Integration-Manager Workflow](https://git-scm.com/book/en/v2/Distributed-Git-Distributed-Workflows#_integration_manager). The LASAGNE Core Team acts in the role of the Integration Manager.
Submissions will only be considered for review and possible inclusion if they meet the following criteria:
* Successfully pass all automated tests and checks
* Comply with the license agreement
* Approach is based upon openly available and published material
The intent is to follow a branching and release strategy along the lines of the established [GitFlow](http://nvie.com/posts/a-successful-git-branching-model/).

## License
The LASAGNE Core is licensed under the terms of the LGPL-3.0 license. See the [LICENSE](LICENSE) file for details.
