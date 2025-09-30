# SmartHome Project Development Log

This development log can serve as an extended version of git commit messages.

## Step 1: Start a Skeleton CMake Project from a Template

Commit: **c085da4**

The CMake skeleton was adapted from [a very helpful and educative blog](https://mjmorse.com/blog/cmake-template/).

For now there isn't any non-trivial adaptation: we simply replace their project name "CMakeDemo" with our "SmartHome" and remove those cmake lines about INSTALL.

## Step 2: CMake Revision and Modern (C++ 20) Features

Commit: **79cabd3**

The original template was designed for a library-like project: files are compiled, "installed", and ready to be used in another project. What we have is a traditional single-executable project with no shipping need for now.

Learned more about orchestration done by the root CMakelists.txt from AI. Especially, I learned how it communicates with CMakelists.txt in subdir. This is very important when dealing with complex real-world CMake projects: those with CMakeLists at root and nested subdir, generated to a VS solution containing multiple projects, multiple executables.

Also, I learned and played with `std::iota()` and `std::ranges::transform()`. Hoestly, the code is no shorter or easier to write than a traditional for loop, but it's worth trying out new features covered in our lectures.
