This version modified by Phil Frisbie for use with HawkVoice. It has
also been modified slightly for speed.


This directory contains version 1.2 of a simple 16 bit PCM <-> 4 bit
ADPCM coder and decoder. See the source and the manpage for comments
on the algorithm.

If you have previously used version 1.0: there was a bug in it. This
version fixes the bug. (1.1 fixed that, but had another slight bug
that caused loss of quality on SGI machines).

The routines have been tested on an SGI Indigo running Irix 4.0.2 and
4.0.5F and on a Sun Sparc SLC running SunOS 4.1.1. There are no real
machine dependencies, though, so there should be no problems on other
architectures. 

There is a timing program to test how fast the stuff runs on your
machine. Here are some sample numbers:

R4000 Indigo:	compress: 1.1Msample/sec, decompress: 1.7Msample/sec
R3000 Indigo:	compress: 410Ksample/sec, decompress: 850Ksample/sec
Sun SLC:	compress: 250Ksample/sec, decompress: 420Ksample/sec
Mac-IIsi	compress   21Ksample/sec, decompress:  35Ksample/sec

I'd be interested in numbers for PC class machines.

Also included are some simple test programs that convert audio files
to/from 4bit ADPCM samples. Rawcaudio/rawdaudio convert to/from raw
16bit PCM files; sgicaudio/sgidaudio convert to/from SGI AIFF files
and suncaudio/sundaudio convert to/from SUN/NeXT format audio files.
The Sun program was donated by Hughes Doug, hughes@sde.mdso.vf.ge.com
(it is actually only one program and looks at the program name to
decide whether to compress or decompress). Oh yeah, the sun and sgi
compressors will probably only compile on those machines. Also, for
the sgi compressor you need the Digital Media Developers Option, which
(I think) is available with Irix 4.0.5.

Note: I have had to make a few changes to the sun audio converter, but
unfortunately I have no way of testing the result (since it needs
Solaris 1.somethingorother which I don't have). If the program does
not work and someone fixes it I would like to get a copy.

Finally, there is a uuencoded adpcm sound file of me saying "hello
world" (with a cold) that you can use to test that everything works
correctly.

If you use this package I would like to hear from you.
I am especially interested in people who can test interoperability
with proven Intel/DVI ADPCM coders. I have tried to get the algorithm
correct, but you never know....

Answers to often-asked questions:
- No, this is *not* a G.721 coder/decoder. The algorithm used by G.721
  is very complicated, requiring oodles of floating-point ops per
  sample (resulting in very poor performance). I have not done any
  tests myself but various people have assured my that 721 quality is
  actually lower than DVI quality.

- No, it is not a CDROM-XA coder either, as far as I know. I haven't
  come across a good description of XA yet.

- No, testc and testd are not full-blown audio-file converters, only
  simple test programs. If you write a file-converter (like caudio.c,
  which compresses Sun audio files) and you're willing to donate it
  I'd be very happy to include it in the distribution.

- Also, I know next-to-nothing about who IMA are. Their address, though,
  from their May 1992 proceedings (where I got the algorithm from):
	IMA Compatability Project Headquarters
	9 Randall Court
	Annapolis MD 21401
	USA

	Phone: (410) 626-1380
	Fax:   (410) 263-0590

They might be able to send back issues of their proceedings, I don't
know.

	Jack Jansen
	Centre for Mathematics and Computer Science
	Kruislaan 413
	Amsterdam
	the Netherlands

	+31 20 592 4098
	Jack.Jansen@cwi.nl
