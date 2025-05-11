# Contributing to Project Guidelines

## Extension Settings

Please make sure there is VSCode at hand which has the following extensions enabled:
- ms-vscode.cpptools-extension-pack
- llvm-vs-code-extensions.vscode-clangd
- xaver.clang-format
- cschlosser.doxdocgen

## Workspace Settings

Please make sure the following configurations are included in `.vscode/settings.json`

```json

{
    "editor.defaultFormatter": "llvm-vs-code-extensions.vscode-clangd",
    "editor.indentSize": "tabSize",
    "editor.tabSize": 4,
    "C_Cpp.intelliSenseEngine": "disabled"
}

```

## Code Style

### Block Brace
``` cpp
[STATEMENT] {

}
```
instead of
``` cpp
[STATEMENT] 
{

}
```

### Namespace

_**Do not use `using namespace xxx;`**_  

Please declare and implement anything scoped by a namespace for clearer codes and disambiguation.

#### Using namespaces

Please use namespace alias for a convenient way for something that has a long namespace.
```cpp 
namespace qi = boost::spirit::qi; 
// Then you can use qi::xxx for boost::spirit::qi::
```

### 

### Names
- **Files**: Capitalize the initial of every word. For example,   `CodeGenerator`, `ObjectManager`, `ASTObject`. C++ source file ends with `.cpp`; C++ header file ends with `.h`.
- **Class/struct/enum**:  
 Capitalize the initial of every word. For example, `CodeGenerator`, `ObjectManager`, `ASTObject`.
- **Class/struct fields**: 
 Capitalize the initial of every word except the first one. For example, `getName`, `genConstExpr`, `ruleCompUnit`. 
    - Add `m` to the front if the field is private. For example, `mState`, `mContextProvider`.
    - Add `s` to the front if the field is static. For example, `sInstance`.
- **Enum members**: Capitalize every character. Divide every word by `_`. For example,
 `BOOL`, `FLOAT`. `FUNC_VOID`.
- **Global constants**: Capitalize every character. Divide every word by `_`. For example,
 `MAX_VOLUME`, `CONFIG_FILE`, `LATENCY`.
- **Namespace**: Divide every word by `_`. For example, `gfw_test`
- **Local variables**: Divide every word by `_`. For example, `array_len`, `object_num`

## Comments
Referring to documentation standards provided by the extension .Doxdocgen,

Documentation:
```cpp 
/*
 * DOCUMENTATION HERE
 *
 */
```

Comments:
```cpp
// SINGLE LINE

/* MULTILINE */
```
- Don't over comment functions or structures. Trivial ones can be omitted increasing readability.
- Comments are served for not only you but other contributors. Please comment well codes you want to make clear.