SA-MP Audio Plugin
==================

v0.5 R2
-------

- Fixed a few minor bugs in the client and server and improved
  compatibility with SA-MP 0.3d+ (optional update)

v0.5
----

- Completely rewrote the networking code and greatly optimized many
  parts of both the client plugin and the server plugin
- Added panning (directional sound) support to Audio_Set3DPosition
- Added Audio_Remove3DPosition
- Added MIDI file support and MOD music support
- Added native and callback for getting an audio file's current
  position in seconds via Audio_GetPosition and Audio_OnGetPosition
- Renamed Audio_Seek to Audio_SetPosition
- Added three natives and one callback for in-game radio station
  adjustment via Audio_SetRadioStation, Audio_StopRadio, and
  Audio_OnRadioStationChange
- Added native for sending custom messages to players via
  Audio_SendMessage
- Added extra parameter to Audio_SetPack for automated file transfers
- Removed the audio file extraction system, OptimFrog (OFR) support,
  EAX support via Audio_SetEAX and Audio_RemoveEAX, manual 3D offset
  adjustment via Audio_Set3DOffsets, and the Audio_OnSetPack callback
  (Audio_SetPack now returns a result)

v0.4
----

- Ported the external client to an ASI plugin that loads
  automatically with SA-MP
- Fixed several bugs and optimized a lot of code in both the client
  plugin and the server plugin
- Added support for real 3D playback (Audio_Set3DPosition now accepts
  game world coordinates)
- Added support for downmixing so that Audio_SetFX, Audio_SetEAX, and
  Audio_Set3DOffsets will work with any audio stream
- Made the TCP server start automatically (using same port as the
  SA-MP server) when a script containing the include file is loaded
- Removed limitations on files, sequence IDs, and handle IDs
- Improved detection of the SA directory in the installer (will not
  install unless gta_sa.exe is found)
- Moved all data files to %APPDATA%\SA-MP Audio Plugin
- Changed to listener's ASI loader
- Added option to remove all data files in the uninstaller
- Fixed bug in client plugin that caused it not to connect to
  password-protected servers (thanks Rebel)
- Added full Unicode support to the client plugin

Pre-v0.4
----------

- Experimental releases with an external client-side program
