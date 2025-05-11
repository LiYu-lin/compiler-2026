# Contributing to Project Guidelines

## Extension Settings

Please make sure there is VSCode at hand which has the following extensions enabled:
- ms-vscode.cpptools-extension-pack
- llvm-vs-code-extensions.vscode-clangd
- xaver.clang-format

## Workspace Settings

Please make sure the following configurations are included in `.vscode/settings.json`

```json

{
    "C_Cpp.formatting": "clangFormat",
    "editor.defaultFormatter": "xaver.clang-format",
    "editor.indentSize": "tabSize",
    "editor.tabSize": 4,
    "C_Cpp.intelliSenseEngine": "disabled"
}

```
