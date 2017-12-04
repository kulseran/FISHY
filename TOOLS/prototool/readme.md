# prototool

## Description
The `prototool` tool is compiled with all the protobuf files in the project
and can use this information to convert a proto data file between different
representations.

## Usage
* `${proto}` the fully qualified protobuf represented within `input`
* `${input}` should be the input file to convert
* `${type}` the format of the input file
* `${mode}` the operation to perform
* `${output}` will be the converted output

```shell
prototool --infile ${input} --outfile ${output} --mode ${mode} --input_type ${type} --proto ${proto}
```

## Flags

### Mode

mode | description
---- |  -----------
PRINT | output to stdout the content of the input file
CONVERT_TO_BIN | write the binary formatted output
CONVERT_TO_TEXT | write the text formatted output
CONVERT_TO_COLUMNS | write a column instead of record formatted binary output

### Input Type

type | description
---- | -----------
TEXT_PROTO | the input file is a textual representation of a protobuf
BIN_PROTO | the input file is a binary representation of a protobuf
RECORD_PROTO | the input file is a binary representation of multiple 'records' worth of protobuf content

## Links
