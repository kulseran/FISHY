# protoc

## Description
Based on [Google Protobuf](https://developers.google.com/protocol-buffers/),
the `protoc` tool parses a protobuffer definition file and generates
the relevant .h and .cpp files to represent the data structure in
code.

## Usage
* `${input}` should be the protobuffer definition file
* `${astyle_bin}` should be the path to the astyle binary. this will be used to
    format the output files.

```shell
compilematerial --infile ${input} [--astyle_path ${astyle_bin}]
```

## Links
* [Google Protobuf](https://developers.google.com/protocol-buffers/).

## Protobuf Definition Files

### A Minimal File
A minimal definiton file will look like the following:

```protobuf
package some.package.name;

// My message
message SomeMessage {
  optional int64 somefield = 1;
}
```

### Packages
Packages define the namespace scope for the resulting code.  A period seperated
list of identifiers becomes the hierchy of namespaces within the output code.

A package defintion looks like the following, and must be placed as the first
non-comment line in the file.

```protobuf
package some.package.name;
```

This package will result in the following namespace defintions in the output
code.

```cpp
namespace some {
namespace package {
namespace name {
// content here
}
}
}
```

### Imports
Imports allow the use of other protobuffer files to define messages that will
be used within the current file.  The import itself must be a valid protobuf
definiton file.

An import definition looks like the following.

```protobuf
import "path/to/another/definition.proto";
```

When referencing the resulting messages, the full package specification is
required.

Given the file ProtoA.proto:

```protobuf
package packages.alpha;

message AMessage {
  optional int64 id = 1;
}
```

The import and usage within ProtoB.proto would look like:
```protobuf
package packages.beta;

import "ProtoA.proto";

message BMessage {
  optional packages.alpha.AMessage id = 1;
}
```

### Messages
Messages are the core content of a protobuffer defition file. They represent
the information that will be packed into a C class for use in serialization.
A message consists of the message name, and a list of fields within the
message which will contain data.  A message can contain definitions for other
messages, or enumerations.

#### Fields
A field within a proto takes the form of a cardnality indicator, a type,
a name, and a unique id.

Cardnality is indicated by the two keywords:

 keyword | description
-------- | -----------
optional | The field will either contain content, or not.
repeated | The field will contain zero or more instances.

The type can be any of the basic types:

 keyword | description
-------- | -----------
int32    | A 32bit `int`.  The wireformat is a varint.
int64    | A 64bit `long long` or `int64_t`.  The wireformat is a varint.
uint32   | A 32bit `unsigned int`.  The wireformat is a varint.
uint64   | A 64bit `unsigned long long` or `uint64_t`.  The wireformat is a varint.
fixed32  | A 32bit `int`.  The wireformat is always 32bits wide.
fixed64  | A 64bit `unsigned long long` or `uint64_t`.  The wireformat is always 64bits wide.
sfixed32 | A 32bit `int`.  The wireformat is always 32bits wide.
sfixed64 | A 64bit `long long` or `int64_t`.  The wireformat is always 64bits wide.
float    | A 32bit `float`.
double   | A 64bit `double`.
string   | A `std::string`.
bytes    | A `std::string`.
bool     | A `bool`.

The type can also be any other message or enumeration defined within the file
or any import.

The name must follow the basic naming conventions of any C variable.
Additionally, names are prefered to be in `lowercase_underscore` format.
It is important that you do not reuse the name of the field at a later time
with a different type or field identifier.  The text parsers will perform a
string match on the field name, and thus can be confused by re-uses.

The identifier is a unique integer tag that will forever be associated with
this field in the message.  It is important that you do not re-use a number
for a different field content.  Once serialized to binary, these tags indicate
the content following the tag.  An unknown tag will be ignored, however any
tag definied in the message will attempt to be parsed as the type defined in
the message.  Field identifier re-use will result in bad parses.

An example field:

```protobuf
message AMessage {
  optional int64 an_integer_field = 1;
  repeated int64 a_repeated_field = 2;
}
```

Which will result in the following output class (abridged):

```cpp
class AMessage {
  public:
    // optional fields only have the following two members:
    bool has_an_integer_field() const;
    int64 get_an_integer_field() const;
    
    // Repeated fields only have the following 3 members:
    int64 get_a_repeated_field(u32 index) const;
    u32 get_a_repeated_field_size() const;
    const_iterator get_a_repeated_field_begin() const;
    const_iterator get_a_repeated_field_end() const; 
};
```

And the output builder class (abridged):

```cpp
class AMessage {
  class Builder {
    public:
      Builder &set_an_integer_field(int64 value);
      Builder &clear_an_integer_field();
      
      Builder &add_a_repeated_field(int64 value);
      Builder &set_a_repeated_field(int64 value, u32 index);
      Builder &clear_a_repeated_field();

      AMessage build() const;
  };
};
```

#### Enumerations

