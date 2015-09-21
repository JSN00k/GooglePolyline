### Google Polyline Tool

You can get this code using Mercurial by doing:

`hg clone http://bitbucket.org/James_Snook/googlepolyline`

or using this [download link](http://bitbucket.org/James_Snook/googlepolyline/get/f5736112c9bd.zip).

This is a C tool for encoding and decoding a Google Polyline.
The C files for encoding and decoding the polyline are in the PolylineC folder
You can use the C Code in a project by including the polylineFunctions.* files, 
and the AppendableDataStore.* files (the polylineFunctions.* files
require the AppendableDataStore code. The makefile will build an executable
called PolylineTool which takes input from stdin and writes a polyline to
stdout. The input needs to look like the text in the  ExampleCoords file.

The code in the googlePolylineTest folder is an iPad test app this is what is
currently being used to test the polylineFunctions code.