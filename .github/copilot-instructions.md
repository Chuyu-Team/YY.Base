# Copilot Instructions

## Project Guidelines
- 项目代码最低支持 C++14，避免使用可能受标准库实现限制的语法糖（如 *_t 别名模板形式）。
- 项目也支持协程，但是协程相关的代码记得使用`#if defined(_HAS_CXX20) && _HAS_CXX20` 条件编译。
