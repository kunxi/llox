---
title: Environment Setup
kind: chapter
part: I
number: 2
order: 5
---

Before we started the journey, I would like to share my development
environment setup. I use `nix-shell` along with `direnv`.

In the `.envrc`, simply

```
use nix
```

this would instruct direnv to load nix-shell defined in `shell.nix`
with binary, library installed, environment variables exported _only_
for this dev environment. Once you change the directory, this environment
is auto unloaded, quite neat.
