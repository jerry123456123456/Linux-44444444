savedcmd_/root/linux-5.6.18/test_myself/vmanumberpages/vmanumberpages.mod := printf '%s\n'   vmanumberpages.o | awk '!x[$$0]++ { print("/root/linux-5.6.18/test_myself/vmanumberpages/"$$0) }' > /root/linux-5.6.18/test_myself/vmanumberpages/vmanumberpages.mod
