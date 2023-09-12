..
      Licensed under the Apache License, Version 2.0 (the "License"); you may
      not use this file except in compliance with the License. You may obtain
      a copy of the License at

          http://www.apache.org/licenses/LICENSE-2.0

      Unless required by applicable law or agreed to in writing, software
      distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
      WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
      License for the specific language governing permissions and limitations
      under the License.

      Convention for heading levels in Open vSwitch documentation:

      =======  Heading 0 (reserved for the title in a document)
      -------  Heading 1
      ~~~~~~~  Heading 2
      +++++++  Heading 3
      '''''''  Heading 4

      Avoid deeper levels because they do not render well.

=============================
Windows Datapath Coding Style
=============================

The :doc:`coding style <coding-style>` guide gives the flexibility for each
platform to use its own coding style for the kernel datapath.  This file
describes the specific coding style used in most of the C files in the Windows
kernel datapath of the Open vSwitch distribution.

Most of the coding conventions applicable for the Open vSwitch distribution are
applicable to the Windows kernel datapath as well.  There are some exceptions
and new guidelines owing to the commonly followed practices in Windows
kernel/driver code.  They are noted as follows:

Basics
------

- Limit lines to 79 characters.

  Many times, this is not possible due to long names of functions and it is
  fine to go beyond the characters limit.  One common example is when calling
  into NDIS functions.

Types
-----

Use data types defined by Windows for most of the code.  This is a common
practice in Windows driver code, and it makes integrating with the data
structures and functions defined by Windows easier.  Example: ``DWORD`` and
``BOOLEAN``.

Use caution in portions of the code that interface with the OVS userspace.  OVS
userspace does not use Windows specific data types, and when copying data back
and forth between kernel and userspace, care should be exercised.

Naming
------

It is common practice to use camel casing for naming variables, functions and
files in Windows.  For types, especially structures, unions and enums, using
all upper case letters with words separated by '_' is common. These practices
can be used for OVS Windows datapath.  However, use the following guidelines:

- Use lower case to begin the name of a variable.

- Do not use '_' to begin the name of the variable. '_' is to be used to begin
  the parameters of a pre-processor macro.

- Use upper case to begin the name of a function, enum, file name etc.

- Static functions whose scope is limited to the file they are defined in can
  be prefixed with '_'. This is not mandatory though.

- For types, use all upper case for all letters with words separated by '_'. If
  camel casing is preferred, use  upper case for the first letter.

- It is a common practice to define a pointer type by prefixing the letter 'P'
  to a data type.  The same practice can be followed here as well.

For example::

    static __inline BOOLEAN
    OvsDetectTunnelRxPkt(POVS_FORWARDING_CONTEXT ovsFwdCtx,
                         POVS_FLOW_KEY flowKey)
    {
        POVS_VPORT_ENTRY tunnelVport = NULL;

        if (!flowKey->ipKey.nwFrag &&
            flowKey->ipKey.nwProto == IPPROTO_UDP &&
            flowKey->ipKey.l4.tpDst == VXLAN_UDP_PORT_NBO) {
            tunnelVport = OvsGetTunnelVport(OVSWIN_VPORT_TYPE_VXLAN);
            ovsActionStats.rxVxlan++;
        } else {
            return FALSE;
        }

        if (tunnelVport) {
            ASSERT(ovsFwdCtx->tunnelRxNic == NULL);
            ovsFwdCtx->tunnelRxNic = tunnelVport;
            return TRUE;
        }

        return FALSE;
    }

For declaring variables of pointer type, use of the pointer data type prefixed
with 'P' is preferred over using '*'. This is not mandatory though, and is only
prescribed since it is a common practice in Windows.

Example, #1 is preferred over #2 though #2 is also equally correct:

1. ``PNET_BUFFER_LIST curNbl;``
2. ``NET_BUFFER_LIST *curNbl;``

Comments
--------

Comments should be written as full sentences that start with a capital letter
and end with a period.  Putting two spaces between sentences is not necessary.

``//`` can be used for comments as long as the comment is a single line
comment.  For block comments, use ``/* */`` comments

Functions
---------

Put the return type, function name, and the braces that surround the function's
code on separate lines, all starting in column 0.

Before each function definition, write a comment that describes the function's
purpose, including each parameter, the return value, and side effects.
References to argument names should be given in single-quotes, e.g. 'arg'.  The
comment should not include the function name, nor need it follow any formal
structure.  The comment does not need to describe how a function does its work,
unless this information is needed to use the function correctly (this is often
better done with comments *inside* the function).

Mention any side effects that the function has that are not obvious based on
the name of the function or based on the workflow it is called from.

In the interest of keeping comments describing functions similar in structure,
use the following template.

::

    /*
     *----------------------------------------------------------------------------
     * Any description of the function, arguments, return types, assumptions and
     * side effects.
     *----------------------------------------------------------------------------
     */

Source Files
------------

Each source file should state its license in a comment at the very top,
followed by a comment explaining the purpose of the code that is in that file.
The comment should explain how the code in the file relates to code in other
files.  The goal is to allow a programmer to quickly figure out where a given
module fits into the larger system.

The first non-comment line in a .c source file should be::

    #include <precomp.h>

``#include`` directives should appear in the following order:

1. ``#include <precomp.h>``

2. The module's own headers, if any.  Including this before any other header
   (besides ``<precomp.h>``) ensures that the module's header file is
   self-contained (see *Header Files*) below.

3. Standard C library headers and other system headers, preferably in
   alphabetical order.  (Occasionally one encounters a set of system headers
   that must be included in a particular order, in which case that order must
   take precedence.)

4. Open vSwitch headers, in alphabetical order.  Use ``""``, not ``<>``, to
   specify Open vSwitch header names.
