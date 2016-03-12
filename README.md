# Tracker-OSVR
TrackIR tracking plugin for OSVR based on OptiTrack v1.3.035 SDK released in 2008

PROJECT BASED ON DEPRECATED SDK : See TrackerV2-OSVR project for a tracking plugin based on the more recent OptiTrack Camera SDK that support the TrackClip PRO.

Assumes a TrackClip PRO is used

Trackclip pro is unsupported in that SDK so orientation tracking does not work well. There are issues with angles suddenly jumping 180 degrees when crossing zero.

The 64 bit version might not work because only a 32 bit version of OptiTrack.dll is provided with the SDK.