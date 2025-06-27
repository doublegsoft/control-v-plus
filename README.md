control-v-plus
==========

```
                 _             _                 
  ___ ___  _ __ | |_ _ __ ___ | |    __   __ _   
 / __/ _ \| '_ \| __| '__/ _ \| |____\ \ / /| |_ 
| (_| (_) | | | | |_| | | (_) | |_____\ V /_   _|
 \___\___/|_| |_|\__|_|  \___/|_|      \_/  |_| 
```

## Issues

```
symbol lookup error: /snap/core20/current/lib/x86_64-linux-gnu/libpthread.so.0: undefined symbol: __libc_pthread_init, version GLIBC_PRIVATE
```

You will probably need to run unset GTK_PATH each new session/login/reboot. To set it permanently, put the command in your profile.

Note: It is a good idea to make a note of what the value of GTK_PATH is, prior to unsetting it - that way you can revert it back to its previous value, should you encounter any issues. To see what its "usual" value is, run echo $GTK_PATH, prior to unsetting it... If you have already unset it, open a new terminal window, which should have the unset value of GTK_PATH (if not, try a reboot).

```bash
unset GTK_PATH
```

```bash

pkg-config --cflags --libs gtk+-3.0 ayatana-appindicator3-0.1
```
