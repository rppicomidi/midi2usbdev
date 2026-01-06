# midi2usbdev
Make a computer USB to old school 5-pin DIN MIDI interface from a Raspberry Pi Pico

This project uses the same hardware as the [midi2usbhost](https://github.com/rppicomidi/midi2usbhost) project, so see the README.md file for instructions and photos. The same
disclaimers apply. This project does not use the micro USB to USB A adapter because it
is a USB Device. If you use a picoprobe to debug this project, do not wire the VBUS pins together like the [midi2usbhost](https://github.com/rppicomidi/midi2usbhost) README.md shows.
Instead, leave the VBUS pin unconnected. Do connect the ground pins, however.

I had some issues using a picoprobe with this project. I had to plug the picoprobe to
a different hub port or else the midi2usbdev Pico board would not enumerate. You may
have a different experience.

