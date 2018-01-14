# bmfontutil

## Description
The `bmfontutil` tool parses the XML output of [Anglecode's BMFont](http://www.angelcode.com/products/bmfont/)
program. The resulting output file is a [protobuffer packed](fontdef.proto)
version of the font info and the associated textures.  The tool can
optionally perform a transform on the font into a distance-field font.

## Usage
* `${input}` should be the xml format font file, parallel to any textures.
* `${output}` will be the protobuffer formatted output font.

```shell
bmfontutil --infile ${input} --outfile ${output} [--export_as_distance_font true]
```

## Links
* [Anglecode BMFont](http://www.angelcode.com/products/bmfont/)
* [Valve: Improved Alpha-Tested Magnification for Vector Textures and Special Effects](http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf)
