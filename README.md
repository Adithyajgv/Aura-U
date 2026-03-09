# Aura-U
ROG Aura for Ubuntu 25 with gui

```
sudo nano /etc/udev/hwdb.d/99-asus-kbd.hwdb
```

99-asus-kbd.hwdb contents:
'''
evdev:input:b0003v0B05p19B6*
 KEYBOARD_KEY_ff3100c5=reserved
 KEYBOARD_KEY_ff3100c4=reserved
 '''