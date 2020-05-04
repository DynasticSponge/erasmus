# erasmus

erasmus is a C++ library for miscellaneous text manipulation.

## Stage/Release

**Development Complete:**
-   string url encode/decode
-   string base64 encode/decode
-   string backslash escape replacement
-   string token replacement
-   string to lowercase/uppercase
-   extract double-quoted sub-string
-   char/int to hex string

**In Development:**
-   string Deflate compress/decompress

## Build

Modify the makefile to address your own folder structure or ensure that "object" and "output" folders exist as peers of the "source" and "headers" folders.

```bash
make static
```
The above will compile the source and generate the liberasmus.a static library in the "output" folder

## Installation

Copy the 'headers' folder and liberasmus.a library file to the appropriate locations within your project's folder structure and update your compiler/linker appropriately.

## Basic Example

```c++
#include "erasmus/erasmus_namespace.hpp"
#include "erasmus/erasmus_director.hpp"

int main (int argc, char **argv)
{
    int returnValue{0};
    
    erasmus::director *director{new erasmus::director()};

    ///////////////////////////////////////////////////////////////////////////////
    // Example Usage Coming Soon
    ///////////////////////////////////////////////////////////////////////////////
    
    delete director;
    director = nullptr;
    return(returnValue);
}
```

## Contributing

This is a hobby project for me to have fun learning new things and tweaking the results.  If someone is interested, I'm more than happy to share the work/results with whomever wants to make use of it.

If you are interested in contributing however, please open an issue and we can discuss what it is you would like to tackle.  

Thanks!

## License
[MIT](https://choosealicense.com/licenses/mit/)

## Project Namesake
-   Learn about: [Erasmus of Rotterdamn](https://en.wikipedia.org/wiki/Erasmus)