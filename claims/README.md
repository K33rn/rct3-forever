# Claims

Who is working on what right now. Each `<name>.txt` here is one claim: who holds it, since when, and what it covers
(function addresses, or repo paths such as a generator in `tools/`). Don't edit these by hand; use the tool:

```
uv run --no-project python tools/claim.py list                         who holds what
uv run --no-project python tools/claim.py check 0x792130               is it free?
uv run --no-project python tools/claim.py take slerp 0x9f0000 0x9f0400 --note "Quaternion::Slerp2 x2"
uv run --no-project python tools/claim.py renew slerp                  still on it
uv run --no-project python tools/claim.py release slerp                giving it up
```

- `take` refuses anything someone else holds, or that is already rewritten. It publishes the claim straight to
  `origin/main` without touching your working tree.
- A claim lapses 5 days after it was taken or last renewed; after that anyone may take it over.
- `tools/land.py` releases your claim when the work lands (`--release NAME`, or by itself once every address the
  claim covers is rewritten). It also refuses to land rewrites of functions someone else holds.
- Your name comes from `RCT3_WHO`, else git's `user.name`. Give every agent a name of its own, e.g.
  `RCT3_WHO="K3rn/agent-2"`.
