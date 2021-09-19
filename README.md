ModExtract
==========

This is a small, crude, C program which will extract the samples from a sountracker MOD file
and save them as 16 bit mono WAV files.

It has many many limitations:

* The output sample rate is hard coded at 8287 Hz (PAL standard rate)
* It only supports basic soundtracker files with 31 samples (not the older
  15 sample variant)
* It doesn't detect any internal sample formats and assumes all samples are raw
  data
