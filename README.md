
Introduction
-----------

Animata is a real-time animation software, designed to create interactive
background projections for concerts, theatre and dance performances.

The peculiarity of the software is that the animation - the movement of the
puppets, the changes of the background - is generated in real-time, making
continuous interaction possible. This ability also permits that physical
sensors, cameras or other environmental variables can be attached to the
animation of characters, creating a cartoon reacting to its environment. For
example, it is quite simple to create a virtual puppet band reacting to live
audio input, or set up a scene of drawn characters controlled by the movement
of dancers.

In contrast with the traditional 3D animation programs, creating characters in
Animata is quite simple and takes only a few minutes. On the basis of the still
images, which serve as the skeleton of the puppets, we produce a network of
triangles, some parts of which we link with the bone structure. The  movement
of the bones is based on a physical model, which allows the characters to be
easily moved.

The software can be run on multiple operation systems like Mac OS X, GNU Linux
and Windows. Animata can be connected with widespread programming environments
(e.g. Max/MSP, Pure Data, EyesWeb) used by multimedia developers and artists in
order to make use of the possibilities of these applications in the fields of
image editing, sound analysis, or motion capture.


Installing
----------

Animata requires:

FLTK (1.1.x)	http://www.fltk.org/

Installing on Windows
---------------------

On Windows Animata is compiled from Cygwin (http://cygwin.org/).

Cygwin packages: python, subversion, fltk-devel, libjpeg-devel, libjpg62
libpng12, libpng12-devel, gcc-core, gcc-g++

Download the Scons tarball from http://scons.org, and copy it to your cygwin
home directory. Then type:
tar -zxvf scons-1.2.0.tar.gz
cd scons-1.2.0
python setup.py install

Installing on OSX
-----------------

On OSX install FLTK from MacPorts
(http://www.macports.org/).

On OS X you can create an application bundle using "scons --app", or you can
also open Animata.xcodeproj and compile and run the Animata Target from XCode.

Installing on Linux
-------------------

In most Linux distributions an FLTK package is available. Animata is known to
compile in Fedora, and Ubuntu (8.04, 8.10).

Generic build instructions
--------------------------

To build Animata, type:

scons 

To run the software, type ./animata in the build directory.


Where to get more information
-----------------------------

There is a web page for Animata at

http://animata.kibu.hu/

This page also has information about mailing lists. Users should consider
subscribing to animata-users:

http://lists.kitchenbudapest.hu/cgi-bin/mailman/listinfo/animata-users

Animata-devel is for people interested in helping with the development:

http://lists.kitchenbudapest.hu/cgi-bin/mailman/listinfo/animata-devel


Reporting problems
------------------

There are two mailing lists, animata-users@lists.kitchenbudapest.hu and
animata-devel@lists.kitchenbudapest.hu. Use the latter if you are prepared
for a more technical discussion with the developers of the package.

Issues should be reported through the google code project page at
http://code.google.com/p/animata/issues/list

