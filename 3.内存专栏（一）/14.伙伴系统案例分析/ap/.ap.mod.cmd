savedcmd_/root/linux-5.6.18/test_myself/ap/ap.mod := printf '%s\n'   ap.o | awk '!x[$$0]++ { print("/root/linux-5.6.18/test_myself/ap/"$$0) }' > /root/linux-5.6.18/test_myself/ap/ap.mod
