### On the server
```nc -vvlnp 5151 >/dev/null```

### On the client:
```dd if=/dev/zero bs=1M count=1K | nc -vvn 10.0.0.1 5151```
