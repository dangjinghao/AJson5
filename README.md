# AJson5
This repo is created for learning c program language.
## Try to support both general json format and the json5 format.

**Only ASCII code supported**

It is not so robust that could not be used for production. ~~Have not enough Unit Test .~~

If you are interested in the source, the file that related to this repo is `AJson5.c` and `AJson5.h` . And I've left some comments to describe the source and how to implement. 

e.g.
```json5
{
  // comments
  unquoted: 'and you can quote me on that',
  singleQuotes: 'I can use "double quotes" here',
  lineBreaks: "Look, Mom! \
No \\n's!",
  hexadecimal: 0xdecaf,
  leadingDecimalPoint: .8675309, andTrailing: 8675309.,
  positiveSign: +1,
  trailingComma: 'in objects', andIn: ['arrays',],
  "backwardsCompatible": "with JSON",
}

```

[JSON5](https://json5.org/) requires:
## Objects
**object keys may be an ECMAScript 5.1 IdentifierName.** (Not sure)

~~Objects may have a single trailing comma.~~

## Arrays
~~Arrays may have a single trailing comma.~~

## Strings
~~Strings may be single quoted.~~

~~Strings may span multiple lines by escaping new line characters.~~

~~Strings may include character escapes.~~
## Numbers
~~Numbers may be hexadecimal.~~

~~Numbers may have a leading or trailing decimal point.~~

~~Numbers may be IEEE 754 positive infinity, negative infinity, and NaN.~~

~~Numbers may begin with an explicit plus sign.~~

## Comments
~~Single and multi-line comments are allowed.~~

**Multi line should not contains the redundant multi line symbols**
e.g.

`/**/`  ok

`/* */ */` bad


`/* /*  */ */` bad

`/* /* */` maybe ok
## White Space
~~Additional white space characters are allowed.~~

**Not implement because of only considering ascii code.**

## TODOLIST:
- make sure the source code just uses all the internal functon and pack the internal function to export 
- skip function is not so complete
- some wrong ways to use `sizeof(s)/sizeof(char)`
- robust(fully error handling)
