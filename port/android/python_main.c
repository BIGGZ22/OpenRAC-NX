// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors
//
// openrac-python (libopenrac-python.so in the APK): the Python program the Android launcher runs
// OpenRAC's tools with (tools/extractor.py, editor/extract.py), as the desktop launcher runs the
// Python an installer carries. CPython's Android release is the interpreter as a library; this is
// the `python3` program around it, nothing more. Its standard library is found through
// PYTHONHOME, which the launcher sets (docs/port/ANDROID.md).

#include <Python.h>

int main(int argc, char** argv) {
    return Py_BytesMain(argc, argv);
}
