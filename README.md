ModExtract
==========

This is a small, crude, C program which will extract the samples from a sountracker MOD file
and save them as 16 bit mono WAV files.

It has many many limitations:

* It only supports basic soundtracker files with 31 samples (not the older
  15 sample variant)
* It doesn't detect any internal sample formats and assumes all samples are raw
  data

Of course being an 8 bit format the samples in a MOD file are pretty crude quality
and the results are no better than the original even when "upsampled" to 16 bit.

Usage
-----

    $ modextract [-r rate] mod.filename

By default it creates WAV files with 11025hz sample rate. You can specify your own
with the `-r` flag.

    $ modextract -r 8287 mod.filename

All samples are extracted to the `samples` folder in the current directory, inside
a sub-folder named after the name of the module (as gathered from the module header
structure).

Samples are accompanied by a small text file describing the sample (loop start/end, etc).

Building
--------

There are no dependencies or requirements. Just run:

    $ make

Then to install in /usr/local/bin:

    $ sudo make install

If you want to install elsewhere (`~/bin/` for example) just copy `modextract` to there.

    $ cp modexract ~/bin/
