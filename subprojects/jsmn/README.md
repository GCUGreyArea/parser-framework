# JSMN C++

## References

- [JSON Paths](https://en.wikipedia.org/wiki/JSONPath)
- [JQ Paths](https://jqlang.org/manual/)
- [JSMN C Library](https://github.com/zserge/jsmn)
- [Path design](docs/path-design.md)
- [Render design](docs/render-design.md)

## Dependancies

- [Google Test Framework](https://github.com/google/googletest)
- [Google Benchmark](https://github.com/google/benchmark)
- [GNU Bison](https://www.gnu.org/software/bison/)
- [GNU Flex](https://gothub.projectsegfau.lt/westes/flex/)

Google test only needs to be installed if you are planning on building and running the unit tests. 
Google Benchmark only needs to be installed if you are planning on building and running the benchmarks.

Flex and Bison can be installed with `apt`

```bash
sudo apt install flex 
sudo apt install bison
```

## Introduction

This is a prot of `JSMN` C library to C++. The ongoing work is to implement full `JQ path` compatibility so that the structures representing parsed `JSON` can be searched using `JQ path` syntax. In order to achieve this end the resulting tokens array can be rendered to a `C++` map that will corespond to the hash value parsed by the `JQ Parser`, which is written using `Flex` and `Bison`.

This implementation adds new featers on top of pure C++ compatibility;

1. Manipulation of a compiled `JSON` object using `add` and `delete` for the various `JSON` substructures.
2. Pure parsing of `JSON` that can then be mapped for `JQ Path` and `JSON Path` search syntax. Both syntax use the same search sturctures so these can be used interchangably.

While this is no longer the fastest `JSON` parser out there, it still has comparable performance. The main goal however is to provide much faster search capability for reading large JSON objects where multiple searchs are required on a single large parsed object.

## Build and run unit tests

Unit tests can be run with

```bash
make test
```

Benchmarks can be built and run with

```bash
make benchmark
```

If you want to run only a subset, pass normal Google Benchmark flags, for example:

```bash
./build/benchmark_jsmn --benchmark_filter=BM_Parse
```

If `simdjson`, `yyjson`, or `RapidJSON` are installed locally, the benchmark target will automatically include side-by-side parse benchmarks for those libraries as well.

## Creating and using a parser

A number of examples are provided in the `examples` directory. The most basic example can be foudn in [cmd_example.cpp](examples/cmd_example.cpp), This code shows how to use of the parser, along with how to serialise and deserialise parser data. This program can be built by running `make cmd_example` from the main project directory.

You can run the command test program with the following command

```bash
./build/cmd_example '{"name":"barry"}' ".name"
```

The project now exposes separate parser entry points for the two path languages:

```c++
struct jqpath * jq = jqpath_parse_string(".name[1]");
struct jqpath * json = jsonpath_parse_string("$.name[1]");
struct jqpath * either = path_parse_string(user_input);
```

All three return the same `jqpath` structure, so the same `get_path()` lookup code works with either language.

The same lookup API also accepts JSONPath-style input such as:

```bash
./build/cmd_example '{"name":["alice","barry"]}' '$.name[1]'
./build/cmd_example '{"user_name":{"first name":"barry"}}' '$["user_name"]["first name"]'
```

You should see the output

```bash
Parsed tokens: 3
Token: 0 type: object, start : 0, end : 16, size : 1, parent : -1, 
Token: 1 type: string, start : 2, end : 6, size : 1, parent : 0, value: "name"
Token: 2 type: string, start : 9, end : 14, size : 0, parent : 1, value: "barry"

Deserialised tokens: 3
Token: 0 type: object, start : 0, end : 16, size : 1, parent : -1, 
Token: 1 type: string, start : 2, end : 6, size : 1, parent : 0, value: "name"
Token: 2 type: string, start : 9, end : 14, size : 0, parent : 1, value: "barry"

Path value for .name  : barry
```

## Library functionality

Most of the functionality has been moved into a `.cpp` file, but there is no reason that it can't be part of a single `.h` or `.hpp` file. None of the alogoythms that parse `JSON` have been altered, but aditional functionality has been addded..

### Dynamic memory management

The parser now has dynamic memory managment through `new`. I debated using `realloc` which is easier to implement and significantly faster, but from a purely `C++` perspective is a `C` function. REimplementing that behaviour (if it is required) is simple, and only requires modification to two functions.

```c++
jsmntok_t *jsmn_parser::jsmn_alloc_token() {
    jsmntok_t *tok;

    // Over the allocated size, realloc.
    if (m_token_next >= m_num_tokens) {
        m_tokens = (jsmntok_t*) realloc(m_tokens,sizeof(jsmntok_t) * m_num_tokens *m_mull);
    }

    // ASsign the token and return
    tok = &m_tokens[m_token_next++];
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

jsmn_parser(const char *js = "{}", unsigned int mull = 2)
    : m_pos(0), 
      m_token_next(0), 
      m_toksuper(-1),
      m_tokens((jsmntok_t*) malloc(sizeof(jsmntok_t(def_size)))), 
      m_num_tokens(def_size),
      m_js(js), 
      m_length(m_js.length()), 
      m_mull(mull), 
      m_depth(0) {}
```

Dynamic memory management in the puer C++ model has a cost as the content of the previous array need to be copied to a new array, however memory is only bounded by that which is availible to the compiler, and hence the program. `realloc` requires that a block of contiguous memory is availible to realocate to, which is not allways possible with very large `JSON` files.

## Code documentation

You can compile the code documentation through the `Doxygen` utility by running `make doc`. 

## TODO

### Full JQ Path functionality 

The functionality to search using an empty array has not been implemented due to the performance overhead of doing so.

```bash
.name[] = "Barry"
```

where the JSON string might be

```json
{"name":["Frank","John", "Fred","Barry"]}
```

Where using JQ 

```bash 
echo '{"name":["Frank","John", "Fred","Barry"]}' | jq .name[] 
```

would produce

```bash
"Frank"
"John"
"Fred"
"Barry"
```

### Parser reentrancy

The `JQ path` and `JSONPath` parsers now use per-parse heap-allocated parser contexts, so parser state is no longer shared through global lexer/parser variables.

### Correct numeric equality

Currently while the `JQ path` parser renders all numeric values to their natural state (`1.1` becomes a `float` and `1` becomes an `integer`. As importantly `0001` and `1` are rendered into `1` and are equivolent), the `JSMN` implementation does not do this yet.
