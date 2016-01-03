SA-MP Audio Plugin
==================

**Note:** This project is no longer under active development. Its most useful feature, streaming audio, has been integrated into SA-MP itself, though it can still be used alongside SA-MP. It has been put here mainly for archival purposes.

This plugin creates a TCP server that can communicate with external clients to transfer and play back audio files, stream audio files from the Internet, and control in-game radio stations. It has several features, including:

- Seamless integration with SA-MP (San Andreas Multiplayer)
- Audio playback with looping, pausing, resuming, and stopping, restarting, seeking, and volume adjusting
- Internet audio file streaming that supports nearly all online stations, including SHOUTcast and Icecast
- Sequence system for gapless playback of multiple audio files
- Support for multiple audio streams per client
- In-game 3D sound support with dynamic volume adjustment and panning
- Support for MOD, WAV, AIFF, MP3/MP2/MP1, OGG, WMA, FLAC, MIDI, WV, SPX, MPC, AC3, AAC, ALAC, TTA, and APE formats
- Support for multiple sound effects that can be applied instantly
- In-game radio station adjustment support
- Audio pack system for organizing audio files and ensuring easy distribution among clients
- Local file transfers with CRC checks and remote file transfers with file size checks to ensure that files do not get re-downloaded

Compilation (Windows)
---------------------

Open the solution file (audio.sln) in Microsoft Visual Studio 2010 or higher. Build the project.

Download
--------

The latest binaries for Windows and Linux can be found [here](https://github.com/samp-incognito/samp-audio-client-plugin/releases).

Documentation
-------------

More information can be found in the [SA-MP forum thread](http://forum.sa-mp.com/showthread.php?t=82162) as well as the README file in the binary package.
