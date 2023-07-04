# simple-chat

Big thanks to [Beej's Guide to Networking Programming] for help and inspiration.  Buy his book!

In particular, see his [section on the `select` system call and example code].

> Why use `select`, you may be thinking to yourself?  After all, it only supports a limited number of connections (1024).
>
> Well, I don't think I even know that many people in the whole, wide world.  It's safe to say that even that limited number won't be on `simple-chat` all at the same time.

## Installation

`make build`

Only tested on Linux.  It uses [`select`], so it *should* be portable :)

## Examples

Start the server and connect via `telnet` or [`netcat`].  By default it binds to port 3333, but this can be changed by specifying a different port number.

Start the server:

```
./simple_chat 1992
Simple chat server started on port 1992...
Clients can connect using telnet, i.e., `telnet 127.0.0.1 1992`
```

Connect a client:

```
telnet 127.0.0.1 1992
```

Or:

```
nc 127.0.0.1 1992
```

## License

[GPLv3](COPYING)

## Author

Benjamin Toll

[Beej's Guide to Networking Programming]: https://beej.us/guide/bgnet/
[section on the `select` system call and example code]: https://beej.us/guide/bgnet/html/split/slightly-advanced-techniques.html#select
[`select`]: https://www.man7.org/linux/man-pages/man2/select.2.html
[`netcat`]: https://www.man7.org/linux/man-pages/man1/ncat.1.html

