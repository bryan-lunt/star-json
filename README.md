# HDF5 STAR-JSON parser

Dependencies
------------

[Jansson](http://www.digip.org/jansson/)
Jansson is a C library for encoding, decoding and manipulating JSON data.
<br /> 

[HDF5](http://www.hdfgroup.org)
HDF5 is a set of software libraries and self-describing, 
machine-independent data format that support the creation, 
access, and sharing of array-oriented scientific data.
<br /> 

[zlib](http://www.zlib.net/)
zlib is a software library used for data compression.
<br /> 


Building from source
------------

Install dependency packages: Debian-based systems (like Ubuntu)
<pre>
sudo apt-get install build-essential
sudo apt-get install libjansson-dev
sudo apt-get install libhdf5-serial-dev
sudo apt-get install zlib1g-dev
</pre>

Install dependency packages: For RPM-based systems (like Fedora and CentOS)
<pre>
sudo yum install zlib-devel
sudo yum install hdf5 hdf5-devel
sudo yum install jansson-devel
</pre>

Get source:
<pre>
git clone https://github.com/pedro-vicente/json-hdf5.git
</pre>

Building from source
------------

Makefile for Ubuntu i386 GNU/Linux 
<pre>
make -f makefile
make -f makefile test
</pre>

Makefile for Ubuntu x86_64 GNU/Linux 
<pre>
make -f makefile.debian.x86_64
make -f makefile.debian.x86_64 test
</pre>

Building with CMake in Unix
------------
On most Unix systems, the Jansson, HDF5 and zlib libraries are found on the default location with
<pre>
cd build
cmake ..
</pre>

CMake build options
------------
If the Jansson, HDF5 and zlib libraries are found on the default location, they can be set. 
<pre>
cmake .. \
-DJANSSON_INCLUDE:PATH=/your/jansson/include/path \
-DJANSSON_LIBRARY=/your/jansson/library/file/name \
-DHDF5_INCLUDE:PATH=/your/hdf5/include/path \
-DHDF5_LIBRARY=/your/hdf5/library/file/name \
-DZLIB_LIBRARY=/your/zlib/library/file/name \
-DSZIP_LIBRARY=/your/zlib/library/file/name
</pre>

For a Windows Visual Studio build a statically build runtime library can be set with. 
<pre>
cmake .. -DSTATIC_CRT:BOOL=ON
</pre>

Usage
------------
./star_json 'JSON file in ./data'

Examples
------------

A file with 1 group named "g1" that has 2 subgroups "g11" and "g12" with no children
<pre>
{
  "g1":{
    "g11":"group",
    "g12":"group"
    }
  }
}
</pre>

A file with 1 dataset named "dset1", with numerical type STAR_INT32 (signed 32 bit integers), 
with a rank of value 2, with dimensions of values 3 and 4, and the data values from 1 to 12
<pre>
{
	"dset1": [{
		"dset1": ["dataset", "STAR_INT32", 2, [3, 4],[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]]
	}]
}
</pre>

Documentation
--------------
[Specification](http://www.space-research.org/blog/star_json.html)
