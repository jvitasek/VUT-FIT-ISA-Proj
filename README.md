#Example
Using TLS on eva.fit.vutbr.cz with explicit specification of the port and the certificate directory.
`./imapcl eva.fit.vutbr.cz -p 993 -o output -c cert/cacert.pem -a auth.conf -h -T`

Using unsecured connection to imap.seznam.cz with implicit 143 port.
`./imapcl imap.seznam.cz -o output -a auth.conf -n`

#Files:
* error.cpp
* error.h
* imap.cpp
* imap.h
* imaps.cpp
* imaps.h
* input_parser.cpp
* input_parser.h
* main.cpp
* ISA.pdf
* README.md
* Makefile

#Known errors:
Unfortunately, I was not able to fix Makefile on eva.fit.vutbr.cz, the linker is not cooperating. Working setup:

```
Ubuntu clang version 3.5.2-3ubuntu1 (tags/RELEASE_352/final) (based on LLVM 3.5.2)
Target: x86_64-pc-linux-gnu
Thread model: posix
Found candidate GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/5.4.0
Found candidate GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/6.0.0
Found candidate GCC installation: /usr/lib/gcc/x86_64-linux-gnu/5.4.0
Found candidate GCC installation: /usr/lib/gcc/x86_64-linux-gnu/6.0.0
Selected GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/5.4.0
Candidate multilib: .;@m64
Selected multilib: .;@m64
```

Also, the parser should decide on the default port by consulting the isSecured class attribute. As of now, it is always 143.
