
Scallop-TK
==========

The Scalable, Adaptive Localization, and Laplacian Object Proposal
(SCALLOP) Toolkit is a brief port of some of my master's thesis code
developed at RPI, described further in the paper:

[Automaic Scallop Detection in Benthic Environments][1]

It has a few more modern optimizations as well, beyond what's in the
paper, such as the ability to run convolutional neural networks on top
of the object proposal framework. It is useful as a general detector
for detecting any ellipsoidal objects, though it mainly targets benthic
organisms such as clams, scallops, urchins, and others. Also included
in the repository is a frame level classifier aimed at detecting whether
or not there are fish in the image.

[1]: https://scholar.google.com/citations?user=ZaCEQN0AAAAJ

Build Instructions
------------------

Requirements:

Windows/Linux
OpenCV
CMake
Caffe (optional)

(1) TODO: Make build instructions

Run Instructions
----------------

TODO: Make run instructions
