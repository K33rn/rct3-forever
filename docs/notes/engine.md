# Engine

The rest of Frontier's engine: resources, files, streams, parsing.

| Address | Name | What it does |
|---|---|---|
| `0xc12770` | Parser::Parse | A bison-generated LALR parser (its tables and state at `0x13E32D0`-`0x13E3318`; messages `syntax error, unexpected %s, expecting %s or %s`), reading tokens from the flex-generated scanner `0xC11160` (`fatal flex scanner internal error`, `input buffer overflow, can't enlarge buffer`). Called from `0xC160B0` and `0xC16180` |
