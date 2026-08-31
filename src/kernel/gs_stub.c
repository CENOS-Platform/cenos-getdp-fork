// mingw does not implement MSVC's /GS stack-cookie stack-protection scheme,
// but cudart.lib's static loader object references these symbols
// unconditionally. Harmless no-op stubs - this is the standard, well-known
// fix for linking MSVC-built /GS-protected .lib/.obj files (cudart.lib
// among them) into a mingw-linked executable.
unsigned long long __security_cookie = 0x2B992DDFA232ULL;
void __security_check_cookie(unsigned long long v) { (void)v; }
void __GSHandlerCheck(void) {}
