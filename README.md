### Overview
This project is a modified clone of the [Stanford Redbase.](https://web.stanford.edu/class/cs346/2015/redbase.html). We'll be working to implement the Indexing Component to add an [R-Tree](http://dl.acm.org/citation.cfm?id=602266) index to it. The system is written in C++. Use of C++11/14 is allowed.

### Structure
Redbase is divided into four components:

```
                    +----------------------------------------+
                    |            Query Language              | 
                    +----------------------------------------+
                    |           System Management            |
                    +-------------------+--------------------+
Add R-Trees here--->|     Indexes       |  Record Management |
                    +-------------------+--------------------+
                    |              Paged File                |
                    +----------------------------------------+
```
1. **Paged File** - The PF component provides facilities for higher-level client components to perform file I/O in terms of pages. In the PF component, methods are provided to create, destroy, open, and close paged files, to scan through the pages of a given file, to read a specific page of a given file, to add and delete pages of a given file, and to obtain and release pages for scratch use. It also implements the buffer pool for use by the other componenets. The C++ API for the PF component is available [here](https://web.stanford.edu/class/cs346/2015/redbase-pf.html). You do not need to make any changes at this layer.

2. **Record Management** -  The RM component provides classes and methods for managing files of unordered records.  It has been implemented for you and no changes should be required here. The API for this component is available [here](https://web.stanford.edu/class/cs346/2015/redbase-rm.html).  

3. **Indexing** - The IX component provides classes and methods for managing persistent indexes over unordered data records stored in paged files. Each data file may have any number of R-tree indexes associated with it. The indexes ultimately will be used to speed up processing of relational selections, joins, and condition-based update and delete operations. Like the data records themselves, the indexes are stored in paged files. This component is similar to the RM component and some code may be reused. The API for this component is specified [here](https://web.stanford.edu/class/cs346/2015/redbase-ix.html). There can be multiple different ways to implement the same functionality, and all of them are equally valid. However, you are expected to submit a design document outlining the choices you make.

4. **System Management** - The SM compoment provides the following functions:
  - __Unix command line utilities__ - for creating and destroying RedBase databases, invoking the system
  - __Data definition language (DDL) commands__ - for creating and dropping relations, creating and dropping indexes
  - __System utilities__ - for bulk loading, help, printing relations, setting parameters
  - __Metadata management__ - for maintaining system catalogs
  Details can be found [here](https://web.stanford.edu/class/cs346/2015/redbase-sm.html).
  
5. **Query Language** - The QL component implements the language RQL (for "RedBase Query Language"). RQL's data retrieval command is a restricted version of the SQL Select statement. RQL's data modification commands are restricted versions of SQL's Insert, Delete, and Update statements. The QL component uses classes and methods from the IX, RM, and SM components. More details can be found [here](https://web.stanford.edu/class/cs346/2015/redbase-ql.html).

### Steps for running

**Install the dependencies**

```
sudo apt-get install flex bison g++ g++-multilib git cmake make 
```


**Clone repository**

```
git clone git@github.com:PayasR/redbase.git 
```


**Build the code**

```
cd redbase
mkdir build
cd build
cmake ..
make
```


**Test**

```
./dbcreate Test
./redbase Test
```

**DDL Commands**

```
create table data(name c20, id i);
drop table data;
```

**DML Commands**

```
insert into data values ("abc", 1);
select * from data;
```

### Submission
- You are to work in groups of two. 
- Please sign up [here](https://docs.google.com/a/ucr.edu/spreadsheets/d/1vCCsw-hrSrAbeOQD2y_U9wIb-gayfbmVCeLJdeYUcvM/edit?usp=sharing) to let us know about who you're working with. 
- Deadline for submission is 7th March, 2017 12:00AM.

### Some other comments
1. We suggest the use of following open source projects to help you with the assignment:
- CMake: To generate build configurations.
- Ninja: Build tool written by the Chromium project, faster than GNU Make.
- Valgrind: For detecting memory leaks.
- GNU Flex and Bison: Used to implement the Query Language component.
- CTags: Great for navigating code.
- GTest: The Google C++ testing framework

### Special Thanks
Yifei Huang (yifeih@cs.stanford.edu)
